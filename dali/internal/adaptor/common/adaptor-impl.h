#ifndef DALI_INTERNAL_ADAPTOR_IMPL_H
#define DALI_INTERNAL_ADAPTOR_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/common/view-mode.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/signals/callback.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/integration-api/render-controller.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor.h>
#include <dali/integration-api/scene.h>
#include <dali/public-api/adaptor-framework/tts-player.h>
#include <dali/devel-api/adaptor-framework/clipboard.h>

#include <dali/internal/graphics/common/graphics-interface.h>

#include <dali/internal/legacy/common/tizen-platform-abstraction.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/system/common/core-event-interface.h>
#include <dali/internal/input/common/drag-and-drop-detector-impl.h>
#include <dali/internal/window-system/common/damage-observer.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>
#include <dali/internal/system/common/kernel-trace.h>
#include <dali/internal/system/common/system-trace.h>
#include <dali/integration-api/trigger-event-factory.h>
#include <dali/internal/network/common/socket-factory.h>


namespace Dali
{

class RenderSurfaceInterface;
class Window;

namespace Integration
{
class Core;
class GlAbstraction;
class Processor;
}

namespace Internal
{

namespace Adaptor
{
class DisplayConnection;
class GraphicsFactory;
class GestureManager;
class GlImplementation;
class GlSyncImplementation;
class ThreadController;
class TriggerEvent;
class CallbackManager;
class FeedbackPluginProxy;
class FeedbackController;
class VSyncMonitor;
class PerformanceInterface;
class LifeCycleObserver;
class ObjectProfiler;

/**
 * Implementation of the Adaptor class.
 */
class Adaptor : public Integration::RenderController,
                public AdaptorInternalServices,
                public CoreEventInterface,
                public DamageObserver,
                public WindowVisibilityObserver
{
public:

  typedef Dali::Adaptor::AdaptorSignalType AdaptorSignalType;

  typedef Uint16Pair SurfaceSize;          ///< Surface size type

  /**
   * Creates a New Adaptor
   * @param[in]  window              The window handle
   * @param[in]  surface             A render surface can be one of the following
   *                                  - Pixmap, adaptor will use existing Pixmap to draw on to
   *                                  - Window, adaptor will use existing Window to draw on to
   * @param[in]  configuration       The context loss configuration ( to choose resource discard policy )
   * @param[in]  environmentOptions  A pointer to the environment options. If NULL then one is created.
   */
  static Dali::Adaptor* New( Dali::Window window,
                             Dali::RenderSurfaceInterface* surface,
                             Dali::Configuration::ContextLoss configuration,
                             EnvironmentOptions* environmentOptions );

  /**
   * Creates a New Adaptor
   * @param[in]  window              The window handle
   * @param[in]  configuration       The context loss configuration ( to choose resource discard policy )
   * @param[in]  environmentOptions  A pointer to the environment options. If NULL then one is created.
   */
  static Dali::Adaptor* New( Dali::Window window,
                             Dali::Configuration::ContextLoss configuration,
                             EnvironmentOptions* environmentOptions );

  /**
   * Creates a New Adaptor
   * @param[in]  graphicsFactory     A factory that creates the graphics interface
   * @param[in]  window              The window handle
   * @param[in]  surface             A render surface can be one of the following
   *                                  - Pixmap, adaptor will use existing Pixmap to draw on to
   *                                  - Window, adaptor will use existing Window to draw on to
   * @param[in]  configuration       The context loss configuration ( to choose resource discard policy )
   * @param[in]  environmentOptions  A pointer to the environment options. If NULL then one is created.
   */
  static Dali::Adaptor* New( GraphicsFactory& graphicsFactory,
                             Dali::Window window,
                             Dali::RenderSurfaceInterface* surface,
                             Dali::Configuration::ContextLoss configuration,
                             EnvironmentOptions* environmentOptions );

  /**
   * Creates a New Adaptor
   * @param[in]  graphicsFactory     A factory that creates the graphics interface
   * @param[in]  window              The window handle
   * @param[in]  configuration       The context loss configuration ( to choose resource discard policy )
   * @param[in]  environmentOptions  A pointer to the environment options. If NULL then one is created.
   */
  static Dali::Adaptor* New( GraphicsFactory& graphicsFactory,
                             Dali::Window window,
                             Dali::Configuration::ContextLoss configuration,
                             EnvironmentOptions* environmentOptions );

  /**
   * 2-step initialisation, this should be called after creating an adaptor instance.
   * @param[in]  graphicsFactory     A factory that creates the graphics interface
   * @param[in]  configuration       The context loss configuration ( to choose resource discard policy )
   */
  void Initialize( GraphicsFactory& graphicsFactory, Dali::Configuration::ContextLoss configuration );

  /**
   * Virtual destructor.
   */
  virtual ~Adaptor();

  /**
   * @copydoc Dali::Adaptor::Get()
   */
  static Dali::Adaptor& Get();

  /**
   * @copydoc Dali::Adaptor::IsAvailable()
   */
  static bool IsAvailable();

  /**
   * @copydoc Dali::Core::SceneCreated();
   */
  void SceneCreated();

public: // AdaptorInternalServices implementation
  /**
   * @copydoc Dali::Adaptor::Start()
   */
  virtual void Start();

  /**
   * @copydoc Dali::Adaptor::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc Dali::Adaptor::Resume()
   */
  virtual void Resume();

  /**
   * @copydoc Dali::Adaptor::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc Dali::Adaptor::ContextLost()
   */
  virtual void ContextLost();

  /**
   * @copydoc Dali::Adaptor::ContextRegained()
   */
  virtual void ContextRegained();

  /**
   * @copydoc Dali::EventFeeder::FeedTouchPoint()
   */
  virtual void FeedTouchPoint( TouchPoint& point, int timeStamp );

  /**
   * @copydoc Dali::EventFeeder::FeedWheelEvent()
   */
  virtual void FeedWheelEvent( WheelEvent& wheelEvent );

  /**
   * @copydoc Dali::EventFeeder::FeedKeyEvent()
   */
  virtual void FeedKeyEvent( KeyEvent& keyEvent );

  /**
   * @copydoc AdaptorInterface::ReplaceSurface()
   */
  virtual void ReplaceSurface( Dali::Window window, Dali::RenderSurfaceInterface& surface );

  /**
   * @copydoc Dali::Adaptor::GetSurface()
   */
  virtual Dali::RenderSurfaceInterface& GetSurface() const;

  /**
   * @copydoc Dali::Adaptor::ReleaseSurfaceLock()
   */
  virtual void ReleaseSurfaceLock();

  /**
   * Retrieve the TtsPlayer.
   * @param[in] mode The mode of TtsPlayer
   * @return A handle to the TtsPlayer.
   */
  virtual Dali::TtsPlayer GetTtsPlayer(Dali::TtsPlayer::Mode mode);

  /**
   * @copydoc Dali::Adaptor::AddIdle()
   */
  virtual bool AddIdle( CallbackBase* callback, bool hasReturnValue, bool forceAdd );

  /**
   * Adds a new Window instance to the Adaptor
   * @param[in]  childWindow The child window instance
   * @param[in]  childWindowName The child window title/name
   * @param[in]  childWindowClassName The class name that the child window belongs to
   * @param[in]  childWindowMode The mode of the child window
   */
  virtual bool AddWindow( Dali::Window* childWindow,
                          const std::string& childWindowName,
                          const std::string& childWindowClassName,
                          const bool& childWindowMode );

  /**
   * Removes an existing Window instance from the Adaptor
   * @param[in]  window The Window instance
   */
  virtual bool RemoveWindow( Dali::Window* childWindow );

  /**
   * Removes an existing Window instance from the Adaptor
   * @param[in]  windowName The Window name
   * @note If two Windows have the same name, the first one that matches will be removed
   */
  virtual bool RemoveWindow( std::string childWindowName );

  /**
   * @copydoc Dali::Adaptor::RemoveIdle()
   */
  virtual void RemoveIdle( CallbackBase* callback );

  /**
   * Sets a pre-render callback.
   */
  void SetPreRenderCallback( CallbackBase* callback );

  /**
   * Removes an existing Window instance from the Adaptor
   * @param[in]  childWindow The Window instance
   */
  bool RemoveWindow( Dali::Internal::Adaptor::Window* childWindow );

public:

  /**
   * @return the Core instance
   */
  virtual Dali::Integration::Core& GetCore();

  /**
   * @copydoc Dali::Adaptor::SetRenderRefreshRate()
   */
  void SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender );

