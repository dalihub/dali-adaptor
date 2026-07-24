/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// winsock2.h must be included before any header that may include windows.h.
#include <winsock2.h>
#include <ws2tcpip.h>

#include <dali/internal/network/windows/socket-impl-win.h>

#include <dali/integration-api/debug.h>

#include <limits>

#ifdef ERROR
#undef ERROR
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr unsigned int MAX_SOCKET_DATA_WRITE_SIZE = 10u * 1024u * 1024u;
constexpr std::uintptr_t INVALID_NATIVE_SOCKET    = static_cast<std::uintptr_t>(INVALID_SOCKET);

SOCKET ToNativeSocket(std::uintptr_t socket)
{
  return static_cast<SOCKET>(socket);
}

void LogWinsockError(const char* operation, int error = WSAGetLastError())
{
  DALI_LOG_ERROR("%s failed with Winsock error %d\n", operation, error);
}
} // unnamed namespace

SocketWin::SocketWin(Protocol protocol)
: mSocket(INVALID_NATIVE_SOCKET),
  mWakeReadSocket(INVALID_NATIVE_SOCKET),
  mWakeWriteSocket(INVALID_NATIVE_SOCKET),
  mQuitPending(false),
  mWinsockReady(false),
  mBound(false),
  mListening(false)
{
  Initialize(protocol, true);
}

SocketWin::SocketWin(Protocol protocol, std::uintptr_t nativeSocket)
: mSocket(nativeSocket),
  mWakeReadSocket(INVALID_NATIVE_SOCKET),
  mWakeWriteSocket(INVALID_NATIVE_SOCKET),
  mQuitPending(false),
  mWinsockReady(false),
  mBound(true),
  mListening(false)
{
  Initialize(protocol, false);
}

void SocketWin::Initialize(Protocol protocol, bool createSocket)
{
  WSADATA winsockData{};
  const int startupResult = WSAStartup(MAKEWORD(2, 2), &winsockData);
  if(startupResult != 0)
  {
    LogWinsockError("WSAStartup", startupResult);
    if(!createSocket && mSocket != INVALID_NATIVE_SOCKET)
    {
      closesocket(ToNativeSocket(mSocket));
      mSocket = INVALID_NATIVE_SOCKET;
    }
    return;
  }
  mWinsockReady = true;

  if(createSocket)
  {
    const int socketType = protocol == UDP ? SOCK_DGRAM : SOCK_STREAM;
    const int transport  = protocol == UDP ? IPPROTO_UDP : IPPROTO_TCP;

    mSocket = static_cast<std::uintptr_t>(socket(AF_INET, socketType, transport));
    if(mSocket == INVALID_NATIVE_SOCKET)
    {
      LogWinsockError("socket");
      ReleaseWinsock();
      return;
    }
  }

  if(!CreateWakeSockets(protocol))
  {
    if(mSocket != INVALID_NATIVE_SOCKET)
    {
      closesocket(ToNativeSocket(mSocket));
      mSocket = INVALID_NATIVE_SOCKET;
    }
    ReleaseWinsock();
    return;
  }
}

