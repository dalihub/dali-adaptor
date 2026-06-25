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
 *
 */

// CLASS HEADER
#include <dali/public-api/adaptor-framework/mouse-in-out-event.h>

namespace Dali
{
struct MouseInOutEvent::Impl
{
  Type                   type;
  uint32_t               modifiers;
  Vector2                point;
  uint32_t               timeStamp;
  Device::Class::Type    deviceClass;
  Device::Subclass::Type deviceSubclass;
};

MouseInOutEvent::MouseInOutEvent(Type type, uint32_t modifiers, Vector2 point, uint32_t timeStamp, Device::Class::Type deviceClass, Device::Subclass::Type deviceSubclass)
: mImpl(new Impl{type, modifiers, point, timeStamp, deviceClass, deviceSubclass})
{
}

MouseInOutEvent::~MouseInOutEvent() = default;

MouseInOutEvent::MouseInOutEvent(const MouseInOutEvent& rhs)
: mImpl(Dali::MakeUnique<Impl>(*rhs.mImpl))
{
}

MouseInOutEvent& MouseInOutEvent::operator=(const MouseInOutEvent& rhs)
{
  if(this != &rhs)
  {
    *mImpl = *rhs.mImpl;
  }
  return *this;
}

MouseInOutEvent::Type MouseInOutEvent::GetType() const
{
  return mImpl->type;
}

uint32_t MouseInOutEvent::GetModifiers() const
{
  return mImpl->modifiers;
}

const Vector2& MouseInOutEvent::GetPoint() const
{
  return mImpl->point;
}

uint32_t MouseInOutEvent::GetTimeStamp() const
{
  return mImpl->timeStamp;
}

Device::Class::Type MouseInOutEvent::GetDeviceClass() const
{
  return mImpl->deviceClass;
}

Device::Subclass::Type MouseInOutEvent::GetDeviceSubclass() const
{
  return mImpl->deviceSubclass;
}

} // namespace Dali
