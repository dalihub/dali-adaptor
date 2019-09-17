#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TEXT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TEXT_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/accessibility/bridge/bridge-base.h>

class BridgeText : public virtual BridgeBase
{
protected:
  BridgeText() = default;

  void RegisterInterfaces();

  Dali::Accessibility::Text* FindSelf() const;

public:
  DBus::ValueOrError< std::string > GetText( int startOffset, int endOffset );
  DBus::ValueOrError< int32_t > GetCharacterCount();
  DBus::ValueOrError< int32_t > GetCaretOffset();
  DBus::ValueOrError< bool > SetCaretOffset( int32_t offset );
  DBus::ValueOrError< std::string, int, int > GetTextAtOffset( int32_t offset, uint32_t boundary );
  DBus::ValueOrError< int, int > GetSelection( int32_t selectionNum );
  DBus::ValueOrError< bool > RemoveSelection( int32_t selectionNum );
  DBus::ValueOrError< bool > SetSelection( int32_t selectionNum, int32_t startOffset, int32_t endOffset );
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TEXT_H
