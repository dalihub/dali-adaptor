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

// CLASS HEADER
#include "widget-impl.h"

// EXTERNAL INCLUDES
#include <aul.h>
#include <aul_app_com.h>
#include <appcore_multiwindow_base.h>
#include <unistd.h>
#include <screen_connector_provider.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

#include <system_info.h>
#include <string.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/math/vector2.h>

// INTERNAL INCLUDES
#include <widget-data.h>
#include <adaptor.h>

namespace Dali
{
namespace Internal
{

namespace Adaptor
{

namespace
{

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
	  DALI_LOG_ERROR("failed to get system info\n"); /* LCOV_EXCL_LINE */
	  return false; /* LCOV_EXCL_LINE */
  }

  retrieved = true;

  return feature;
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


  char temp[256] = {0, };
  char *packageId = NULL;
  if(aul_app_get_pkgid_bypid(getpid(), temp, sizeof(temp)) == 0)
  {
	  packageId = strdup(temp);
  }

  if(!packageId)
  {
	  DALI_LOG_ERROR("package_id is NULL");
	  return -1;
  }
  bundle_add_str(bundleData, AUL_K_PKGID, packageId);

  ret = aul_app_com_send("widget.status", bundleData);

  if (ret < 0)
    DALI_LOG_ERROR("send lifecycle error:%d\n", ret);

  bundle_free(bundleData);

  return ret;
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

} // anonymous namespace

Dali::Widget Widget::New( const std::string& id )
{
  if(!IsWidgetFeatureEnabled())
  {
    DALI_LOG_ERROR("not supported");
    Dali::Widget handle(NULL);
    return handle;
  }

  if( id.size() < 1 )
  {
    DALI_LOG_ERROR("class id is NULL");
    Dali::Widget handle(NULL);
    return handle;
  }

  WidgetPtr widget ( new Widget( id ) );
  Dali::Widget handle( widget.Get() );
  return handle;
}

void Widget::InsertWidgetData( const char* id, Dali::WidgetData widgetData )
{
  mIdWidgetContainer.push_back( std::make_pair( id, widgetData ) );
}

Dali::WidgetData* Widget::FindWidgetData( const char* key )
{
  for( IdWidgetDataContainer::iterator iter = mIdWidgetContainer.begin(), endIter = mIdWidgetContainer.end(); iter != endIter; ++iter )
  {
    if( iter->first == key )
    {
      return &iter->second;
    }
  }
  return NULL; // Not found
}

void Widget::DeleteWidgetData( const char* key )
{
  for( IdWidgetDataContainer::iterator iter = mIdWidgetContainer.begin(), endIter = mIdWidgetContainer.end(); iter != endIter; ++iter )
  {
    if( iter->first == key )
    {
      iter = mIdWidgetContainer.erase(iter);
      return;
    }
  }
}

static void OnInit(appcore_multiwindow_base_instance_h context, void *data)
{
  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( data );
  bundle* bundleData;
  bundle* contentData = NULL;
  char* operation = NULL;
  char* content = NULL;
  char* widthStr = NULL;
  char* heightStr = NULL;
  char* remain = NULL;
  uint32_t width = 0;
  uint32_t height = 0;

  // currently, there is no implementaion in appcore;
  appcore_multiwindow_base_class_on_create(context);

  const char* id = appcore_multiwindow_base_instance_get_id(context);
  bundle* createBundle = static_cast<bundle*>(appcore_multiwindow_base_instance_get_extra(context));

  Dali::WidgetData widgetData = Dali::WidgetData::New( id, createBundle, NULL );

  char* createContent = NULL;
  bundle_get_str(createBundle, "__WIDGET_CONTENT_INFO__", &createContent);
  if(createContent)
  {
    widgetData.SetContent( const_cast<char *>(std::string(createContent).c_str()) );
  }
  widget->InsertWidgetData( id, widgetData );

  bundleData = widgetData.GetArgs();
  bundle_get_str(bundleData, "__WIDGET_OP__", &operation);

  if(!operation)
  {
    DALI_LOG_ERROR("no operation provided");
    return;
  }

  bundle_get_str(bundleData, "__WIDGET_CONTENT_INFO__", &content);
  bundle_get_str(bundleData, "__WIDGET_WIDTH__", &widthStr);
  bundle_get_str(bundleData, "__WIDGET_HEIGHT__", &heightStr);

  if(widthStr)
    width = static_cast<uint32_t>(g_ascii_strtoll(widthStr, &remain, 10));

  if(heightStr)
    height = static_cast<uint32_t>(g_ascii_strtoll(heightStr, &remain, 10));

  if(content)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    contentData = bundle_decode((const bundle_raw*)content, strlen(content));
#pragma GCC diagnostic pop

  Any nativeHandle = Dali::Adaptor::Get().GetNativeWindowHandle();
  Ecore_Wl_Window * wlWindow = AnyCast<Ecore_Wl_Window*>( nativeHandle );

  struct wl_surface* surface = ecore_wl_window_surface_get(wlWindow);

  screen_connector_provider_remote_enable(id, surface);
  ecore_wl_window_class_name_set(wlWindow, id);
  appcore_multiwindow_base_window_bind( context, wlWindow );

  if(!widget->mCreateSignal.Empty())
  {
    widget->mCreateSignal.Emit(widgetData, contentData, Dali::Widget::WindowSize(width, height));
  }

  SendUpdateStatus(widget->mClassId.c_str(), const_cast<char*>(id), AUL_WIDGET_INSTANCE_EVENT_CREATE, NULL);

  aul_widget_instance_add(widget->mClassId.c_str(), const_cast<char*>(id));

  if(contentData)
    bundle_free(contentData);
}

static void OnTerminate(appcore_multiwindow_base_instance_h context, void *data)
{
  bundle* bundleData;
  char* operation = NULL;
  bundle* contentData;

  Dali::Widget::WidgetTerminateType reason = Dali::Widget::WidgetTerminateType::TEMPORARY;
  int event = AUL_WIDGET_INSTANCE_EVENT_TERMINATE;
  const char* id;

  id = appcore_multiwindow_base_instance_get_id(context);

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( data );

  Dali::WidgetData* widgetData = widget->FindWidgetData(id);

  bundleData = widgetData->GetArgs();

  if(bundleData)
  {
    bundle_get_str(bundleData, "__WIDGET_OP__", &operation);
    if (operation && strcmp(operation, "destroy") == 0)
    {
      reason = Dali::Widget::WidgetTerminateType::PERMANENT;
    }
  }

  char* content = widgetData->GetContent();
  if( content )
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    contentData = bundle_decode((const bundle_raw *)content, strlen(content));
#pragma GCC diagnostic pop
  }
  else
  {
    contentData = bundle_create();
  }

