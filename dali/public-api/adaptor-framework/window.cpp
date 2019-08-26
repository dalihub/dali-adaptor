/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/window.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/orientation-impl.h>

namespace Dali
{

class DALI_INTERNAL DragAndDropDetector : public BaseHandle {}; // Empty class only required to compile Deprecated API GetDragAndDropDetector

Window Window::New(PositionSize posSize, const std::string& name, bool isTransparent)
{
  Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(posSize, name, "", isTransparent);

  Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
  Integration::SceneHolder sceneHolder = Integration::SceneHolder( window );
  Internal::Adaptor::Adaptor::GetImplementation( adaptor ).AddWindow( sceneHolder, name, "", isTransparent );

  return Window(window);
}

Window Window::New(PositionSize posSize, const std::string& name, const std::string& className, bool isTransparent)
{
  Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(posSize, name, className, isTransparent);

  Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
  Integration::SceneHolder sceneHolder = Integration::SceneHolder( window );
  Internal::Adaptor::Adaptor::GetImplementation( adaptor ).AddWindow( sceneHolder, name, className, isTransparent );

  return Window(window);
}

Window::Window()
{
}

Window::~Window()
{
}

Window::Window(const Window& handle)
: BaseHandle(handle)
{
}

Window& Window::operator=(const Window& rhs)
{
  BaseHandle::operator=(rhs);
  return *this;
}

void Window::Add( Dali::Actor actor )
{
  GetImplementation( *this ).Add( actor );
}

void Window::Remove( Dali::Actor actor )
{
  GetImplementation( *this ).Remove( actor );
}

void Window::SetBackgroundColor( const Vector4& color )
{
  GetImplementation( *this ).SetBackgroundColor( color );
}

Vector4 Window::GetBackgroundColor() const
{
  return GetImplementation( *this ).GetBackgroundColor();
}

Layer Window::GetRootLayer() const
{
  return GetImplementation( *this ).GetRootLayer();
}

uint32_t Window::GetLayerCount() const
{
  return GetImplementation( *this ).GetLayerCount();
}

Layer Window::GetLayer( uint32_t depth ) const
{
  return GetImplementation( *this ).GetLayer( depth );
}

void Window::ShowIndicator( IndicatorVisibleMode visibleMode )
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: ShowIndicator is deprecated and will be removed from next release.\n" );

  GetImplementation(*this).ShowIndicator( visibleMode );
}

Window::IndicatorSignalType& Window::IndicatorVisibilityChangedSignal()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: IndicatorVisibilityChangedSignal is deprecated and will be removed from next release.\n" );

  return GetImplementation(*this).IndicatorVisibilityChangedSignal();
}

void Window::SetIndicatorBgOpacity( IndicatorBgOpacity opacity )
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: SetIndicatorBgOpacity is deprecated and will be removed from next release.\n" );

  GetImplementation(*this).SetIndicatorBgOpacity( opacity );
}

void Window::RotateIndicator( WindowOrientation orientation )
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: RotateIndicator is deprecated and will be removed from next release.\n" );

  GetImplementation(*this).RotateIndicator( orientation );
}

void Window::SetClass( std::string name, std::string klass )
{
  GetImplementation(*this).SetClass( name, klass );
}

void Window::Raise()
{
  GetImplementation(*this).Raise();
}

void Window::Lower()
{
  GetImplementation(*this).Lower();
}

void Window::Activate()
{
  GetImplementation(*this).Activate();
}

void Window::AddAvailableOrientation( WindowOrientation orientation )
{
  GetImplementation(*this).AddAvailableOrientation( orientation );
}

void Window::RemoveAvailableOrientation( WindowOrientation orientation )
{
  GetImplementation(*this).RemoveAvailableOrientation( orientation );
}

void Window::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
  GetImplementation(*this).SetPreferredOrientation( orientation );
}

Dali::Window::WindowOrientation Window::GetPreferredOrientation()
{
  return GetImplementation(*this).GetPreferredOrientation();
}

DragAndDropDetector Window::GetDragAndDropDetector() const
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: GetDragAndDropDetector is deprecated and will be removed from the next release.\n" );
  DALI_ASSERT_ALWAYS( &GetImplementation( *this ) == GetObjectPtr() && "Empty Handle" );
  return Dali::DragAndDropDetector();
}

