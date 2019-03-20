/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/internal/window-system/android/display-connection-factory-android.h>
#include <dali/internal/window-system/android/display-connection-impl-android.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

std::unique_ptr<Dali::Internal::Adaptor::DisplayConnection> DisplayConnectionFactoryAndroid::CreateDisplayConnection()
{
  return Utils::MakeUnique<DisplayConnectionAndroid>();
}

// this should be created from somewhere
std::unique_ptr<DisplayConnectionFactory> GetDisplayConnectionFactory()
{
  // returns X display factory
  return Utils::MakeUnique<DisplayConnectionFactoryAndroid>();
}

}
}
}
