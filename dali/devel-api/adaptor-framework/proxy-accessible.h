#ifndef DALI_ADAPTOR_PROXY_ACCESSIBLE_H
#define DALI_ADAPTOR_PROXY_ACCESSIBLE_H

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

// EXTERNAL INCLUDES
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-integ.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief The minimalistic, always empty Accessible object with settable Dali::Devel::Accessibility::Address.
 *
 * To be used as a proxy object, in those situations where you want to return an Dali::Devel::Accessibility::Address in
 * a different bridge (embedding for example), but the object itself isn't planned to be used
 * otherwise. This object has a settable parent, no children, an empty name and so on.
 */
class DALI_ADAPTOR_API ProxyAccessible : public Accessible
{
public:
  ProxyAccessible()
  : mAddress{},
    mParent{nullptr}
  {
  }

  void SetAddress(Dali::Devel::Accessibility::Address address)
  {
    mAddress = std::move(address);
  }

  void SetParent(Accessible* parent)
  {
    mParent = parent;
  }

  std::string GetName() const override
  {
    return "";
  }

  std::string GetDescription() const override
  {
    return "";
  }

  std::string GetValue() const override
  {
    return "";
  }

  Accessible* GetParent() override
  {
    return mParent;
  }

  size_t GetChildCount() const override
  {
    return 0;
  }

  std::vector<Accessible*> GetChildren() override
  {
    return {};
  }

  Accessible* GetChildAtIndex(size_t index) override
  {
    throw std::domain_error{"out of bounds index (" + std::to_string(index) + ") - no children"};
  }

  size_t GetIndexInParent() override
  {
    return static_cast<size_t>(-1);
  }

  std::string GetStringProperty(std::string propertyName) const override
  {
    return {};
  }

  Dali::Integration::Accessibility::Role GetRole() const override
  {
    return {};
  }

  std::string GetRoleName() const override
  {
    return {};
  }

  Dali::Integration::Accessibility::States GetStates() override
  {
    return {};
  }

  Dali::Devel::Accessibility::Attributes GetAttributes() const override
  {
    return {};
  }

  bool IsProxy() const override
  {
    return true;
  }

  Dali::Devel::Accessibility::Address GetAddress() const override
  {
    return mAddress;
  }

  bool DoGesture(const Dali::Devel::Accessibility::GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Dali::Devel::Accessibility::Relation> GetRelationSet() override
  {
    return {};
  }

  Dali::Actor GetInternalActor() const override
  {
    return Dali::Actor{};
  }

  Bounds GetExtents(Dali::Devel::Accessibility::CoordinateType type) const override
  {
    return mParent ? mParent->GetExtents(type) : Bounds{};
  }

  Dali::Devel::Accessibility::ComponentLayer GetLayer() const override
  {
    return Dali::Devel::Accessibility::ComponentLayer::WINDOW;
  }

  int16_t GetMdiZOrder() const override
  {
    return false;
  }

  bool GrabFocus() override
  {
    return false;
  }

  double GetAlpha() const override
  {
    return 0.0;
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

private:
  Dali::Devel::Accessibility::Address     mAddress;
  Accessible* mParent;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_PROXY_ACCESSIBLE_H
