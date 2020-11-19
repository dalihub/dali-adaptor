#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_EDITABLE_TEXT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_EDITABLE_TEXT_H

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

class BridgeEditableText : public virtual BridgeBase
{
protected:
  BridgeEditableText() = default;

  void RegisterInterfaces();

  Dali::Accessibility::EditableText* FindSelf() const;

public:
  DBus::ValueOrError< bool > CopyText( int32_t startPos, int32_t endPos );
  DBus::ValueOrError< bool > CutText( int32_t startPos, int32_t endPos );
  DBus::ValueOrError< bool > PasteText( int32_t pos );
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_EDITABLE_TEXT_H
