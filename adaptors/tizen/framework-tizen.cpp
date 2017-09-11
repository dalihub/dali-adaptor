/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include "framework.h"

// EXTERNAL INCLUDES
#include <aul.h>
#include <aul_app_com.h>
#include <appcore_ui_base.h>
#include <appcore_multiwindow_base.h>
#include <app_control_internal.h>
#include <app_common.h>
#include <bundle.h>
#include <bundle_internal.h>
#include <Ecore.h>
#include <screen_connector_provider.h>
#include <system_info.h>
#include <string.h>
#include <unistd.h>
#include <vconf.h>
#include <vconf-internal-keys.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

// CONDITIONAL INCLUDES
#ifdef APPCORE_WATCH_AVAILABLE
#include <appcore-watch/watch_app.h>
#endif
#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

#if defined( TIZEN_PLATFORM_CONFIG_SUPPORTED ) && TIZEN_PLATFORM_CONFIG_SUPPORTED
#include <tzplatform_config.h>
#endif // TIZEN_PLATFORM_CONFIG_SUPPORTED

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <callback-manager.h>
#include <window.h>
#include <widget-impl.h>
#include <widget-data.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New( Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS" );
#endif

// TODO: remove these global variables
static bool gForegroundState;
static char* gAppId;
static char* gPackageId;
static char* gViewerEndpoint;

} // anonymous namespace

namespace AppCore
{

typedef enum
{
  LOW_MEMORY,                 //< The low memory event
  LOW_BATTERY,                //< The low battery event
  LANGUAGE_CHANGED,           //< The system language changed event
  DEVICE_ORIENTATION_CHANGED, //< The device orientation changed event
  REGION_FORMAT_CHANGED,      //< The region format changed event
  SUSPENDED_STATE_CHANGED,    //< The suspended state changed event of the application
  UPDATE_REQUESTED,           //< The update requested event. This event can occur when an app needs to be updated. It is dependent on target devices.
} AppEventType;

static int AppEventConverter[APPCORE_BASE_EVENT_MAX] =
{
  [LOW_MEMORY] = APPCORE_BASE_EVENT_LOW_MEMORY,
  [LOW_BATTERY] = APPCORE_BASE_EVENT_LOW_BATTERY,
  [LANGUAGE_CHANGED] = APPCORE_BASE_EVENT_LANG_CHANGE,
  [DEVICE_ORIENTATION_CHANGED] = APPCORE_BASE_EVENT_DEVICE_ORIENTATION_CHANGED,
  [REGION_FORMAT_CHANGED] = APPCORE_BASE_EVENT_REGION_CHANGE,
  [SUSPENDED_STATE_CHANGED] = APPCORE_BASE_EVENT_SUSPENDED_STATE_CHANGE,
};

struct AppEventInfo
{
  AppEventType type;
  void *value;
};

typedef struct AppEventInfo *AppEventInfoPtr;

typedef void (*AppEventCallback)(AppEventInfoPtr eventInfo, void *userData);

struct AppEventHandler
{
  AppEventType type;
  AppEventCallback cb;
  void *data;
  void *raw;
};

typedef struct AppEventHandler *AppEventHandlerPtr;

int EventCallback(void *event, void *data)
{
  AppEventHandlerPtr handler = static_cast<AppEventHandlerPtr>(data);

  struct AppEventInfo appEvent;

  appEvent.type = handler->type;
  appEvent.value = event;

  if (handler->cb)
    handler->cb(&appEvent, handler->data);

  return 0;
}

int AppAddEventHandler(AppEventHandlerPtr *eventHandler, AppEventType eventType, AppEventCallback callback, void *userData)
{
  AppEventHandlerPtr handler;

  handler = static_cast<AppEventHandlerPtr>( calloc(1, sizeof(struct AppEventHandler)) );
  if (!handler)
  {
    DALI_LOG_ERROR( "failed to create handler" );
    return TIZEN_ERROR_UNKNOWN;
  }
  else
  {
    handler->type = eventType;
    handler->cb = callback;
    handler->data = userData;
    handler->raw = appcore_base_add_event( static_cast<appcore_base_event>(AppEventConverter[static_cast<int>(eventType)]), EventCallback, handler);

    *eventHandler = handler;

    return TIZEN_ERROR_NONE;
  }
}

} // namespace Appcore

