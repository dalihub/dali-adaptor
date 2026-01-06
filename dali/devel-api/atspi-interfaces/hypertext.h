#ifndef DALI_ADAPTOR_ATSPI_HYPERTEXT_H
#define DALI_ADAPTOR_ATSPI_HYPERTEXT_H

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
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/atspi-interfaces/hyperlink.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface representing hypertext that can store a collection of hyperlinks.
 */
class DALI_ADAPTOR_API Hypertext
{
public:
  /**
   * @brief Gets the handle to hyperlink object from a specified index in hyperlink collection of this hypertext.
   *
   * @param[in] linkIndex The 0-based index in hyperlink collection.
   *
   * @return Handle to hyperlink object at a specified index in hyperlink collection of hypertext.
   */
  virtual Hyperlink* GetLink(std::int32_t linkIndex) const;

  /**
   * @brief Gets the index in hyperlink collection occupied by hyperlink which spans over a specified character offset in this hypertext.
   *
   * @param[in] characterOffset The 0-based index of character in hypertext.
   *
   * @return The value of 0-based index in hyperlink collection (-1 if there is no hyperlink at the specified character offset).
   */
  virtual std::int32_t GetLinkIndex(std::int32_t characterOffset) const;

  /**
   * @brief Gets number of hyperlinks stored in this hypertext.
   *
   * @return The number of hyperlinks (zero if none or -1 if the number cannot be determined)
   */
  virtual std::int32_t GetLinkCount() const;
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::HYPERTEXT>
{
  using Type = Hypertext;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_HYPERTEXT_H
