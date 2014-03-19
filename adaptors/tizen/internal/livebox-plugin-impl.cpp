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

// CLASS HEADER
#include "livebox-plugin-impl.h"

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>

#include <Ecore_X.h>
#include <livebox-service.h>

// INTERNAL HEADERS
#include <internal/command-line-options.h>
#include <internal/common/adaptor-impl.h>
#include <internal/common/ecore-x/ecore-x-render-surface-factory.h>
#include <internal/common/ecore-x/pixmap-render-surface.h>
#include <slp-logging.h>


namespace Dali
{

namespace SlpPlatform
{
class SlpPlatformAbstraction;
}

namespace Integration
{
class Core;
}

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
namespace
{
Integration::Log::Filter* gLiveboxPluginLogFilter  = Integration::Log::Filter::New(Debug::Verbose, false);
} // unnamed namespace

#endif

namespace
{
boost::thread_specific_ptr<LiveboxPlugin> gThreadLocalLivebox;
}


// Static methods

/////////////////////////////////////////////////////////////////////////////////////////////////
// Client Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * LiveboxCreated will be called when the livebox instance was created on viewer
 * It is called once.
 */
static int LiveboxCreated(const char *content_info, int w, int h, double period, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );

  DALI_LOG_INFO(gLiveboxPluginLogFilter, Debug::General, "[%s] buffer %d x %d\n", __FUNCTION__, w, h);

  // error handling for buffer size
  if(w < 1 || h < 1)
  {
    livebox_service_get_size(LB_SIZE_TYPE_1x1, &w, &h); // default size
  }

  liveboxPlugin->OnLiveboxCreated(content_info, w, h, period);

  return 0;
}

static int LiveboxDestroyed(void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnLiveboxDestroyed();

  return 0;

}

static int LiveboxEventCallback(enum livebox_event_type event, double timestamp, double x, double y, void *data)
{
  //3 feed event to adaptor for livebox
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  PositionSize pixmapSize = liveboxPlugin->mSurface->GetPositionSize();

  // calculate pixel value from relative value[0.0~1.0f] of position
  int boxX = pixmapSize.width * x;
  int boxY = pixmapSize.height * y;

  switch (event)
  {
    case LIVEBOX_EVENT_MOUSE_IN:
    {
      TouchPoint point(0, TouchPoint::Motion, boxX, boxY);
      // the timestamp was calculated by ( tv_sec + tv_usec / 1000000 )
      liveboxPlugin->OnLiveboxTouchEvent(point, static_cast<unsigned int>(timestamp * 1000 * 1000));
      break;
    }
    case LIVEBOX_EVENT_MOUSE_OUT:
    {
      TouchPoint point(0, TouchPoint::Leave, boxX, boxY);
      liveboxPlugin->OnLiveboxTouchEvent(point, static_cast<unsigned int>(timestamp * 1000 * 1000));
      break;
    }
    case LIVEBOX_EVENT_MOUSE_DOWN:
    {
      TouchPoint point(0, TouchPoint::Down, boxX, boxY);
      liveboxPlugin->OnLiveboxTouchEvent(point, static_cast<unsigned int>(timestamp * 1000 * 1000));
      break;
    }
    case LIVEBOX_EVENT_MOUSE_MOVE:
    {
      TouchPoint point(0, TouchPoint::Motion, boxX, boxY);
      liveboxPlugin->OnLiveboxTouchEvent(point, static_cast<unsigned int>(timestamp * 1000 * 1000));
      break;
    }
    case LIVEBOX_EVENT_MOUSE_UP:
    {
      TouchPoint point(0, TouchPoint::Up, boxX, boxY);
      liveboxPlugin->OnLiveboxTouchEvent(point, static_cast<unsigned int>(timestamp * 1000 * 1000));
      break;
    }
    case LIVEBOX_EVENT_KEY_DOWN:
    case LIVEBOX_EVENT_KEY_UP:

    default:
      break;
  }

  return 0;
}

static int LiveboxResized(int w, int h, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );

  liveboxPlugin->OnLiveboxResized(w, h);

  return 0;
}

static int LiveboxClientSetPeriod(double period, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnPeriodUpdated(period);

  return 0;
}

static int LiveboxClientUpdated(void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnUpdateRequested();

  return 0;
}

static int LiveboxClientPaused(void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnPause();

  return 0;
}
static int LiveboxClientResumed(void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnResume();

  return 0;
}

static int GlanceCreated(double x, double y, int w, int h, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );

  // calculate pixel value from relative value[0.0~1.0f] of position
  int arrowX = w * x;
  int arrowY = h * y;

  liveboxPlugin->OnGlanceCreated(arrowX, arrowY, w, h);

  return 0;
}

static int GlanceDestroyed(void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnGlanceDestroyed();

  return 0;
}

