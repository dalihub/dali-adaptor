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
#include <dali/internal/window-system/common/event-handler.h>

// EXTERNAL INCLUDES
#include <cstring>
#include <sys/time.h>

#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/styling/common/style-monitor-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
namespace
{
Integration::Log::Filter* gSelectionEventLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_SELECTION");
} // unnamed namespace
#endif

#if 0 and defined DALI_ELDBUS_AVAILABLE
namespace
{

// Copied from x server
static uint32_t GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec tp;
  static clockid_t clockid;

  if (!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
      (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if (clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return static_cast<uint32_t>( (tp.tv_sec * 1000 ) + (tp.tv_nsec / 1000000L) );
  }

  gettimeofday(&tv, NULL);
  return static_cast<uint32_t>( (tv.tv_sec * 1000 ) + (tv.tv_usec / 1000) );
}

} // unnamed namespace
#endif

EventHandler::EventHandler( WindowRenderSurface* surface, DamageObserver& damageObserver )
: mStyleMonitor( StyleMonitor::Get() ),
  mDamageObserver( damageObserver ),
  mClipboardEventNotifier( ClipboardEventNotifier::Get() ),
  mClipboard( Clipboard::Get() ),
  mPaused( false )
{
  if( surface )
  {
    WindowBase* windowBase = surface->GetWindowBase();

    // Connect signals
    windowBase->WindowDamagedSignal().Connect( this, &EventHandler::OnWindowDamaged );
    windowBase->FocusChangedSignal().Connect( this, &EventHandler::OnFocusChanged );
    windowBase->RotationSignal().Connect( this, &EventHandler::OnRotation );
    windowBase->TouchEventSignal().Connect( this, &EventHandler::OnTouchEvent );
    windowBase->WheelEventSignal().Connect( this, &EventHandler::OnWheelEvent );
    windowBase->KeyEventSignal().Connect( this, &EventHandler::OnKeyEvent );
    windowBase->SelectionDataSendSignal().Connect( this, &EventHandler::OnSelectionDataSend );
    windowBase->SelectionDataReceivedSignal().Connect( this, &EventHandler::OnSelectionDataReceived );
    windowBase->StyleChangedSignal().Connect( this, &EventHandler::OnStyleChanged );
  }
}

EventHandler::~EventHandler()
{
}

void EventHandler::SendEvent( StyleChange::Type styleChange )
{
  DALI_ASSERT_DEBUG( mStyleMonitor && "StyleMonitor Not Available" );
  GetImplementation( mStyleMonitor ).StyleChanged(styleChange);
}

void EventHandler::SendEvent( const DamageArea& area )
{
  mDamageObserver.OnDamaged( area );
}

void EventHandler::Pause()
{
  mPaused = true;
}

void EventHandler::Resume()
{
  mPaused = false;
}

void EventHandler::OnTouchEvent( Integration::Point& point, uint32_t timeStamp )
{
  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnTouchPoint( point, timeStamp );
  }
}

void EventHandler::OnWheelEvent( WheelEvent& wheelEvent )
{
  Integration::WheelEvent event( static_cast< Integration::WheelEvent::Type >(wheelEvent.type), wheelEvent.direction, wheelEvent.modifiers, wheelEvent.point, wheelEvent.z, wheelEvent.timeStamp );

  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnWheelEvent( event );
  }
}

void EventHandler::OnKeyEvent( Integration::KeyEvent& keyEvent )
{
  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnKeyEvent( keyEvent );
  }
}

void EventHandler::OnFocusChanged( bool focusIn )
{
  // If the window gains focus and we hid the keyboard then show it again.
  if( focusIn )
  {
    Dali::Clipboard clipboard = Clipboard::Get();
    if ( clipboard )
    {
      clipboard.HideClipboard();
    }
  }
  else
  {
    // Hiding clipboard event will be ignored once because window focus out event is always received on showing clipboard
    Dali::Clipboard clipboard = Clipboard::Get();
    if ( clipboard )
    {
      Clipboard& clipBoardImpl( GetImplementation( clipboard ) );
      clipBoardImpl.HideClipboard(true);
    }
  }
}

void EventHandler::OnRotation( const RotationEvent& event )
{
  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnRotation( event );
  }
}

void EventHandler::OnWindowDamaged( const DamageArea& area )
{
  SendEvent( area );
}

void EventHandler::OnSelectionDataSend( void* event )
{
  Dali::Clipboard clipboard = Clipboard::Get();
  if( clipboard )
  {
    Clipboard& clipBoardImpl( GetImplementation( clipboard ) );
    clipBoardImpl.ExcuteBuffered( true, event );
  }
}

void EventHandler::OnSelectionDataReceived( void* event )
{
  // We have got the selected content, inform the clipboard event listener (if we have one).
  Dali::Clipboard clipboard = Clipboard::Get();
  char* selectionData = NULL;
  if( clipboard )
  {
    Clipboard& clipBoardImpl( GetImplementation( clipboard ) );
    selectionData = clipBoardImpl.ExcuteBuffered( false, event );
  }

  if( selectionData && mClipboardEventNotifier )
  {
    ClipboardEventNotifier& clipboardEventNotifier( ClipboardEventNotifier::GetImplementation( mClipboardEventNotifier ) );
    std::string content( selectionData, strlen( selectionData ) );

    clipboardEventNotifier.SetContent( content );
    clipboardEventNotifier.EmitContentSelectedSignal();

    DALI_LOG_INFO( gSelectionEventLogFilter, Debug::General, "EcoreEventSelectionNotify: Content(%d): %s\n" , strlen(selectionData), selectionData );
  }
}

void EventHandler::OnStyleChanged( StyleChange::Type styleChange )
{
  SendEvent( styleChange );
}

void EventHandler::AddObserver( Observer& observer )
{
  ObserverContainer::iterator match ( find(mObservers.begin(), mObservers.end(), &observer) );

  if ( match == mObservers.end() )
  {
    mObservers.push_back( &observer );
  }
}

void EventHandler::RemoveObserver( Observer& observer )
{
  ObserverContainer::iterator match ( find(mObservers.begin(), mObservers.end(), &observer) );

  if ( match != mObservers.end() )
  {
    mObservers.erase( match );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
