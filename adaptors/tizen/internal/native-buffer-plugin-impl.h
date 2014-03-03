#ifndef __DALI_INTERNAL_NATIVE_BUFFER_PLUGIN_IMPL_H__
#define __DALI_INTERNAL_NATIVE_BUFFER_PLUGIN_IMPL_H__

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
#include <native-buffer-pool.h>
#include <boost/bind.hpp>

#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/vector2.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL INCLUDES

#include <dali/public-api/adaptor-framework/native-buffer-plugin.h>

namespace Dali
{

class Adaptor;

namespace Internal
{

namespace Adaptor
{

class TriggerEvent;

typedef Dali::Rect<int> PositionSize;

namespace ECoreX
{

class RenderSurface;

}

/**
 * Implementation of the NativeBufferPlugin class.
 */
class NativeBufferPlugin
{
public:

  typedef Dali::NativeBufferPlugin::NativeBufferPluginSignalV2 NativeBufferPluginSignalV2;

  /**
   * Constructor
   * @param[in] nbPlugin      The public instance of the NativeBufferPlugin
   * @param[in] nbPool        A pointer to a native_buffer_pool object
   * @param[in] initialWidth  The initial width of the plugin and DALis stage
   * @param[in] initialWidth  The initial height of the plugin and DALis stage
   * @param[in] mode          The rendering mode to decide frame rate
   */
  NativeBufferPlugin( Dali::NativeBufferPlugin& nbPlugin,
                      unsigned int initialWidth,
                      unsigned int initialHeight,
                      bool isTransparent,
                      unsigned int maxBufferCount,
                      RenderSurface::RenderMode mode,
                      const DeviceLayout& baseLayout);

  /**
   * Destructor
   */
  virtual ~NativeBufferPlugin();

public:

  /**
   * @copydoc Dali::NativeBufferPlugin::Run()
   */
  virtual void Run();

  /**
   * @copydoc Dali::NativeBufferPlugin::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc Dali::NativeBufferPlugin::Resume()
   */
  virtual void Resume();

  /**
   * @copydoc Dali::NativeBufferPlugin::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc Dali::NativeBufferPlugin::GetNativeBufferFromOutput()
   */
   native_buffer* GetNativeBufferFromOutput();

  /**
   * @copydoc Dali::NativeBufferPlugin::AddNativeBufferToInput()
   */
   bool AddNativeBufferToInput(native_buffer* nativeBuffer);

  /**
   * @copydoc Dali::NativeBufferPlugin::ChangeSurfaceSize()
   */
   void ChangeSurfaceSize(unsigned int width, unsigned int height);

  /**
   * @copydoc Dali::NativeBufferPlugin::GetBufferCount()
   */
   Vector2 GetBufferCount();

  /**
   * @copydoc Dali::NativeBufferPlugin::GetAdaptor()
   */
  Dali::Adaptor* GetAdaptor()
  {
    return mAdaptor;
  }

public:

  /**
   * Called when the adaptor is initialised.
   */
  void OnInit();

  /**
   * Called when the result of rendering was posted to on-screen.
   * Adaptor (i.e. render thread) can be waiting this sync to render next frame.
   */
  void RenderSync();

  /**
   * Called to notify that Dali has started rendering and atleast one frame has been rendered
   */
  void OnFirstRenderCompleted();

  /**
   * Called to notify that Dali has rendered one frame
   */
  void OnRender();

public:  // Signals

  /**
   * @copydoc Dali::NativeBufferPlugin::InitSignal()
   */
  NativeBufferPluginSignalV2& InitSignal()
  {
    return mInitSignalV2;
  }

  /**
   * @copydoc Dali::NativeBufferPlugin::TerminateSignal()
   */
  NativeBufferPluginSignalV2& TerminateSignal()
  {
    return mTerminateSignalV2;
  }

  /**
   * @copydoc Dali::NativeBufferPlugin::PauseSignal()
   */
  NativeBufferPluginSignalV2& PauseSignal()
  {
    return mPauseSignalV2;
  }

  /**
   * @copydoc Dali::NativeBufferPlugin::ResumeSignal()
   */
  NativeBufferPluginSignalV2& ResumeSignal()
  {
    return mResumeSignalV2;
  }

  /**
   * @copydoc Dali::NativeBufferPlugin::FirstRenderCompletedSignal()
   */
  NativeBufferPluginSignalV2& FirstRenderCompletedSignal()
  {
    return mFirstRenderCompletedSignalV2;
  }

  /**
   * @copydoc Dali::NativeBufferPlugin::RenderSignal()
   */
  NativeBufferPluginSignalV2& RenderSignal()
  {
    return mRenderSignalV2;
  }

private:

  // Undefined copy constructor
  NativeBufferPlugin(const NativeBufferPlugin&);

  // Undefined assignment operator
  NativeBufferPlugin& operator=(NativeBufferPlugin&);

private:

  /**
   * Create the adaptor
   */
  void CreateAdaptor( ECoreX::RenderSurface &surface, const DeviceLayout& baseLayout );

  /**
   * Creates a render surface
   * @param[in] width  Width of the surface
   * @param[in] height Height of the surface
   * @param[in] isTransparent Whether the surface is transparent
   * @return A pointer to the new surface
   */
  ECoreX::RenderSurface* CreateSurface( int width, int height, bool isTransparent, unsigned int maxBufferCount );

public:

  enum State
  {
    Ready,
    Running,
    Suspended,
    Stopped,
  };


private:

  // Signals
  NativeBufferPluginSignalV2 mInitSignalV2;
  NativeBufferPluginSignalV2 mTerminateSignalV2;
  NativeBufferPluginSignalV2 mPauseSignalV2;
  NativeBufferPluginSignalV2 mResumeSignalV2;
  NativeBufferPluginSignalV2 mResetSignalV2;
  NativeBufferPluginSignalV2 mFirstRenderCompletedSignalV2;
  NativeBufferPluginSignalV2 mRenderSignalV2;

  Dali::NativeBufferPlugin& mNativeBufferPlugin;
  native_buffer_provider* mProvider;
  native_buffer_pool*       mPool;
  Dali::Adaptor*            mAdaptor;
  ECoreX::RenderSurface*    mSurface;
  TriggerEvent*             mRenderNotification; ///< Render Notification trigger
  State                     mState;
  bool                      mInitialized;
  bool                      mFirstRenderCompleteNotified;

public:

  inline static NativeBufferPlugin& GetImplementation(Dali::NativeBufferPlugin& nbPlugin)
  {
    return *nbPlugin.mImpl;
  }

}; // class NativeBufferPlugin

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_NATIVE_BUFFER_PLUGIN_IMPL_H__