static int GlanceEventCallback(enum livebox_event_type event, double timestamp, double x, double y, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnGlanceTouchEvent();

  return 0;
}
static int GlanceMoved(double x, double y, int w, int h, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );

  // calculate pixel value from relative value[0.0~1.0f] of position
  int arrowX = w * x;
  int arrowY = h * y;
  liveboxPlugin->OnGlanceMoved(arrowX, arrowY, w, h);

  return 0;
}

static int GlanceScriptEventCalback(const char *emission, const char *source, struct livebox_event_info *info, void *data)
{
  LiveboxPlugin* liveboxPlugin( (LiveboxPlugin*)data );
  liveboxPlugin->OnGlanceScriptEventCallback(emission, source, info);

  return 0;
}

static Eina_Bool LiveboxDamageCallback(void *data, int type, void *event)
{
  LiveboxPlugin* liveboxPlugin = (LiveboxPlugin*)data;
  Ecore_X_Event_Damage *ev = (Ecore_X_Event_Damage *)event;

  // get the EFL type for the surface
  Any surface = liveboxPlugin->mSurface->GetSurface();

  Ecore_X_Pixmap pixmap = AnyCast<Ecore_X_Pixmap>(surface);

  if (!ev || !liveboxPlugin->mDamage)
  {
    return ECORE_CALLBACK_PASS_ON;
  }
  if ((Ecore_X_Drawable)pixmap != ev->drawable)
  {
    return ECORE_CALLBACK_PASS_ON;
  }

  // handle with damage area : we need it to get damage notify continuously
  Ecore_X_Region parts;
  parts = ecore_x_region_new(NULL, 0);
  ecore_x_damage_subtract(liveboxPlugin->mDamage, 0, parts);
  ecore_x_region_free(parts);

  // send update signal to provider
  liveboxPlugin->OnDamaged();

  return EINA_TRUE;
}

LiveboxPlugin::LiveboxPlugin(Dali::LiveboxPlugin& liveboxPlugin, int* argc, char **argv[], const std::string& name, const DeviceLayout& baseLayout)
: mLiveboxPlugin(liveboxPlugin),
  mFramework(NULL),
  mCommandLineOptions(NULL),
  mAdaptor(NULL),
  mName(name),
  mInitialized(false),
  mBaseLayout(baseLayout),
  mLiveboxHandle(NULL),
  mTitle(""),
  mContent(""),
  mPeriod(0.0f),
  mGlanceGeometry(-1, -1, -1, -1),
  mDamageEventHandler(NULL),
  mDamage(0),
  mSurface(NULL)
{
  // make sure we don't create the local thread liveboxPlugin instance twice
  DALI_ASSERT_ALWAYS(gThreadLocalLivebox.get() == NULL && "Cannot create more than one LiveboxPlugin per thread" );

  // reset is used to store a new value associated with this thread
  gThreadLocalLivebox.reset(this);

  // get log settings
  const char* resourceLogOption = std::getenv(DALI_ENV_ENABLE_LOG);
  unsigned int logOpts = Integration::Log::ParseLogOptions(resourceLogOption);

  // livebox plugin thread will send its logs to SLP Platform's LogMessage handler.
  Dali::Integration::Log::InstallLogFunction(Dali::SlpPlatform::LogMessage, logOpts);

  mCommandLineOptions = new CommandLineOptions(*argc, *argv);

  mFramework = new Framework(*this, argc, argv, name);
}

LiveboxPlugin::~LiveboxPlugin()
{
  delete mFramework;
  delete mCommandLineOptions;
  delete mAdaptor;

  // uninstall it on this thread.
  Dali::Integration::Log::UninstallLogFunction();

  gThreadLocalLivebox.release();
}

void LiveboxPlugin::Run()
{
  // Run the liveboxPlugin
  mFramework->Run();
}

void LiveboxPlugin::Quit()
{
  AddIdle(boost::bind(&LiveboxPlugin::QuitFromMainLoop, this));
}

void LiveboxPlugin::QuitFromMainLoop()
{
  if(mAdaptor)
  {
    mAdaptor->Stop();
  }

  mTerminatedSignalV2.Emit( mLiveboxPlugin );
  mTerminatedSignal(mLiveboxPlugin); // deprecated

  mFramework->Quit();

  mInitialized = false;
}


void LiveboxPlugin::OnInit()
{
  mFramework->AddAbortCallback(boost::bind(&LiveboxPlugin::QuitFromMainLoop, this));

  // real initialize process should be done in OnReset()
}