bool SocketWin::CreateWakeSockets(Protocol protocol)
{
  WSAPROTOCOL_INFO protocolInfo{};
  int              protocolInfoSize = static_cast<int>(sizeof(protocolInfo));
  if(getsockopt(ToNativeSocket(mSocket),
                SOL_SOCKET,
                SO_PROTOCOL_INFO,
                reinterpret_cast<char*>(&protocolInfo),
                &protocolInfoSize) == SOCKET_ERROR)
  {
    LogWinsockError("getsockopt(SO_PROTOCOL_INFO)");
    return false;
  }

  // select() on Windows only accepts sockets from the same service provider.
  // Create every wake socket from the target socket's exact provider rather
  // than relying on the system's current default provider ordering.
  const auto createProviderSocket = [&protocolInfo]() {
    WSAPROTOCOL_INFO socketProtocolInfo = protocolInfo;
    return WSASocket(FROM_PROTOCOL_INFO,
                     FROM_PROTOCOL_INFO,
                     FROM_PROTOCOL_INFO,
                     &socketProtocolInfo,
                     0,
                     WSA_FLAG_OVERLAPPED);
  };

  SOCKET wakeListener = INVALID_SOCKET;
  SOCKET wakeReader   = INVALID_SOCKET;
  SOCKET wakeWriter   = INVALID_SOCKET;

  const auto closeSocket = [](SOCKET& socketHandle) {
    if(socketHandle != INVALID_SOCKET)
    {
      closesocket(socketHandle);
      socketHandle = INVALID_SOCKET;
    }
  };

  const auto closeWakeSockets = [&]() {
    closeSocket(wakeListener);
    closeSocket(wakeReader);
    closeSocket(wakeWriter);
  };

  sockaddr_in readerAddress{};
  readerAddress.sin_family      = AF_INET;
  readerAddress.sin_port        = htons(0u);
  readerAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if(protocol == TCP)
  {
    wakeListener = createProviderSocket();
    if(wakeListener == INVALID_SOCKET)
    {
      LogWinsockError("wake WSASocket");
      return false;
    }

    if(bind(wakeListener,
            reinterpret_cast<const sockaddr*>(&readerAddress),
            static_cast<int>(sizeof(readerAddress))) == SOCKET_ERROR)
    {
      LogWinsockError("wake bind");
      closeWakeSockets();
      return false;
    }

    if(listen(wakeListener, 1) == SOCKET_ERROR)
    {
      LogWinsockError("wake listen");
      closeWakeSockets();
      return false;
    }

    int addressLength = static_cast<int>(sizeof(readerAddress));
    if(getsockname(wakeListener,
                   reinterpret_cast<sockaddr*>(&readerAddress),
                   &addressLength) == SOCKET_ERROR)
    {
      LogWinsockError("wake getsockname");
      closeWakeSockets();
      return false;
    }

    wakeWriter = createProviderSocket();
    if(wakeWriter == INVALID_SOCKET)
    {
      LogWinsockError("wake WSASocket");
      closeWakeSockets();
      return false;
    }

    if(connect(wakeWriter,
               reinterpret_cast<const sockaddr*>(&readerAddress),
               static_cast<int>(sizeof(readerAddress))) == SOCKET_ERROR)
    {
      LogWinsockError("wake connect");
      closeWakeSockets();
      return false;
    }

    wakeReader = accept(wakeListener, nullptr, nullptr);
    if(wakeReader == INVALID_SOCKET)
    {
      LogWinsockError("wake accept");
      closeWakeSockets();
      return false;
    }

    closeSocket(wakeListener);
  }
  else
  {
    wakeReader = createProviderSocket();
    wakeWriter = createProviderSocket();
    if(wakeReader == INVALID_SOCKET || wakeWriter == INVALID_SOCKET)
    {
      LogWinsockError("wake WSASocket");
      closeWakeSockets();
      return false;
    }

    if(bind(wakeReader,
            reinterpret_cast<const sockaddr*>(&readerAddress),
            static_cast<int>(sizeof(readerAddress))) == SOCKET_ERROR)
    {
      LogWinsockError("wake bind");
      closeWakeSockets();
      return false;
    }

    int addressLength = static_cast<int>(sizeof(readerAddress));
    if(getsockname(wakeReader,
                   reinterpret_cast<sockaddr*>(&readerAddress),
                   &addressLength) == SOCKET_ERROR)
    {
      LogWinsockError("wake getsockname");
      closeWakeSockets();
      return false;
    }

    sockaddr_in writerAddress{};
    writerAddress.sin_family      = AF_INET;
    writerAddress.sin_port        = htons(0u);
    writerAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(wakeWriter,
            reinterpret_cast<const sockaddr*>(&writerAddress),
            static_cast<int>(sizeof(writerAddress))) == SOCKET_ERROR)
    {
      LogWinsockError("wake bind");
      closeWakeSockets();
      return false;
    }

    if(connect(wakeWriter,
               reinterpret_cast<const sockaddr*>(&readerAddress),
               static_cast<int>(sizeof(readerAddress))) == SOCKET_ERROR)
    {
      LogWinsockError("wake connect");
      closeWakeSockets();
      return false;
    }

    addressLength = static_cast<int>(sizeof(writerAddress));
    if(getsockname(wakeWriter,
                   reinterpret_cast<sockaddr*>(&writerAddress),
                   &addressLength) == SOCKET_ERROR)
    {
      LogWinsockError("wake getsockname");
      closeWakeSockets();
      return false;
    }

    // Connect the receiver as well so only this object's writer can wake it.
    if(connect(wakeReader,
               reinterpret_cast<const sockaddr*>(&writerAddress),
               static_cast<int>(sizeof(writerAddress))) == SOCKET_ERROR)
    {
      LogWinsockError("wake connect");
      closeWakeSockets();
      return false;
    }
  }

  mWakeReadSocket  = static_cast<std::uintptr_t>(wakeReader);
  mWakeWriteSocket = static_cast<std::uintptr_t>(wakeWriter);
  return true;
}

