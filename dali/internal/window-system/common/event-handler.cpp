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
#include <dali/internal/window-system/common/event-handler.h>

// EXTERNAL INCLUDES
#include <cstring>
#include <sys/time.h>

#include <dali/devel-api/events/touch-point.h>
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

static constexpr auto QUICKPANEL_TYPE_SYSTEM_DEFAULT = 1;
static constexpr auto QUICKPANEL_TYPE_APPS_MENU = 3;

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

EventHandler::EventHandler( WindowBase* windowBase, DamageObserver& damageObserver )
: mStyleMonitor( StyleMonitor::Get() ),
  mDamageObserver( damageObserver ),
  mAccessibilityAdaptor( AccessibilityAdaptor::Get() ),
  mClipboardEventNotifier( ClipboardEventNotifier::Get() ),
  mClipboard( Clipboard::Get() ),
  mWindowBase( windowBase ),
  mPaused( false )
{
  // Connect signals
  if( windowBase )
  {
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
    windowBase->QuickPanelSignal().Connect( this, &EventHandler::OnAccessibilityQuickpanelChanged );
  }
  else
  {
    DALI_LOG_ERROR("WindowBase is invalid!!!\n");
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

void EventHandler::OnWheelEvent( Integration::WheelEvent& wheelEvent )
{
  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnWheelEvent( wheelEvent );
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

#ifdef DALI_ELDBUS_AVAILABLE
    // When dali window gains the focus, Accessibility should be enabled.
    if ( mAccessibilityAdaptor )
    {
      AccessibilityAdaptor* accessibilityAdaptor( &AccessibilityAdaptor::GetImplementation( mAccessibilityAdaptor ) );
      if ( accessibilityAdaptor )
      {
        DALI_LOG_ERROR("(Focus gained): EnableAccessibility \n");
        accessibilityAdaptor->EnableAccessibility();
      }
    }
#endif
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

#ifdef DALI_ELDBUS_AVAILABLE
    // When dali window loses the focus, Accessibility should be disabled.
    if ( mAccessibilityAdaptor )
    {
      AccessibilityAdaptor* accessibilityAdaptor( &AccessibilityAdaptor::GetImplementation( mAccessibilityAdaptor ) );
      if ( accessibilityAdaptor && accessibilityAdaptor->IsEnabled() )
      {
        DALI_LOG_ERROR("(Focus lost): DisableAccessibility \n");
        accessibilityAdaptor->DisableAccessibility();
      }
    }
#endif
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

  if( info.gestureValue == 15 ) // ONE_FINGER_SINGLE_TAP
  {
    DALI_LOG_ERROR("[FYI] Native window resource ID : %s, Touched window resource ID : %d \n", mWindowBase->GetNativeWindowResourceId().c_str(), info.resourceId);
    if( std::to_string( info.resourceId ) == mWindowBase->GetNativeWindowResourceId() )
    {
      if( !accessibilityAdaptor->IsEnabled() )
      {
        // Accessibility gesture was sent to this window, so enable accessibility
        accessibilityAdaptor->EnableAccessibility();
      }
    }
    else
    {
      // Ignore tap gesture - it should be handled by another window (possibly in another application),
      // for which the resource ID matches with the one stored in AccessibilityInfo.
      DALI_LOG_ERROR("Ignore Single tap gesture because the gesture should be handled by other window. \n");
      return;
    }
  }

  if( accessibilityAdaptor->IsForcedShown() )
  {
    DALI_LOG_ERROR("InsideFridge app calls the API \n");
    if( accessibilityAdaptor->IsInsideFridgeShown() )
    {
      DALI_LOG_ERROR("Is Inside Fridge shown [TRUE] -> Enable \n");
      accessibilityAdaptor->EnableAccessibility();
    }
    else
    {
      DALI_LOG_ERROR("Is Inside Fridge shown [FALSE] -> Disable \n");
      accessibilityAdaptor->DisableAccessibility();
    }
  }

  if( !accessibilityAdaptor->IsEnabled() )
  {
    DALI_LOG_ERROR( "The current dali accessibility is not available. \n" );
    return;
  }

  if( ( info.quickpanelInfo & ( 1 << QUICKPANEL_TYPE_SYSTEM_DEFAULT ) ) && ( info.quickpanelInfo & ( 1 << QUICKPANEL_TYPE_APPS_MENU ) ) )
  {
    DALI_LOG_ERROR("Quickpanel is top now, so all dali apps should be stopped \n");
    return;
  }

  if( !(info.quickpanelInfo & ( 1 << QUICKPANEL_TYPE_APPS_MENU ) ) && accessibilityAdaptor->IsForcedEnable() )
  {
    // When other apps, which are not implemented by DALi, are on top of Apps application,
    // no event should be occurred on Apps because it's disabled by force already.
    DALI_LOG_ERROR("Currently, all DALi applications do not receive any events because another app, which is not implemented by DALi, is at the top of the layer. \n");
    return;
  }

  DALI_LOG_ERROR("[FYI] Accessibility gesture value : %d, state : %d \n", info.gestureValue, info.state);

  // When gesture is ONE_FINGER_SINGLE_TAP, the gesture value is 15.
  // When the state is aborted, the state of accessibility info is 3.
  if( info.gestureValue == 15 && info.state == 3 )
  {
    Vector2 localPosition = accessibilityAdaptor->GetFocusedActorPosition();

    eldbus_proxy_call( info.proxy, "HighlightedObjectInfo", NULL, NULL, -1, "ii", static_cast<int>( localPosition.x ), static_cast<int>( localPosition.y ) );
  }

  // Create a touch point object.
  PointState::Type touchPointState( PointState::DOWN );
  if( info.state == 0 )
  {
    touchPointState = PointState::DOWN; // Mouse down.
  }
  else if( info.state == 1 )
  {
    touchPointState = PointState::MOTION; // Mouse move.
  }
  else if( info.state == 2 )
  {
    touchPointState = PointState::UP; // Mouse up.
  }
  else
  {
    touchPointState = PointState::INTERRUPTED; // Error.
  }

  // Send touch event to accessibility adaptor.
  TouchPoint point( 0, touchPointState, static_cast< float >( info.startX ), static_cast< float >( info.startY ) );

  // Forward the event to the application.
  // This is a kind of backdoor to bypass the normal behaviour.
  if( info.gestureValue == 15 && info.state == 3 )
  {
    accessibilityAdaptor->HandleActionForwardToAppEvent();
    return;
  }

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

void EventHandler::OnAccessibilityQuickpanelChanged( const unsigned char& info )
{
#ifdef DALI_ELDBUS_AVAILABLE
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

  if( info & ( 1 << QUICKPANEL_TYPE_SYSTEM_DEFAULT ) ) // QuickPanel is shown
  {
    // dali apps should be disabled.
    DALI_LOG_ERROR("[FYI] Quickpanel show -> DisableAccessibility \n");
    accessibilityAdaptor->DisableAccessibility();
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
