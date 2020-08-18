/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/actors/layer.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

// CLASS HEADER
#include <dali/internal/offscreen/android/offscreen-window-impl-android.h>

namespace Dali
{

namespace Internal
{

OffscreenWindowAndroid* OffscreenWindowAndroid::New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
{
  OffscreenWindowAndroid* window = new OffscreenWindowAndroid( width, height, surface, isTranslucent );
  return window;
}

OffscreenWindowAndroid::OffscreenWindowAndroid( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
: mIsTranslucent( isTranslucent )
{
}

void OffscreenWindowAndroid::Initialize( bool isDefaultWindow )
{
}

void OffscreenWindowAndroid::Initialize()
{
}

OffscreenWindowAndroid::~OffscreenWindowAndroid()
{
}

uint32_t OffscreenWindowAndroid::GetLayerCount() const
{
  return 0;
}

Dali::Layer OffscreenWindowAndroid::GetLayer( uint32_t depth ) const
{
  return Dali::Layer();
}

OffscreenWindow::WindowSize OffscreenWindowAndroid::GetSize() const
{
  return OffscreenWindow::WindowSize();
}

Dali::Any OffscreenWindowAndroid::GetNativeHandle() const
{
  return Any();
}

NativeRenderSurface* OffscreenWindowAndroid::GetNativeRenderSurface() const
{
  return nullptr;
}

void OffscreenWindowAndroid::OnPostRender()
{
}

OffscreenWindow::PostRenderSignalType& OffscreenWindowAndroid::PostRenderSignal()
{
  return mPostRenderSignal;
}

} // namespace Internal

} // namespace Dali