void SocketWin::DeleteWakeSockets()
{
  if(mWakeReadSocket != INVALID_NATIVE_SOCKET)
  {
    if(closesocket(ToNativeSocket(mWakeReadSocket)) == SOCKET_ERROR)
    {
      LogWinsockError("wake closesocket");
    }
    mWakeReadSocket = INVALID_NATIVE_SOCKET;
  }

  if(mWakeWriteSocket != INVALID_NATIVE_SOCKET)
  {
    if(closesocket(ToNativeSocket(mWakeWriteSocket)) == SOCKET_ERROR)
    {
      LogWinsockError("wake closesocket");
    }
    mWakeWriteSocket = INVALID_NATIVE_SOCKET;
  }
}

SocketWin::~SocketWin()
{
  if(SocketIsOpen())
  {
    CloseSocket();
  }

  DeleteWakeSockets();
  ReleaseWinsock();
}

void SocketWin::ReleaseWinsock()
{
  if(mWinsockReady)
  {
    if(WSACleanup() == SOCKET_ERROR)
    {
      LogWinsockError("WSACleanup");
    }
    mWinsockReady = false;
  }
}

bool SocketWin::SocketIsOpen() const
{
  return mSocket != INVALID_NATIVE_SOCKET;
}

bool SocketWin::CloseSocket()
{
  if(!SocketIsOpen())
  {
    DALI_LOG_ERROR("%s", "Socket is already closed or invalid\n");
    return false;
  }

  const int result = closesocket(ToNativeSocket(mSocket));
  mSocket          = INVALID_NATIVE_SOCKET;
  mListening       = false;
  mBound           = false;

  if(result == SOCKET_ERROR)
  {
    LogWinsockError("closesocket");
    return false;
  }
  return true;
}

bool SocketWin::Bind(uint16_t port)
{
  if(!SocketIsOpen() || mBound)
  {
    DALI_LOG_ERROR("%s", "Socket is invalid or already bound\n");
    return false;
  }

  sockaddr_in serverAddress{};
  serverAddress.sin_family      = AF_INET;
  serverAddress.sin_port        = htons(port);
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  const int result = bind(ToNativeSocket(mSocket),
                          reinterpret_cast<const sockaddr*>(&serverAddress),
                          static_cast<int>(sizeof(serverAddress)));
  if(result == SOCKET_ERROR)
  {
    LogWinsockError("bind");
    return false;
  }

  mBound = true;
  return true;
}

