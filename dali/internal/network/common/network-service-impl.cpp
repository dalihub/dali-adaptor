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

// CLASS HEADER
#include <dali/internal/network/common/network-service-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

namespace Dali::Internal::Adaptor
{
NetworkServicePtr NetworkService::Get()
{
  NetworkServicePtr networkService;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(NetworkService));
    if(handle)
    {
      // If so, downcast the handle
      networkService = NetworkServicePtr(dynamic_cast<NetworkService*>(handle.GetObjectPtr()));
    }
    else
    {
      // Create a singleon instance
      networkService = NetworkServicePtr(new NetworkService());
      service.Register(typeid(NetworkService), BaseHandle(networkService.Get()));
    }
  }

  return networkService;
}

void NetworkService::EmitCustomCommandReceivedSignal(const std::string& message)
{
  if(!mCustomCommandReceivedSignal.Empty())
  {
    mCustomCommandReceivedSignal.Emit(message);
  }
}

} // namespace Dali::Internal::Adaptor