  if(!widget->mTerminateSignal.Empty())
  {
    widget->mTerminateSignal.Emit(*widgetData, contentData, reason );
  }

  if(reason == Dali::Widget::WidgetTerminateType::PERMANENT)
  {
    event = AUL_WIDGET_INSTANCE_EVENT_DESTROY;
    aul_widget_instance_del(widget->mClassId.c_str(), id);
  }
  else
  {
    SendUpdateStatus(widget->mClassId.c_str(), id, AUL_WIDGET_INSTANCE_EVENT_EXTRA_UPDATED, contentData);
  }

  if(contentData)
  {
    bundle_free(contentData);
  }

  SendUpdateStatus(widget->mClassId.c_str(), id, event, NULL);

  // currently, there is no implementaion in appcore;
  appcore_multiwindow_base_class_on_terminate(context);
  widget->DeleteWidgetData(id);
}

static void OnPause(appcore_multiwindow_base_instance_h context, void *data)
{
  // currently, there is no implementaion in appcore;
  appcore_multiwindow_base_class_on_pause(context);

  const char *id = appcore_multiwindow_base_instance_get_id(context);
  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( data );

  Dali::WidgetData* widgetData = widget->FindWidgetData(id);

  if(!widget->mPauseSignal.Empty())
  {
    widget->mPauseSignal.Emit(*widgetData);
  }

  SendUpdateStatus(widget->mClassId.c_str(), id, AUL_WIDGET_INSTANCE_EVENT_PAUSE, NULL);
}

static void OnResume(appcore_multiwindow_base_instance_h context, void *data)
{
  // currently, there is no implementaion in appcore;
  appcore_multiwindow_base_class_on_resume(context);

  const char *id = appcore_multiwindow_base_instance_get_id(context);
  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( data );

  Dali::WidgetData* widgetData = widget->FindWidgetData(id);

  if(!widget->mResumeSignal.Empty())
  {
    widget->mResumeSignal.Emit(*widgetData);
  }

  SendUpdateStatus(widget->mClassId.c_str(), id, AUL_WIDGET_INSTANCE_EVENT_RESUME, NULL);
}

void Widget::OnResize(appcore_multiwindow_base_instance_h context, Dali::Widget::WindowSize windowSize)
{
  const char *id = appcore_multiwindow_base_instance_get_id(context);
  Dali::WidgetData *widgetData = FindWidgetData(id);

  if(!mResizeSignal.Empty())
  {
    mResizeSignal.Emit(*widgetData, windowSize);
  }

  SendUpdateStatus(mClassId.c_str(), id, AUL_WIDGET_INSTANCE_EVENT_SIZE_CHANGED, NULL);
}

void Widget::OnUpdate(appcore_multiwindow_base_instance_h context, bundle* content, int force)
{
  const char *id = appcore_multiwindow_base_instance_get_id(context);
  Dali::WidgetData *widgetData = static_cast<Dali::WidgetData*>(appcore_multiwindow_base_instance_get_extra(context));

  if(!mUpdateSignal.Empty())
  {
    mUpdateSignal.Emit(*widgetData, content, force);
  }

  SendUpdateStatus(mClassId.c_str(), id, AUL_WIDGET_INSTANCE_EVENT_UPDATE, NULL);
}

Widget::Widget( const std::string& id )
: mCreateSignal(),
  mTerminateSignal(),
  mPauseSignal(),
  mResumeSignal(),
  mResizeSignal(),
  mUpdateSignal(),
  mClassId(id),
  mSlotDelegate(this)
{
  appcore_multiwindow_base_class cls;

  cls.id = const_cast<char*>(mClassId.c_str());
  cls.data = this;
  cls.create = OnInit;
  cls.terminate = OnTerminate;
  cls.pause = OnPause;
  cls.resume = OnResume;

  appcore_multiwindow_base_class_add(cls);
}

Widget::~Widget()
{
}

} // Adaptor

} // Internal

} // Dali