bool SocketWin::Listen(int backlog)
{
  if(!SocketIsOpen() || !mBound || mListening)
  {
    DALI_LOG_ERROR("%s", "Socket is invalid, not bound, or already listening\n");
    return false;
  }

  if(listen(ToNativeSocket(mSocket), backlog) == SOCKET_ERROR)
  {
    LogWinsockError("listen");
    return false;
  }

  mListening = true;
  return true;
}

SocketInterface* SocketWin::Accept() const
{
  if(!SocketIsOpen() || !mListening)
  {
    DALI_LOG_ERROR("%s", "Socket is invalid or is not listening\n");
    return nullptr;
  }

  sockaddr_storage clientAddress{};
  int              addressLength = static_cast<int>(sizeof(clientAddress));
  const SOCKET     clientSocket  = accept(ToNativeSocket(mSocket),
                                      reinterpret_cast<sockaddr*>(&clientAddress),
                                      &addressLength);
  if(clientSocket == INVALID_SOCKET)
  {
    LogWinsockError("accept");
    return nullptr;
  }

  SocketWin* client = new SocketWin(TCP, static_cast<std::uintptr_t>(clientSocket));
  if(!client->SocketIsOpen())
  {
    delete client;
    return nullptr;
  }
  return client;
}

SocketInterface::SelectReturn SocketWin::Select()
{
  if(!SocketIsOpen() ||
     mWakeReadSocket == INVALID_NATIVE_SOCKET ||
     mWakeWriteSocket == INVALID_NATIVE_SOCKET)
  {
    DALI_LOG_ERROR("%s", "Cannot select on an invalid socket\n");
    return ERROR;
  }

  const SOCKET nativeSocket   = ToNativeSocket(mSocket);
  const SOCKET wakeReadSocket = ToNativeSocket(mWakeReadSocket);

  fd_set readSockets;
  fd_set exceptionSockets;
  FD_ZERO(&readSockets);
  FD_ZERO(&exceptionSockets);
  FD_SET(nativeSocket, &readSockets);
  FD_SET(wakeReadSocket, &readSockets);
  FD_SET(nativeSocket, &exceptionSockets);

  // Winsock ignores nfds.  Unlike WSAEventSelect, select() leaves the target
  // socket in blocking mode while another thread writes to it.
  const int result = select(0, &readSockets, nullptr, &exceptionSockets, nullptr);
  if(result == SOCKET_ERROR)
  {
    LogWinsockError("select");
    return ERROR;
  }

  if(FD_ISSET(wakeReadSocket, &readSockets))
  {
    char wakeByte = 0;
    const int bytesRead = recv(wakeReadSocket, &wakeByte, 1, 0);
    mQuitPending.store(false, std::memory_order_release);
    if(bytesRead != 1)
    {
      if(bytesRead == SOCKET_ERROR)
      {
        LogWinsockError("wake recv");
      }
      else
      {
        DALI_LOG_ERROR("%s", "Wake socket closed unexpectedly\n");
      }
      return ERROR;
    }
    return QUIT;
  }

  if(FD_ISSET(nativeSocket, &readSockets))
  {
    return DATA_AVAILABLE;
  }

  if(FD_ISSET(nativeSocket, &exceptionSockets))
  {
    DALI_LOG_ERROR("%s", "Socket exception detected by select\n");
  }
  return ERROR;
}

