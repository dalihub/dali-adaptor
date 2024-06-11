/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/application-model/widget/widget-base-tizen.h>


// INTERNAL INCLUDE
#include <dali/internal/system/tizen-wayland/widget-controller-tizen.h>

#include <dali/devel-api/events/key-event-devel.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>

#include <dali/public-api/adaptor-framework/widget-impl.h>
#include <dali/public-api/adaptor-framework/widget.h>
#include <dali/internal/system/tizen-wayland/widget-application-impl-tizen.h>

// EXTERNAL INCLUDES
#include <bundle.h>
#include <widget_base.h>
#include <dlog.h>
#include <tizen.h>

#define DEBUG_PRINTF(fmt, arg...) LOGD(" " fmt, ##arg)

namespace Dali
{
namespace Internal
{
namespace
{

int OnInstanceInit(widget_base_instance_h instanceHandle, bundle* content, int w, int h, void* classData)
{
  char* id;
  widget_base_context_get_id(instanceHandle, &id);

  widget_base_class_on_create(instanceHandle, content, w, h);

  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  Dali::Window window;
  if(application->GetWidgetCount() == 0)
  {
    window = application->GetWidgetWindow();
    DALI_LOG_RELEASE_INFO("Widget Instance use default Window(win:%p), so it need to bind widget (%dx%d) (id:%s) \n", window, w, h, std::string(id).c_str());

  }
  else
  {
    window = Dali::Window::New(PositionSize(0, 0, w, h), "", false);
    if(window)
    {
      DALI_LOG_RELEASE_INFO("Widget Instance create new Window  (win:%p, cnt:%d) (%dx%d) (id:%s )\n", window, application->GetWidgetCount(), w, h, std::string(id).c_str());
    }
    else
    {
      DALI_LOG_ERROR("This device can't support Multi Widget. it means UI may not be properly drawn.");
      window = application->GetWidgetWindow();
    }
  }

  Any nativeHandle = window.GetNativeHandle();

#ifdef ECORE_WAYLAND2
  Ecore_Wl2_Window* wlWindow = AnyCast<Ecore_Wl2_Window*>(nativeHandle);
#else
  Ecore_Wl_Window* wlWindow = AnyCast<Ecore_Wl_Window*>(nativeHandle);
#endif

  widget_base_context_window_bind(instanceHandle, id, wlWindow);
  window.SetSize(Dali::Window::WindowSize(w, h));

  Dali::Internal::Adaptor::WidgetApplication::CreateWidgetFunctionPair pair           = application->GetWidgetCreatingFunctionPair(std::string(id));
  Dali::WidgetApplication::CreateWidgetFunction                        createFunction = pair.second;

  Dali::Widget widgetInstance = createFunction(pair.first);

  application->InitializeWidget(instanceHandle, widgetInstance);

  application->AddWidget(instanceHandle, widgetInstance, window, std::string(id));

  std::string encodedContentString = "";

  if(bundle_get_count(content))
  {
    bundle_raw* bundleRaw;
    int         len;
    bundle_encode(content, &bundleRaw, &len);
    char* encodedContent = reinterpret_cast<char*>(bundleRaw);
    encodedContentString = std::string(encodedContent);
    free(bundleRaw);
  }

  Internal::Adaptor::GetImplementation(widgetInstance).OnCreate(encodedContentString, window);

  // connect keyEvent for widget
#ifdef OVER_TIZEN_VERSION_7
  application->ConnectKeyEvent(window);
#endif

  return 0;
}

int OnInstanceDestroy(widget_base_instance_h instanceHandle, widget_base_destroy_type_e reason, bundle* content, void* classData)
{
  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget(instanceHandle);

  Dali::Widget::Termination destroyReason = Dali::Widget::Termination::TEMPORARY;

  if(reason == WIDGET_BASE_DESTROY_TYPE_PERMANENT)
  {
    destroyReason = Dali::Widget::Termination::PERMANENT;
  }

  std::string encodedContentString = "";

  if(bundle_get_count(content))
  {
    bundle_raw* bundleRaw;
    int         len;
    bundle_encode(content, &bundleRaw, &len);
    char* encodedContent = reinterpret_cast<char*>(bundleRaw);
    encodedContentString = std::string(encodedContent);
    free(bundleRaw);
  }

  Internal::Adaptor::GetImplementation(widgetInstance).OnTerminate(encodedContentString, destroyReason);

  widget_base_class_on_destroy(instanceHandle, reason, content);

  application->DeleteWidget(instanceHandle);

  return 0;
}

int OnInstancePause(widget_base_instance_h instanceHandle, void* classData)
{
  widget_base_class_on_pause(instanceHandle);

  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget(instanceHandle);

  Internal::Adaptor::GetImplementation(widgetInstance).OnPause();

  return 0;
}

int OnInstanceResume(widget_base_instance_h instanceHandle, void* classData)
{
  widget_base_class_on_resume(instanceHandle);

  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget(instanceHandle);

  Internal::Adaptor::GetImplementation(widgetInstance).OnResume();
  return 0;
}

int OnInstanceResize(widget_base_instance_h instanceHandle, int w, int h, void* classData)
{
  widget_base_class_on_resize(instanceHandle, w, h);

  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget(instanceHandle);
  Dali::Window window         = application->GetWindowFromWidget(widgetInstance);
  window.SetSize(Dali::Window::WindowSize(w, h));
  Internal::Adaptor::GetImplementation(widgetInstance).OnResize(window);

  return 0;
}

int OnInstanceUpdate(widget_base_instance_h instanceHandle, bundle* content, int force, void* classData)
{
  widget_base_class_on_update(instanceHandle, content, force);

  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget(instanceHandle);

  std::string encodedContentString = "";

  if(bundle_get_count(content))
  {
    bundle_raw* bundleRaw;
    int         len;
    bundle_encode(content, &bundleRaw, &len);
    char* encodedContent = reinterpret_cast<char*>(bundleRaw);
    encodedContentString = std::string(encodedContent);
    free(bundleRaw);
  }
  Internal::Adaptor::GetImplementation(widgetInstance).OnUpdate(encodedContentString, force);

  return 0;
}
}
namespace Adaptor
{
extern "C" DALI_ADAPTOR_API void RegisterWidgetCallback(const char* widgetName, void* data) {
  widget_base_class cls = widget_base_class_get_default();
  cls.ops.create        = OnInstanceInit;
  cls.ops.destroy       = OnInstanceDestroy;
  cls.ops.pause         = OnInstancePause;
  cls.ops.resume        = OnInstanceResume;
  cls.ops.resize        = OnInstanceResize;
  cls.ops.update        = OnInstanceUpdate;

  widget_base_class_add(cls, widgetName, data);
}

extern "C" DALI_ADAPTOR_API void SetContentInfo(void* handle, bundle* bundleData) {
  widget_base_instance_h handle_instance = static_cast<widget_base_instance_h>(handle);
  //bundle* bundle = static_cast<bundle*>(bundleData);
  widget_base_context_set_content_info(handle_instance, bundleData);
}
}
}
}
