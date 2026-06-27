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
#include <dali/devel-api/adaptor-framework/accessibility-devel.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-bridge.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-service.h>
#include <dali/public-api/math/rect.h>
#include <algorithm>
#include <string>
#include <vector>

using namespace Dali;
using namespace Dali::Accessibility;
using namespace Dali::Devel::Accessibility;
using namespace Dali::Integration::Accessibility;

void utc_dali_accessibility_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_accessibility_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
class TestAccessible : public Accessible
{
public:
  explicit TestAccessible(Role role, Bounds bounds = Bounds{0.0f, 0.0f, 100.0f, 100.0f})
  : mRole(role),
    mBounds(bounds)
  {
  }

  std::string GetName() const override
  {
    return "test-accessible";
  }

  std::string GetDescription() const override
  {
    return "test description";
  }

  std::string GetValue() const override
  {
    return "test value";
  }

  Accessible* GetParent() override
  {
    return mParent;
  }

  std::size_t GetChildCount() const override
  {
    return mChildren.size();
  }

  std::vector<Accessible*> GetChildren() override
  {
    return mChildren;
  }

  Accessible* GetChildAtIndex(std::size_t index) override
  {
    return mChildren.at(index);
  }

  std::size_t GetIndexInParent() override
  {
    return 0u;
  }

  Role GetRole() const override
  {
    return mRole;
  }

  States GetStates() override
  {
    States states;
    states[State::ENABLED] = true;
    states[State::VISIBLE] = true;
    return states;
  }

  Attributes GetAttributes() const override
  {
    return {{"automationId", "accessibility-test"}};
  }

  Address GetAddress() const override
  {
    return {"org.test.Accessibility", "/org/test/Accessibility"};
  }

  bool DoGesture(const GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Relation> GetRelationSet() override
  {
    return {};
  }

  Actor GetInternalActor() const override
  {
    return {};
  }

  std::string GetStringProperty(std::string propertyName) const override
  {
    return propertyName == "automationId" ? "accessibility-test" : "";
  }

  Bounds GetExtents(CoordinateType type) const override
  {
    return mBounds;
  }

  ComponentLayer GetLayer() const override
  {
    return ComponentLayer::WINDOW;
  }

  int16_t GetMdiZOrder() const override
  {
    return 0;
  }

  bool GrabFocus() override
  {
    return false;
  }

  double GetAlpha() const override
  {
    return 1.0;
  }

  bool GrabHighlight() override
  {
    return false;
  }

  bool ClearHighlight() override
  {
    return false;
  }

  bool IsScrollable() const override
  {
    return false;
  }

  void InitDefaultFeatures() override
  {
  }

  void AddChild(TestAccessible& child)
  {
    child.mParent = this;
    mChildren.push_back(&child);
  }

private:
  Role                     mRole;
  Bounds                   mBounds;
  Accessible*              mParent{nullptr};
  std::vector<Accessible*> mChildren;
};

bool ContainsString(const std::vector<std::string>& values, const std::string& expected)
{
  return std::find(values.begin(), values.end(), expected) != values.end();
}
} // namespace

int UtcDaliAccessibilityServiceWithoutActiveBridgeP(void)
{
  auto bridge = Bridge::GetCurrentBridge();
  if(bridge)
  {
    bridge->ForceDown();
  }

  DALI_TEST_CHECK(!IsEnabled());
  DALI_TEST_CHECK(!IsScreenReaderEnabled());

  Pause();
  Resume();
  StopReading();
  Say("ignored without active bridge", true, [](std::string) {});

  END_TEST;
}

