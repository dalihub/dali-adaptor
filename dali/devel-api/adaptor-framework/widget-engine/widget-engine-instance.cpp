/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine-instance.h>

// EXTERNAL INCLUDES
#include <utility>

// INTERNAL INCLUDES
#include <dali/internal/widget-engine/common/widget-engine-impl.h>

namespace DALI_NAMESPACE
{
WidgetEngineInstance::WidgetEngineInstance()                                                    = default;
WidgetEngineInstance::~WidgetEngineInstance()                                                   = default;
WidgetEngineInstance::WidgetEngineInstance(const WidgetEngineInstance& instance)                = default;
WidgetEngineInstance& WidgetEngineInstance::operator=(const WidgetEngineInstance& instance)     = default;
WidgetEngineInstance::WidgetEngineInstance(WidgetEngineInstance&& instance) noexcept            = default;
WidgetEngineInstance& WidgetEngineInstance::operator=(WidgetEngineInstance&& instance) noexcept = default;

WidgetEngineInstance::WidgetEngineInstance(Internal::Adaptor::WidgetEngineInstance* implementation)
: BaseHandle(implementation)
{
}

WidgetEngineInstance WidgetEngineInstance::DownCast(BaseHandle handle)
{
  return WidgetEngineInstance(dynamic_cast<Internal::Adaptor::WidgetEngineInstance*>(handle.GetObjectPtr()));
}

const std::string& WidgetEngineInstance::GetWidgetId() const
{
  return Internal::Adaptor::GetImplementation(*this).GetWidgetId();
}
const std::string& WidgetEngineInstance::GetInstanceId() const
{
  return Internal::Adaptor::GetImplementation(*this).GetInstanceId();
}
const std::string& WidgetEngineInstance::GetContentInfo() const
{
  return Internal::Adaptor::GetImplementation(*this).GetContentInfo();
}
const std::string& WidgetEngineInstance::GetTitle() const
{
  return Internal::Adaptor::GetImplementation(*this).GetTitle();
}
std::string WidgetEngineInstance::GetPreviewImagePath()
{
  return Internal::Adaptor::GetImplementation(*this).GetPreviewImagePath();
}
int WidgetEngineInstance::GetProcessId() const
{
  return Internal::Adaptor::GetImplementation(*this).GetProcessId();
}
float WidgetEngineInstance::GetUpdatePeriod() const
{
  return Internal::Adaptor::GetImplementation(*this).GetUpdatePeriod();
}
bool WidgetEngineInstance::IsFaulted() const
{
  return Internal::Adaptor::GetImplementation(*this).IsFaulted();
}
bool WidgetEngineInstance::Pause()
{
  return Internal::Adaptor::GetImplementation(*this).Pause();
}
bool WidgetEngineInstance::Resume()
{
  return Internal::Adaptor::GetImplementation(*this).Resume();
}
bool WidgetEngineInstance::CancelTouchEvent()
{
  return Internal::Adaptor::GetImplementation(*this).CancelTouchEvent();
}
void WidgetEngineInstance::ActivateFaultedWidget()
{
  Internal::Adaptor::GetImplementation(*this).ActivateFaultedWidget();
}
void WidgetEngineInstance::Resize(int width, int height)
{
  Internal::Adaptor::GetImplementation(*this).Resize(width, height);
}
void WidgetEngineInstance::SetVisible(bool visible)
{
  Internal::Adaptor::GetImplementation(*this).SetVisible(visible);
}
bool WidgetEngineInstance::FeedTouchEvent(const TouchEvent& event)
{
  return Internal::Adaptor::GetImplementation(*this).FeedTouchEvent(event);
}
bool WidgetEngineInstance::FeedKeyEvent(const KeyEvent& event)
{
  return Internal::Adaptor::GetImplementation(*this).FeedKeyEvent(event);
}
bool WidgetEngineInstance::FeedWheelEvent(const WheelEvent& event)
{
  return Internal::Adaptor::GetImplementation(*this).FeedWheelEvent(event);
}
void WidgetEngineInstance::SetEventCallback(WidgetEngineInstancePlugin::EventCallback callback)
{
  Internal::Adaptor::GetImplementation(*this).SetEventCallback(std::move(callback));
}
void WidgetEngineInstance::SetFrameUpdatedCallback(WidgetEngineInstancePlugin::FrameUpdatedCallback callback)
{
  Internal::Adaptor::GetImplementation(*this).SetFrameUpdatedCallback(std::move(callback));
}
void WidgetEngineInstance::ResetCallbacks()
{
  Internal::Adaptor::GetImplementation(*this).ResetCallbacks();
}

} // namespace DALI_NAMESPACE
