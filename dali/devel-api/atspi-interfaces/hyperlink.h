#ifndef DALI_ADAPTOR_ATSPI_HYPERLINK_H
#define DALI_ADAPTOR_ATSPI_HYPERLINK_H

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
#include <cstdint>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface representing a hyperlink in hypertext .
 */
class Hyperlink : public virtual Accessible
{
public:
  /**
   * @brief Gets the index of character in originating hypertext at which this hyperlink ends.
   *
   * @return The 0-based index of hyperlink's last character + 1, in its originating hypertext.
   */
  virtual std::int32_t GetEndIndex() const = 0;

  /**
   * @brief Gets the index of character in originating hypertext at which this hyperlink starts.
   *
   * @return The 0-based index of hyperlink's first character, in its originating hypertext.
   */
  virtual std::int32_t GetStartIndex() const = 0;

  /**
   * @brief Gets the total number of anchors which this hyperlink has. Though, typical hyperlinks will have only one anchor.
   *
   * @return The number of anchors.
   */
  virtual std::int32_t GetAnchorCount() const = 0;

  /**
   * @brief Gets the object associated with a particular hyperlink's anchor.
   *
   * @param[in] anchorIndex The 0-based index in anchor collection.
   *
   * @return The handle to accessible object.
   */
  virtual Accessible* GetAnchorAccessible(std::int32_t anchorIndex) const = 0;

  /**
   * @brief Gets the URI associated with a particular hyperlink's anchor.
   *
   * @param[in] anchorIndex The 0-based index in anchor collection.
   *
   * @return The string containing URI.
   */
  virtual std::string GetAnchorUri(std::int32_t anchorIndex) const = 0;

  /**
   * @brief Tells whether this hyperlink object is still valid with respect to its originating hypertext object.
   *
   * @return True if hyperlink object is valid, false otherwise
   */
  virtual bool IsValid() const = 0;

  /**
   * @brief Downcasts an Accessible to a Hyperlink.
   *
   * @param obj The Accessible
   * @return A Hyperlink or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Hyperlink* DownCast(Accessible* obj);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::HYPERLINK>
{
  using Type = Hyperlink;
};
} // namespace Internal

inline Hyperlink* Hyperlink::DownCast(Accessible* obj)
{
  return Accessible::DownCast<AtspiInterface::HYPERLINK>(obj);
}

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_HYPERLINK_H