void LiveboxPlugin::CreateSurface(Ecore_X_Pixmap pixmap)
{
  Any display;

  if(mSurface)
  {
    display = mSurface->GetMainDisplay();
  }

  PositionSize pixmapSize;
  int depth = ecore_x_pixmap_depth_get(pixmap);

  ecore_x_pixmap_geometry_get(pixmap, &pixmapSize.x, &pixmapSize.y, &pixmapSize.width, &pixmapSize.height);

  mSurface = ECoreX::CreatePixmapSurface( pixmapSize, pixmap, display, "", depth == 32 ? true : false );

  DALI_LOG_INFO(gLiveboxPluginLogFilter, Debug::General, "[%s] pixmap[%x] %d x %d (%d)\n", __FUNCTION__, pixmap, pixmapSize.width, pixmapSize.height, depth);

  // limit 30 fps
  mSurface->SetRenderMode(Dali::RenderSurface::RENDER_30FPS);

  // callback damage notify : in damanage callback it should call 'livebox_client_update_box'
  if(mDamage)
  {
    ecore_x_damage_free(mDamage);
  }
  else
  {
    // register damage notify callback
    mDamageEventHandler = ecore_event_handler_add(ECORE_X_EVENT_DAMAGE_NOTIFY, LiveboxDamageCallback, this);
  }
  mDamage = ecore_x_damage_new (pixmap, ECORE_X_DAMAGE_REPORT_DELTA_RECTANGLES);
}