/**
 * Impl to hide EFL data members
 */
struct Framework::Impl
{
// Constructor
  Impl(void* data, Type type )
  : mAbortCallBack( NULL ),
    mCallbackManager( NULL )
#ifdef APPCORE_WATCH_AVAILABLE
    , mWatchCallback()
#endif
  {
    mFramework = static_cast<Framework*>(data);
    gForegroundState = false;

#ifndef APPCORE_WATCH_AVAILABLE
    if ( type == WATCH )
    {
      throw Dali::DaliException( "", "Watch Application is not supported." );
    }
#endif
    mApplicationType = type;
    mCallbackManager = CallbackManager::New();
  }

  ~Impl()
  {
    delete mAbortCallBack;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;
  }

  int AppMain()
  {
    int ret;

    if (mApplicationType == NORMAL)
    {
      ret = AppNormalMain();
    }
    else if(mApplicationType == WIDGET)
    {
      ret = AppWidgetMain();
    }
    else
    {
      ret = AppWatchMain();
    }
    return ret;
  }

  void AppExit()
  {
    if (mApplicationType == NORMAL)
    {
      AppNormalExit();
    }
    else if(mApplicationType == WIDGET)
    {
      AppWidgetExit();
    }
    else
    {
      AppWatchExit();
    }
  }


  // Data
  Type mApplicationType;
  CallbackBase* mAbortCallBack;
  CallbackManager *mCallbackManager;

  Framework* mFramework;
  AppCore::AppEventHandlerPtr handlers[5];

#ifdef APPCORE_WATCH_AVAILABLE
  watch_app_lifecycle_callback_s mWatchCallback;
  app_event_handler_h watchHandlers[5];
#endif

  static int AppCreate(void *data)
  {
    appcore_ui_base_on_create();
    return static_cast<int>( static_cast<Framework*>(data)->Create() );
  }

  static int AppTerminate(void *data)
  {
    appcore_ui_base_on_terminate();
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnTerminate();

    return 0;
  }

  static int AppPause(void *data)
  {
    appcore_ui_base_on_pause();
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnPause();

    return 0;
  }

  static int AppResume(void *data)
  {
    appcore_ui_base_on_resume();
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnResume();

    return 0;
  }

  static void ProcessBundle(Framework* framework, bundle *bundleData)
  {
    if(bundleData == NULL)
    {
      return;
    }

    // get bundle name
    char* bundleName = const_cast<char*>(bundle_get_val(bundleData, "name"));
    if(bundleName != NULL)
    {
      framework->SetBundleName(bundleName);
    }

    // get bundle? id
    char* bundleId = const_cast<char*>(bundle_get_val(bundleData, "id"));
    if(bundleId != NULL)
    {
      framework->SetBundleId(bundleId);
    }
  }

  /**
   * Called by AppCore when the application is launched from another module (e.g. homescreen).
   * @param[in] b the bundle data which the launcher module sent
   */
  static int AppControl(bundle* bundleData, void *data)
  {
    app_control_h appControl = NULL;

    appcore_ui_base_on_control(bundleData);

    if (bundleData)
    {
      if (app_control_create_event(bundleData, &appControl) != TIZEN_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to create an app_control handle");
      }
    }
    else
    {
      if (app_control_create(&appControl) != TIZEN_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to create an app_control handle");
      }
    }

    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;

    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(appControl);

    app_control_destroy(appControl);

    return 0;
  }

  static int AppInit(int argc, char **argv, void *data)
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

