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
#include <dali/internal/system/tizen-wayland/widget-application-impl-tizen.h>

// INTERNAL INCLUDE
#include <dali/internal/system/tizen-wayland/widget-controller-tizen.h>
#include <dali/devel-api/events/key-event-devel.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>

#include <dali/public-api/adaptor-framework/widget-impl.h>
#include <dali/public-api/adaptor-framework/widget.h>

// EXTERNAL INCLUDES
#include <bundle.h>
#include <dlfcn.h>
#include <dlog.h>
#include <tizen.h>
namespace Dali
{
namespace Internal
{
namespace
{
constexpr char const* const kApplicationNamePrefix     = "libdali2-adaptor-application-";
constexpr char const* const kApplicationNamePostfix    = ".so";

std::string MakePluginName(const char* appModelName)
{
  std::stringstream fullName;
  fullName << kApplicationNamePrefix << appModelName << kApplicationNamePostfix;
  return fullName.str();
}

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
  const WindowData&  windowData)
{
  return new WidgetApplicationTizen(argc, argv, stylesheet, windowData);
}

WidgetApplicationTizen::WidgetApplicationTizen(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData)
: WidgetApplication(argc, argv, stylesheet, windowData),
  mConnectedKeyEvent(false),
  mReceivedKeyEvent(false)
{
}

WidgetApplicationTizen::~WidgetApplicationTizen()
{
}

void WidgetApplicationTizen::InitializeWidget(void* instanceHandle, Dali::Widget widgetInstance)
{
  Dali::Internal::Adaptor::Widget::Impl* widgetImpl = new Dali::Internal::Adaptor::WidgetImplTizen(instanceHandle);
  Internal::Adaptor::GetImplementation(widgetInstance).SetImpl(widgetImpl);
}

void WidgetApplicationTizen::RegisterWidgetCreatingFunction(const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction)
{
  AddWidgetCreatingFunctionPair(CreateWidgetFunctionPair(widgetName, createFunction));

  using RegisterFunction          = void (*)(const char*, void*);
  RegisterFunction                registerFunctionPtr;
  std::string pluginName = MakePluginName("widget");

  void* mHandle = dlopen(pluginName.c_str(), RTLD_LAZY);

  if(mHandle == nullptr)
  {
    print_log(DLOG_INFO, "DALI", "error : %s", dlerror() );
    return;
  }

  registerFunctionPtr = reinterpret_cast<RegisterFunction>(dlsym(mHandle, "RegisterWidgetCallback"));
  if(registerFunctionPtr != nullptr)
  {
    registerFunctionPtr(widgetName.c_str(), this);
  }
  else
  {
    print_log(DLOG_INFO, "DALI", "registerFunctionPtr is null\n");
  }

  if(mHandle != NULL)
  {
    dlclose(mHandle);
  }
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

void WidgetApplicationTizen::AddWidget(void* widgetBaseInstance, Dali::Widget widget, Dali::Window window, const std::string& widgetId)
{
  mWidgetInstanceContainer.push_back(WidgetInstancePair(widgetBaseInstance, widget));
  Internal::Adaptor::GetImplementation(widget).SetInformation(window, widgetId);
}

Dali::Widget WidgetApplicationTizen::GetWidget(void* widgetBaseInstance) const
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

void WidgetApplicationTizen::DeleteWidget(void* widgetBaseInstance)
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

void* WidgetApplicationTizen::GetWidgetInstanceFromWidgetId(std::string& widgetId) const
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

Dali::Window WidgetApplicationTizen::GetWidgetWindow()
{
  return GetWindow();
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

bool WidgetApplicationTizen::FeedKeyEvent(void* instanceHandle, const Dali::KeyEvent& keyEvent)
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
Dali::Internal::Adaptor::WidgetApplicationPtr Create(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData)
{
  return WidgetApplicationTizen::New(argc, argv, stylesheet, windowData);
}
} // namespace WidgetApplicationFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
