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
#include <dali/internal/window-system/android/display-connection-impl-android.h>

// INTERNAL HEADERS

// EXTERNAL HEADERS

namespace Dali::Internal::Adaptor
{
DisplayConnection* DisplayConnectionAndroid::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionAndroid());

  return pDisplayConnection;
}

DisplayConnectionAndroid::DisplayConnectionAndroid()
: mDisplay(nullptr)
{
}

DisplayConnectionAndroid::~DisplayConnectionAndroid() = default;

Any DisplayConnectionAndroid::GetDisplay()
{
  return {mDisplay};
}

void DisplayConnectionAndroid::ConsumeEvents()
{
}

void DisplayConnectionAndroid::SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type)
{
  mDisplay = EGL_DEFAULT_DISPLAY;
}

} // namespace Dali::Internal::Adaptor
