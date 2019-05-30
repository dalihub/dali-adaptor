#ifndef DALI_INTERNAL_ADAPTOR_CLIENT_SEND_DATA_INTERFACE_H
#define DALI_INTERNAL_ADAPTOR_CLIENT_SEND_DATA_INTERFACE_H

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

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * @brief  Abstract interface used to transmit data to a client
 */
class ClientSendDataInterface
{
public:

  /**
   * @brief Sends data to the client
   * @param[in] data pointer to some data
   * @param[in] bufferSizeInBytes how big the buffer is in bytes
   * @param[in] clientId unique client id to send the data to
   */
  virtual void SendData( const char* const data, unsigned int bufferSizeInBytes, unsigned int clientId ) = 0;


protected:

  /**
   * @brief  Constructor
   */
  ClientSendDataInterface()
  {
  }

  /**
   * @brief Virtual Destructor
   */
  virtual ~ClientSendDataInterface()
  {
  }

private:

  // Undefined copy constructor.
  ClientSendDataInterface( const ClientSendDataInterface& );

  // Undefined assignment operator.
  ClientSendDataInterface& operator=( const ClientSendDataInterface& );

};


} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_CLIENT_SEND_DATA_INTERFACE_H
