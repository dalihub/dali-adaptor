#ifndef DALI_ADAPTOR_ATSPI_EDITABLE_TEXT_H
#define DALI_ADAPTOR_ATSPI_EDITABLE_TEXT_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface representing objects which can store editable texts.
 *
 * @note Paste method is entirely implemented inside bridge
 *
 * @see Dali::Accessibility::EditableText
 */
class DALI_ADAPTOR_API EditableText : public virtual Accessible
{
public:
  /**
   * @brief Copies text in range to system clipboard.
   *
   * @param[in] startPosition The index of first character
   * @param[in] endPosition The index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool CopyText(size_t startPosition, size_t endPosition) = 0;

  /**
   * @brief Cuts text in range to system clipboard.
   *
   * @param[in] startPosition The index of first character
   * @param[in] endPosition The index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool CutText(size_t startPosition, size_t endPosition) = 0;

  /**
   * @brief Deletes text in range.
   *
   * @param[in] startPosition The index of first character
   * @param[in] endPosition The index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool DeleteText(size_t startPosition, size_t endPosition) = 0;

  /**
   * @brief Inserts text at startPosition.
   *
   * @param[in] startPosition The index of first character
   * @param[in] text The text content
   *
   * @return true on success, false otherwise
   */
  virtual bool InsertText(size_t startPosition, std::string text) = 0;

  /**
   * @brief Replaces text with content.
   *
   * @param[in] newContents The text content
   *
   * @return true on success, false otherwise
   */
  virtual bool SetTextContents(std::string newContents) = 0;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_EDITABLE_TEXT_H
