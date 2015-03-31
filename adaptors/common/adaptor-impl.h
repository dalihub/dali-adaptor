#ifndef __DALI_INTERNAL_ADAPTOR_IMPL_H__
#define __DALI_INTERNAL_ADAPTOR_IMPL_H__

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
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/common/view-mode.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/signals/callback.h>
#include <dali/integration-api/render-controller.h>

// INTERNAL INCLUDES
#include <adaptor.h>
#include <render-surface.h>
#include <tts-player.h>
#include <imf-manager.h>
#include <device-layout.h>
#include <clipboard.h>

#include <tizen-platform-abstraction.h>
#include <base/interfaces/adaptor-internal-services.h>
#include <base/environment-options.h>
#include <base/core-event-interface.h>
#include <drag-and-drop-detector-impl.h>
#include <damage-observer.h>
#include <window-visibility-observer.h>
#include <kernel-trace.h>
#include <trigger-event-factory.h>
#include <networking/socket-factory.h>

namespace Dali
{

namespace Integration
{
class Core;
class GlAbstraction;
}

namespace Internal
{

namespace Adaptor
{
class EventHandler;
class EglFactory;
class GestureManager;
class GlImplementation;
class GlSyncImplementation;
class RenderSurface;
class UpdateRenderController;
class TriggerEvent;
class CallbackManager;
class FeedbackPluginProxy;
class FeedbackController;
class RotationObserver;
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

  /**
   * Creates a New Adaptor
   * @param[in]  surface     A render surface can be one of the following
   *                         - Pixmap, adaptor will use existing Pixmap to draw on to
   *                         - Window, adaptor will use existing Window to draw on to
   * @param[in]  baseLayout  The base layout that the application has been written for
   * @param[in]  configuration The context loss configuration ( to choose resource discard policy )
   */
  static Dali::Adaptor* New( RenderSurface* surface,
                             const DeviceLayout& baseLayout,
                             Dali::Configuration::ContextLoss configuration );

  /**
   * 2-step initialisation, this should be called after creating an adaptor instance.
   */
  void Initialize(Dali::Configuration::ContextLoss configuration);

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
   * @copydoc Dali::EventFeeder::FeedTouchPoint()
   */
  virtual void FeedTouchPoint( TouchPoint& point, int timeStamp );

  /**
   * @copydoc Dali::EventFeeder::FeedWheelEvent()
   */
  virtual void FeedWheelEvent( MouseWheelEvent& wheelEvent );

  /**
   * @copydoc Dali::EventFeeder::FeedKeyEvent()
   */
  virtual void FeedKeyEvent( KeyEvent& keyEvent );

  /**
   * @copydoc AdaptorInterface::MoveResize()
   */
  virtual bool MoveResize( const PositionSize& positionSize );

  /**
   * @copydoc AdaptorInterface::SurfaceResized()
   */
  virtual void SurfaceResized( const PositionSize& positionSize );

  /**
   * @copydoc AdaptorInterface::ReplaceSurface()
   */
  virtual void ReplaceSurface( Dali::RenderSurface& surface );

  /**
   * @copydoc Dali::Adaptor::GetSurface()
   */
  virtual Dali::RenderSurface& GetSurface() const;

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
  virtual bool AddIdle( CallbackBase* callback );

  /**
   * @copydoc Internal::Framework::CallFromMainLoop()
   */
  virtual bool CallFromMainLoop( CallbackBase* callback );

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
   * Overrides DPI.
   * Primarily for host/simulation testing
   * @param[in] hDpi The Horizontal DPI
   * @param[in] vDpi The Vertical DPI
   */
  void SetDpi(size_t hDpi, size_t vDpi);

  /**
   * @return reference to EglFactory class
   */
  EglFactory& GetEGLFactory() const;

  /**
   * Return GlAbstraction.
   * @return the GlAbstraction.
   */
  Integration::GlAbstraction& GetGlAbstraction() const;

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
   * Sets a rotation observer, or set to NULL to remove.
   * @pre Adaptor::Start() has been called ( to create EventHandler )
   * @param[in] observer The observer to listen for window rotation events
   */
  void SetRotationObserver( RotationObserver* observer );

  /**
   * Destroy the TtsPlayer of sepcific mode.
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
   * @copydoc Dali::Adaptor::NotifyLanguageChanged()
   */
  void NotifyLanguageChanged();

public:  //AdaptorInternalServices

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetPlatformAbstractionInterface()
   */
  virtual Dali::Integration::PlatformAbstraction& GetPlatformAbstractionInterface();

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetGlesInterface()
   */
  virtual Dali::Integration::GlAbstraction& GetGlesInterface();

  /**
  * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetEGLFactoryInterface()
  */
  virtual EglFactoryInterface& GetEGLFactoryInterface() const;

  /**
   * @copydoc Dali::Internal::Adaptor::AdaptorInternalServices::GetTriggerEventInterface()
   */
  virtual TriggerEventInterface& GetTriggerEventInterface();

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
  virtual RenderSurface* GetRenderSurfaceInterface();

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
  virtual KernelTraceInterface& GetKernelTraceInterface();

public: // Stereoscopy

  /**
   * @copydoc Dali::Integration::Core::SetViewMode()
   */
  void SetViewMode( ViewMode viewMode );