  /**
   * @copydoc Dali::Adaptor::SetUseHardwareVSync()
   */
  void SetUseHardwareVSync(bool useHardware);

  /**
   * Return the PlatformAbstraction.
   * @return The PlatformAbstraction.
   */
  Integration::PlatformAbstraction& GetPlatformAbstraction() const;

  /**
   * Sets the Drag & Drop Listener.
   * @param[in] detector The detector to send Drag & Drop events to.
   */
  void SetDragAndDropDetector( DragAndDropDetectorPtr detector );

  /**
   * Destroy the TtsPlayer of specific mode.
   * @param[in] mode The mode of TtsPlayer to destroy
   */
  void DestroyTtsPlayer(Dali::TtsPlayer::Mode mode);

  /**
   * @brief Sets minimum distance in pixels that the fingers must move towards/away from each other in order to
   * trigger a pinch gesture
   *
   * @param[in] distance The minimum pinch distance in pixels
   */
  void SetMinimumPinchDistance(float distance);

  /**
   * Gets native window handle
   *
   * @return native window handle
   */
  Any GetNativeWindowHandle();

  /**
   * Get the native display associated with the graphics backend
   *
   * @return A handle to the native display
   */
  Any GetGraphicsDisplay();

  /**
   * Sets use remote surface for Surface output
   * @param[in] useRemoteSurface True if the remote surface is used
   */
  void SetUseRemoteSurface(bool useRemoteSurface);

public:

