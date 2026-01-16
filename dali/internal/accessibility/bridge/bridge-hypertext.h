#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_HYPERTEXT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_HYPERTEXT_H

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
#include <dali/devel-api/atspi-interfaces/hypertext.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

class BridgeHypertext : public virtual BridgeBase
{
protected:
  BridgeHypertext() = default;

  /**
   * @brief Registers Hypertext functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Hypertext object of the currently executed DBus method call.
   *
   * @return The Hypertext object
   */
  std::shared_ptr<Dali::Accessibility::Hypertext> FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Hypertext::GetLink()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetLink(int32_t linkIndex);

  /**
   * @copydoc Dali::Accessibility::Hypertext::GetLinkIndex()
   */
  DBus::ValueOrError<int32_t> GetLinkIndex(int32_t characterOffset);

  /**
   * @copydoc Dali::Accessibility::Hypertext::GetLinkCount()
   */
  DBus::ValueOrError<int32_t> GetLinkCount();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_HYPERTEXT_H