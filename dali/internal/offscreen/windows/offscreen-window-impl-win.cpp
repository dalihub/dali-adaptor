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
#include <dali/internal/offscreen/windows/offscreen-window-impl-win.h>

namespace Dali
{

namespace Internal
{

OffscreenWindowWin* OffscreenWindowWin::New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
{
  OffscreenWindowWin* window = new OffscreenWindowWin( width, height, surface, isTranslucent );
  return window;
}

OffscreenWindowWin::OffscreenWindowWin( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
: mIsTranslucent( isTranslucent )
{
}

void OffscreenWindowWin::Initialize( bool isDefaultWindow )
{
}

void OffscreenWindowWin::Initialize()
{
}

OffscreenWindowWin::~OffscreenWindowWin()
{
}

uint32_t OffscreenWindowWin::GetLayerCount() const
{
  return 0;
}

Dali::Layer OffscreenWindowWin::GetLayer( uint32_t depth ) const
{
  return Dali::Layer();
}

OffscreenWindow::WindowSize OffscreenWindowWin::GetSize() const
{
  return OffscreenWindow::WindowSize();
}

Dali::Any OffscreenWindowWin::GetNativeHandle() const
{
  return Any();
}

NativeRenderSurface* OffscreenWindowWin::GetNativeRenderSurface() const
{
  return nullptr;
}

void OffscreenWindowWin::OnPostRender()
{
}

OffscreenWindow::PostRenderSignalType& OffscreenWindowWin::PostRenderSignal()
{
  return mPostRenderSignal;
}

} // namespace Internal

} // namespace Dali
