/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/scene-holder.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <dali/public-api/events/wheel-event.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/public-api/actors/layer.h>

namespace Dali
{
namespace Integration
{
SceneHolder::SceneHolder()
{
}

SceneHolder::~SceneHolder()
{
}

SceneHolder::SceneHolder(const SceneHolder& handle)
: BaseHandle(handle)
{
}

SceneHolder::SceneHolder(Internal::Adaptor::SceneHolder* internal)
: BaseHandle(internal)
{
}

SceneHolder& SceneHolder::operator=(const SceneHolder& rhs)
{
  BaseHandle::operator=(rhs);
  return *this;
}

void SceneHolder::Add(Actor actor)
{
  GetImplementation(*this).Add(actor);
}

void SceneHolder::Remove(Actor actor)
{
  GetImplementation(*this).Remove(actor);
}

Layer SceneHolder::GetRootLayer() const
{
  return GetImplementation(*this).GetRootLayer();
}

void SceneHolder::SetBackgroundColor(Vector4 color)
{
  GetImplementation(*this).SetBackgroundColor(color);
}

Vector4 SceneHolder::GetBackgroundColor() const
{
  return GetImplementation(*this).GetBackgroundColor();
}

Any SceneHolder::GetNativeHandle() const
{
  return GetImplementation(*this).GetNativeHandle();
}

void SceneHolder::FeedTouchPoint(Dali::TouchPoint& point, int timeStamp)
{
  Integration::Point convertedPoint(point);
  GetImplementation(*this).FeedTouchPoint(convertedPoint, timeStamp);
}

void SceneHolder::FeedWheelEvent(Dali::WheelEvent& wheelEvent)
{
  Integration::WheelEvent event(static_cast<Integration::WheelEvent::Type>(wheelEvent.GetType()), wheelEvent.GetDirection(), wheelEvent.GetModifiers(), wheelEvent.GetPoint(), wheelEvent.GetDelta(), wheelEvent.GetTime());
  GetImplementation(*this).FeedWheelEvent(event);
}

void SceneHolder::FeedKeyEvent(Dali::KeyEvent& keyEvent)
{
  Integration::KeyEvent event(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
  GetImplementation(*this).FeedKeyEvent(event);
}

SceneHolder SceneHolder::Get(Actor actor)
{
  return Internal::Adaptor::SceneHolder::Get(actor);
}

SceneHolder::KeyEventSignalType& SceneHolder::KeyEventSignal()
{
  return GetImplementation(*this).KeyEventSignal();
}

SceneHolder::KeyEventGeneratedSignalType& SceneHolder::KeyEventGeneratedSignal()
{
  return GetImplementation(*this).KeyEventGeneratedSignal();
}

SceneHolder::KeyEventGeneratedSignalType& SceneHolder::InterceptKeyEventSignal()
{
  return GetImplementation(*this).InterceptKeyEventSignal();
}

SceneHolder::TouchEventSignalType& SceneHolder::TouchedSignal()
{
  return GetImplementation(*this).TouchedSignal();
}

SceneHolder::WheelEventSignalType& SceneHolder::WheelEventSignal()
{
  return GetImplementation(*this).WheelEventSignal();
}

SceneHolder::WheelEventGeneratedSignalType& SceneHolder::WheelEventGeneratedSignal()
{
  return GetImplementation(*this).WheelEventGeneratedSignal();
}

} // namespace Integration

} // namespace Dali
