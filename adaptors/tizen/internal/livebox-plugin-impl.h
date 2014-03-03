#ifndef __DALI_INTERNAL_LIVEBOX_H__
#define __DALI_INTERNAL_LIVEBOX_H__

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
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <dali/public-api/math/rect.h>
#include <dali/integration-api/events/touch-event-integ.h>

#include <Ecore_X.h>
#include <Ecore.h>

#include <livebox-client.h>

// INTERNAL INCLUDES
#include <public-api/adaptor-framework/livebox-plugin.h>
#include <internal/framework.h>

namespace Dali
{

class Adaptor;

namespace Internal
{

namespace Adaptor
{

class CommandLineOptions;
class EventLoop;

typedef Dali::Rect<int> PositionSize;

namespace ECoreX
{
class RenderSurface;
}

/**
 * Implementation of the LiveboxPlugin class.
 */
class LiveboxPlugin : public Framework::Observer
{
public:

  typedef Dali::LiveboxPlugin::LiveboxPluginSignalV2 LiveboxPluginSignalV2;

  /**
   * Constructor
   * @param[in]  livebox         The public instance of the LiveboxPlugin
   * @param[in]  argc        A pointer to the number of arguments
   * @param[in]  argv        A pointer to the argument list
   * @param[in]  name        A name of livebox
   * @param[in]  baseLayout  The base layout that the livebox has been written for
   */
  LiveboxPlugin(Dali::LiveboxPlugin& livebox, int* argc, char **argv[], const std::string& name, const DeviceLayout& baseLayout);

  /**
   * Destructor
   */
  virtual ~LiveboxPlugin();

public:

  /**
   * @copydoc Dali::LiveboxPlugin::SetTitle()
   */
  void SetTitle(const std::string& title) {mTitle = title; };

  /**
   * @copydoc Dali::LiveboxPlugin::SetContent()
   */
  void SetContent(const std::string& content) {mContent = content; };

  /**
   * @copydoc Dali::LiveboxPlugin::GetGlanceBarGeometry()
   */
  const PositionSize& GetGlanceBarGeometry() const { return mGlanceGeometry; };

  /**
   * @copydoc Dali::LiveboxPlugin::GetGlanceBarEventInfo()
   */
  const GlanceBarEventInfo& GetGlanceBarEventInfo() const { return mGlanceBarEventInfo; };

  /**
   * @copydoc Dali::LiveboxPlugin::GetLiveboxSizeType()
   */
  LiveboxSizeType GetLiveboxSizeType() const { return mBoxSizeType; };

  /**
   * @copydoc Dali::LiveboxPlugin::Run()
   */
  void Run();

  /**
   * @copydoc Dali::LiveboxPlugin::Quit()
   */
  void Quit();

  /**
   * @copydoc Dali::LiveboxPlugin::AddIdle()
   */
  bool AddIdle(boost::function<void(void)> callBack);

  /**
   * @copydoc Dali::LiveboxPlugin::Get();
   */
  static Dali::LiveboxPlugin& Get();


public: // From Framework::Observer

  /**
   * Called when the framework is initialised.
   */
  virtual void OnInit();

  /**
   * Called when the framework is terminated.
   */
  virtual void OnTerminate();

  /**
   * Called when the framework is paused.
   */
  virtual void OnPause();

  /**
   * Called when the framework resumes from a paused state.
   */
  virtual void OnResume();

  /**
   * Called when the framework informs the livebox that it should reset itself.
   */
  virtual void OnReset();

  /**
   * Called when the framework informs the livebox that the language of the device has changed.
   */
  virtual void OnLanguageChanged();

public:
  /**
   * Client handlers
   */

  /**
   * Notify the livebox was created.
   *
   * @param[in] pkgname package name
   * @param[in] id livebox id string
   * @param[in] content content string of livebox
   * @param[in] width width of livebox
   * @param[in] height height of livebox
   */
  void OnLiveboxCreated(const std::string& content, int width, int height, double period);

  /**
   * Notify the livebox should be destroyed.
   */
  void OnLiveboxDestroyed();

  /**
   * Notify the client should be resized.
   *
   * @param[in] resizedWidth new width of livebox
   * @param[in] resizedHeight new height of livebox
   */
  void OnLiveboxResized(int resizedWidth, int resizedHeight);

  /**
   * Send event to livebox.
   *
   * @param[in] point touch point
   * @param[in] timeStamp time value of event
   */
  void OnLiveboxTouchEvent(TouchPoint& point, unsigned int timeStamp);

  void OnPeriodUpdated(double period);
  void OnUpdateRequested();