    ecore_init();
    ecore_app_args_set( argc, (const char **)argv );

#pragma GCC diagnostic pop
    return 0;
  }

  static void AppFinish(void)
  {
    ecore_shutdown();

    if(getenv("AUL_LOADER_INIT"))
    {
      unsetenv("AUL_LOADER_INIT");
      ecore_shutdown();
    }
  }

  static void AppRun(void *data)
  {
    ecore_main_loop_begin();
  }

  static void AppExit(void *data)
  {
    ecore_main_loop_quit();
  }

  static void AppLanguageChanged(AppCore::AppEventInfoPtr event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnLanguageChanged();
  }

  static void AppDeviceRotated(AppCore::AppEventInfoPtr event_info, void *data)
  {
  }

  static void AppRegionChanged(AppCore::AppEventInfoPtr event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnRegionChanged();
  }

  static void AppBatteryLow(AppCore::AppEventInfoPtr event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnBatteryLow();
  }

  static void AppMemoryLow(AppCore::AppEventInfoPtr event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnMemoryLow();
  }


  int AppNormalMain()
  {
    int ret;

    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_BATTERY], AppCore::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_MEMORY], AppCore::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::DEVICE_ORIENTATION_CHANGED], AppCore::DEVICE_ORIENTATION_CHANGED, AppDeviceRotated, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LANGUAGE_CHANGED], AppCore::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::REGION_FORMAT_CHANGED], AppCore::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    appcore_ui_base_ops ops = appcore_ui_base_get_default_ops();

    /* override methods */
    ops.base.create = AppCreate;
    ops.base.control = AppControl;
    ops.base.terminate = AppTerminate;
    ops.pause = AppPause;
    ops.resume = AppResume;
    ops.base.init = AppInit;
    ops.base.finish = AppFinish;
    ops.base.run = AppRun;
    ops.base.exit = AppExit;

    ret = appcore_ui_base_init(ops, *mFramework->mArgc, *mFramework->mArgv, mFramework, APPCORE_UI_BASE_HINT_WINDOW_GROUP_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_WINDOW_STACK_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_BG_LAUNCH_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_HW_ACC_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_WINDOW_AUTO_CONTROL );

    if (ret != TIZEN_ERROR_NONE)
      return ret;

    appcore_ui_base_fini();

    return TIZEN_ERROR_NONE;
  }

  void AppNormalExit()
  {
    appcore_ui_base_exit();
  }

  void AppWidgetExit()
  {
    if( !IsWidgetFeatureEnabled() )
    {
      DALI_LOG_ERROR("widget feature is not supported");
      return;
    }

    appcore_multiwindow_base_exit();
    aul_widget_notify_exit();
  }

  int AppWidgetMain()
  {
    if( !IsWidgetFeatureEnabled() )
    {
      DALI_LOG_ERROR("widget feature is not supported");
      return 0;
    }

    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_BATTERY], AppCore::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_MEMORY], AppCore::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::DEVICE_ORIENTATION_CHANGED], AppCore::DEVICE_ORIENTATION_CHANGED, AppDeviceRotated, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LANGUAGE_CHANGED], AppCore::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::REGION_FORMAT_CHANGED], AppCore::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    appcore_multiwindow_base_ops ops = appcore_multiwindow_base_get_default_ops();

    /* override methods */
    ops.base.create = WidgetAppCreate;
    ops.base.control = WidgetAppControl;
    ops.base.terminate = WidgetAppTerminate;
    ops.base.receive = WidgetAppReceive;
    ops.base.init = AppInit;
    ops.base.finish = AppFinish;
    ops.base.run = AppRun;
    ops.base.exit = AppExit;

    bundle *bundleFromArgv = bundle_import_from_argv(*mFramework->mArgc, *mFramework->mArgv);

    char* viewerEndpoint = NULL;

    if (bundleFromArgv)
    {
      bundle_get_str(bundleFromArgv, "__WIDGET_ENDPOINT__", &viewerEndpoint);
      if (viewerEndpoint)
      {
        gViewerEndpoint = strdup(viewerEndpoint);
      }
      else
      {
        DALI_LOG_ERROR("endpoint is missing");
        return 0;
      }

      bundle_free(bundleFromArgv);
    }
    else
    {
      DALI_LOG_ERROR("failed to get launch argv");
      return 0;
    }

    appcore_multiwindow_base_init(ops, *mFramework->mArgc, *mFramework->mArgv, mFramework);
    appcore_multiwindow_base_fini();
    return TIZEN_ERROR_NONE;
  }

  static void WidgetAppPoweroff(keynode_t *key, void *data)
  {
    int val;

    val = vconf_keynode_get_int(key);
    switch (val) {
      case VCONFKEY_SYSMAN_POWER_OFF_DIRECT:
      case VCONFKEY_SYSMAN_POWER_OFF_RESTART:
      {
        static_cast<Internal::Adaptor::Framework::Impl*>(data)->AppWidgetExit();
        break;
      }
      case VCONFKEY_SYSMAN_POWER_OFF_NONE:
      case VCONFKEY_SYSMAN_POWER_OFF_POPUP:
      default:
        break;
    }
  }

  static int WidgetAppCreate(void *data)
  {
    char pkgid[256] = {0, };

    appcore_multiwindow_base_on_create();
    app_get_id(&gAppId);

    if(aul_app_get_pkgid_bypid(getpid(), pkgid, sizeof(pkgid)) == 0)
    {
      gPackageId = strdup(pkgid);
    }

    if(!gPackageId || !gAppId)
    {
      DALI_LOG_ERROR("package_id is NULL");
      return -1;
    }

    screen_connector_provider_init();
    vconf_notify_key_changed(VCONFKEY_SYSMAN_POWER_OFF_STATUS, WidgetAppPoweroff, data);

    return static_cast<int>( static_cast<Framework*>(data)->Create() );
  }

  static int WidgetAppTerminate(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnTerminate();

    vconf_ignore_key_changed(VCONFKEY_SYSMAN_POWER_OFF_STATUS, WidgetAppPoweroff);
    screen_connector_provider_fini();

    appcore_multiwindow_base_on_terminate();
    return 0;
  }

  static void WidgetAppInstResume(const char* classId, const char* id, appcore_multiwindow_base_instance_h context, void* data)
  {
    WidgetInstanceResume(classId, id, static_cast<bundle*>(data));
  }

  static void WidgetInstanceResume(const char* classId, const char* id, bundle* bundleData)
  {
    appcore_multiwindow_base_instance_h context;

    context = appcore_multiwindow_base_instance_find(id);

    if(!context)
    {
      DALI_LOG_ERROR("context not found: %s", id);
      return;
    }

    appcore_multiwindow_base_instance_resume(context);

    SendUpdateStatus(classId, id, AUL_WIDGET_INSTANCE_EVENT_RESUME, NULL);

    if(!gForegroundState)
    {
      aul_send_app_status_change_signal( getpid(), gAppId, gPackageId, "fg", "widgetapp" );
      gForegroundState = true;
    }

    return;
  }

  static int SendUpdateStatus(const char* classId, const char* instanceId, int status, bundle* extra )
  {
    bundle* bundleData;
    int lifecycle = -1;
    bundle_raw *raw = NULL;
    int length;

    bundleData = bundle_create();
    if(!bundleData)
    {
      DALI_LOG_ERROR("out of memory");
      return -1;
    }

    bundle_add_str(bundleData, AUL_K_WIDGET_ID, classId);
    bundle_add_str(bundleData, AUL_K_WIDGET_INSTANCE_ID, instanceId);
    bundle_add_byte(bundleData, AUL_K_WIDGET_STATUS, &status, sizeof(int));

    if(extra)
    {
      bundle_encode(extra, &raw, &length);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
      bundle_add_str(bundleData, "__WIDGET_CONTENT_INFO__", (const char*)raw);
#pragma GCC diagnostic pop

      aul_widget_instance_add(classId, instanceId);
    }

    aul_app_com_send(gViewerEndpoint, bundleData);

    switch(status)
    {
      case AUL_WIDGET_INSTANCE_EVENT_CREATE:
        lifecycle = Dali::Widget::WidgetLifecycleEventType::CREATE;
        break;
      case AUL_WIDGET_INSTANCE_EVENT_DESTROY:
        lifecycle = Dali::Widget::WidgetLifecycleEventType::DESTROY;
        break;
      case AUL_WIDGET_INSTANCE_EVENT_PAUSE:
        lifecycle = Dali::Widget::WidgetLifecycleEventType::PAUSE;
        break;
      case AUL_WIDGET_INSTANCE_EVENT_RESUME:
        lifecycle = Dali::Widget::WidgetLifecycleEventType::RESUME;
        break;
    }

    if (lifecycle > -1)
      SendLifecycleEvent(classId, instanceId, lifecycle);

    bundle_free(bundleData);
    if (raw)
      free(raw);

    return 0;
  }

  static int SendLifecycleEvent(const char* classId, const char* instanceId, int status)
  {
    bundle* bundleData = bundle_create();
    int ret;

    if (bundleData == NULL)
    {
      DALI_LOG_ERROR("out of memory");
      return -1;
    }

    bundle_add_str(bundleData, AUL_K_WIDGET_ID, classId);
    bundle_add_str(bundleData, AUL_K_WIDGET_INSTANCE_ID, instanceId);
    bundle_add_byte(bundleData, AUL_K_WIDGET_STATUS, &status, sizeof(int));
    bundle_add_str(bundleData, AUL_K_PKGID, gPackageId);

    ret = aul_app_com_send("widget.status", bundleData);

    if (ret < 0)
      DALI_LOG_ERROR("send lifecycle error:%d", ret);

    bundle_free(bundleData);

    return ret;
  }

  static int WidgetAppReceive(aul_type type, bundle *bundleData, void *data)
  {
    appcore_multiwindow_base_on_receive(type, bundleData);

    switch(type)
    {
      case AUL_RESUME:
      {
        appcore_multiwindow_base_instance_foreach_full(WidgetAppInstResume, bundleData);
        break;
      }
      case AUL_TERMINATE:
      {
        static_cast<Internal::Adaptor::Framework::Impl*>(data)->AppWidgetExit();
        break;
      }
      case AUL_WIDGET_CONTENT:
      {
        GetContent(bundleData);
        break;
      }
      default:
        break;
    }
    return 0;
  }

  static void GetContent( bundle* bundleData )
  {
    char* instanceId = NULL;
    appcore_multiwindow_base_instance_h context;
    const appcore_multiwindow_base_class *cls;
    Internal::Adaptor::Widget *widgetInstance;

    bundle_get_str(bundleData, AUL_K_WIDGET_INSTANCE_ID, &instanceId);
    if(!instanceId)
    {
      DALI_LOG_ERROR("instance id is NULL");
      return;
    }

    context = static_cast<appcore_multiwindow_base_instance_h>(appcore_multiwindow_base_instance_find(instanceId));
    if(!context)
    {
      DALI_LOG_ERROR("could not find widget obj: %s", instanceId);
      return;
    }

    cls = appcore_multiwindow_base_instance_get_class(context);
    if(!cls)
    {
      DALI_LOG_ERROR("widget class is NULL");
      return;
    }

    widgetInstance = static_cast<Internal::Adaptor::Widget*>(cls->data);
    if(!widgetInstance)
    {
      DALI_LOG_ERROR("widget instance is NULL");
      return;
    }

    Dali::WidgetData *widgetData = widgetInstance->FindWidgetData( instanceId );
    if(!widgetData)
    {
      DALI_LOG_ERROR("widget extra is NULL");
      return;
    }

    char* widgetContent = widgetData->GetContent();
    if(widgetContent)
    {
      bundle_add_str(bundleData, AUL_K_WIDGET_CONTENT_INFO, widgetContent);
    }
    else
    {
      bundle_add_str(bundleData, AUL_K_WIDGET_CONTENT_INFO, "");
    }
  }

  /**
   * Called by AppCore when the application is launched from another module (e.g. homescreen).
   * @param[in] b the bundle data which the launcher module sent
   */
  static int WidgetAppControl(bundle* bundleData, void *data)
  {
    char *classId = NULL;
    char *id = NULL;
    char *operation = NULL;

    appcore_multiwindow_base_on_control(bundleData);

    bundle_get_str(bundleData, AUL_K_WIDGET_ID, &classId);
    bundle_get_str(bundleData, AUL_K_WIDGET_INSTANCE_ID, &id);
    bundle_get_str(bundleData, "__WIDGET_OP__", &operation);

    if(!operation)
    {
      DALI_LOG_ERROR("operation is NULL");
      return 0;
    }

    if(strcmp(operation, "create") == 0)
    {
      InstanceCreate( classId, id, bundleData );
    }
    else if (strcmp(operation, "resize") == 0)
    {
      InstanceResize( classId, id, bundleData );
    }
    else if (strcmp(operation, "update") == 0)
    {
      InstanceUpdate( classId, id, bundleData );
    }
    else if (strcmp(operation, "destroy") == 0)
    {
      InstanceDestroy( classId, id, bundleData, data );
    }
    else if (strcmp(operation, "resume") == 0)
    {
      InstanceResume( classId, id, bundleData );
    }
    else if (strcmp(operation, "pause") == 0)
    {
      InstancePause( classId, id, bundleData );
    }
    else if (strcmp(operation, "terminate") == 0)
    {
      InstanceDestroy( classId, id, bundleData, data );
    }

    return 0;
  }

  static void InstanceCreate(const char* classId, const char* id, bundle* bundleData)
  {
    appcore_multiwindow_base_instance_run(classId, id, bundle_dup(bundleData));
  }

  static void InstanceResize(const char *classId, const char *id, bundle *bundleData)
  {
    appcore_multiwindow_base_instance_h context;
    Internal::Adaptor::Widget *widgetInstance;
    const appcore_multiwindow_base_class *cls;
    char *remain = NULL;
    char *widthStr = NULL;
    char *heightStr = NULL;
    uint32_t width = 0;
    uint32_t height = 0;

    context = appcore_multiwindow_base_instance_find(id);

    if(!context)
    {
      DALI_LOG_ERROR("context not found: %s", id);
      return;
    }

    cls = appcore_multiwindow_base_instance_get_class(context);
    if(!cls)
    {
      DALI_LOG_ERROR("widget class is NULL");
      return;
    }

    widgetInstance = static_cast<Internal::Adaptor::Widget*>(cls->data);
    if(!widgetInstance)
    {
      DALI_LOG_ERROR("widget instance is NULL");
      return;
    }

    bundle_get_str(bundleData, "__WIDGET_WIDTH__", &widthStr);
    bundle_get_str(bundleData, "__WIDGET_HEIGHT__", &heightStr);

    if(widthStr)
      width = static_cast<uint32_t>(g_ascii_strtoll(widthStr, &remain, 10));

    if(heightStr)
      height = static_cast<uint32_t>(g_ascii_strtoll(heightStr, &remain, 10));

    widgetInstance->OnResize( context, Dali::Widget::WindowSize(width,height) );
  }

  static void InstanceUpdate(const char* classId, const char* id, bundle* bundleData)
  {
    appcore_multiwindow_base_instance_h context;

    if(!id)
    {
      appcore_multiwindow_base_instance_foreach(classId, UpdateCallback, bundleData);
      return;
    }

    context = appcore_multiwindow_base_instance_find(id);

    if(!context)
    {
      DALI_LOG_ERROR("context not found: %s", id);
      return;
    }

    UpdateCallback(classId, id, context, bundleData);
  }

  static void UpdateCallback(const char* classId, const char* id, appcore_multiwindow_base_instance_h context, void* data)
  {
    Internal::Adaptor::Widget *widgetInstance;
    const appcore_multiwindow_base_class *cls;
    bundle* content = NULL;
    char* contentRaw = NULL;
    char* forceStr = NULL;
    int force;
    bundle* bundleData = static_cast<bundle*>(data);

    if(!bundleData)
    {
      DALI_LOG_ERROR("bundle is NULL");
      return;
    }

    cls = appcore_multiwindow_base_instance_get_class(context);
    if(!cls)
    {
      DALI_LOG_ERROR("class is NULL");
      return;
    }

    widgetInstance = static_cast<Internal::Adaptor::Widget*>(cls->data);
    if(!widgetInstance)
    {
      DALI_LOG_ERROR("widget instance is NULL");
      return;
    }

    bundle_get_str(bundleData, "__WIDGET_FORCE__", &forceStr);

    if(forceStr && strcmp(forceStr, "true") == 0)
    {
      force = 1;
    }
    else
    {
      force = 0;
    }

    bundle_get_str(bundleData, "__WIDGET_CONTENT_INFO__", &contentRaw);

    if(contentRaw)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
      content = bundle_decode((const bundle_raw *)contentRaw, strlen(contentRaw));
#pragma GCC diagnostic pop

    }

    widgetInstance->OnUpdate(context, content, force);

    if(content)
    {
      bundle_free(content);
    }
  }

  static void InstanceDestroy(const char* classId, const char* id, bundle* bundleData, void* data)
  {
    appcore_multiwindow_base_instance_h context;

    Internal::Adaptor::Widget *widgetInstance;
    const appcore_multiwindow_base_class *cls;

    context = appcore_multiwindow_base_instance_find(id);

    if(!context)
    {
      DALI_LOG_ERROR("could not find widget obj: %s, clear amd info", id);
      aul_widget_instance_del(classId, id);
      return;
    }

    cls = appcore_multiwindow_base_instance_get_class(context);
    if(!cls)
    {
      DALI_LOG_ERROR("widget class is NULL");
      return;
    }

    widgetInstance = static_cast<Internal::Adaptor::Widget*>(cls->data);
    if(!widgetInstance)
    {
      DALI_LOG_ERROR("widget instance is NULL");
      return;
    }

    Dali::WidgetData *widgetData  = widgetInstance->FindWidgetData(id);

    widgetData->SetArgs( bundleData );
    appcore_multiwindow_base_instance_exit(context);
    CheckEmptyInstance(data);
  }

  static void CheckEmptyInstance(void* data)
  {
    int cnt = appcore_multiwindow_base_instance_get_cnt();

    if(cnt == 0)
    {
      static_cast<Internal::Adaptor::Framework::Impl*>(data)->AppWidgetExit();
    }
  }

  static void InstanceResume(const char* classId, const char* id, bundle* bundleData)
  {
    appcore_multiwindow_base_instance_h context;

    context = appcore_multiwindow_base_instance_find(id);

    if(!context)
    {
      DALI_LOG_ERROR("context not found: %s", id);
      return;
    }

    appcore_multiwindow_base_instance_resume(context);

    SendUpdateStatus(classId, id, AUL_WIDGET_INSTANCE_EVENT_RESUME, NULL);
    if(!gForegroundState)
    {
      aul_send_app_status_change_signal(getpid(), gAppId, gPackageId, "fg", "widgetapp" );
      gForegroundState = true;
    }
  }

  static void InstancePause(const char* classId, const char* id, bundle* bundleData)
  {
    appcore_multiwindow_base_instance_h context;

    context = appcore_multiwindow_base_instance_find(id);

    if(!context)
    {
      DALI_LOG_ERROR("context not found: %s", id);
      return;
    }

    appcore_multiwindow_base_instance_pause(context);

    if(gForegroundState)
    {
      aul_send_app_status_change_signal(getpid(), gAppId, gPackageId, "bg", "widgetapp" );
      gForegroundState = false;
    }
  }

  static bool IsWidgetFeatureEnabled()
  {
    static bool feature = false;
    static bool retrieved = false;
    int ret;

    if(retrieved == true)
      return feature;

    ret = system_info_get_platform_bool("http://tizen.org/feature/shell.appwidget", &feature);
    if(ret != SYSTEM_INFO_ERROR_NONE)
    {
      DALI_LOG_ERROR("failed to get system info"); /* LCOV_EXCL_LINE */
      return false; /* LCOV_EXCL_LINE */
    }

    retrieved = true;
    return feature;
  }


