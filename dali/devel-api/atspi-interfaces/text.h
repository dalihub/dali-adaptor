#ifndef DALI_ADAPTOR_ATSPI_TEXT_H
#define DALI_ADAPTOR_ATSPI_TEXT_H

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
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface representing objects which can store immutable texts.
 *
 * @see Dali::Accessibility::EditableText
 */
class DALI_ADAPTOR_API Text : public virtual Accessible
{
public:
  /**
   * @brief Gets stored text in given range.
   *
   * @param[in] startOffset The index of first character
   * @param[in] endOffset The index of first character after the last one expected
   *
   * @return The substring of stored text
   */
  virtual std::string GetText(size_t startOffset, size_t endOffset) = 0;

  /**
   * @brief Gets number of all stored characters.
   *
   * @return The number of characters
   * @remarks This method is `CharacterCount` in DBus method.
   */
  virtual size_t GetCharacterCount() = 0;

  /**
   * @brief Gets the cursor offset.
   *
   * @return Value of cursor offset
   * @remarks This method is `CaretOffset` in DBus method.
   */
  virtual size_t GetCursorOffset() = 0;

  /**
   * @brief Sets the cursor offset.
   *
   * @param[in] offset Cursor offset
   *
   * @return True if successful
   * @remarks This method is `SetCaretOffset` in DBus method.
   */
  virtual bool SetCursorOffset(size_t offset) = 0;

  /**
   * @brief Gets substring of stored text truncated in concrete gradation.
   *
   * @param[in] offset The position in stored text
   * @param[in] boundary The enumeration describing text gradation
   *
   * @return Range structure containing acquired text and offsets in original string
   *
   * @see Dali::Accessibility::Range
   */
  virtual Range GetTextAtOffset(size_t offset, TextBoundary boundary) = 0;

  /**
   * @brief Gets selected text.
   *
   * @param[in] selectionIndex The selection index
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return Range structure containing acquired text and offsets in original string
   *
   * @remarks This method is `GetSelection` in DBus method.
   * @see Dali::Accessibility::Range
   */
  virtual Range GetRangeOfSelection(size_t selectionIndex) = 0;

  /**
   * @brief Removes the whole selection.
   *
   * @param[in] selectionIndex The selection index
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return bool on success, false otherwise
   */
  virtual bool RemoveSelection(size_t selectionIndex) = 0;

  /**
   * @brief Sets selected text.
   *
   * @param[in] selectionIndex The selection index
   * @param[in] startOffset The index of first character
   * @param[in] endOffset The index of first character after the last one expected
   *
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return true on success, false otherwise
   * @remarks This method is `SetSelection` in DBus method.
   */
  virtual bool SetRangeOfSelection(size_t selectionIndex, size_t startOffset, size_t endOffset) = 0;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_TEXT_H
