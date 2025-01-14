/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <bundle.h>
#include <dali/public-api/actors/layer.h>
#include <dlfcn.h>
#include <dlog.h>
#include <tizen.h>
#include <unistd.h>

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
constexpr char const* const kApplicationNamePrefix  = "libdali2-adaptor-application-";
constexpr char const* const kApplicationNamePostfix = ".so";

std::string MakePluginName(const char* appModelName)
{
  std::stringstream fullName;
  fullName << kApplicationNamePrefix << appModelName << kApplicationNamePostfix;
  return fullName.str();
}

} // namespace
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
  uint32_t    len              = static_cast<uint32_t>(contentInfo.length());

  if(DALI_UNLIKELY(contentBundleRaw == nullptr || len == 0u))
  {
    print_log(DLOG_ERROR, "DALI", "error : contentInfo was empty!\n");
    return;
  }

  contentBundle = bundle_decode(contentBundleRaw, static_cast<int>(len));

  using SetContentInfoFunc = void (*)(void*, bundle*);
  SetContentInfoFunc setContentInfoFuncPtr;
  std::string        pluginName = MakePluginName("widget");

  void* mHandle = dlopen(pluginName.c_str(), RTLD_LAZY);

  if(mHandle == nullptr)
  {
    print_log(DLOG_ERROR, "DALI", "error : %s\n", dlerror());
    bundle_free(contentBundle);
    return;
  }

  setContentInfoFuncPtr = reinterpret_cast<SetContentInfoFunc>(dlsym(mHandle, "SetContentInfo"));
  if(setContentInfoFuncPtr != nullptr)
  {
    setContentInfoFuncPtr(mInstanceHandle, contentBundle);
  }
  else
  {
    print_log(DLOG_ERROR, "DALI", "SetContentInfo is null\n");
  }

  bundle_free(contentBundle);

  if(mHandle != NULL)
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
  mWindow   = window;
  mWidgetId = widgetId;

  auto bridge           = Accessibility::Bridge::GetCurrentBridge();
  auto preferredBusName = Accessibility::Bridge::MakeBusNameForWidget(widgetId, getpid());

  // Ensure the bridge is at least in an unlocked state. Normal application callbacks that would
  // call Bridge::ApplicationPaused/Resumed() elsewhere are not operational in widget scenarios.
  bridge->ApplicationResumed();
  bridge->SetPreferredBusName(preferredBusName);

  // Widget should not send window events (which could narrow down the navigation context)
  if(auto accessible = Accessibility::Accessible::Get(window.GetRootLayer()))
  {
    auto& suppressedEvents                                      = accessible->GetSuppressedEvents();
    suppressedEvents[Accessibility::AtspiEvent::STATE_CHANGED]  = true;
    suppressedEvents[Accessibility::AtspiEvent::WINDOW_CHANGED] = true;
  }
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