#ifdef APPCORE_WATCH_AVAILABLE
  static bool WatchAppCreate(int width, int height, void *data)
  {
    return static_cast<Framework*>(data)->Create();
  }

  static void WatchAppTimeTick(watch_time_h time, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnTimeTick(curTime);
  }

  static void WatchAppAmbientTick(watch_time_h time, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnAmbientTick(curTime);
  }

  static void WatchAppAmbientChanged(bool ambient, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnAmbientChanged(ambient);
  }

  static void WatchAppLanguageChanged(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnLanguageChanged();
  }

  static void WatchAppRegionChanged(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnRegionChanged();
  }

  static void WatchAppBatteryLow(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnBatteryLow();
  }

  static void WatchAppMemoryLow(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnMemoryLow();
  }

  static void WatchAppControl(app_control_h app_control, void *data)
  {
    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;
    bundle *bundleData = NULL;

    app_control_to_bundle(app_control, &bundleData);
    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(app_control);
  }

  static void WatchAppTerminate(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnTerminate();
  }

  static void WatchAppPause(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnPause();
  }

  static void WatchAppResume(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnResume();
  }
#endif

  int AppWatchMain()
  {
    int ret = true;

#ifdef APPCORE_WATCH_AVAILABLE
    mWatchCallback.create = WatchAppCreate;
    mWatchCallback.app_control = WatchAppControl;
    mWatchCallback.terminate = WatchAppTerminate;
    mWatchCallback.pause = WatchAppPause;
    mWatchCallback.resume = WatchAppResume;
    mWatchCallback.time_tick = WatchAppTimeTick;
    mWatchCallback.ambient_tick = WatchAppAmbientTick;
    mWatchCallback.ambient_changed = WatchAppAmbientChanged;

    watch_app_add_event_handler(&watchHandlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, WatchAppBatteryLow, mFramework);
    watch_app_add_event_handler(&watchHandlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, WatchAppMemoryLow, mFramework);
    watch_app_add_event_handler(&watchHandlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, WatchAppLanguageChanged, mFramework);
    watch_app_add_event_handler(&watchHandlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, WatchAppRegionChanged, mFramework);

    ret = watch_app_main(*mFramework->mArgc, *mFramework->mArgv, &mWatchCallback, mFramework);
#endif
    return ret;
  }

  void AppWatchExit()
  {
#ifdef APPCORE_WATCH_AVAILABLE
    watch_app_exit();
#endif
  }


