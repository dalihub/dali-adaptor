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

// CLASS HEADER
#include "window-impl.h"

// EXTERNAL HEADERS
#include <dali/integration-api/core.h>
#include <dali/integration-api/system-overlay.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <orientation.h>

// INTERNAL HEADERS
#include "render-surface/render-surface-wl.h"
#include <drag-and-drop-detector-impl.h>
#include <window-visibility-observer.h>
#include <orientation-impl.h>



namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowLogFilter = Debug::Filter::New(Debug::Concise, false, "LOG_WINDOW");
#endif

struct Window::EventHandler
{
  // place holder
};

Window* Window::New( const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent )
{
  Window* window = new Window();
  window->mIsTransparent = isTransparent;
  window->Initialize( positionSize, name, className );
  return window;
}

void Window::SetAdaptor(Dali::Adaptor& adaptor)
{
  DALI_ASSERT_ALWAYS( !mStarted && "Adaptor already started" );
  mStarted = true;

  // Only create one overlay per window
  Internal::Adaptor::Adaptor& adaptorImpl = Internal::Adaptor::Adaptor::GetImplementation(adaptor);
  Integration::Core& core = adaptorImpl.GetCore();
  mOverlay = &core.GetSystemOverlay();

  Dali::RenderTaskList taskList = mOverlay->GetOverlayRenderTasks();
  taskList.CreateTask();

  mAdaptor = &adaptorImpl;
  mAdaptor->AddObserver( *this );

  // Can only create the detector when we know the Core has been instantiated.
  mDragAndDropDetector = DragAndDropDetector::New();
  mAdaptor->SetDragAndDropDetector( &GetImplementation( mDragAndDropDetector ) );


}

RenderSurface* Window::GetSurface()
{
  return mSurface;
}

void Window::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode )
{

}

void Window::RotateIndicator(Dali::Window::WindowOrientation orientation)
{
}

void Window::SetIndicatorBgOpacity( Dali::Window::IndicatorBgOpacity opacityMode )
{
}

void Window::SetClass(std::string name, std::string klass)
{
}

Window::Window()
: mSurface( NULL ),
  mIndicatorVisible( Dali::Window::VISIBLE ),
  mIndicatorIsShown( false ),
  mShowRotatedIndicatorOnClose( false ),
  mStarted( false ),
  mIsTransparent( false ),
  mWMRotationAppSet( false ),
  mIsFocusAcceptable( true ),
  mVisible( true ),
  mOpaqueState( false ),
  mResizeEnabled( true ),
  mIndicator( NULL ),
  mIndicatorOrientation( Dali::Window::PORTRAIT ),
  mNextIndicatorOrientation( Dali::Window::PORTRAIT ),
  mIndicatorOpacityMode( Dali::Window::OPAQUE ),
  mOverlay( NULL ),
  mAdaptor( NULL ),
  mType( Dali::DevelWindow::NORMAL ),
  mPreferredOrientation( Dali::Window::PORTRAIT ),
  mSupportedAuxiliaryHints(),
  mAuxiliaryHints(),
  mIndicatorVisibilityChangedSignal(),
  mFocusChangedSignal(),
  mResizedSignal(),
  mDeleteRequestSignal()
{
  mEventHandler = NULL;
}

Window::~Window()
{
  delete mEventHandler;

  if ( mAdaptor )
  {
    mAdaptor->RemoveObserver( *this );
    mAdaptor->SetDragAndDropDetector( NULL );
    mAdaptor = NULL;
  }

  delete mSurface;
}

void Window::Initialize(const PositionSize& positionSize, const std::string& name, const std::string& className)
{
  // create an Wayland window by default
  Any surface;
  Wayland::RenderSurface* windowSurface = new Wayland::RenderSurface( positionSize, surface, name, mIsTransparent );

  mSurface = windowSurface;

  mOrientation = Orientation::New(this);
}

void Window::DoShowIndicator( Dali::Window::WindowOrientation lastOrientation )
{

}

void Window::DoRotateIndicator( Dali::Window::WindowOrientation orientation )
{

}

void Window::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
}

void Window::IndicatorTypeChanged(IndicatorInterface::Type type)
{
}

void Window::IndicatorClosed( IndicatorInterface* indicator )
{

}

void Window::IndicatorVisibilityChanged(bool isVisible)
{

}

void Window::SetIndicatorActorRotation()
{

}

void Window::Raise()
{
}

void Window::Lower()
{
}

void Window::Activate()
{
}

Dali::DragAndDropDetector Window::GetDragAndDropDetector() const
{
  return mDragAndDropDetector;
}

Dali::Any Window::GetNativeHandle() const
{
  Wayland::RenderSurface* surface = static_cast<  Wayland::RenderSurface* >( mSurface );

  return surface->GetWindow();
}

void Window::OnStart()
{
}

void Window::OnPause()
{
}

void Window::OnResume()
{
}

void Window::OnStop()
{
}

void Window::OnDestroy()
{
  mAdaptor = NULL;
}

