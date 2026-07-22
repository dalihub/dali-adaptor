#ifndef DALI_INTERNAL_ADAPTOR_SOCKET_IMPL_WIN_H
#define DALI_INTERNAL_ADAPTOR_SOCKET_IMPL_WIN_H

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

#include <atomic>
#include <cstdint>

#include <dali/internal/network/common/socket-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Native Winsock implementation of SocketInterface.
 *
 * The native socket is stored in a pointer-width integer because SOCKET is a
 * UINT_PTR on Windows.  In particular, it must never be narrowed to int on a
 * 64-bit build.
 */
class SocketWin : public SocketInterface
{
public:
  /**
   * @brief Create a socket using @p protocol.
   */
  explicit SocketWin(Protocol protocol);

  ~SocketWin() override;

  bool SocketIsOpen() const override;
  bool CloseSocket() override;
  bool Bind(uint16_t port) override;
  bool Listen(int backlog) override;
  SocketInterface* Accept() const override;
  SelectReturn Select() override;
  void ExitSelect() override;
  bool Read(void* buffer, unsigned int bufferSizeInBytes, unsigned int& bytesRead) override;
  bool Write(const void* buffer, unsigned int bufferSizeInBytes) override;
  bool ReuseAddress(bool reuse) override;
  bool SetBufferSize(SocketInterface::BufferType type, unsigned int bufferSizeInBytes) override;

private:
  /**
   * @brief Adopt an already accepted native socket.
   */
  SocketWin(Protocol protocol, std::uintptr_t nativeSocket);

  void Initialize(Protocol protocol, bool createSocket);
  bool CreateWakeSockets(Protocol protocol);
  void DeleteWakeSockets();
  void ReleaseWinsock();

private:
  std::uintptr_t   mSocket;          ///< Pointer-width native SOCKET value.
  std::uintptr_t   mWakeReadSocket;  ///< Loopback socket monitored by Select().
  std::uintptr_t   mWakeWriteSocket; ///< Loopback socket used by ExitSelect().
  std::atomic_bool mQuitPending;     ///< Coalesces repeated wake requests.
  bool             mWinsockReady;    ///< Whether this instance owns a WSAStartup reference.
  bool             mBound;           ///< Whether the socket is bound or connected.
  bool             mListening;       ///< Whether the socket is listening.

  SocketWin(const SocketWin&)            = delete;
  SocketWin& operator=(const SocketWin&) = delete;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SOCKET_IMPL_WIN_H