  /**
   * Adds an adaptor observer so that we can observe the adaptor's lifetime events.
   * @param[in]  observer  The observer.
   * @note Observers should remove themselves when they are destroyed.
   */
  void AddObserver( LifeCycleObserver& observer );

  /**
   * Removes the observer from the adaptor.
   * @param[in]  observer  The observer to remove.
   * @note Observers should remove themselves when they are destroyed.
   */
  void RemoveObserver( LifeCycleObserver& observer );

  /**
   * Emits the Notification event to the Dali core.
   */
  void SendNotificationEvent();

  /**
   * Request adaptor to update once
   */
  void RequestUpdateOnce();

  /**
   * @copydoc Dali::Adaptor::NotifySceneCreated()
   */
  void NotifySceneCreated();

  /**
   * @copydoc Dali::Adaptor::NotifyLanguageChanged()
   */
  void NotifyLanguageChanged();

  /**
   * Gets AppId of current application
   */
  void GetAppId( std::string& appId );

  /**
   * Informs core the surface size has changed
   */
  void SurfaceResizePrepare( Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize );

  /**
   * Informs ThreadController the surface size has changed
   */
  void SurfaceResizeComplete( Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize );

  /**
   * Sets layout direction of root by system language
   * @param[in] locale System locale
   */
  void SetRootLayoutDirection( std::string locale );

  /**
   * @copydoc Dali::Adaptor::RenderOnce
   */
  void RenderOnce();

  /**
   * @copydoc Dali::Adaptor::GetLogFactory
   */
  const LogFactoryInterface& GetLogFactory();

  /**
   * @copydoc Dali::Adaptor::RegisterProcessor
   */
  void RegisterProcessor( Integration::Processor& processor );