void LiveboxPlugin::OnLiveboxCreated(const std::string& content, int width, int height, double period)
{
  DALI_ASSERT_ALWAYS( NULL == mAdaptor && "OnLiveboxCreated must only be called once" );

  Ecore_X_Pixmap pixmap;

  mLiveboxHandle = livebox_client_create(false, width, height);

  if(mLiveboxHandle == NULL)
  {
    // error
    DALI_LOG_WARNING("failed to create livebox handle\n");
    return;
  }

  pixmap = static_cast<Ecore_X_Pixmap>(livebox_client_get_pixmap(mLiveboxHandle));

  if (pixmap == 0)
  {
    // error
    DALI_LOG_WARNING("invalid pixmap\n");
    return;
  }

  // create pixmap surface
  CreateSurface(pixmap);

  mContent = content;
  mPeriod = period;

  // create adaptor
  mAdaptor = new Dali::Adaptor(*mSurface);

  DALI_LOG_INFO(gLiveboxPluginLogFilter, Debug::General, "[%s] pixmap[%x] content(%s)\n", __FUNCTION__, pixmap, content.c_str());

  // adaptor start
  mAdaptor->Start();

  // get size type
  mBoxSizeType = static_cast<LiveboxSizeType>(livebox_service_size_type(width, height));

  // signal init to livebox
  mInitializedSignalV2.Emit( mLiveboxPlugin );
  mInitializedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnLiveboxDestroyed()
{
  livebox_client_destroy(mLiveboxHandle);
  mLiveboxHandle = NULL;

  // disconnect pixmap damage notify
  ecore_event_handler_del(mDamageEventHandler);
  QuitFromMainLoop();
}

void LiveboxPlugin::OnLiveboxResized(int resizedWidth, int resizedHeight)
{
  Ecore_X_Pixmap pixmap;

  livebox_client_destroy(mLiveboxHandle);

  mLiveboxHandle = livebox_client_create(false, resizedWidth, resizedHeight);

  // get new livebox buffer
  pixmap = static_cast<Ecore_X_Pixmap>(livebox_client_get_pixmap(mLiveboxHandle));

  if (pixmap == 0)
  {
    // error
    DALI_LOG_WARNING("invalid pixmap\n");
    return;
  }

  //2 replace surface for adaptor
  // remember old surface
  Dali::RenderSurface* oldSurface = mSurface;

  CreateSurface(pixmap);

  // the surface will be replaced the next time Dali draws
  mAdaptor->ReplaceSurface( *mSurface ); // this method is synchronous
  // its now safe to delete the old surface
  delete oldSurface;

  // get size type
  mBoxSizeType = static_cast<LiveboxSizeType>(livebox_service_size_type(resizedWidth, resizedHeight));

  // Emit resized signal to application
  mResizedSignalV2.Emit( mLiveboxPlugin );
  mResizedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnLiveboxTouchEvent(TouchPoint& point, unsigned int timeStamp)
{
  mAdaptor->FeedTouchPoint( point, timeStamp );
}

void LiveboxPlugin::OnPeriodUpdated(double period)
{
  mPeriod = period;

  // Emit period updated signal to application
  mPeriodUpdatedSignal(mLiveboxPlugin);
}

void LiveboxPlugin::OnUpdateRequested()
{
  // Emit update requested signal to application
  mUpdateRequestedSignal(mLiveboxPlugin);
}

void LiveboxPlugin::OnGlanceCreated(int arrowX, int arrowY, int width, int height)
{
  // store pd information
  mGlanceGeometry = PositionSize(arrowX, arrowY, width, height);
  mGlanceCreatedSignalV2.Emit( mLiveboxPlugin );
  mGlanceCreatedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnGlanceDestroyed()
{
  // make pd information as invalid
  mGlanceGeometry = PositionSize(-1, -1, -1, -1);
  mGlanceDestroyedSignalV2.Emit( mLiveboxPlugin );
  mGlanceDestroyedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnGlanceTouchEvent()
{
  // TODO: handle the touch point

  mGlanceTouchedSignalV2.Emit( mLiveboxPlugin );
  mGlanceTouchedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnGlanceMoved(int arrowX, int arrowY, int width, int height)
{
  // TODO: need to do something here?
  mGlanceGeometry = PositionSize(arrowX, arrowY, width, height);
  mGlanceMovedSignalV2.Emit( mLiveboxPlugin );
  mGlanceMovedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnGlanceScriptEventCallback(std::string emission, std::string source, struct livebox_event_info *info)
{
  mGlanceBarEventInfo.emission = emission;
  mGlanceBarEventInfo.source = source;

  mGlanceBarEventInfo.pointer.x = info->pointer.x;
  mGlanceBarEventInfo.pointer.y = info->pointer.y;
  mGlanceBarEventInfo.pointer.down = info->pointer.down;

  mGlanceBarEventInfo.part.sx = info->part.sx;
  mGlanceBarEventInfo.part.sy = info->part.sy;
  mGlanceBarEventInfo.part.ex = info->part.ex;
  mGlanceBarEventInfo.part.ey = info->part.ey;

  mGlanceScriptEventSignalV2.Emit( mLiveboxPlugin );
  mGlanceScriptEventSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnDamaged()
{
  PositionSize pixmapSize = mSurface->GetPositionSize();

  if(!mTitle.empty())
  {
    livebox_client_update_box(pixmapSize.width, pixmapSize.height, mContent.c_str(), mTitle.c_str());
  }
  else
  {
    livebox_client_update_box(pixmapSize.width, pixmapSize.height, mContent.c_str(), mName.c_str());
  }
}

void LiveboxPlugin::OnTerminate()
{
  // we've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  QuitFromMainLoop();
}

void LiveboxPlugin::OnPause()
{
  mAdaptor->Pause();
  mPausedSignalV2.Emit( mLiveboxPlugin );
  mPausedSignal(mLiveboxPlugin); // deprecated
}

void LiveboxPlugin::OnResume()
{
  mResumedSignalV2.Emit( mLiveboxPlugin );
  mResumedSignal(mLiveboxPlugin); // deprecated
  mAdaptor->Resume();
}

void LiveboxPlugin::OnReset()
{
  // initialize liblivebox-client
  int ret;

  // callback functions which will be called from liblivebox-client
  struct livebox_event_table table;

  /**
   * For Livebox
   */
  table.create = LiveboxCreated; // called once the livebox instance is created
  table.destroy = LiveboxDestroyed; // called when the livebox instance was deleted
  table.event = LiveboxEventCallback;
  table.resize = LiveboxResized; // called whenever the livebox instance is resized

  /**
   * For Glance bar
   */
  table.create_glance = GlanceCreated; // called when pd should be opened
  table.destroy_glance = GlanceDestroyed; // called when pd should be closed
  table.event_glance = GlanceEventCallback;
  table.move_glance = GlanceMoved;
  table.script_event = GlanceScriptEventCalback;

  /**
   * For Client
   */
  table.pause = LiveboxClientPaused; // called when the livebox is invisible
  table.resume = LiveboxClientResumed; // called when the livebox is visible again
  table.update = LiveboxClientUpdated;
  table.set_period = LiveboxClientSetPeriod;

  table.data = this;

  ret = livebox_client_initialize(mFramework->GetBundleId().c_str(), &table);

  DALI_LOG_INFO(gLiveboxPluginLogFilter, Debug::General, "[%s] livebox_client_initialize with bundle id(%s) returns %d\n", __FUNCTION__, mFramework->GetBundleId().c_str(), ret);

  if(ret < 0)
  {
    OnTerminate();
    return;
  }

  mInitialized = true;
}

void LiveboxPlugin::OnLanguageChanged()
{
  mLanguageChangedSignalV2.Emit( mLiveboxPlugin );
  mLanguageChangedSignal(mLiveboxPlugin); // deprecated
}

bool LiveboxPlugin::AddIdle(boost::function<void(void)> callBack)
{
  return mAdaptor->AddIdle(callBack);
}

Dali::LiveboxPlugin& LiveboxPlugin::Get()
{
  DALI_ASSERT_ALWAYS( gThreadLocalLivebox.get() != NULL && "LiveboxPlugin not instantiated" );

  return gThreadLocalLivebox->mLiveboxPlugin;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