int UtcDaliAccessibilityBitSetP(void)
{
  States states;
  DALI_TEST_CHECK(!static_cast<bool>(states));

  states[State::CHECKED] = true;
  states[State::ENABLED] = true;

  DALI_TEST_CHECK(states[State::CHECKED]);
  DALI_TEST_CHECK(states[State::ENABLED]);
  DALI_TEST_CHECK(!states[State::BUSY]);

  States roundTrip(states.GetRawData());
  DALI_TEST_CHECK(roundTrip == states);

  States busy;
  busy[State::BUSY] = true;
  auto combined     = states | busy;
  DALI_TEST_CHECK(combined[State::CHECKED]);
  DALI_TEST_CHECK(combined[State::BUSY]);

  RoleSet roles;
  roles[Role::PUSH_BUTTON] = true;
  roles[Role::CHECK_BOX]   = true;
  DALI_TEST_CHECK(roles[Role::PUSH_BUTTON]);
  DALI_TEST_CHECK(roles[Role::CHECK_BOX]);
  DALI_TEST_CHECK(!roles[Role::SLIDER]);

  END_TEST;
}

int UtcDaliAccessibilityTypesP(void)
{
  Address emptyAddress;
  DALI_TEST_CHECK(!emptyAddress);
  DALI_TEST_EQUALS(emptyAddress.ToString(), std::string("::null"), TEST_LOCATION);

  Address address{"org.test.Accessibility", "/org/test/Accessibility"};
  DALI_TEST_CHECK(address);
  DALI_TEST_EQUALS(address.GetBus(), std::string("org.test.Accessibility"), TEST_LOCATION);
  DALI_TEST_EQUALS(address.GetPath(), std::string("/org/test/Accessibility"), TEST_LOCATION);
  DALI_TEST_EQUALS(address.ToString(), std::string("org.test.Accessibility:/org/test/Accessibility"), TEST_LOCATION);

  Point point{10, 20};
  DALI_TEST_CHECK(point == Point(10, 20));
  DALI_TEST_CHECK(point != Point(20, 10));

  Dali::Devel::Accessibility::Size size{30, 40};
  DALI_TEST_CHECK(size == Dali::Devel::Accessibility::Size(30, 40));
  DALI_TEST_CHECK(size != Dali::Devel::Accessibility::Size(40, 30));

  TestAccessible target(Role::LABEL);
  Relation       relation(RelationType::LABELLED_BY, {&target});
  DALI_TEST_EQUALS(relation.mRelationType, RelationType::LABELLED_BY, TEST_LOCATION);
  DALI_TEST_EQUALS(relation.mTargets.size(), 1u, TEST_LOCATION);
  DALI_TEST_CHECK(relation.mTargets[0] == &target);

  END_TEST;
}

int UtcDaliAccessibilityAccessibleDefaultsP(void)
{
  TestAccessible root(Role::PUSH_BUTTON, Bounds{0.0f, 0.0f, 100.0f, 100.0f});
  TestAccessible child(Role::LABEL, Bounds{10.0f, 10.0f, 20.0f, 20.0f});
  root.AddChild(child);

  DALI_TEST_EQUALS(root.GetRoleName(), std::string("push button"), TEST_LOCATION);
  DALI_TEST_EQUALS(root.GetLocalizedRoleName(), root.GetRoleName(), TEST_LOCATION);
  DALI_TEST_CHECK(!root.IsHidden());
  DALI_TEST_CHECK(!root.IsProxy());

  auto interfaces = root.GetInterfaces();
  DALI_TEST_CHECK(interfaces[AccessibilityInterface::ACCESSIBLE]);
  DALI_TEST_CHECK(interfaces[AccessibilityInterface::COMPONENT]);
  DALI_TEST_CHECK(!interfaces[AccessibilityInterface::ACTION]);

  auto interfaceNames = root.GetInterfacesAsStrings();
  DALI_TEST_CHECK(ContainsString(interfaceNames, "org.a11y.atspi.Accessible"));
  DALI_TEST_CHECK(ContainsString(interfaceNames, "org.a11y.atspi.Component"));

  DALI_TEST_CHECK(root.IsAccessibleContainingPoint(Point{5, 5}, CoordinateType::WINDOW));
  DALI_TEST_CHECK(!root.IsAccessibleContainingPoint(Point{150, 5}, CoordinateType::WINDOW));
  DALI_TEST_CHECK(root.GetAccessibleAtPoint(Point{15, 15}, CoordinateType::WINDOW) == &child);
  DALI_TEST_CHECK(root.GetAccessibleAtPoint(Point{90, 90}, CoordinateType::WINDOW) == nullptr);

  END_TEST;
}
