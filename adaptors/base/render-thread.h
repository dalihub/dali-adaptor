#ifndef __DALI_INTERNAL_RENDER_THREAD_H__
#define __DALI_INTERNAL_RENDER_THREAD_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// EXTERNAL INCLUDES
#include <boost/thread.hpp>

// INTERNAL INCLUDES
#include <base/interfaces/egl-interface.h>
#include <internal/common/render-surface-impl.h> // needed for Dali::Internal::Adaptor::RenderSurface


namespace Dali
{

namespace Integration
{
class GlAbstraction;
class Core;
}

namespace Internal
{
namespace Adaptor
{

class AdaptorInternalServices;
class RenderSurface;
class UpdateRenderSynchronization;
class EglFactoryInterface;
class LogOptions;

/**
 * The render-thread is responsible for calling Core::Render() after each update.
 */
class RenderThread
{
public:
  /**
   * Create the render-thread; this will not do anything until Start() is called.
   * @param[in] sync update-render synchronization object
   * @param[in] adaptorInterfaces base adaptor interface
   * @param[in] logOptions log options

   */
  RenderThread( UpdateRenderSynchronization& sync,
                AdaptorInternalServices& adaptorInterfaces,
                const LogOptions& logOptions );

  /**
   * Virtual Destructor
   */
  virtual ~RenderThread();

public:

  /**
   * Starts the render-thread
   */
  void Start();

  /**
   * Stops the render-thread
   */
  void Stop();

  /**
   * Replaces the rendering surface. This method returns immediately
   * You can call WaitForSurfaceReplaceComplete to block until the
   * replace is completed in render thread. Note, you need to make sure
   * that render thread is actually running!!!
   */
  void ReplaceSurface( RenderSurface* surface );

  /**
   * Blocks until surface replace has been completed
   */
  void WaitForSurfaceReplaceComplete();

  /**
   * Sets the EGL VSync mode synchronisation with the display.
   * @param syncMode to use
   */
  void SetVSyncMode( EglInterface::SyncMode syncMode );

  /**
   * Offscreen was posted to onscreen
   */
  void RenderSync();

private: // Render thread side helpers

  /**
   * This method is used by the Render thread for rendering the Core to the screen.
   * Called from render thread
   * @return true, if the thread finishes properly.
   */
  bool Run();

  /**
   * Initializes EGL.
   * Called from render thread
   */
  void InitializeEgl();

  /**
   * Check if display has events
   * Called from render thread
   */
  void ConsumeEvents();

  /**
   * Check if main thread posted updates
   * Called from render thread
   */
  void CheckForUpdates();

  /**
   * Changes the rendering surface
   * Used for replacing pixmaps due to resizing
   * Called from render thread
   * @param newSurface to use
   */
  void ChangeSurface( RenderSurface* newSurface );

  /**
   * Notify the main thread that surface has really been changed
   */
  void NotifySurfaceChangeCompleted();

  /**
   * Shuts down EGL.
   * Called from render thread
   */
  void ShutdownEgl();

  /**
   * Called before core renders the scene
   * Called from render thread
   * @return true if successful and Core::Render should be called.
   */
  bool PreRender();

  /**
   * Called after core has rendered the scene
   * Called from render thread
   * @param[in] timeDelta Time since PostRender was last called in microseconds
   */
  void PostRender( unsigned int timeDelta );

private: // Data

  UpdateRenderSynchronization&        mUpdateRenderSync; ///< Used to synchronize the update & render threads
  Dali::Integration::Core&            mCore;             ///< Dali core reference
  Integration::GlAbstraction&         mGLES;             ///< GL abstraction rerefence
  EglFactoryInterface*                mEglFactory;       ///< Factory class to create EGL implementation
  EglInterface*                       mEGL;              ///< Interface to EGL implementation

  boost::thread*                      mThread;           ///< render thread
  bool                                mUsingPixmap;      ///< whether we're using a pixmap or a window
  bool                                mSurfaceReplacing; ///< whether the surface is replacing. If true, need to notify surface changing after rendering

  /**
   * Structure to hold values that are set by main thread and read in render thread
   * There is two copies of this data to avoid locking and prevent concurrent access
   */
  struct RenderData
  {
    /**
     * Default constructor to reset values
     */
    RenderData()
    : replaceSurface( false ),
      syncMode( EglInterface::FULL_SYNC ),
      surface( NULL )
    {
    }

    volatile int                replaceSurface; ///< whether the surface needs replacing
    EglInterface::SyncMode      syncMode;       ///< sync mode for EGL
    RenderSurface*              surface;        ///< Current surface
  };

  RenderData                mCurrent;             ///< Current values, must not be used from main thread
  RenderData                mNewValues;           ///< New values, sent from main thread to render thread
  boost::mutex              mThreadDataLock;      ///< mutex to lock values while reading them into render thread
  volatile int              mNewDataAvailable;    ///< atomic flag to notify the render thread that there's new data

  /**
   * Helper class for sending message to render thread
   */
  class SendMessageGuard
  {
  public: // API
    /**
     * Constructor, sets the lock
     */
    SendMessageGuard( RenderThread& parent )
    : mLock( parent.mThreadDataLock ), mFlag( &parent.mNewDataAvailable )
    { // Nothing to do, unique lock will lock automatically and unlock when destructed
    }
    /**
     * Destructor, releases lock and sets flag
     */
    ~SendMessageGuard()
    {
      // set the flag to tell render thread there are new values, ignoring the return value here
      (void)__sync_or_and_fetch( mFlag, 1 );
    }

  private: // Data
    boost::unique_lock< boost::mutex > mLock;
    volatile int* mFlag;
  };

  // sync for waiting for surface change
  boost::mutex              mSurfaceChangedMutex;  ///< mutex to lock during surface replacing
  boost::condition_variable mSurfaceChangedNotify; ///< condition to notify main thread that surface has been changed
  bool                      mSurfaceReplaceCompleted;///< true, while render thread is running and needs to wait for pixmap syncs
  const LogOptions&         mLogOptions;           ///< Log options

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_RENDER_THREAD_H__

