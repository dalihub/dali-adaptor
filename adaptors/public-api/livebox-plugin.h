#ifndef __DALI_LIVEBOX_H__
#define __DALI_LIVEBOX_H__

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

// EXTERNAL INCLUDS
#include <boost/function.hpp>
#include <string>
#include <dali/public-api/math/rect.h>

// INTERNAL INCLUDES
#include "device-layout.h"
#include "style-monitor.h"


namespace Dali DALI_IMPORT_API
{
typedef Dali::Rect<int> PositionSize;

/**
* Livebox's size types
* It refers "livebox-service.h"
*/
enum LiveboxSizeType
{
  LIVEBOX_SIZE_TYPE_1x1 = 0x0001,
  LIVEBOX_SIZE_TYPE_2x1 = 0x0002,
  LIVEBOX_SIZE_TYPE_2x2 = 0x0004,
  LIVEBOX_SIZE_TYPE_4x1 = 0x0008,
  LIVEBOX_SIZE_TYPE_4x2 = 0x0010,
  LIVEBOX_SIZE_TYPE_4x3 = 0x0020,
  LIVEBOX_SIZE_TYPE_4x4 = 0x0040,
  LIVEBOX_SIZE_TYPE_EASY_1x1 = 0x0100,
  LIVEBOX_SIZE_TYPE_EASY_3x1 = 0x0200,
  LIVEBOX_SIZE_TYPE_EASY_3x3 = 0x0400,
  LIVEBOX_SIZE_TYPE_UNKNOWN = 0xFFFF,
};

struct GlanceBarEventInfo
{
  std::string emission;
  std::string source;

  struct
  {
    double x;
    double y;
    int down;
  } pointer; ///< touch information for script

  struct
  {
    double sx;
    double sy;
    double ex;
    double ey;
  } part; ///<part information for script
};

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{

class LiveboxPlugin;
}
}

/**
 * An LiveboxPlugin class object should be created by every livebox
 * that wishes to use Dali.  It provides a means for initialising the
 * resources required by the Dali::Core.
 *
 * The LiveboxPlugin class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * LiveboxPlugin and create the Dali objects in the connected callback.
 *
 * SLP and Linux Liveboxs should follow the example below:
 *
 * @code
 * void CreateProgram(LiveboxPlugin& livebox)
 * {
 *   // Create Dali components...
 *   // Can instantiate here, if required
 * }
 *
 * int main (int argc, char **argv)
 * {
 *   LiveboxPlugin livebox(&argc, &argv);
 *   livebox.InitSignal().Connect(&CreateProgram);
 *   livebox.Run();
 * }
 * @endcode
 *
 * If required, you can also connect class member functions to a signal:
 *
 * @code
 * MyLivebox livebox;
 * livebox.SignalResumed().Connect(&app, &MyLivebox::OnResumed);
 * @endcode
 */
class LiveboxPlugin
{
public:

  typedef SignalV2< void (LiveboxPlugin&) > LiveboxPluginSignalV2;

public:

  /**
   * This is the constructor for Linux & SLP liveboxs.
   * @param  argc        A pointer to the number of arguments
   * @param  argv        A pointer the the argument list
   * @note The default base layout (DeviceLayout::DEFAULT_BASE_LAYOUT) will be used with this constructor.
   */
  LiveboxPlugin( int* argc, char **argv[] );

  /**
   * This is the constructor for Linux & SLP liveboxs with a name
   * @param  argc  A pointer to the number of arguments
   * @param  argv  A pointer the the argument list
   * @param  name  A name of livebox
   * @note The default base layout (DeviceLayout::DEFAULT_BASE_LAYOUT) will be used with this constructor.
   */
  LiveboxPlugin( int* argc, char **argv[], const std::string& name );

  /**
   * This is the constructor for Linux & SLP liveboxs when a layout for the livebox is specified.
   * @param  argc        A pointer to the number of arguments
   * @param  argv        A pointer the the argument list
   * @param  baseLayout  The base layout that the livebox has been written for
   */
  LiveboxPlugin( int* argc, char **argv[], const DeviceLayout& baseLayout );

