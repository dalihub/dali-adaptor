#ifndef __DALI_NATIVE_BUFFER_PLUGIN_H__
#define __DALI_NATIVE_BUFFER_PLUGIN_H__

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

/**
 * @addtogroup CAPI_DALI_ADAPTOR_MODULE
 * @{
 */

// EXTERNAL INCLUDES
#include <native-buffer-pool.h>
#include <boost/function.hpp>

#include <dali/public-api/signals/dali-signal-v2.h>
#include <dali/public-api/math/vector2.h>
#include "render-surface.h"
#include "device-layout.h"

namespace Dali DALI_IMPORT_API
{

class Adaptor;

namespace Internal DALI_INTERNAL
{

namespace Adaptor
{

class NativeBufferPlugin;

} // namespace Adaptor

} // namespace Internal

/**
 * @brief Used by Tizen applications that wish to capture Dali output in a buffer.
 *
 * A NativeBufferPlugin class object should be created by Tizen application
 * that wish to use Dali, capturing it's output using the Native Buffer Provider API.
 * It provides a means for initialising the resources required by the Dali::Core.
 *
 * The NativeBufferPlugin class emits several signals which the user may
 * connect to.  The user should not create any Dali objects in the main()
 * function and instead should connect to the InitSignal of the
 * NativeBufferPlugin and create DALi objects in the signal handler.
 *
 * Tizen applications should follow the example below:
 *
 * @code
 * void OnConsumeTimerCallback()
 * {
 *   native_buffer* buffer = nbPlugin.GetNativeBufferFromOutput();
 *   if(buffer != NULL)
 *   {
 *     // Consume the buffer
 *   }
 *   // return back the buffer to plugin
 *   AddNativeBufferToInput(buffer);
 * }
 *
 * void Created(NativeBufferPlugin& nbPlugin)
 * {
 *   // Create Dali components...
 *   // Can instantiate here, if required
 *   mTimer = Dali::Timer::New(1000/30); // 30fps
 *   mTimer.TickSignal().Connect(this, &OnConsumeTimerCallback);
 *   mTimer.Start();
 * }
 *
 * void Resized(NativeBufferPlugin& nbPlugin)
 * {
 *   // Set size properties of Dali components
 *   // Set screen layout
 * }
 *
 * int main (int argc, char **argv)
 * {
 *   Dali::NativeBufferPlugin nbPlugin( 640, 480, false, 2, RENDER_30FPS );
 *   nbPlugin.InitSignal().Connect(&Created);
 *   nbPlugin.ResizeSignal().Connect(&Resized);
 *   nbPlugin.Run();
 * }
 * @endcode
 */
class NativeBufferPlugin
{
public:

  typedef SignalV2< void (NativeBufferPlugin&) > NativeBufferPluginSignalV2; ///< Generic native buffer signal type

public:

  /**
   * @brief This is the constructor for Tizen applications.
   * @param[in] initialWidth   The initial width of Dali view port
   * @param[in] initialHeight  The initial height of Dali view port
   * @param[in] isTransparent Whether the surface will be transparent or not
   * @param[in] maxBufferCount The maximum number of buffers to render
   * @param[in] mode The rendering mode to decide frame rate
   * @param[in] baseLayout  The base layout that the application has been written for
   */
  NativeBufferPlugin( unsigned int initialWidth, unsigned int initialHeight, bool isTransparent = false, unsigned int maxBufferCount = 2, RenderSurface::RenderMode mode = RenderSurface::RENDER_60FPS, const DeviceLayout& baseLayout = DeviceLayout::DEFAULT_BASE_LAYOUT);

  /**
   * @brief Virtual destructor.
   */
  virtual ~NativeBufferPlugin();

public:

  /**
   * @brief Run the NativeBufferPlugin.
   */
  void Run();

  /**
   * @brief Pause the NativeBufferPlugin.
   */
  void Pause();

  /**
   * @brief Resume the NativeBufferPlugin.
   */
  void Resume();

  /**
   * @brief Stop the NativeBufferPlugin.
   */
  void Stop();

  /**
   * @brief Get the internal Adaptor instance.
   * @return A pointer to the internal adaptor instance.
   */
   Dali::Adaptor* GetAdaptor();

   /**
    * @brief Get the native buffer object which contain rendered result.
    *
    * Application should return back the buffer object to plugin by using AddNativeBufferToInput().
    * @return A pointer to the native buffer object.
    * @note do not destroy the native-buffer returned from this plugin.
    */
   native_buffer* GetNativeBufferFromOutput();

   /**
    * @brief Add the native buffer object which was consumed in application.
    *
    * The added buffer will be re-used with render target
    * @param nativeBuffer A pointer to the native buffer object.
    * @return True if the operation is successful
    * @pre the nativeBuffer should be got by GetNativeBufferFromOutput()
    */
    bool AddNativeBufferToInput(native_buffer* nativeBuffer);

    /**
     * @brief Get number of native buffers in pool.
     * @return vector2 which has input buffer count and output buffer count
     */
    Vector2 GetBufferCount();

    /**
     * @brief Change surface size.
     *
     * NOT YET SUPPORTED
     * @param width The width of target size
     * @param height The height of target size
     */
    void ChangeSurfaceSize(unsigned int width, unsigned int height);

public:  // Signals

  /**
   * @brief Signal to notify the client when the application is ready to be initialized.
   * @return The signal
   */
  NativeBufferPluginSignalV2& InitSignal();

  /**
   * @brief Signal to notify the user when the application is about to be terminated.
   * @return The signal
   */
  NativeBufferPluginSignalV2& TerminateSignal();

  /**
   * @brief Signal to notify the client when the adaptor is about to be paused.
   *
   * The user should connect to this signal if they need to perform any special
   * activities when the application is about to be paused.
   * @return The signal
   */
  NativeBufferPluginSignalV2& PauseSignal();

  /**
   * @brief Signal to notify the client when the adpator has resumed.
   *
   * The user should connect to this signal if they need to perform any special
   * activities when the application has resumed.
   * @return The signal
   */
  NativeBufferPluginSignalV2& ResumeSignal();

  /**
   * @brief Signal to notify the client when Dali has rendered at least one frame.
   *
   * The user should connect to this signal to be notified when Dali has started rendering
   * and atleast one frame has been rendered.
   * @return The signal
   */
  NativeBufferPluginSignalV2& FirstRenderCompletedSignal();

  /**
   * @brief Signal to notify the client when Dali has rendered one frame
   * @return The signal
   */
  NativeBufferPluginSignalV2& RenderSignal();

private:

  // Undefined copy constructor
  NativeBufferPlugin(const NativeBufferPlugin&);

  // Undefined assignment operator
  NativeBufferPlugin& operator=(NativeBufferPlugin&);

private:

  Internal::Adaptor::NativeBufferPlugin *mImpl; ///< Pointer to implementation
  friend class Internal::Adaptor::NativeBufferPlugin;

}; // class NativeBufferPlugin

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_NATIVE_BUFFER_PLUGIN_H__
