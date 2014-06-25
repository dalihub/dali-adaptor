#ifndef __DALI_INTERNAL_EVAS_PLUGIN_H__
#define __DALI_INTERNAL_EVAS_PLUGIN_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <Elementary.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Input.h>

#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/vector2.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES
#include <imf-manager.h>
#include <evas-plugin.h>

#include <virtual-keyboard-impl.h>
#include <clipboard-impl.h>

namespace Dali
{

class Adaptor;

namespace Internal
{

namespace Adaptor
{
class TriggerEvent;

typedef Dali::Rect<int> PositionSize;

namespace ECore
{
class RenderSurface;
}

/**
 * Implementation of the EvasPlugin class.
 */
class EvasPlugin : public ConnectionTrackerInterface
{
public:

  typedef Dali::EvasPlugin::EvasPluginSignalV2 EvasPluginSignalV2;

  /**
   * Constructor
   * @param[in] evasPlugin The public instance of the EvasPlugin
   * @param[in] parent A pointer of the parent object
   * @param[in] isTransparent Whether the object is transparent or not
   * @param[in] initialWidth width for canvas
   * @param[in] initialHeight height for canvas
   */
  EvasPlugin(Dali::EvasPlugin& evasPlugin, Evas_Object* parent, bool isTransparent, unsigned int initialWidth, unsigned int initialHeight);

  /**
   * Destructor
   */
  virtual ~EvasPlugin();

public:

  /**
   * @copydoc Dali::EvasPlugin::Start()
   */
  virtual void Run();

  /**
   * @copydoc Dali::EvasPlugin::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc Dali::EvasPlugin::Resume()
   */
  virtual void Resume();

  /**
   * @copydoc Dali::EvasPlugin::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc Dali::EvasPlugin::GetEvasImageObject()
   */
  Evas_Object* GetEvasImageObject();

  /**
   * @copydoc Dali::EvasPlugin::GetElmAccessObject()
   */
  Evas_Object* GetElmAccessObject();

  /**
   * @copydoc Dali::EvasPlugin::GetElmFocusObject()
   */
  Evas_Object* GetElmFocusObject();

  /**
   * @copydoc Dali::EvasPlugin::GetAdaptor()
   */
  Dali::Adaptor* GetAdaptor() { return mAdaptor;}

public:

  /**
   * Called when the adaptor is initialised.
   */
  void OnInit();

  /**
   * Called to notify that Dali has started rendering and atleast one frame has been rendered
   */
  void OnFirstRenderCompleted();

  /**
   * Resize the surface, Called when evas_object_image resized
   */
  void Resize();

  /**
   * Move the surface, Called when evas_object_image moved
   */
  void Move();

  /**
   * Called when the rendering surface is resized
   */
  void OnResize();

  /**
   * Render the pixmap
   */
  void Render();

  /**
   * Called when the event dispatched in evas object area.
   * @param[in] point touch point structure
   * @param[in] timeStamp event timestamp, if it is less than 1, this function will generate current time stamp
   */
  void OnTouchEvent(TouchPoint& point, int timeStamp);

  /**
   * Called when the mouse wheel event dispatched  in evas object area.
   * @param[in]  wheelEvent  The mouse wheel event
   */
  void OnMouseWheelEvent( MouseWheelEvent& wheelEvent );

  /**
   * Called when the key event dispatched in evas object area.
   * @param[in]  keyEvent  The key event.
   */
  void OnKeyEvent(KeyEvent& keyEvent);

  /**
   * Called when the accessibility action event dispatched from elm_access.
   * @param[in] actionType elm accessibility action type structure
   * @param[in] x x position for action, it could be unnecessary
   * @param[in] y y position for action, it could be unnecessary
   * @param[in] type mouse event type, it could be unnecessary
   * @return True if the event was handled
   */
  bool OnAccessibilityActionEvent(Elm_Access_Action_Type actionType, Elm_Access_Action_Info* actionInfo, int x = -1, int y = -1);

  /**
   * Called when evqas object gain focus.
   */
  void OnEvasObjectFocusedIn();

  /**
   * Called when evas object lost focus
   */
  void OnEvasObjectFocusedOut();

  /**
   * Called when the source window notifies us the content in clipboard is selected.
   * @param[in] data A data pointer.
   * @param[in] type A type of event.
   * @param[in] event A event information pointer.
   */
  void OnEcoreEventSelectionCleared(void* data, int type, void* event);

  /**
   * Called when the source window sends us about the selected content.
   * For example, when dragged items are dragged INTO our window or when items are selected in the clipboard.
   * @param[in] data A data pointer.
   * @param[in] type A type of event.
   * @param[in] event A event information pointer.
   */
  void OnEcoreEventSelectionNotified(void* data, int type, void* event);

  /**
   * Called when the client messages (i.e. the accessibility events) are received.
   * @param[in] data A data pointer.
   * @param[in] type A type of event.
   * @param[in] event A event information pointer.
   */
  void OnEcoreEventClientMessaged(void* data, int type, void* event);

  /**
   * Called when the result of rendering was posted to on-screen.
   * Adaptor (i.e. render thread) can be waiting this sync to render next frame.
   */
  void RenderSync();

  /**
   * It returns geometry information of evas-object
   * @return geometry information of evas-object
   */
  PositionSize GetEvasObjectGeometry() const { return mEvasImageObjectGeometry; }

