/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/input/common/gesture-manager.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/physical-keyboard-impl.h>
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
Integration::Log::Filter* gTouchEventLogFilter  = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_TOUCH");
Integration::Log::Filter* gSelectionEventLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_SELECTION");
} // unnamed namespace
#endif

namespace
{

// Copied from x server
static unsigned int GetCurrentMilliSeconds(void)
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
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

} // unnamed namespace

EventHandler::EventHandler( RenderSurface* surface, CoreEventInterface& coreEventInterface, GestureManager& gestureManager, DamageObserver& damageObserver, DragAndDropDetectorPtr dndDetector )
: mCoreEventInterface( coreEventInterface ),
  mGestureManager( gestureManager ),
  mStyleMonitor( StyleMonitor::Get() ),
  mDamageObserver( damageObserver ),
  mRotationObserver( NULL ),
  mDragAndDropDetector( dndDetector ),
  mAccessibilityAdaptor( AccessibilityAdaptor::Get() ),
  mClipboardEventNotifier( ClipboardEventNotifier::Get() ),
  mClipboard( Clipboard::Get() ),
  mRotationAngle( 0 ),
  mWindowWidth( 0 ),
  mWindowHeight( 0 ),
  mPaused( false )
{
  // this code only works with the WindowRenderSurface so need to downcast
  WindowRenderSurface* windowRenderSurface = static_cast< WindowRenderSurface* >( surface );
  if( windowRenderSurface )
  {
    WindowBase* windowBase = windowRenderSurface->GetWindowBase();

    // Connect signals
    windowBase->WindowDamagedSignal().Connect( this, &EventHandler::OnWindowDamaged );
    windowBase->FocusChangedSignal().Connect( this, &EventHandler::OnFocusChanged );
    windowBase->RotationSignal().Connect( this, &EventHandler::SendRotationPrepareEvent );
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
  mGestureManager.Stop();
}

void EventHandler::SendEvent(Integration::Point& point, unsigned long timeStamp)
{
  if(timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  ConvertTouchPosition( point );

  Integration::TouchEvent touchEvent;
  Integration::HoverEvent hoverEvent;
  Integration::TouchEventCombiner::EventDispatchType type = mCombiner.GetNextTouchEvent(point, timeStamp, touchEvent, hoverEvent);
  if(type != Integration::TouchEventCombiner::DispatchNone )
  {
    DALI_LOG_INFO(gTouchEventLogFilter, Debug::General, "%d: Device %d: Button state %d (%.2f, %.2f)\n", timeStamp, point.GetDeviceId(), point.GetState(), point.GetScreenPosition().x, point.GetScreenPosition().y);

    // First the touch and/or hover event & related gesture events are queued
    if(type == Integration::TouchEventCombiner::DispatchTouch || type == Integration::TouchEventCombiner::DispatchBoth)
    {
      mCoreEventInterface.QueueCoreEvent( touchEvent );
      mGestureManager.SendEvent(touchEvent);
    }

    if(type == Integration::TouchEventCombiner::DispatchHover || type == Integration::TouchEventCombiner::DispatchBoth)
    {
      mCoreEventInterface.QueueCoreEvent( hoverEvent );
    }

    // Next the events are processed with a single call into Core
    mCoreEventInterface.ProcessCoreEvents();
  }
}

void EventHandler::SendEvent(Integration::KeyEvent& keyEvent)
{
  Dali::PhysicalKeyboard physicalKeyboard = PhysicalKeyboard::Get();
  if ( physicalKeyboard )
  {
    if ( ! KeyLookup::IsDeviceButton( keyEvent.keyName.c_str() ) )
    {
      GetImplementation( physicalKeyboard ).KeyReceived( keyEvent.time > 1 );
    }
  }

  // Create send KeyEvent to Core.
  mCoreEventInterface.QueueCoreEvent( keyEvent );
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::SendWheelEvent( WheelEvent& wheelEvent )
{
  // Create WheelEvent and send to Core.
  Integration::WheelEvent event( static_cast< Integration::WheelEvent::Type >(wheelEvent.type), wheelEvent.direction, wheelEvent.modifiers, wheelEvent.point, wheelEvent.z, wheelEvent.timeStamp );
  mCoreEventInterface.QueueCoreEvent( event );
  mCoreEventInterface.ProcessCoreEvents();
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

void EventHandler::SendRotationPrepareEvent( const RotationEvent& event )
{
  if( mRotationObserver != NULL )
  {
    mRotationAngle = event.angle;
    mWindowWidth = event.width;
    mWindowHeight = event.height;

    mRotationObserver->OnRotationPrepare( event );
    mRotationObserver->OnRotationRequest();
  }
}

void EventHandler::SendRotationRequestEvent( )
{
  // No need to separate event into prepare and request
}

void EventHandler::FeedTouchPoint( TouchPoint& point, int timeStamp)
{
  Integration::Point convertedPoint( point );
  SendEvent(convertedPoint, timeStamp);
}

void EventHandler::FeedWheelEvent( WheelEvent& wheelEvent )
{
  SendWheelEvent( wheelEvent );
}

void EventHandler::FeedKeyEvent( KeyEvent& event )
{
  Integration::KeyEvent convertedEvent( event );
  SendEvent( convertedEvent );
}

void EventHandler::FeedEvent( Integration::Event& event )
{
  mCoreEventInterface.QueueCoreEvent( event );
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::Reset()
{
  mCombiner.Reset();

  // Any touch listeners should be told of the interruption.
  Integration::TouchEvent event;
  Integration::Point point;
  point.SetState( PointState::INTERRUPTED );
  event.AddPoint( point );

  // First the touch event & related gesture events are queued
  mCoreEventInterface.QueueCoreEvent( event );
  mGestureManager.SendEvent( event );

  // Next the events are processed with a single call into Core
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::Pause()
{
  mPaused = true;
  Reset();
}

void EventHandler::Resume()
{
  mPaused = false;
  Reset();
}

void EventHandler::SetDragAndDropDetector( DragAndDropDetectorPtr detector )
{
  mDragAndDropDetector = detector;
}

void EventHandler::SetRotationObserver( RotationObserver* observer )
{
  mRotationObserver = observer;
}

void EventHandler::OnTouchEvent( Integration::Point& point, unsigned long timeStamp )
{
  SendEvent( point, timeStamp );
}

void EventHandler::OnWheelEvent( WheelEvent& wheelEvent )
{
  SendWheelEvent( wheelEvent );
}

void EventHandler::OnKeyEvent( Integration::KeyEvent& keyEvent )
{
  SendEvent( keyEvent );
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
      accessibilityAdaptor->HandleActionReadIndicatorInformationEvent();
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

void EventHandler::ConvertTouchPosition( Integration::Point& point )
{
  Vector2 position = point.GetScreenPosition();
  Vector2 convertedPosition;

  switch( mRotationAngle )
  {
    case 90:
    {
      convertedPosition.x = mWindowWidth - position.y;
      convertedPosition.y = position.x;
      break;
    }
    case 180:
    {
      convertedPosition.x = mWindowWidth - position.x;
      convertedPosition.y = mWindowHeight - position.y;
      break;
    }
    case 270:
    {
      convertedPosition.x = position.y;
      convertedPosition.y = mWindowHeight - position.x;
      break;
    }
    default:
    {
      convertedPosition = position;
      break;
    }
  }

  point.SetScreenPosition( convertedPosition );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
