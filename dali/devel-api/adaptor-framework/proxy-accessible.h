#ifndef DALI_ADAPTOR_PROXY_ACCESSIBLE_H
#define DALI_ADAPTOR_PROXY_ACCESSIBLE_H

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
 */

// EXTERNAL INCLUDES
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/component.h>

namespace Dali::Accessibility
{
/**
 * @brief The minimalistic, always empty Accessible object with settable address.
 *
 * To be used as a proxy object, in those situations where you want to return an address in
 * a different bridge (embedding for example), but the object itself isn't planned to be used
 * otherwise. This object has a settable parent, no children, an empty name and so on.
 */
class DALI_ADAPTOR_API ProxyAccessible : public virtual Accessible, public virtual Component
{
public:
  ProxyAccessible()
  : mAddress{},
    mParent{nullptr}
  {
  }

  void SetAddress(Address address)
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

  Role GetRole() const override
  {
    return {};
  }

  std::string GetRoleName() const override
  {
    return {};
  }

  States GetStates() override
  {
    return {};
  }

  Attributes GetAttributes() const override
  {
    return {};
  }

  bool IsProxy() const override
  {
    return true;
  }

  Address GetAddress() const override
  {
    return mAddress;
  }

  bool DoGesture(const GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Relation> GetRelationSet() override
  {
    return {};
  }

  Dali::Actor GetInternalActor() const override
  {
    return Dali::Actor{};
  }

  Rect<> GetExtents(CoordinateType type) const override
  {
    auto* parent = Component::DownCast(mParent);

    return parent ? parent->GetExtents(type) : Rect<>{};
  }

  ComponentLayer GetLayer() const override
  {
    return ComponentLayer::WINDOW;
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

private:
  Address     mAddress;
  Accessible* mParent;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_PROXY_ACCESSIBLE_H
