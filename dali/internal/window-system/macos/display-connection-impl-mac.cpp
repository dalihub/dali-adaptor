/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/macos/display-connection-impl-mac.h>

// EXTERNAL INCLUDES
// INTERNAL INCLUDES

namespace Dali::Internal::Adaptor
{
DisplayConnection* DisplayConnectionCocoa::New()
{
  return new DisplayConnectionCocoa();
}

DisplayConnectionCocoa::DisplayConnectionCocoa() = default;

DisplayConnectionCocoa::~DisplayConnectionCocoa() = default;

Any DisplayConnectionCocoa::GetDisplay()
{
  return EGL_DEFAULT_DISPLAY;
}

Any DisplayConnectionCocoa::GetNativeGraphicsDisplay()
{
#if defined(VULKAN_ENABLED)
  return {nullptr};
#else
  return {EGL_DEFAULT_DISPLAY};
#endif
}

void DisplayConnectionCocoa::ConsumeEvents()
{
}

void DisplayConnectionCocoa::SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type)
{
}

} // namespace Dali::Internal::Adaptor
