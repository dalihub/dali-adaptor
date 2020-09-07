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

#ifdef DALI_ELDBUS_AVAILABLE
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
  mAccessibilityAdaptor( AccessibilityAdaptor::Get() ),
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
    windowBase->AccessibilitySignal().Connect( this, &EventHandler::OnAccessibilityNotification );
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

void EventHandler::OnAccessibilityNotification( const WindowBase::AccessibilityInfo& info )
{
#ifdef DALI_ELDBUS_AVAILABLE
  if( mPaused )
  {
    return;
  }

  if( !mAccessibilityAdaptor )
  {
    DALI_LOG_ERROR( "Invalid accessibility adaptor\n" );
    return;
  }

  AccessibilityAdaptor* accessibilityAdaptor( &AccessibilityAdaptor::GetImplementation( mAccessibilityAdaptor ) );
  if( !accessibilityAdaptor )
  {
    DALI_LOG_ERROR( "Cannot access accessibility adaptor\n" );
    return;
  }

  // Create a touch point object.
  TouchPoint::State touchPointState( TouchPoint::Down );
  if( info.state == 0 )
  {
    touchPointState = TouchPoint::Down; // Mouse down.
  }
  else if( info.state == 1 )
  {
    touchPointState = TouchPoint::Motion; // Mouse move.
  }
  else if( info.state == 2 )
  {
    touchPointState = TouchPoint::Up; // Mouse up.
  }
  else
  {
    touchPointState = TouchPoint::Interrupted; // Error.
  }

  // Send touch event to accessibility adaptor.
  TouchPoint point( 0, touchPointState, static_cast< float >( info.startX ), static_cast< float >( info.startY ) );

  // Perform actions based on received gestures.
  // Note: This is seperated from the reading so we can have other input readers without changing the below code.
  switch( info.gestureValue )
  {
    case 0: // OneFingerHover
    {
      // Focus, read out.
      accessibilityAdaptor->HandleActionReadEvent( static_cast< unsigned int >( info.startX ), static_cast< unsigned int >( info.startY ), true /* allow read again */ );
      break;
    }
    case 1: // TwoFingersHover
    {
      // In accessibility mode, scroll action should be handled when the currently focused actor is contained in scrollable control
      accessibilityAdaptor->HandleActionScrollEvent( point, GetCurrentMilliSeconds() );
      break;
    }
    case 2: // ThreeFingersHover
    {
      // Read from top item on screen continuously.
      accessibilityAdaptor->HandleActionReadFromTopEvent();
      break;
    }
    case 3: // OneFingerFlickLeft
    {
      // Move to previous item.
      accessibilityAdaptor->HandleActionReadPreviousEvent();
      break;
    }
    case 4: // OneFingerFlickRight
    {
      // Move to next item.
      accessibilityAdaptor->HandleActionReadNextEvent();
      break;
    }
    case 5: // OneFingerFlickUp
    {
      // Move to previous item.
      accessibilityAdaptor->HandleActionPreviousEvent();
      break;
    }
    case 6: // OneFingerFlickDown
    {
      // Move to next item.
      accessibilityAdaptor->HandleActionNextEvent();
      break;
    }
    case 7: // TwoFingersFlickUp
    {
      // Scroll up the list.
      accessibilityAdaptor->HandleActionScrollUpEvent();
      break;
    }
    case 8: // TwoFingersFlickDown
    {
      // Scroll down the list.
      accessibilityAdaptor->HandleActionScrollDownEvent();
      break;
    }
    case 9: // TwoFingersFlickLeft
    {
      // Scroll left to the previous page
      accessibilityAdaptor->HandleActionPageLeftEvent();
      break;
    }
    case 10: // TwoFingersFlickRight
    {
      // Scroll right to the next page
      accessibilityAdaptor->HandleActionPageRightEvent();
      break;
    }
    case 11: // ThreeFingersFlickLeft
    {
      // Not exist yet
      break;
    }
    case 12: // ThreeFingersFlickRight
    {
      // Not exist yet
      break;
    }
    case 13: // ThreeFingersFlickUp
    {
      // Not exist yet
      break;
    }
    case 14: // ThreeFingersFlickDown
    {
      // Not exist yet
      break;
    }
    case 15: // OneFingerSingleTap
    {
      // Focus, read out.
      accessibilityAdaptor->HandleActionReadEvent( static_cast< unsigned int >( info.startX ), static_cast< unsigned int >( info.startY ), true /* allow read again */ );
      break;
    }
    case 16: // OneFingerDoubleTap
    {
      // Activate selected item / active edit mode.
      accessibilityAdaptor->HandleActionActivateEvent();
      break;
    }
    case 17: // OneFingerTripleTap
    {
      // Zoom
      accessibilityAdaptor->HandleActionZoomEvent();
      break;
    }
    case 18: // TwoFingersSingleTap
    {
      // Pause/Resume current speech
      accessibilityAdaptor->HandleActionReadPauseResumeEvent();
      break;
    }
    case 19: // TwoFingersDoubleTap
    {
      // Start/Stop current action
      accessibilityAdaptor->HandleActionStartStopEvent();
      break;
    }
    case 20: // TwoFingersTripleTap
    {
      // Read information from indicator
      // Not supported
      break;
    }
    case 21: // ThreeFingersSingleTap
    {
      // Read from top item on screen continuously.
      accessibilityAdaptor->HandleActionReadFromTopEvent();
      break;
    }
    case 22: // ThreeFingersDoubleTap
    {
      // Read from next item continuously.
      accessibilityAdaptor->HandleActionReadFromNextEvent();
      break;
    }
    case 23: // ThreeFingersTripleTap
    {
      // Not exist yet
      break;
    }
    case 24: // OneFingerFlickLeftReturn
    {
      // Scroll up to the previous page
      accessibilityAdaptor->HandleActionPageUpEvent();
      break;
    }
    case 25: // OneFingerFlickRightReturn
    {
      // Scroll down to the next page
      accessibilityAdaptor->HandleActionPageDownEvent();
      break;
    }
    case 26: // OneFingerFlickUpReturn
    {
      // Move to the first item on screen
      accessibilityAdaptor->HandleActionMoveToFirstEvent();
      break;
    }
    case 27: // OneFingerFlickDownReturn
    {
      // Move to the last item on screen
      accessibilityAdaptor->HandleActionMoveToLastEvent();
      break;
    }
    case 28: // TwoFingersFlickLeftReturn
    {
      // Not exist yet
      break;
    }
    case 29: // TwoFingersFlickRightReturn
    {
      // Not exist yet
      break;
    }
    case 30: // TwoFingersFlickUpReturn
    {
      // Not exist yet
      break;
    }
    case 31: // TwoFingersFlickDownReturn
    {
      // Not exist yet
      break;
    }
    case 32: // ThreeFingersFlickLeftReturn
    {
      // Not exist yet
      break;
    }
    case 33: // ThreeFingersFlickRightReturn
    {
      // Not exist yet
      break;
    }
    case 34: // ThreeFingersFlickUpReturn
    {
      // Not exist yet
      break;
    }
    case 35: // ThreeFingersFlickDownReturn
    {
      // Not exist yet
      break;
    }
  }
#endif
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
