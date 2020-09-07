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

#include <dali/devel-api/adaptor-framework/clipboard.h>
#include <dali/devel-api/adaptor-framework/style-monitor.h>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/common/accessibility-adaptor-impl.h>
#include <dali/internal/clipboard/common/clipboard-event-notifier-impl.h>
#include <dali/internal/window-system/common/damage-observer.h>
#include <dali/internal/window-system/common/window-base.h>

namespace Dali
{

namespace Integration
{

struct Point;
struct KeyEvent;
struct WheelEvent;

}

namespace Internal
{

namespace Adaptor
{

class StyleMonitor;
class WindowRenderSurface;

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
   * The observer can be overridden in order to listen to the events.
   */
  class Observer
  {
  public:

    /**
     * Deriving classes should override this to be notified when we receive a touch point event.
     * @param[in] point The touch point
     * @param[in] timeStamp The time stamp
     */
    virtual void OnTouchPoint( Dali::Integration::Point& point, int timeStamp ) = 0;

    /**
     * Deriving classes should override this to be notified when we receive a wheel event.
     * @param[in] wheelEvent The wheel event
     */
    virtual void OnWheelEvent( Dali::Integration::WheelEvent& wheelEvent ) = 0;

    /**
     * Deriving classes should override this to be notified when we receive a key event.
     * @param[in] keyEvent The key event holding the key information.
     */
    virtual void OnKeyEvent( Dali::Integration::KeyEvent& keyEvent ) = 0;

    /**
     * Deriving classes should override this to be notified when the window is rotated.
     * @param[in] rotation The rotation event.
     */
    virtual void OnRotation( const RotationEvent& rotation ) = 0;

  protected:

    /**
     * Protected Constructor.
     */
    Observer() {}

    /**
     * Protected virtual destructor.
     */
    virtual ~Observer() {}
  };

public:

  /**
   * Constructor.
   * @param[in]  surface                  The render surface of the window.
   * @param[in]  damageObserver           The damage observer (to pass damage events to).
   */
  EventHandler( WindowRenderSurface* surface, DamageObserver& damageObserver );

  /**
   * Destructor.
   */
  ~EventHandler();

  /**
   * Called when the adaptor is paused.
   */
  void Pause();

  /**
   * Called when the adaptor is resumed (from pause).
   */
  void Resume();

  /**
   * Adds an observer so that we can observe the events.
   * @param[in] observer The observer.
   */
  void AddObserver( Observer& observer );

  /**
   * Removes the observer from the EventHandler.
   * @param[in] observer The observer to remove.
   * @note Observers should remove themselves when they are destroyed.
   */
  void RemoveObserver( Observer& observer );

private:

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
   * Called when the window is rotated.
   * @param[in] event The rotation event
   */
  void OnRotation( const RotationEvent& event );

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

  // Undefined
  EventHandler( const EventHandler& eventHandler );

  // Undefined
  EventHandler& operator=( const EventHandler& eventHandler );

private:

  Dali::StyleMonitor mStyleMonitor; ///< Handle to the style monitor, set on construction, to send font size and font change events to.
  DamageObserver& mDamageObserver; ///< Reference to the DamageObserver, set on construction, to sent damage events to.

  Dali::AccessibilityAdaptor mAccessibilityAdaptor; ///< Pointer to the accessibility adaptor
  Dali::ClipboardEventNotifier mClipboardEventNotifier; ///< Pointer to the clipboard event notifier
  Dali::Clipboard mClipboard;///< Pointer to the clipboard

  using ObserverContainer = std::vector<Observer*>;
  ObserverContainer mObservers;   ///< A list of event observer pointers

  bool mPaused; ///< The paused state of the adaptor.
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_EVENT_HANDLER_H
