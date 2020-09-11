#ifndef DALI_INTERNAL_ADAPTOR_SOCKET_IMPL_H
#define DALI_INTERNAL_ADAPTOR_SOCKET_IMPL_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <dali/internal/network/common/socket-interface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * @brief Concrete implementation of a socket under Linux.
 *
 * Provides automatic closing of socket on destruction.
 */
class Socket : public SocketInterface
{
public:

  /**
   * @brief Constructor
   * @param protocol network protocol
   * @param fileDescriptor option file descriptor if the socket is already open
   */
  Socket( Protocol protocol , int fileDescriptor = -1 );

  /**
   * @copydoc Dali::Internal::Adaptor::SocketIsOpen()
   */
  bool SocketIsOpen() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::CloseSocket
   */
  bool CloseSocket() override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::Bind
   */
  bool Bind( uint16_t port )  override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::Listen
   */
  bool Listen( int blacklog) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::Accept
   */
  SocketInterface* Accept() const  override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::Select
   */
  SelectReturn Select( ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::ExitSelect
   */
  void ExitSelect() override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::Recieve
   */
  bool Read( void* buffer, unsigned int bufferSizeInBytes, unsigned int& bytesRead ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::Send
   */
  bool Write( const void* buffer, unsigned int bufferLength ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::ReuseAddress
   */
  bool ReuseAddress( bool reUse ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SocketInterface::SetBufferSize
   *
   */
  bool SetBufferSize( SocketInterface::BufferType type, unsigned int bufferSizeInBytes ) override;

  /**
   * @brief Virtual destructor
   */
  virtual ~Socket();

private:


  /**
   * @brief Helper to create the quit pipe
   */
  bool CreateQuitPipe();

  /**
   * @brief  Helper to delete the quit pipe
   */
  void DeleteQuitPipe();

  int mSocketFileDescriptor; ///< file descriptor
  int mQuitPipe[2];          ///< Pipe to inform Select to quit.
  bool mBound:1;             ///< whether the socket is bound
  bool mListening:1;         ///< whether the socket is being listen to
  bool mQuitPipeCreated:1;   ///< whether the quit pipe has been created
  bool mBlocked:1;           ///< whether the socket is blocked waiting for a connection
};



} // Adaptor

} // Internal

} // Dali

#endif // DALI_INTERNAL_ADAPTOR_SOCKET_IMPL_H