  /**
   * This is the constructor for Linux & SLP liveboxs with a name and when a layout for the livebox is specified.
   * @param  argc  A pointer to the number of arguments
   * @param  argv  A pointer the the argument list
   * @param  name  A name of livebox
   * @param  baseLayout  The base layout that the livebox has been written for
   */
  LiveboxPlugin( int* argc, char **argv[], const std::string& name, const DeviceLayout& baseLayout );

  /**
   * Virtual destructor
   */
  virtual ~LiveboxPlugin();

public:

  /**
   * Set title string of the livebox
   * @param[in] title title string
   */
  void SetTitle(const std::string& title);

  /**
   * Set content string of the livebox
   * @param[in] content content string
   */
  void SetContent(const std::string& content);

  /**
   * Get glance bar's geometry information
   * x, y mean arrow position
   * w, h mean glance size
   * User can use this method in the glance-created signal handler
   * @return PositionSize structure for glance bar information. {-1, -1, -1, -1} means invalid status for glance
   */
  const PositionSize& GetGlanceBarGeometry() const;

  /**
   * Get glance bar's event information
   * @return GlanceBarEventInfo structure for glance bar event
   */
  const GlanceBarEventInfo& GetGlanceBarEventInfo() const;

  /**
   * Get current size type of livebox
   */
  LiveboxSizeType GetLiveboxSizeType() const;

  /**
   * This starts the livebox providing.
   */
  void Run();

  /**
   * This quits the livebox providing.
   */
  void Quit();

  /**
   * Ensures that the function passed in is called from the main loop when it is idle.
   *
   * A callback of the following type may be used:
   * @code
   *   void MyFunction();
   * @endcode
   *
   * @param[in]  callBack  The function to call.
   * @return true if added successfully, false otherwise
   */
  bool AddIdle(boost::function<void(void)> callBack);

  /**
   * Returns the local thread's instance of the LiveboxPlugin class.
   * @return A reference to the local thread's LiveboxPlugin class instance.
   * @pre The LiveboxPlugin class has been initialised.
   * @note This is only valid in the main thread.
   */
  static LiveboxPlugin& Get();

public:  // Signals

  /**
   * The user should connect to this signal to determine when they should initialise
   * their livebox
   */
  LiveboxPluginSignalV2& InitializedSignal();

  /**
   * The user should connect to this signal to determine when they should terminate
   * their livebox
   */
  LiveboxPluginSignalV2& TerminatedSignal();

  /**
   * The user should connect to this signal if they need to perform any special
   * activities when the livebox is about to be paused.
   */
  LiveboxPluginSignalV2& PausedSignal();

  /**
   * The user should connect to this signal if they need to perform any special
   * activities when the livebox has resumed.
   */
  LiveboxPluginSignalV2& ResumedSignal();

  /**
   * This signal is emitted when the surface the livebox is rendering on is resized.
   */
  LiveboxPluginSignalV2& ResizedSignal();

  /**
   * This signal is emitted when the glance bar popup was created.
   */
  LiveboxPluginSignalV2& GlanceCreatedSignal();

  /**
   * This signal is emitted when the glance bar popup was destroyed.
   */
  LiveboxPluginSignalV2& GlanceDestroyedSignal();

  /**
   * This signal is emitted when the glance bar popup was touched.
   */
  LiveboxPluginSignalV2& GlanceTouchedSignal();

  /**
   * This signal is emitted when the glance bar popup was moved.
   */
  LiveboxPluginSignalV2& GlanceMovedSignal();

  /**
   * This signal is emitted when the glance bar popup was got script event callback.
   * If application registered the edje file for glance bar,
   * this signal will be emitted instead of SignalGlanceTouched.
   * Application can get the event information by using GetGlanceBarEventInfo()
   */
  LiveboxPluginSignalV2& GlanceScriptEventSignal();

  /**
   * This signal is emitted when the language is changed on the device.
   */
  LiveboxPluginSignalV2& LanguageChangedSignal();

private:

  // Undefined
  LiveboxPlugin(const LiveboxPlugin&);
  LiveboxPlugin& operator=(const LiveboxPlugin&);

private:

  Internal::Adaptor::LiveboxPlugin *mImpl;
  friend class Internal::Adaptor::LiveboxPlugin;
};

} // namespace Dali

#endif // __DALI_LIVEBOX_H__