  /**
   * Clear ecore idler handler
   * @param[in] deleteHandle false if it does not need to delete the idler handle
   *            (e.g. when idler callback returns ECORE_CALLBACK_CANCEL, handler will be deleted automatically)
   */
  void ClearIdler(bool deleteHandle = true);

  /**
   * @copydoc ConnectionTrackerInterface::SignalConnected
   */
  virtual void SignalConnected( SlotObserver* slotObserver, CallbackBase* callback );

  /**
   * @copydoc ConnectionTrackerInterface::SignalDisconnected
   */
  virtual void SignalDisconnected( SlotObserver* slotObserver, CallbackBase* callback );

  /**
   * @copydoc ConnectionTrackerInterface::GetConnectionCount
   */
  virtual std::size_t GetConnectionCount() const;

public:  // Signals

  /**
   * @copydoc Dali::EvasPlugin::InitSignal()
   */
  EvasPluginSignalV2& InitSignal() { return mInitSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::FirstRenderCompletedSignal()
   */
  EvasPluginSignalV2& FirstRenderCompletedSignal() { return mFirstRenderCompletedSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::TerminateSignal()
   */
  EvasPluginSignalV2& TerminateSignal() { return mTerminateSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::PauseSignal()
   */
  EvasPluginSignalV2& PauseSignal() { return mPauseSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::ResumeSignal()
   */
  EvasPluginSignalV2& ResumeSignal() { return mResumeSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::ResizeSignal()
   */
  EvasPluginSignalV2& ResizeSignal() { return mResizeSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::FocusedSignal()
   */
  EvasPluginSignalV2& FocusedSignal() { return mFocusedSignalV2; }

  /**
   * @copydoc Dali::EvasPlugin::UnFocusedSignal()
   */
  EvasPluginSignalV2& UnFocusedSignal() { return mUnFocusedSignalV2; }

private:

  // Undefined
  EvasPlugin(const EvasPlugin&);
  EvasPlugin& operator=(EvasPlugin&);

private:

  /**
   * Create / Delete the evas image object
   */
  void CreateEvasImageObject(Evas* evas, unsigned int initialWidth, unsigned int initialHeight, bool isTransparent);
  void DeleteEvasImageObject();

  /**
   * Create / Delete the elm access object
   */
  void CreateElmAccessObject(Evas_Object* parent);
  void DeleteElmAccessObject();

  /**
   * Create / Delete the elm focus object
   */
  void CreateElmFocusObject(Evas_Object* parent);
  void DeleteElmFocusObject();

  /**
   * Creates the adaptor
   */
  void CreateAdaptor(unsigned int initialWidth, unsigned int initialHeight);

  /**
   * Creates the render surface
   * @param width of the surface
   * @param height of the surface
   * @return the new surface
   */
  ECore::RenderSurface* CreateSurface( int width, int height );

  /**
   * Resize the surface
   * To resize, create new surface with evas object's size and replace with it.
   */
  void ResizeSurface();

  /**
   * Connect ecore event if the evas plugin has Ecore_X_Window.
   */
  void ConnectEcoreEvent();

  /**
   * Disconnect all connected event.
   */
  void DisconnectEcoreEvent();

  /**
   * For ImfActivated signal
   * When the imf is activated, it will handle the focus
   * @param[in] imfManager imfManager instance
   */
  void OnImfActivated(Dali::ImfManager& imfManager);

public:
  enum State
  {
    Ready,
    Running,
    Suspended,
    Stopped,
  };

  State                                       mState;

public:
  /* for rendering control : these public members will be used in static function */
  Evas_Object*                                mEvasImageObject;
  Evas_Object*                                mElmAccessObject;
  Evas_Object*                                mElmFocusObject;
  ECore::RenderSurface*                       mSurface;
  bool                                        mFirstRenderCompleteNotified;

private:

  EvasPluginSignalV2      mInitSignalV2;
  EvasPluginSignalV2      mFirstRenderCompletedSignalV2;
  EvasPluginSignalV2      mTerminateSignalV2;
  EvasPluginSignalV2      mPauseSignalV2;
  EvasPluginSignalV2      mResumeSignalV2;
  EvasPluginSignalV2      mResizeSignalV2;
  EvasPluginSignalV2      mFocusedSignalV2;
  EvasPluginSignalV2      mUnFocusedSignalV2;

  Dali::EvasPlugin&                           mEvasPlugin;

  Dali::Adaptor*                              mAdaptor;

  Evas*                                       mEvas;
  PositionSize                                mEvasImageObjectGeometry;

  bool                                        mInitialized;
  bool                                        mIsTransparent;
  bool                                        mHasFocus;
  TriggerEvent*                               mRenderNotification; ///< Render Notification trigger

  Ecore_Idler *                               mEvasDirtyIdler; ///< Ecore idler for updating image object when it resumed

  std::vector<Ecore_Event_Handler*>           mEcoreEventHandler; ///< Vector of Ecore_Event_Handler

  Dali::ConnectionTracker                     mConnectionTracker; ///< Used to implement ConnectionTrackerInterface

public:
  inline static EvasPlugin& GetImplementation(Dali::EvasPlugin& evasPlugin) { return *evasPlugin.mImpl; }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_EVAS_PLUGIN_H__
