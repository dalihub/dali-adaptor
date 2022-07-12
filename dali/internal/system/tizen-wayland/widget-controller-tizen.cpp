/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <widget_base.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
WidgetImplTizen::WidgetImplTizen(widget_base_instance_h instanceHandle)
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

  widget_base_context_set_content_info(mInstanceHandle, contentBundle);

  bundle_free(contentBundle);
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
