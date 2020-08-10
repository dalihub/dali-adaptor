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

// EXTENRAL INCLUDES
#include <dali/public-api/actors/layer.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-application-impl.h>
#include <dali/internal/offscreen/common/offscreen-window-impl.h>
#include <dali/internal/offscreen/common/offscreen-window-factory.h>

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/offscreen-window.h>

namespace Dali
{
OffscreenWindow OffscreenWindow::New( uint16_t width, uint16_t height, bool isTranslucent )
{
  auto windowFactory = Dali::Internal::GetOffscreenWindowFactory();
  Any surface;
  IntrusivePtr < Internal::OffscreenWindow > impl = windowFactory->CreateOffscreenWindow( width, height, surface, isTranslucent ).release();
  OffscreenWindow window = OffscreenWindow( impl.Get() );
  impl->Initialize( false );

  return window;
}

OffscreenWindow OffscreenWindow::New( Any surface, bool isTranslucent )
{
  auto windowFactory = Dali::Internal::GetOffscreenWindowFactory();
  IntrusivePtr < Internal::OffscreenWindow > impl = windowFactory->CreateOffscreenWindow( 0, 0, surface, isTranslucent ).release();
  OffscreenWindow window = OffscreenWindow( impl.Get() );
  impl->Initialize( false );

  return window;
}

OffscreenWindow::OffscreenWindow()
{
}

OffscreenWindow::OffscreenWindow( const OffscreenWindow& window )
: BaseHandle( window )
{
}

OffscreenWindow& OffscreenWindow::operator=( const OffscreenWindow& window )
{
  if( *this != window )
  {
    BaseHandle::operator=( window );
  }
  return *this;
}

OffscreenWindow::~OffscreenWindow()
{
}

void OffscreenWindow::Add( Actor actor )
{
  Internal::GetImplementation( *this ).Add( actor );
}

void OffscreenWindow::Remove( Actor actor )
{
  Internal::GetImplementation( *this ).Remove( actor );
}

void OffscreenWindow::SetBackgroundColor( const Vector4& color )
{
  Internal::GetImplementation( *this ).SetBackgroundColor( color );
}

Vector4 OffscreenWindow::GetBackgroundColor() const
{
  return Internal::GetImplementation( *this ).GetBackgroundColor();
}

Layer OffscreenWindow::GetRootLayer() const
{
  return Internal::GetImplementation( *this ).GetRootLayer();
}

uint32_t OffscreenWindow::GetLayerCount() const
{
  return Internal::GetImplementation( *this ).GetLayerCount();
}

Layer OffscreenWindow::GetLayer( uint32_t depth ) const
{
  return Internal::GetImplementation( *this ).GetLayer( depth );
}

OffscreenWindow::WindowSize OffscreenWindow::GetSize() const
{
  return Internal::GetImplementation( *this ).GetSize();
}

Any OffscreenWindow::GetNativeHandle() const
{
  return Internal::GetImplementation( *this ).GetNativeHandle();
}

OffscreenWindow::PostRenderSignalType& OffscreenWindow::PostRenderSignal()
{
  return Internal::GetImplementation( *this ).PostRenderSignal();
}


OffscreenWindow::OffscreenWindow( Internal::OffscreenWindow* window )
: BaseHandle( window )
{
}

}  // namespace Dali
