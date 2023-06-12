/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/tizen-wayland/widget-application-impl-tizen.h>

// INTERNAL INCLUDE
#include <dali/devel-api/events/key-event-devel.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/tizen-wayland/widget-controller-tizen.h>
#include <dali/public-api/adaptor-framework/widget-impl.h>
#include <dali/public-api/adaptor-framework/widget.h>

// EXTERNAL INCLUDES
#include <bundle.h>
#include <widget_base.h>

namespace Dali
{
namespace Internal
{
namespace
{
/**
 * This Api is called when widget viewer send keyEvent.
 * In this API, widget framework create a new keyEvent, find the proper widget and send this event.
 * Finally widget framework receive feedback from widget.
 */
#ifdef OVER_TIZEN_VERSION_7
bool OnKeyEventCallback(const char* id, screen_connector_event_type_e eventType, int keyCode, const char* keyName, long long cls, long long subcls, const char* identifier, long long timestamp, void* userData)
{
  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(userData);

  // Create new key for widget
  Dali::KeyEvent::State state = Dali::KeyEvent::DOWN;
  if(eventType == SCREEN_CONNECTOR_EVENT_TYPE_KEY_DOWN)
  {
    state = Dali::KeyEvent::DOWN;
  }
  else if(eventType == SCREEN_CONNECTOR_EVENT_TYPE_KEY_UP)
  {
    state = Dali::KeyEvent::UP;
  }

  bool           consumed     = true;
  std::string    keyEventName = std::string(keyName);
  Dali::KeyEvent event        = Dali::DevelKeyEvent::New(keyEventName, "", "", keyCode, 0, timestamp, state, "", "", Device::Class::NONE, Device::Subclass::NONE);

  if(application)
  {
    std::string            widgetId       = std::string(id);
    widget_base_instance_h instanceHandle = application->GetWidgetInstanceFromWidgetId(widgetId);
    if(instanceHandle)
    {
      consumed = application->FeedKeyEvent(instanceHandle, event);
    }
  }

  return consumed;
}
#endif

int OnInstanceInit(widget_base_instance_h instanceHandle, bundle* content, int w, int h, void* classData)
{
  char* id;
  widget_base_context_get_id(instanceHandle, &id);

  widget_base_class_on_create(instanceHandle, content, w, h);

  Dali::Internal::Adaptor::WidgetApplicationTizen* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(classData);

  Dali::Window window;
  if(application->GetWidgetCount() == 0)
  {
    window = application->GetWindow();
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
      window = application->GetWindow();
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

  Dali::Internal::Adaptor::Widget::Impl* widgetImpl = new Dali::Internal::Adaptor::WidgetImplTizen(instanceHandle);
  Internal::Adaptor::GetImplementation(widgetInstance).SetImpl(widgetImpl);

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

unsigned int GetEnvWidgetRenderRefreshRate()
{
  const char* envVariable = std::getenv(DALI_WIDGET_REFRESH_RATE);

  return envVariable ? std::atoi(envVariable) : 1u; // Default 60 fps
}

} // anonymous namespace

namespace Adaptor
{
WidgetApplicationPtr WidgetApplicationTizen::New(
  int*               argc,
  char**             argv[],
  const std::string& stylesheet,
  WindowData&        windowData)
{
  return new WidgetApplicationTizen(argc, argv, stylesheet, windowData);
}

WidgetApplicationTizen::WidgetApplicationTizen(int* argc, char** argv[], const std::string& stylesheet, WindowData& windowData)
: WidgetApplication(argc, argv, stylesheet, windowData),
  mConnectedKeyEvent(false),
  mReceivedKeyEvent(false)
{
}

WidgetApplicationTizen::~WidgetApplicationTizen()
{
}

void WidgetApplicationTizen::RegisterWidgetCreatingFunction(const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction)
{
  AddWidgetCreatingFunctionPair(CreateWidgetFunctionPair(widgetName, createFunction));

  // Register widget class to widget framework
  widget_base_class cls = widget_base_class_get_default();
  cls.ops.create        = OnInstanceInit;
  cls.ops.destroy       = OnInstanceDestroy;
  cls.ops.pause         = OnInstancePause;
  cls.ops.resume        = OnInstanceResume;
  cls.ops.resize        = OnInstanceResize;
  cls.ops.update        = OnInstanceUpdate;

  widget_base_class_add(cls, widgetName.c_str(), this);
}

void WidgetApplicationTizen::AddWidgetCreatingFunctionPair(CreateWidgetFunctionPair pair)
{
  mCreateWidgetFunctionContainer.push_back(pair);
}

WidgetApplicationTizen::CreateWidgetFunctionPair WidgetApplicationTizen::GetWidgetCreatingFunctionPair(const std::string& widgetName)
{
  int         idx      = widgetName.find(":");
  std::string widgetID = widgetName.substr(idx + 1);
  for(CreateWidgetFunctionContainer::const_iterator iter = mCreateWidgetFunctionContainer.begin(); iter != mCreateWidgetFunctionContainer.end(); ++iter)
  {
    if(widgetID.compare((*iter).first) == 0)
    {
      return *iter;
    }
  }

  return CreateWidgetFunctionPair("", NULL);
}

void WidgetApplicationTizen::AddWidget(widget_base_instance_h widgetBaseInstance, Dali::Widget widget, Dali::Window window, const std::string& widgetId)
{
  mWidgetInstanceContainer.push_back(WidgetInstancePair(widgetBaseInstance, widget));
  Internal::Adaptor::GetImplementation(widget).SetInformation(window, widgetId);
}

Dali::Widget WidgetApplicationTizen::GetWidget(widget_base_instance_h widgetBaseInstance) const
{
  for(auto&& iter : mWidgetInstanceContainer)
  {
    if((iter).first == widgetBaseInstance)
    {
      return (iter).second;
    }
  }
  return Dali::Widget();
}

void WidgetApplicationTizen::DeleteWidget(widget_base_instance_h widgetBaseInstance)
{
  // Delete WidgetInstance
  auto widgetInstance = std::find_if(mWidgetInstanceContainer.begin(),
                                     mWidgetInstanceContainer.end(),
                                     [widgetBaseInstance](WidgetInstancePair pair) { return (pair.first == widgetBaseInstance); });

  if(widgetInstance != mWidgetInstanceContainer.end())
  {
    mWidgetInstanceContainer.erase(widgetInstance);
  }
}

Dali::Window WidgetApplicationTizen::GetWindowFromWidget(Dali::Widget widgetInstance) const
{
  if(widgetInstance)
  {
    return Internal::Adaptor::GetImplementation(widgetInstance).GetWindow();
  }

  return Dali::Window();
}

widget_base_instance_h WidgetApplicationTizen::GetWidgetInstanceFromWidgetId(std::string& widgetId) const
{
  for(auto&& iter : mWidgetInstanceContainer)
  {
    if(widgetId == Internal::Adaptor::GetImplementation((iter).second).GetWidgetId())
    {
      return (iter).first;
    }
  }

  return nullptr;
}

int WidgetApplicationTizen::GetWidgetCount()
{
  return mWidgetInstanceContainer.size();
}

void WidgetApplicationTizen::ConnectKeyEvent(Dali::Window window)
{
  if(!mConnectedKeyEvent)
  {
#ifdef OVER_TIZEN_VERSION_7
    screen_connector_provider_set_key_event_cb(OnKeyEventCallback, this);
#endif
    mConnectedKeyEvent = true;
  }
  window.KeyEventSignal().Connect(this, &WidgetApplicationTizen::OnWindowKeyEvent);
}

void WidgetApplicationTizen::OnWindowKeyEvent(const Dali::KeyEvent& event)
{
  // If Widget Application consume key event, this api is not called.
  mReceivedKeyEvent = true;
}

bool WidgetApplicationTizen::FeedKeyEvent(widget_base_instance_h instanceHandle, const Dali::KeyEvent& keyEvent)
{
  bool consumed = true;

  // Check if application consume key event
  Dali::Widget widgetInstance = GetWidget(instanceHandle);
  if(widgetInstance)
  {
    Dali::Window window = GetWindowFromWidget(widgetInstance);

    // Reset the state of key received
    mReceivedKeyEvent = false;

    // Feed the keyEvent to widget window
    DevelWindow::FeedKeyEvent(window, keyEvent);

    // if the application is not using a key event, verify that the window in the widget has received a key event.
    if(Internal::Adaptor::GetImplementation(widgetInstance).IsKeyEventUsing() == false)
    {
      // if the window has received a key event, widget need to consume its key event
      consumed = (mReceivedKeyEvent) ? false : true;
    }
  }

  return consumed;
}

void WidgetApplicationTizen::OnInit()
{
  WidgetApplication::OnInit();

  Dali::Adaptor::Get().SetRenderRefreshRate(GetEnvWidgetRenderRefreshRate());
}

// factory function, must be implemented
namespace WidgetApplicationFactory
{
/**
 * Create a new widget application
 * @param[in]  argc         A pointer to the number of arguments
 * @param[in]  argv         A pointer to the argument list
 * @param[in]  stylesheet   The path to user defined theme file
 * @param[in]  windowData   The window data
 */
Dali::Internal::Adaptor::WidgetApplicationPtr Create(int* argc, char** argv[], const std::string& stylesheet, WindowData& windowData)
{
  return WidgetApplicationTizen::New(argc, argv, stylesheet, windowData);
}

} // namespace WidgetApplicationFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
