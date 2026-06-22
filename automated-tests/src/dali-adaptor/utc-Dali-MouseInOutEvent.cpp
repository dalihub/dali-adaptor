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

#include <dali-test-suite-utils.h>
#include <dali/public-api/adaptor-framework/mouse-in-out-event.h>

using namespace Dali;

void utc_dali_mouse_in_out_event_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_mouse_in_out_event_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliMouseInOutEvent01(void)
{
  // Test constructor and all getters with Type::IN
  Vector2         point(100.0f, 200.0f);
  MouseInOutEvent event(MouseInOutEvent::Type::IN, 0x01, point, 12345u, Device::Class::MOUSE, Device::Subclass::NONE);

  DALI_TEST_EQUALS(event.GetType(), MouseInOutEvent::Type::IN, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetModifiers(), 0x01u, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetPoint(), point, 0.001f, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetTimeStamp(), 12345u, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetDeviceClass(), Device::Class::MOUSE, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetDeviceSubclass(), Device::Subclass::NONE, TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent02(void)
{
  // Test constructor with Type::OUT
  Vector2         point(50.0f, 75.0f);
  MouseInOutEvent event(MouseInOutEvent::Type::OUT, 0u, point, 9999u, Device::Class::NONE, Device::Subclass::NONE);

  DALI_TEST_EQUALS(event.GetType(), MouseInOutEvent::Type::OUT, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetPoint().x, 50.0f, 0.001f, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetPoint().y, 75.0f, 0.001f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent03(void)
{
  // Test constructor with Type::NONE
  MouseInOutEvent event(MouseInOutEvent::Type::NONE, 0u, Vector2::ZERO, 0u, Device::Class::NONE, Device::Subclass::NONE);

  DALI_TEST_EQUALS(event.GetType(), MouseInOutEvent::Type::NONE, TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent04(void)
{
  // Test copy constructor
  Vector2         point(30.0f, 40.0f);
  MouseInOutEvent original(MouseInOutEvent::Type::IN, 0x02u, point, 500u, Device::Class::TOUCH, Device::Subclass::FINGER);
  MouseInOutEvent copy(original);

  DALI_TEST_EQUALS(copy.GetType(), original.GetType(), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetModifiers(), original.GetModifiers(), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetPoint(), original.GetPoint(), 0.001f, TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetTimeStamp(), original.GetTimeStamp(), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetDeviceClass(), original.GetDeviceClass(), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetDeviceSubclass(), original.GetDeviceSubclass(), TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent05(void)
{
  // Test copy assignment operator
  MouseInOutEvent event1(MouseInOutEvent::Type::IN, 0x03u, Vector2(10.0f, 20.0f), 100u, Device::Class::MOUSE, Device::Subclass::NONE);
  MouseInOutEvent event2(MouseInOutEvent::Type::OUT, 0u, Vector2::ZERO, 0u, Device::Class::NONE, Device::Subclass::NONE);

  event2 = event1;

  DALI_TEST_EQUALS(event2.GetType(), MouseInOutEvent::Type::IN, TEST_LOCATION);
  DALI_TEST_EQUALS(event2.GetModifiers(), 0x03u, TEST_LOCATION);
  DALI_TEST_EQUALS(event2.GetPoint(), Vector2(10.0f, 20.0f), 0.001f, TEST_LOCATION);
  DALI_TEST_EQUALS(event2.GetTimeStamp(), 100u, TEST_LOCATION);
  DALI_TEST_EQUALS(event2.GetDeviceClass(), Device::Class::MOUSE, TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent06(void)
{
  // Test self-assignment
  MouseInOutEvent event(MouseInOutEvent::Type::IN, 0xFFu, Vector2(1.0f, 2.0f), 42u, Device::Class::NONE, Device::Subclass::NONE);
  event = event;

  DALI_TEST_EQUALS(event.GetType(), MouseInOutEvent::Type::IN, TEST_LOCATION);
  DALI_TEST_EQUALS(event.GetModifiers(), 0xFFu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent07(void)
{
  // Test GetDeviceClass with various device classes
  MouseInOutEvent eventMouse(MouseInOutEvent::Type::IN, 0u, Vector2::ZERO, 0u, Device::Class::MOUSE, Device::Subclass::NONE);
  MouseInOutEvent eventTouch(MouseInOutEvent::Type::IN, 0u, Vector2::ZERO, 0u, Device::Class::TOUCH, Device::Subclass::NONE);
  MouseInOutEvent eventKeyboard(MouseInOutEvent::Type::IN, 0u, Vector2::ZERO, 0u, Device::Class::KEYBOARD, Device::Subclass::NONE);

  DALI_TEST_EQUALS(eventMouse.GetDeviceClass(), Device::Class::MOUSE, TEST_LOCATION);
  DALI_TEST_EQUALS(eventTouch.GetDeviceClass(), Device::Class::TOUCH, TEST_LOCATION);
  DALI_TEST_EQUALS(eventKeyboard.GetDeviceClass(), Device::Class::KEYBOARD, TEST_LOCATION);

  END_TEST;
}

int UtcDaliMouseInOutEvent08(void)
{
  // Test GetDeviceSubclass with various subclasses
  MouseInOutEvent eventFinger(MouseInOutEvent::Type::IN, 0u, Vector2::ZERO, 0u, Device::Class::TOUCH, Device::Subclass::FINGER);
  MouseInOutEvent eventStylus(MouseInOutEvent::Type::IN, 0u, Vector2::ZERO, 0u, Device::Class::TOUCH, Device::Subclass::PEN_TIP);

  DALI_TEST_EQUALS(eventFinger.GetDeviceSubclass(), Device::Subclass::FINGER, TEST_LOCATION);
  DALI_TEST_EQUALS(eventStylus.GetDeviceSubclass(), Device::Subclass::PEN_TIP, TEST_LOCATION);

  END_TEST;
}
