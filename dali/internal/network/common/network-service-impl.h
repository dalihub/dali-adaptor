#ifndef DALI_INTERNAL_NETWORK_SERVICE_H
#define DALI_INTERNAL_NETWORK_SERVICE_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>

namespace Dali
{
namespace Internal::Adaptor
{

class NetworkService;
using NetworkServicePtr = IntrusivePtr<NetworkService>;

/**
 * Provides methods to interface with the network services offered by DALi.
 *
 * This is only available when the adaptor is built with with -DENABLE_NETWORK_LOGGING=ON
 * and when running, DALI_NETWORK_CONTROL=1 must also be set.
 */
class NetworkService : public Dali::BaseObject
{
public:
  NetworkService()                      = default;      ///< Constructor
  ~NetworkService() override            = default;      ///< Default Destructor
  NetworkService(const NetworkService&) = delete;       ///< Deleted copy constructor
  NetworkService(NetworkService&&)      = delete;       ///< Deleted move constructor
  NetworkService& operator=(NetworkService&) = delete;  ///< Deleted copy assignment operator
  NetworkService& operator=(NetworkService&&) = delete; ///< Deleted move assignment operator

  /**
   * @brief Retrieve the NetworkService Singleton.
   * @return An intrusive pointer to the NetworkService
   */
  static NetworkServicePtr Get();

  /**
   * @brief This signal will be triggered when a custom command is received.
   * @return The signal when a custom command is received
   */
  DevelApplication::CustomCommandReceivedSignalType& CustomCommandReceivedSignal()
  {
    return mCustomCommandReceivedSignal;
  }

  /**
   * @brief Emit the Custom Command Received Signal
   */
  void EmitCustomCommandReceivedSignal(const std::string& message);

private:
  DevelApplication::CustomCommandReceivedSignalType mCustomCommandReceivedSignal;

}; // class NetworkService

} // namespace Internal::Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_NETWORK_SERVICE_H
