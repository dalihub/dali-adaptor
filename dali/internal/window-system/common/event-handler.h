#ifndef DALI_INTERNAL_EVENT_HANDLER_H
#define DALI_INTERNAL_EVENT_HANDLER_H

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

// EXTERNAL INCLUDES
#include <cstdint> // uint32_t
#include <dali/public-api/common/intrusive-ptr.h>

#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/point.h>
#include <dali/integration-api/events/touch-event-combiner.h>
#include <dali/devel-api/adaptor-framework/clipboard.h>
#include <dali/devel-api/adaptor-framework/style-monitor.h>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/common/accessibility-adaptor-impl.h>
#include <dali/internal/clipboard/common/clipboard-event-notifier-impl.h>
#include <dali/internal/input/common/drag-and-drop-detector-impl.h>
#include <dali/internal/system/common/core-event-interface.h>
#include <dali/internal/window-system/common/damage-observer.h>
#include <dali/internal/window-system/common/rotation-observer.h>
#include <dali/internal/window-system/common/window-base.h>

namespace Dali
{

namespace Integration
{

class RenderSurface;
class Scene;
}

namespace Internal
{

namespace Adaptor
{

class GestureManager;
class StyleMonitor;

/**
 * The Event Handler class is responsible for setting up receiving of Ecore events and then converts them
 * to TouchEvents when it does receive them.
 *
 * These TouchEvents are then passed on to Core.
 */
class EventHandler : public ConnectionTracker, public Dali::RefObject
{
public:

  /**
   * Constructor.
   * @param[in]  scene                    The scene where events will be sent to.
   * @param[in]  coreEventInterface       Used to send events to Core.
   * @param[in]  gestureManager           The Gesture Manager.
   * @param[in]  damageObserver           The damage observer (to pass damage events to).
   */
  EventHandler( Dali::Integration::Scene scene, CoreEventInterface& coreEventInterface, GestureManager& gestureManager, DamageObserver& damageObserver );

  /**
   * Destructor.
   */
  ~EventHandler();

  /**
   * Feed (Send) touch event to core and gesture manager
   * @param[in] touchEvent  The touch event holding the touch point information.
   */
  void FeedTouchPoint( TouchPoint& point, uint32_t timeStamp );

  /**
   * Feed (Send) wheel event to core and gesture manager
   * @param[in]  wheelEvent The wheel event
   */
  void FeedWheelEvent( WheelEvent& wheelEvent );

  /**
   * Feed (Send) key event to core
   * @param[in] keyEvent The key event holding the key information.
   */
  void FeedKeyEvent( KeyEvent& keyEvent );

  /**
   * Feed (Send) an event to core
   * @param[in] event  The event information.
   */
  void FeedEvent( Integration::Event& event );

  /**
   * Called when the adaptor is paused.
   */
  void Pause();

  /**
   * Called when the adaptor is resumed (from pause).
   */
  void Resume();

  /**
   * Set the rotation observer (note, some adaptors may not have a rotation observer)
   * @param[in] observer The rotation observer
   */
  void SetRotationObserver( RotationObserver* observer );

private:

  /**
   * Send touch event to core.
   * @param[in]  point      The touch point information.
   * @param[in]  timeStamp  The time the touch occurred.
   */
  void SendEvent(Integration::Point& point, uint32_t timeStamp);

  /**
   * Send key event to core.
   * @param[in]  keyEvent The KeyEvent to send.
   */
  void SendEvent(Integration::KeyEvent& keyEvent);

  /**
   * Send wheel event to core.
   * @param[in]  wheelEvent The wheel event
   */
  void SendWheelEvent( WheelEvent& wheelEvent );

  /**
   * Send a style change event to the style monitor.
   * @param[in]  styleChange  The style that has changed.
   */
  void SendEvent( StyleChange::Type styleChange );

  /**
   * Send a window damage event to the observer.
   * @param[in]  area  Damaged area.
   */
  void SendEvent( const DamageArea& area );

  /**
   * Inform rotation observer of rotation prepare event
   * @param[in] rotation The rotation event
   */
  void SendRotationPrepareEvent( const RotationEvent& rotation );

  /**
   * Inform rotation observer of rotation prepare event
   */
  void SendRotationRequestEvent();

  /**
   * Resets the event handler.
   * Called when the adaptor is paused or resumed.
   */
  void Reset();

  /**
   * Called when a touch event is received.
   */
  void OnTouchEvent( Integration::Point& point, uint32_t timeStamp );

  /**
   * Called when a mouse wheel is received.
   */
  void OnWheelEvent( WheelEvent& wheelEvent );

  /**
   * Called when a key event is received.
   */
  void OnKeyEvent( Integration::KeyEvent& keyEvent );

  /**
   * Called when the window focus is changed.
   */
  void OnFocusChanged( bool focusIn );

  /**
   * Called when the window is damaged.
   */
  void OnWindowDamaged( const DamageArea& area );

  /**
   * Called when the source window notifies us the content in clipboard is selected.
   */
  void OnSelectionDataSend( void* event );

  /**
   * Called when the source window sends us about the selected content.
   */
  void OnSelectionDataReceived( void* event );

  /**
   * Called when the style is changed.
   */
  void OnStyleChanged( StyleChange::Type styleChange );

  /**
   * Called when Ecore ElDBus accessibility event is received.
   */
  void OnAccessibilityNotification( const WindowBase::AccessibilityInfo& info );

private:

  /**
   * Convert touch event position
   */
  void ConvertTouchPosition( Integration::Point& point );

private:

  // Undefined
  EventHandler( const EventHandler& eventHandler );

  // Undefined
  EventHandler& operator=( const EventHandler& eventHandler );

private:

  Dali::Integration::Scene mScene; ///< The scene the event handler is created for.
  CoreEventInterface& mCoreEventInterface; ///< Used to send events to Core.
  Dali::Integration::TouchEventCombiner mCombiner; ///< Combines multi-touch events.
  GestureManager& mGestureManager; ///< Reference to the GestureManager, set on construction, to send touch events to for analysis.
  Dali::StyleMonitor mStyleMonitor; ///< Handle to the style monitor, set on construction, to send font size and font change events to.
  DamageObserver& mDamageObserver; ///< Reference to the DamageObserver, set on construction, to sent damage events to.
  RotationObserver* mRotationObserver; ///< Pointer to rotation observer, if present.

  Dali::AccessibilityAdaptor mAccessibilityAdaptor; ///< Pointer to the accessibility adaptor
  Dali::ClipboardEventNotifier mClipboardEventNotifier; ///< Pointer to the clipboard event notifier
  Dali::Clipboard mClipboard;///< Pointer to the clipboard

  int mRotationAngle;
  int mWindowWidth;
  int mWindowHeight;

  bool mPaused; ///< The paused state of the adaptor.
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_EVENT_HANDLER_H