  /**
   * Notify the glance was created.
   *
   * @param[in] pdWidth width of pd
   * @param[in] pdHeight height of pd
   * @param[in] arrowX x position of pd's arrow
   * @param[in] arrowY y position of pd's arrow
   */
  void OnGlanceCreated(int width, int height, int arrowX, int arrowY);

  /**
   * Notify the glance should be destroyed.
   */
  void OnGlanceDestroyed();

  void OnGlanceTouchEvent();
  void OnGlanceMoved(int arrowX, int arrowY, int width, int height);
  void OnGlanceScriptEventCallback(std::string emission, std::string source, struct livebox_event_info *info);

  /**
   * Notify the surface was damaged.
   * When it was damager, this client should send update to master
   */
  void OnDamaged();

public:  // Signals

  /**
   * @copydoc Dali::LiveboxPlugin::InitializedSignal()
   */
  LiveboxPluginSignalV2& InitializedSignal() { return mInitializedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::TerminatedSignal()
   */
  LiveboxPluginSignalV2& TerminatedSignal() { return mTerminatedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::PausedSignal()
   */
  LiveboxPluginSignalV2& PausedSignal() { return mPausedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::ResumedSignal()
   */
  LiveboxPluginSignalV2& ResumedSignal() { return mResumedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::ResizedSignal()
   */
  LiveboxPluginSignalV2& ResizedSignal() { return mResizedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::GlanceCreatedSignal()
   */
  LiveboxPluginSignalV2& GlanceCreatedSignal() { return mGlanceCreatedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::GlanceDestroyedSignal()
   */
  LiveboxPluginSignalV2& GlanceDestroyedSignal() { return mGlanceDestroyedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::GlanceTouchedSignal()
   */
  LiveboxPluginSignalV2& GlanceTouchedSignal() { return mGlanceTouchedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::GlanceMovedSignal()
   */
  LiveboxPluginSignalV2& GlanceMovedSignal() { return mGlanceMovedSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::GlanceScriptEventSignal()
   */
  LiveboxPluginSignalV2& GlanceScriptEventSignal() { return mGlanceScriptEventSignalV2; }

  /**
   * @copydoc Dali::LiveboxPlugin::LanguageChangedSignal()
   */
  LiveboxPluginSignalV2& LanguageChangedSignal() { return mLanguageChangedSignalV2; }

private:

  // Undefined
  LiveboxPlugin(const LiveboxPlugin&);
  LiveboxPlugin& operator=(LiveboxPlugin&);

private:

  /**
   * Create pixmap surface for mSurface.
   * If there is no mSurface, it will create mSurface with new display.
   * If mSurface is existed already, then it uses existed display but overwrite mSurface by new one
   * It make a callback for damage notify automatically
   */
  void CreateSurface(Ecore_X_Pixmap pixmap);

  /**
   * Quits from the main loop
   */
  void QuitFromMainLoop();

private:

  LiveboxPluginSignalV2      mInitializedSignalV2;
  LiveboxPluginSignalV2      mTerminatedSignalV2;
  LiveboxPluginSignalV2      mPausedSignalV2;
  LiveboxPluginSignalV2      mResumedSignalV2;
  LiveboxPluginSignalV2      mResizedSignalV2;
  LiveboxPluginSignalV2      mGlanceCreatedSignalV2;
  LiveboxPluginSignalV2      mGlanceDestroyedSignalV2;
  LiveboxPluginSignalV2      mGlanceTouchedSignalV2;
  LiveboxPluginSignalV2      mGlanceMovedSignalV2;
  LiveboxPluginSignalV2      mGlanceScriptEventSignalV2;
  LiveboxPluginSignalV2      mLanguageChangedSignalV2;

  Dali::LiveboxPlugin&                    mLiveboxPlugin;

  Framework*                            mFramework;

  CommandLineOptions*                   mCommandLineOptions;

  Dali::Adaptor*                        mAdaptor;
  std::string                           mName;

  bool                                  mInitialized;
  DeviceLayout                          mBaseLayout;

  // client properties
  livebox_h mLiveboxHandle;
  std::string mTitle;
  std::string mContent;
  double mPeriod;

  PositionSize mGlanceGeometry;
  GlanceBarEventInfo mGlanceBarEventInfo;

  LiveboxSizeType mBoxSizeType;

public:
  /* for rendering control : these public members will be used in static function */
  Ecore_Event_Handler*                        mDamageEventHandler;
  Ecore_X_Damage                              mDamage;
  ECoreX::RenderSurface*                      mSurface;

public:
  inline static LiveboxPlugin& GetImplementation(Dali::LiveboxPlugin& livebox) { return *livebox.mImpl; }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_LIVEBOX_H__