void SocketWin::ExitSelect()
{
  if(mWakeWriteSocket == INVALID_NATIVE_SOCKET)
  {
    DALI_LOG_ERROR("%s", "Cannot wake an invalid socket\n");
    return;
  }

  bool expected = false;
  if(!mQuitPending.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
  {
    return;
  }

  const char wakeByte = 1;
  const int  result   = send(ToNativeSocket(mWakeWriteSocket), &wakeByte, 1, 0);
  if(result != 1)
  {
    mQuitPending.store(false, std::memory_order_release);
    if(result == SOCKET_ERROR)
    {
      LogWinsockError("wake send");
    }
    else
    {
      DALI_LOG_ERROR("%s", "Wake socket sent an unexpected byte count\n");
    }
  }
}

bool SocketWin::Read(void* buffer, unsigned int bufferSizeInBytes, unsigned int& bytesRead)
{
  bytesRead = 0u;
  if(!SocketIsOpen())
  {
    DALI_LOG_ERROR("%s", "Socket is invalid\n");
    return false;
  }
  if(bufferSizeInBytes > static_cast<unsigned int>(std::numeric_limits<int>::max()))
  {
    DALI_LOG_ERROR("%s", "Socket read buffer is too large\n");
    return false;
  }

  const int result = recv(ToNativeSocket(mSocket),
                          static_cast<char*>(buffer),
                          static_cast<int>(bufferSizeInBytes),
                          0);
  if(result == SOCKET_ERROR)
  {
    LogWinsockError("recv");
    return false;
  }

  bytesRead = static_cast<unsigned int>(result);
  return true;
}

bool SocketWin::Write(const void* buffer, unsigned int bufferSizeInBytes)
{
  if(!SocketIsOpen())
  {
    DALI_LOG_ERROR("%s", "Socket is invalid\n");
    return false;
  }
  if(bufferSizeInBytes > MAX_SOCKET_DATA_WRITE_SIZE)
  {
    DALI_LOG_ERROR("Writing %u bytes exceeds the %u-byte socket limit\n",
                   bufferSizeInBytes,
                   MAX_SOCKET_DATA_WRITE_SIZE);
    return false;
  }

  unsigned int bytesWritten = 0u;
  while(bytesWritten < bufferSizeInBytes)
  {
    const char* byteBuffer = static_cast<const char*>(buffer) + bytesWritten;
    const int result = send(ToNativeSocket(mSocket),
                            byteBuffer,
                            static_cast<int>(bufferSizeInBytes - bytesWritten),
                            0);
    if(result == SOCKET_ERROR)
    {
      LogWinsockError("send");
      return false;
    }
    if(result == 0)
    {
      DALI_LOG_ERROR("%s", "Socket send made no progress\n");
      return false;
    }
    bytesWritten += static_cast<unsigned int>(result);
  }
  return true;
}

bool SocketWin::ReuseAddress(bool reuse)
{
  if(!SocketIsOpen() || mBound)
  {
    DALI_LOG_ERROR("%s", "Socket is invalid or already bound\n");
    return false;
  }

  const BOOL option = reuse ? TRUE : FALSE;
  if(setsockopt(ToNativeSocket(mSocket),
                SOL_SOCKET,
                SO_REUSEADDR,
                reinterpret_cast<const char*>(&option),
                static_cast<int>(sizeof(option))) == SOCKET_ERROR)
  {
    LogWinsockError("setsockopt(SO_REUSEADDR)");
    return false;
  }
  return true;
}

bool SocketWin::SetBufferSize(SocketInterface::BufferType type, unsigned int bufferSizeInBytes)
{
  if(!SocketIsOpen() || mBound)
  {
    DALI_LOG_ERROR("%s", "Socket is invalid or already bound\n");
    return false;
  }
  if(bufferSizeInBytes > static_cast<unsigned int>(std::numeric_limits<int>::max()))
  {
    DALI_LOG_ERROR("%s", "Socket buffer size is too large\n");
    return false;
  }

  const int option     = type == SocketInterface::SEND_BUFFER ? SO_SNDBUF : SO_RCVBUF;
  const int bufferSize = static_cast<int>(bufferSizeInBytes);
  if(setsockopt(ToNativeSocket(mSocket),
                SOL_SOCKET,
                option,
                reinterpret_cast<const char*>(&bufferSize),
                static_cast<int>(sizeof(bufferSize))) == SOCKET_ERROR)
  {
    LogWinsockError("setsockopt(SO_SNDBUF/SO_RCVBUF)");
    return false;
  }
  return true;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
