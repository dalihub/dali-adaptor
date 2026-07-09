#ifndef DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_TYPES_H
#define DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_TYPES_H
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

#include <dali/integration-api/adaptor-framework/accessibility/accessibility-integ.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Dali
{
namespace Accessibility
{
class Accessible;
} // namespace Accessibility

namespace Devel
{
namespace Accessibility
{

using Dali::Accessibility::Accessible;
using Dali::Integration::Accessibility::RelationType;

using Attributes = std::unordered_map<std::string, std::string>;

/**
 * @brief Class representing unique object address on accessibility bus.
 */
class DALI_ADAPTOR_API Address
{
public:
  Address() = default;

  Address(std::string bus, std::string path)
  : mBus(std::move(bus)),
    mPath(std::move(path))
  {
  }

  explicit operator bool() const
  {
    return !mPath.empty();
  }

  std::string ToString() const
  {
    return *this ? mBus + ":" + mPath : "::null";
  }

  const std::string& GetBus() const;

  const std::string& GetPath() const
  {
    return mPath;
  }

  bool operator==(const Address& a) const
  {
    return mBus == a.mBus && mPath == a.mPath;
  }

  bool operator!=(const Address& a) const
  {
    return !(*this == a);
  }

private:
  mutable std::string mBus, mPath;
};

/**
 * @brief Class representing accessibility relations.
 */
struct DALI_ADAPTOR_API Relation
{
  Relation(Dali::Integration::Accessibility::RelationType relationType, const std::vector<Dali::Accessibility::Accessible*>& targets)
  : mRelationType(relationType),
    mTargets(targets)
  {
  }

  Dali::Integration::Accessibility::RelationType mRelationType;
  std::vector<Dali::Accessibility::Accessible*> mTargets;
};

} // namespace Accessibility
} // namespace Devel
} // namespace Dali

#endif // DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_TYPES_H