void Window::AddAvailableOrientation(Dali::Window::WindowOrientation orientation)
{
  bool found = false;

  for( std::size_t i=0; i<mAvailableOrientations.size(); i++ )
  {
    if(mAvailableOrientations[i] == orientation)
    {
      found = true;
      break;
    }
  }

  if( ! found )
  {
    mAvailableOrientations.push_back(orientation);
    SetAvailableOrientations( mAvailableOrientations );
  }
}

void Window::RemoveAvailableOrientation(Dali::Window::WindowOrientation orientation)
{
  for( std::vector<Dali::Window::WindowOrientation>::iterator iter = mAvailableOrientations.begin();
       iter != mAvailableOrientations.end(); ++iter )
  {
    if( *iter == orientation )
    {
      mAvailableOrientations.erase( iter );
      break;
    }
  }
  SetAvailableOrientations( mAvailableOrientations );
}

void Window::SetAvailableOrientations(const std::vector<Dali::Window::WindowOrientation>& orientations)
{
  DALI_ASSERT_ALWAYS( mAvailableOrientations.size() <= 4 && "Incorrect number of available orientations" );
}

const std::vector<Dali::Window::WindowOrientation>& Window::GetAvailableOrientations()
{
  return mAvailableOrientations;
}

void Window::SetPreferredOrientation(Dali::Window::WindowOrientation orientation)
{
  mPreferredOrientation = orientation;
}

Dali::Window::WindowOrientation Window::GetPreferredOrientation()
{
  return mPreferredOrientation;
}

void Window::SetAcceptFocus( bool accept )
{
  mIsFocusAcceptable = accept;
}

bool Window::IsFocusAcceptable()
{
  return mIsFocusAcceptable;
}

void Window::Show()
{
  mVisible = true;
}

void Window::Hide()
{
  mVisible = false;
}

bool Window::IsVisible() const
{
  return mVisible;
}


void Window::RotationDone( int orientation, int width, int height )
{
}

unsigned int Window::GetSupportedAuxiliaryHintCount()
{
  return 0;
}

std::string Window::GetSupportedAuxiliaryHint( unsigned int index )
{
  return std::string();
}

unsigned int Window::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return -1;
}

bool Window::RemoveAuxiliaryHint( unsigned int id )
{
  return false;
}

bool Window::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return false;
}

std::string Window::GetAuxiliaryHintValue( unsigned int id ) const
{
  return std::string();
}

unsigned int Window::GetAuxiliaryHintId( const std::string& hint ) const
{
  return -1;
}

void Window::SetInputRegion( const Rect< int >& inputRegion )
{
}

void Window::SetType( Dali::DevelWindow::Type type )
{
  mType = type;
}

Dali::DevelWindow::Type Window::GetType() const
{
  return mType;
}

bool Window::SetNotificationLevel( Dali::DevelWindow::NotificationLevel::Type level )
{
  return false;
}

Dali::DevelWindow::NotificationLevel::Type Window::GetNotificationLevel()
{
  return Dali::DevelWindow::NotificationLevel::NONE;
}

void Window::SetOpaqueState( bool opaque )
{
  mOpaqueState = opaque;
}

bool Window::IsOpaqueState()
{
  return mOpaqueState;
}

bool Window::SetScreenMode( Dali::DevelWindow::ScreenMode::Type screenMode )
{
  return false;
}

Dali::DevelWindow::ScreenMode::Type Window::GetScreenMode()
{
  return Dali::DevelWindow::ScreenMode::DEFAULT;
}

bool Window::SetBrightness( int brightness )
{
  return false;
}

int Window::GetBrightness()
{
  return 0;
}

void Window::SetSize( Dali::DevelWindow::WindowSize size )
{
  PositionSize positionSize = mSurface->GetPositionSize();

  if( positionSize.width != size.GetWidth() || positionSize.height != size.GetHeight() )
  {
    positionSize.width = size.GetWidth();
    positionSize.height = size.GetHeight();

    mSurface->MoveResize( positionSize );

    mAdaptor->SurfaceSizeChanged( Dali::Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );

    // Emit signal
    mResizedSignal.Emit( Dali::DevelWindow::WindowSize( positionSize.width, positionSize.height ) );
  }
}

Dali::DevelWindow::WindowSize Window::GetSize()
{
  PositionSize positionSize = mSurface->GetPositionSize();

  return Dali::DevelWindow::WindowSize( positionSize.width, positionSize.height );
}

void Window::SetPosition( Dali::DevelWindow::WindowPosition position )
{
  PositionSize positionSize = mSurface->GetPositionSize();

  if( positionSize.x != position.GetX() || positionSize.y != position.GetY() )
  {
    positionSize.x = position.GetX();
    positionSize.y = position.GetY();

    mSurface->MoveResize( positionSize );
  }
}

Dali::DevelWindow::WindowPosition Window::GetPosition()
{
  PositionSize positionSize = mSurface->GetPositionSize();

  return Dali::DevelWindow::WindowPosition( positionSize.x, positionSize.y );
}

} // Adaptor
} // Internal
} // Dali