  /**
   * @copydoc Dali::Integration::Core::GetViewMode()
   */
  ViewMode GetViewMode() const;

  /**
   * @copydoc Dali::Integration::Core::SetStereoBase()
   */
  void SetStereoBase( float stereoBase );

  /**
   * @copydoc Dali::Integration::Core::GetStereoBase()
   */
  float GetStereoBase() const;

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
   * Called by the Dali core when it requires another update
   */
  virtual void RequestUpdate();

  /**
   * Called by Dali core when it requires an notification event being sent on idle.
   * Multi-threading note: this method must be called from the main thread only.
   */
  virtual void RequestProcessEventsOnIdle();

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
  Adaptor(const Adaptor&);
  Adaptor& operator=(Adaptor&);

private:

  /**
   * Helper to parse log options
   */
  void ParseEnvironmentOptions();

  /**
   * Informs core the surface size has changed
   */
  void SurfaceSizeChanged(const PositionSize& positionSize);

  /**
   * Assigns the render surface to the adaptor
   *
   */
  void SetSurface(Dali::RenderSurface *surface);

  /**
   * Sends an notification message from main loop idle handler
   */
  void ProcessCoreEventsFromIdle();

  /**
   * Gets path for data/resource storage.
   * @param[out] path Path for data/resource storage
   */
  void GetDataStoragePath(std::string& path);

private:

  /**
   * Constructor
   * @param[in]  adaptor     The public adaptor
   * @param[in]  surface     A render surface can be one of the following
   *                         - Pixmap, adaptor will use existing Pixmap to draw on to
   *                         - Window, adaptor will use existing Window to draw on to
   * @param[in]  baseLayout  The base layout that the application has been written for
   */
  Adaptor( Dali::Adaptor& adaptor, RenderSurface* surface, const DeviceLayout& baseLayout );

private: // Types

  enum State
  {
    READY,               ///< Initial state before Adaptor::Start is called.
    RUNNING,             ///< Adaptor is running.
    PAUSED,              ///< Adaptor has been paused.
    PAUSED_WHILE_HIDDEN, ///< Adaptor is paused while window is hidden (& cannot be resumed until window is shown).
    STOPPED,             ///< Adaptor has been stopped.
  };

  typedef std::vector<LifeCycleObserver*>  ObserverContainer;

private: // Data

  AdaptorSignalType                       mResizedSignal;             ///< Resized signal.
  AdaptorSignalType                       mLanguageChangedSignal;     ///< Language changed signal.

  Dali::Adaptor&                        mAdaptor;                     ///< Reference to public adaptor instance.
  State                                 mState;                       ///< Current state of the adaptor
  Dali::Integration::Core*              mCore;                        ///< Dali Core
  UpdateRenderController*               mUpdateRenderController;      ///< Controls update/render threads
  VSyncMonitor*                         mVSyncMonitor;                ///< Monitors VSync events
  GlImplementation*                     mGLES;                        ///< GL implementation
  GlSyncImplementation*                 mGlSync;                      ///< GL Sync implementation
  EglFactory*                           mEglFactory;                  ///< EGL Factory

  RenderSurface*                        mSurface;                     ///< Current surface
  TizenPlatform::TizenPlatformAbstraction*  mPlatformAbstraction;         ///< Platform abstraction

  EventHandler*                         mEventHandler;                ///< event handler
  CallbackManager*                      mCallbackManager;             ///< Used to install callbacks
  bool                                  mNotificationOnIdleInstalled; ///< whether the idle handler is installed to send an notification event
  TriggerEvent*                         mNotificationTrigger;         ///< Notification event trigger
  GestureManager*                       mGestureManager;              ///< Gesture manager
  size_t                                mHDpi;                        ///< Override horizontal DPI
  size_t                                mVDpi;                        ///< Override vertical DPI
  FeedbackPluginProxy*                  mDaliFeedbackPlugin;          ///< Used to access feedback support
  FeedbackController*                   mFeedbackController;          ///< Plays feedback effects for Dali-Toolkit UI Controls.
  Dali::TtsPlayer                       mTtsPlayers[Dali::TtsPlayer::MODE_NUM];                   ///< Provides TTS support
  ObserverContainer                     mObservers;                   ///< A list of adaptor observer pointers
  DragAndDropDetectorPtr                mDragAndDropDetector;         ///< The Drag & Drop detector
  RotationObserver*                     mDeferredRotationObserver;    ///< deferred Rotation observer needs event handler
  DeviceLayout                          mBaseLayout;                  ///< The base layout of the application
  EnvironmentOptions                    mEnvironmentOptions;          ///< environment options
  PerformanceInterface*                 mPerformanceInterface;        ///< Performance interface
  KernelTrace                           mKernelTracer;                ///< Kernel tracer
  TriggerEventFactory                   mTriggerEventFactory;         ///< Trigger event factory
  ObjectProfiler*                       mObjectProfiler;              ///< Tracks object lifetime for profiling
  SocketFactory                         mSocketFactory;               ///< Socket factory
public:
  inline static Adaptor& GetImplementation(Dali::Adaptor& adaptor) {return *adaptor.mImpl;}
};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // __DALI_INTERNAL_ADAPTOR_IMPL_H__