  /**
   * @coydoc Dali::Adaptor::UnregisterProcessor
   */
  void UnregisterProcessor( Integration::Processor& processor );

public:  //AdaptorInternalServices

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetPlatformAbstractionInterface()
   */
  virtual Dali::Integration::PlatformAbstraction& GetPlatformAbstractionInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetDisplayConnectionInterface()
   */
  virtual Dali::DisplayConnection& GetDisplayConnectionInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetGraphicsInterface()
   */
  virtual GraphicsInterface& GetGraphicsInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetTriggerEventInterface()
   */
  virtual TriggerEventInterface& GetProcessCoreEventsTrigger();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetTriggerEventFactoryInterface()
   */
  virtual TriggerEventFactoryInterface& GetTriggerEventFactoryInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetSocketFactoryInterface()
   */
  virtual SocketFactoryInterface& GetSocketFactoryInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetRenderSurfaceInterface()
   */
  virtual Dali::RenderSurfaceInterface* GetRenderSurfaceInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetVSyncMonitorInterface()
   */
  virtual VSyncMonitorInterface* GetVSyncMonitorInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetPerformanceInterface()
   */
  virtual PerformanceInterface* GetPerformanceInterface();

  /**
   * copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetKernelTraceInterface()
   */
  virtual TraceInterface& GetKernelTraceInterface();

  /**
   * copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetSystemTraceInterface()
   */
  virtual TraceInterface& GetSystemTraceInterface();

public: // Signals

  /**
   * @copydoc Dali::Adaptor::SignalResized
   */
  AdaptorSignalType& ResizedSignal()
  {
    return mResizedSignal;
  }

  /**
   * @copydoc Dali::Adaptor::LanguageChangedSignal
   */
  AdaptorSignalType& LanguageChangedSignal()
  {
    return mLanguageChangedSignal;
  }

  /**
   * Gets the gesture manager.
   * @return The GestureManager
   */
  GestureManager* GetGestureManager() const
  {
    return mGestureManager;
  }

private: // From Dali::Internal::Adaptor::CoreEventInterface

  /**
   * @copydoc Dali::Internal::Adaptor::CoreEventInterface::QueueCoreEvent()
   */
  virtual void QueueCoreEvent(const Dali::Integration::Event& event);

  /**
   * @copydoc Dali::Internal::Adaptor:CoreEventInterface:::ProcessCoreEvents()
   */
  virtual void ProcessCoreEvents();

private: // From Dali::Integration::RenderController

  /**
   * @copydoc Dali::Integration::RenderController::RequestUpdate()
   */
  virtual void RequestUpdate( bool forceUpdate );

  /**
   * @copydoc Dali::Integration::RenderController::RequestProcessEventsOnIdle()
   */
  virtual void RequestProcessEventsOnIdle( bool forceProcess );

private: // From Dali::Internal::Adaptor::WindowVisibilityObserver

  /**
   * Called when the window becomes fully or partially visible.
   */
  virtual void OnWindowShown();

  /**
   * Called when the window is fully hidden.
   */
  virtual void OnWindowHidden();

private: // From Dali::Internal::Adaptor::DamageObserver

  /**
   * @copydoc Dali::Internal::Adaptor::DamageObserver::OnDamaged()
   */
  void OnDamaged( const DamageArea& area );

private:

  // Undefined
  Adaptor(const Adaptor&) = delete;
  Adaptor& operator=(Adaptor&) = delete;

private:

  /**
   * Assigns the render surface to the adaptor
   *
   */
  void SetSurface(Dali::RenderSurfaceInterface *surface);

  /**
   * called after surface is created
   */
  void SurfaceInitialized();

  /**
   * Sends an notification message from main loop idle handler
   */
  bool ProcessCoreEventsFromIdle();

  /**
   * Gets path for data/resource storage.
   * @param[out] path Path for data/resource storage
   */
  void GetDataStoragePath(std::string& path);

  /**
   * Sets up system information if needs
   */
  void SetupSystemInformation();

  /**
   * Adds a callback to be run when entering an idle state.
   *
   * A callback of the following type should be used:
   * @code
   *   bool MyFunction();
   * @endcode
   * This callback will be called repeatedly as long as it returns true. A return of 0 deletes this callback.
   */
  bool AddIdleEnterer( CallbackBase* callback, bool forceAdd );