private:
  // Undefined
  Impl( const Impl& impl );

  // Undefined
  Impl& operator=( const Impl& impl );
};

Framework::Framework( Framework::Observer& observer, int *argc, char ***argv, Type type )
: mObserver(observer),
  mInitialised(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler( MakeCallback( this, &Framework::AbortCallback ) ),
  mImpl(NULL)
{
  bool featureFlag = true;
  system_info_get_platform_bool( "tizen.org/feature/opengles.version.2_0", &featureFlag );

  if( featureFlag == false )
  {
    set_last_result( TIZEN_ERROR_NOT_SUPPORTED );
  }
#ifdef DALI_ELDBUS_AVAILABLE
  // Initialize ElDBus.
  eldbus_init();
#endif
  InitThreads();

  mImpl = new Impl(this, type);
}

Framework::~Framework()
{
  if (mRunning)
  {
    Quit();
  }

#ifdef DALI_ELDBUS_AVAILABLE
  // Shutdown ELDBus.
  eldbus_shutdown();
#endif

  delete mImpl;
}

bool Framework::Create()
{
  mInitialised = true;
  mObserver.OnInit();
  return true;
}

void Framework::Run()
{
  mRunning = true;
  int ret;

  ret = mImpl->AppMain();
  if (ret != APP_ERROR_NONE)
  {
    DALI_LOG_ERROR("Framework::Run(), ui_app_main() is failed. err = %d\n", ret);
  }
  mRunning = false;
}

void Framework::Quit()
{
  mImpl->AppExit();
}

bool Framework::IsMainLoopRunning()
{
  return mRunning;
}

void Framework::AddAbortCallback( CallbackBase* callback )
{
  mImpl->mAbortCallBack = callback;
}

std::string Framework::GetBundleName() const
{
  return mBundleName;
}

void Framework::SetBundleName(const std::string& name)
{
  mBundleName = name;
}

std::string Framework::GetBundleId() const
{
  return mBundleId;
}

std::string Framework::GetResourcePath()
{
  std::string resourcePath = "";
#if defined( TIZEN_PLATFORM_CONFIG_SUPPORTED ) && TIZEN_PLATFORM_CONFIG_SUPPORTED
  resourcePath = app_get_resource_path();
#else // For backwards compatibility with older Tizen versions

  // "DALI_APPLICATION_PACKAGE" is used to get the already configured Application package path.
  const char* environmentVariable = "DALI_APPLICATION_PACKAGE";
  char* value = getenv( environmentVariable );
  if ( value != NULL )
  {
    resourcePath = value;
  }
#endif //TIZEN_PLATFORM_CONFIG_SUPPORTED

  return resourcePath;
}

void Framework::SetBundleId(const std::string& id)
{
  mBundleId = id;
}

void Framework::AbortCallback( )
{
  // if an abort call back has been installed run it.
  if (mImpl->mAbortCallBack)
  {
    CallbackBase::Execute( *mImpl->mAbortCallBack );
  }
  else
  {
    Quit();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
