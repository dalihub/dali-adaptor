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

namespace Dali::Accessibility
{
/**
 * @brief The minimalistic, always empty Accessible object with settable address.
 *
 * To be used as a proxy object, in those situations where you want to return an address in
 * a different bridge (embedding for example), but the object itself isn't planned to be used
 * otherwise. This object has no parent, no children, an empty name and so on.
 */
class DALI_ADAPTOR_API ProxyAccessible : public virtual Accessible
{
public:
  ProxyAccessible() = default;

  ProxyAccessible(Address address)
  : mAddress(std::move(address))
  {
  }

  void SetAddress(Address address)
  {
    this->mAddress = std::move(address);
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
    return nullptr;
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

  Dali::Actor GetInternalActor() override
  {
    return Dali::Actor{};
  }

private:
  Address mAddress;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_PROXY_ACCESSIBLE_H
