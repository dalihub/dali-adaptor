#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TEXT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TEXT_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/text.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeText class is to correspond with Dali::Accessibility::Text.
 */
class BridgeText : public virtual BridgeBase
{
protected:
  BridgeText() = default;

  /**
   * @brief Registers Text functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Text object of the currently executed DBus method call.
   *
   * @return The Text object
   */
  Dali::Accessibility::Text* FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Text::GetText()
   */
  DBus::ValueOrError<std::string> GetText(int startOffset, int endOffset);

  /**
   * @copydoc Dali::Accessibility::Text::GetCharacterCount()
   */
  DBus::ValueOrError<int32_t> GetCharacterCount();

  /**
   * @copydoc Dali::Accessibility::Text::GetCursorOffset()
   */
  DBus::ValueOrError<int32_t> GetCursorOffset();

  /**
   * @copydoc Dali::Accessibility::Text::SetCursorOffset()
   */
  DBus::ValueOrError<bool> SetCursorOffset(int32_t offset);

  /**
   * @copydoc Dali::Accessibility::Text::GetTextAtOffset()
   */
  DBus::ValueOrError<std::string, int, int> GetTextAtOffset(int32_t offset, uint32_t boundary);

  /**
   * @copydoc Dali::Accessibility::Text::GetRangeOfSelection()
   */
  DBus::ValueOrError<int, int> GetRangeOfSelection(int32_t selectionIndex);

  /**
   * @copydoc Dali::Accessibility::Text::RemoveSelection()
   */
  DBus::ValueOrError<bool> RemoveSelection(int32_t selectionIndex);

  /**
   * @copydoc Dali::Accessibility::Text::SetRangeOfSelection()
   */
  DBus::ValueOrError<bool> SetRangeOfSelection(int32_t selectionIndex, int32_t startOffset, int32_t endOffset);

  /**
   * @copydoc Dali::Accessibility::Text::GetRangeExtents()
   */
  DBus::ValueOrError<int32_t, int32_t, int32_t, int32_t> GetRangeExtents(int32_t startOffset, int32_t endOffset, uint32_t coordType);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TEXT_H
