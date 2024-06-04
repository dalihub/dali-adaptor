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
#include <dali/internal/system/tizen-wayland/widget-controller-tizen.h>

// EXTERNAL INCLUDES
#include <dali/public-api/actors/layer.h>
#include <bundle.h>
#include <dlfcn.h>
#include <dlog.h>
#include <tizen.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/integration-api/debug.h>

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

}
namespace Adaptor
{
WidgetImplTizen::WidgetImplTizen(void* instanceHandle)
: Widget::Impl(),
  mInstanceHandle(instanceHandle),
  mWindow(),
  mWidgetId(),
  mUsingKeyEvent(false)
{
}

WidgetImplTizen::~WidgetImplTizen()
{
}

void WidgetImplTizen::SetContentInfo(const std::string& contentInfo)
{
  bundle*     contentBundle;
  bundle_raw* contentBundleRaw = reinterpret_cast<bundle_raw*>(const_cast<char*>(contentInfo.c_str()));
  int         len              = contentInfo.length();
  contentBundle                = bundle_decode(contentBundleRaw, len);


  using SetContentInfoFunc          = void (*)(void*, bundle*);
  SetContentInfoFunc                setContentInfoFuncPtr;
  std::string pluginName = MakePluginName("widget");

  void* mHandle = dlopen(pluginName.c_str(), RTLD_LAZY);

  if(mHandle == nullptr)
  {
    print_log(DLOG_ERROR, "DALI", "error : %s", dlerror() );
    return;
  }

  setContentInfoFuncPtr = reinterpret_cast<SetContentInfoFunc>(dlsym(mHandle, "SetContentInfo"));
  if(setContentInfoFuncPtr != nullptr)
  {
    setContentInfoFuncPtr(mInstanceHandle, contentBundle);
  }
  else
  {
    print_log(DLOG_ERROR, "DALI", "SetContentInfo is null\n" );
  }

  bundle_free(contentBundle);

  if(mHandle!=NULL)
  {
    dlclose(mHandle);
  }
}

bool WidgetImplTizen::IsKeyEventUsing() const
{
  return mUsingKeyEvent;
}

void WidgetImplTizen::SetUsingKeyEvent(bool flag)
{
  mUsingKeyEvent = flag;
}

void WidgetImplTizen::SetInformation(Dali::Window window, const std::string& widgetId)
{
  using Dali::Accessibility::Bridge;

  mWindow = window;
  mWidgetId = widgetId;

  auto preferredBusName = Bridge::MakeBusNameForWidget(widgetId);
  Bridge::GetCurrentBridge()->SetPreferredBusName(preferredBusName);

  // Widget should not send window events (which could narrow down the navigation context)
  auto& suppressedEvents = Accessibility::Accessible::Get(window.GetRootLayer())->GetSuppressedEvents();
  suppressedEvents[Accessibility::AtspiEvent::STATE_CHANGED] = true;
  suppressedEvents[Accessibility::AtspiEvent::WINDOW_CHANGED] = true;
}

Dali::Window WidgetImplTizen::GetWindow() const
{
  return mWindow;
}

std::string WidgetImplTizen::GetWidgetId() const
{
  return mWidgetId;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
