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
#include <dali/internal/offscreen/ubuntu-x11/offscreen-window-impl-x.h>

namespace Dali
{

namespace Internal
{

OffscreenWindowX* OffscreenWindowX::New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
{
  OffscreenWindowX* window = new OffscreenWindowX( width, height, surface, isTranslucent );
  return window;
}

OffscreenWindowX::OffscreenWindowX( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
: mIsTranslucent( isTranslucent )
{
}

void OffscreenWindowX::Initialize( bool isDefaultWindow )
{
}

void OffscreenWindowX::Initialize()
{
}

OffscreenWindowX::~OffscreenWindowX()
{
}

uint32_t OffscreenWindowX::GetLayerCount() const
{
  return 0;
}

Dali::Layer OffscreenWindowX::GetLayer( uint32_t depth ) const
{
  return Dali::Layer();
}

OffscreenWindow::WindowSize OffscreenWindowX::GetSize() const
{
  return OffscreenWindow::WindowSize();
}

Dali::Any OffscreenWindowX::GetNativeHandle() const
{
  return Any();
}

NativeRenderSurface* OffscreenWindowX::GetNativeRenderSurface() const
{
  return nullptr;
}

void OffscreenWindowX::OnPostRender()
{
}

OffscreenWindow::PostRenderSignalType& OffscreenWindowX::PostRenderSignal()
{
  return mPostRenderSignal;
}

} // namespace Internal

} // namespace Dali