  /**
   * Removes a previously added the idle enterer callback.
   */
  void RemoveIdleEnterer( CallbackBase* callback );

private:

  /**
   * Constructor
   * @param[in]  window       window handle
   * @param[in]  adaptor      The public adaptor
   * @param[in]  surface      A render surface can be one of the following
   *                          - Pixmap, adaptor will use existing Pixmap to draw on to
   *                          - Window, adaptor will use existing Window to draw on to
   * @param[in]  environmentOptions  A pointer to the environment options. If NULL then one is created.
   */
  Adaptor( Dali::Window window, Dali::Adaptor& adaptor, Dali::RenderSurfaceInterface* surface, EnvironmentOptions* environmentOptions );

private: // Types

  enum State
  {
    READY,               ///< Initial state before Adaptor::Start is called.
    RUNNING,             ///< Adaptor is running.
    PAUSED,              ///< Adaptor has been paused.
    PAUSED_WHILE_HIDDEN, ///< Adaptor is paused while window is hidden (& cannot be resumed until window is shown).
    STOPPED,             ///< Adaptor has been stopped.
  };

  using WindowPtr = IntrusivePtr< Window >;
  using WindowContainer = std::vector<WindowPtr>;
  using ObserverContainer = std::vector<LifeCycleObserver*>;

private: // Data

  AdaptorSignalType                     mResizedSignal;               ///< Resized signal.
  AdaptorSignalType                     mLanguageChangedSignal;       ///< Language changed signal.

  Dali::Adaptor&                        mAdaptor;                     ///< Reference to public adaptor instance.
  State                                 mState;                       ///< Current state of the adaptor
  Dali::Integration::Core*              mCore;                        ///< Dali Core
  ThreadController*                     mThreadController;            ///< Controls the threads
  VSyncMonitor*                         mVSyncMonitor;                ///< Monitors VSync events

  GraphicsInterface*                    mGraphics;                    ///< Graphics interface
  Dali::DisplayConnection*              mDisplayConnection;           ///< Display connection
  WindowContainer                       mWindows;                     ///< A container of all the Windows that are currently created

  TizenPlatform::TizenPlatformAbstraction* mPlatformAbstraction;      ///< Platform abstraction

  CallbackManager*                      mCallbackManager;             ///< Used to install callbacks
  bool                                  mNotificationOnIdleInstalled; ///< whether the idle handler is installed to send an notification event
  TriggerEventInterface*                mNotificationTrigger;         ///< Notification event trigger
  GestureManager*                       mGestureManager;              ///< Gesture manager
  FeedbackPluginProxy*                  mDaliFeedbackPlugin;          ///< Used to access feedback support
  FeedbackController*                   mFeedbackController;          ///< Plays feedback effects for Dali-Toolkit UI Controls.
  Dali::TtsPlayer                       mTtsPlayers[Dali::TtsPlayer::MODE_NUM];                   ///< Provides TTS support
  ObserverContainer                     mObservers;                   ///< A list of adaptor observer pointers
  DragAndDropDetectorPtr                mDragAndDropDetector;         ///< The Drag & Drop detector
  EnvironmentOptions*                   mEnvironmentOptions;          ///< environment options
  PerformanceInterface*                 mPerformanceInterface;        ///< Performance interface
  KernelTrace                           mKernelTracer;                ///< Kernel tracer
  SystemTrace                           mSystemTracer;                ///< System tracer
  TriggerEventFactory                   mTriggerEventFactory;         ///< Trigger event factory
  ObjectProfiler*                       mObjectProfiler;              ///< Tracks object lifetime for profiling
  SocketFactory                         mSocketFactory;               ///< Socket factory
  const bool                            mEnvironmentOptionsOwned:1;   ///< Whether we own the EnvironmentOptions (and thus, need to delete it)
  bool                                  mUseRemoteSurface;            ///< whether the remoteSurface is used or not

public:
  inline static Adaptor& GetImplementation(Dali::Adaptor& adaptor) { return *adaptor.mImpl; }
};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_IMPL_H
