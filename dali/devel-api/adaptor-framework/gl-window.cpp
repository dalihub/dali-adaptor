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

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/gl-window.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/window.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/gl-window-impl.h>
#include <dali/internal/window-system/common/orientation-impl.h>

namespace Dali
{

GlWindow GlWindow::New()
{
  PositionSize positionSize( 0, 0, 0, 0 );
  return Dali::GlWindow::New( positionSize, "", "", false );
}

GlWindow GlWindow::New(PositionSize positionSize, const std::string& name, const std::string& className, bool isTransparent )
{
  GlWindow newWindow;
  Internal::Adaptor::GlWindow* window = Internal::Adaptor::GlWindow::New( positionSize, name, className, isTransparent );
  newWindow = GlWindow(window);

  const bool isAdaptorAvailable = Dali::Adaptor::IsAvailable();
  if( isAdaptorAvailable )
  {
    Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
    Dali::WindowContainer windows = adaptor.GetWindows();
    if( !windows.empty() )
    {
      window->SetChild( windows[0] );
    }
  }
  return newWindow;
}

GlWindow::GlWindow()
{
}

GlWindow::~GlWindow()
{
}

GlWindow::GlWindow(const GlWindow& handle) = default;

GlWindow& GlWindow::operator=(const GlWindow& rhs) = default;

GlWindow::GlWindow(GlWindow&& rhs) = default;

GlWindow& GlWindow::operator=(GlWindow&& rhs) = default;

void GlWindow::SetEglConfig( bool depth, bool stencil, int msaa, GlesVersion version )
{
  GetImplementation(*this).SetEglConfig( depth, stencil, msaa, version );
}

void GlWindow::Raise()
{
  GetImplementation(*this).Raise();
}

void GlWindow::Lower()
{
  GetImplementation(*this).Lower();
}

void GlWindow::Activate()
{
  GetImplementation(*this).Activate();
}

void GlWindow::Show()
{
  GetImplementation(*this).Show();
}

void GlWindow::Hide()
{
  GetImplementation(*this).Hide();
}

unsigned int GlWindow::GetSupportedAuxiliaryHintCount() const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHintCount();
}

std::string GlWindow::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHint( index );
}

unsigned int GlWindow::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return GetImplementation(*this).AddAuxiliaryHint( hint, value );
}

bool GlWindow::RemoveAuxiliaryHint( unsigned int id )
{
  return GetImplementation(*this).RemoveAuxiliaryHint( id );
}

bool GlWindow::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return GetImplementation(*this).SetAuxiliaryHintValue( id, value );
}

std::string GlWindow::GetAuxiliaryHintValue( unsigned int id ) const
{
  return GetImplementation(*this).GetAuxiliaryHintValue( id );
}

unsigned int GlWindow::GetAuxiliaryHintId( const std::string& hint ) const
{
  return GetImplementation(*this).GetAuxiliaryHintId( hint );
}

void GlWindow::SetInputRegion( const Rect< int >& inputRegion )
{
  GetImplementation(*this).SetInputRegion( inputRegion );
}

void GlWindow::SetOpaqueState( bool opaque )
{
  GetImplementation(*this).SetOpaqueState( opaque );
}

bool GlWindow::IsOpaqueState() const
{
  return GetImplementation(*this).IsOpaqueState();
}

void GlWindow::SetPositionSize( PositionSize positionSize )
{
  GetImplementation(*this).SetPositionSize( positionSize );
}

PositionSize GlWindow::GetPositionSize() const
{
  return GetImplementation(*this).GetPositionSize();
}

WindowOrientation GlWindow::GetCurrentOrientation() const
{
  return GetImplementation( *this ).GetCurrentOrientation();
}

void GlWindow::SetAvailableOrientations( const Dali::Vector< WindowOrientation >& orientations )
{
  GetImplementation( *this ).SetAvailableOrientations( orientations );
}

void GlWindow::SetPreferredOrientation( WindowOrientation orientation )
{
  GetImplementation(*this).SetPreferredOrientation( orientation );
}

void GlWindow::RegisterGlCallback( CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback )
{
  GetImplementation(*this).RegisterGlCallback( initCallback, renderFrameCallback, terminateCallback );
}

void GlWindow::RenderOnce()
{
  GetImplementation(*this).RenderOnce();
}

GlWindow::FocusChangeSignalType& GlWindow::FocusChangeSignal()
{
  return GetImplementation(*this).FocusChangeSignal();
}

GlWindow::ResizeSignalType& GlWindow::ResizeSignal()
{
  return GetImplementation(*this).ResizeSignal();
}

GlWindow::KeyEventSignalType& GlWindow::KeyEventSignal()
{
  return GetImplementation(*this).KeyEventSignal();
}

GlWindow::TouchEventSignalType& GlWindow::TouchedSignal()
{
  return GetImplementation(*this).TouchedSignal();
}

GlWindow::VisibilityChangedSignalType& GlWindow::VisibilityChangedSignal()
{
  return GetImplementation(*this).VisibilityChangedSignal();
}

GlWindow::GlWindow( Internal::Adaptor::GlWindow* window )
: BaseHandle( window )
{
}

}// Dali
