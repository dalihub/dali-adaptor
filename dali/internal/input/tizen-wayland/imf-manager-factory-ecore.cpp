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

#include <memory>
#ifdef ECORE_WL2
#include <dali/internal/input/tizen-wayland/ecore-wl2/imf-manager-impl-ecore-wl2.h>
#else
#include <dali/internal/input/tizen-wayland/ecore-wl/imf-manager-impl-ecore-wl.h>
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class ImfManager;

template<typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

namespace ImfManagerFactory
{

// ImfManager Factory to be implemented by the platform
Dali::ImfManager CreateImfManager()
{
  return Dali::Internal::Adaptor::ImfManagerEcoreWl::Get();
}

}

}



}
}