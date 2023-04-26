/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
 */

#include <dali/devel-api/adaptor-framework/graphics-capabilities.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/configuration-manager.h>

namespace Dali::GraphicsCapabilities
{
uint32_t GetMaxCombinedTextureUnits()
{
  Dali::Adaptor&                                       adaptor              = Adaptor::Get();
  Internal::Adaptor::Adaptor&                          adaptorImpl          = Internal::Adaptor::Adaptor::GetImplementation(adaptor);
  const Dali::Internal::Adaptor::ConfigurationManager* configurationManager = adaptorImpl.GetConfigurationManager();
  if(configurationManager)
  {
    return const_cast<Dali::Internal::Adaptor::ConfigurationManager*>(configurationManager)->GetMaxCombinedTextureUnits();
  }
  return 8; // Gles2 max.
}

} // namespace Dali::GraphicsCapabilities