Any Window::GetNativeHandle() const
{
  return GetImplementation(*this).GetNativeHandle();
}

Window::FocusSignalType& Window::FocusChangedSignal()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: FocusChangedSignal is deprecated and will be removed from next release.\n" );
  return GetImplementation(*this).FocusChangedSignal();
}

Window::FocusChangeSignalType& Window::FocusChangeSignal()
{
  return GetImplementation(*this).FocusChangeSignal();
}

void Window::SetAcceptFocus( bool accept )
{
  GetImplementation(*this).SetAcceptFocus( accept );
}

bool Window::IsFocusAcceptable() const
{
  return GetImplementation(*this).IsFocusAcceptable();
}

void Window::Show()
{
  GetImplementation(*this).Show();
}

void Window::Hide()
{
  GetImplementation(*this).Hide();
}

bool Window::IsVisible() const
{
  return GetImplementation(*this).IsVisible();
}

unsigned int Window::GetSupportedAuxiliaryHintCount() const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHintCount();
}

std::string Window::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHint( index );
}

unsigned int Window::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return GetImplementation(*this).AddAuxiliaryHint( hint, value );
}

bool Window::RemoveAuxiliaryHint( unsigned int id )
{
  return GetImplementation(*this).RemoveAuxiliaryHint( id );
}

bool Window::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return GetImplementation(*this).SetAuxiliaryHintValue( id, value );
}

std::string Window::GetAuxiliaryHintValue( unsigned int id ) const
{
  return GetImplementation(*this).GetAuxiliaryHintValue( id );
}

unsigned int Window::GetAuxiliaryHintId( const std::string& hint ) const
{
  return GetImplementation(*this).GetAuxiliaryHintId( hint );
}

void Window::SetInputRegion( const Rect< int >& inputRegion )
{
  return GetImplementation(*this).SetInputRegion( inputRegion );
}

void Window::SetType( Window::Type type )
{
  GetImplementation(*this).SetType( type );
}

Window::Type Window::GetType() const
{
  return GetImplementation(*this).GetType();
}

bool Window::SetNotificationLevel( Window::NotificationLevel::Type level )
{
  return GetImplementation(*this).SetNotificationLevel( level );
}

Window::NotificationLevel::Type Window::GetNotificationLevel() const
{
  return GetImplementation(*this).GetNotificationLevel();
}

void Window::SetOpaqueState( bool opaque )
{
  GetImplementation(*this).SetOpaqueState( opaque );
}

bool Window::IsOpaqueState() const
{
  return GetImplementation(*this).IsOpaqueState();
}

bool Window::SetScreenOffMode(Window::ScreenOffMode::Type screenMode)
{
  return GetImplementation(*this).SetScreenOffMode(screenMode);
}

Window::ScreenOffMode::Type Window::GetScreenOffMode() const
{
  return GetImplementation(*this).GetScreenOffMode();
}

bool Window::SetBrightness( int brightness )
{
  return GetImplementation(*this).SetBrightness( brightness );
}

int Window::GetBrightness() const
{
  return GetImplementation(*this).GetBrightness();
}

Window::ResizedSignalType& Window::ResizedSignal()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: ResizedSignal is deprecated and will be removed from next release.\n" );
  return GetImplementation(*this).ResizedSignal();
}

Window::ResizeSignalType& Window::ResizeSignal()
{
  return GetImplementation(*this).ResizeSignal();
}

void Window::SetSize( Window::WindowSize size )
{
  GetImplementation(*this).SetSize( size );
}

Window::WindowSize Window::GetSize() const
{
  return GetImplementation(*this).GetSize();
}

void Window::SetPosition( Window::WindowPosition position )
{
  GetImplementation(*this).SetPosition( position );
}

Window::WindowPosition Window::GetPosition() const
{
  return GetImplementation(*this).GetPosition();
}

void Window::SetTransparency( bool transparent )
{
  GetImplementation(*this).SetTransparency( transparent );
}

Window::Window( Internal::Adaptor::Window* window )
: BaseHandle( window )
{
}

} // namespace Dali
