#ifndef DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_TEXT_H
#define DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_TEXT_H
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

#include <dali/public-api/dali-adaptor-common.h>
#include <cstdint>
#include <cstddef>
#include <string>
#include <utility>

namespace Dali
{
namespace Devel
{
namespace Accessibility
{

/**
 * @brief Enumeration used to acquire bounded text from accessible object having textual content.
 */
enum class TextBoundary : uint32_t
{
  CHARACTER, ///< Only one character is acquired.
  WORD,      ///< Not supported.
  SENTENCE,  ///< Not supported.
  LINE,      ///< Not supported.
  PARAGRAPH, ///< Not supported.
  MAX_COUNT
};

/**
 * @brief Helper class used to store data related with Accessibility::Text interface.
 */
struct DALI_ADAPTOR_API Range
{
  int32_t     startOffset = 0;
  int32_t     endOffset   = 0;
  std::string content;

  Range() = default;

  Range(size_t start, size_t end)
  : startOffset(start),
    endOffset(end)
  {
  }

  Range(size_t start, size_t end, std::string content)
  : startOffset(start),
    endOffset(end),
    content(std::move(content))
  {
  }
};

} // namespace Accessibility
} // namespace Devel
} // namespace Dali

#endif // DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_TEXT_H
