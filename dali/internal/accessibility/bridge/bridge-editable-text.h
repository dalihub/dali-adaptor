#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_EDITABLE_TEXT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_EDITABLE_TEXT_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/editable-text.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeEditableText class is to correspond with Dali::Accessibility::EditableText.
 */
class BridgeEditableText : public virtual BridgeBase
{
protected:
  BridgeEditableText() = default;

  /**
   * @brief Registers EditableText functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the EditableText object of the currently executed DBus method call.
   *
   * @return The EditableText object
   */
  std::shared_ptr<Dali::Accessibility::EditableText> FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::EditableText::CopyText()
   */
  DBus::ValueOrError<bool> CopyText(int32_t startPosition, int32_t endPosition);

  /**
   * @copydoc Dali::Accessibility::EditableText::CutText()
   */
  DBus::ValueOrError<bool> CutText(int32_t startPosition, int32_t endPosition);

  /**
   * @copydoc Dali::Accessibility::EditableText::DeleteText()
   */
  DBus::ValueOrError<bool> DeleteText(int32_t startPosition, int32_t endPosition);

  /**
   * @copydoc Dali::Accessibility::EditableText::InsertText()
   */
  DBus::ValueOrError<bool> InsertText(int32_t startPosition, std::string text, int32_t length);

  /**
   * @brief Pastes text at position.
   *
   * @param[in] position The text position
   * @return True on success, false otherwise
   * @note Currently, this function is not implemented yet.
   */
  DBus::ValueOrError<bool> PasteText(int32_t position);

  /**
   * @copydoc Dali::Accessibility::EditableText::SetTextContents()
   */
  DBus::ValueOrError<bool> SetTextContents(std::string newContents);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_EDITABLE_TEXT_H
