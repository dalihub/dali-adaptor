#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COMPONENT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COMPONENT_H

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

// EXTERNAL INCLUDES
#include <array>
#include <string>
#include <tuple>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeComponent class is to correspond with Dali::Accessibility::Component.
 */
class BridgeComponent : public virtual BridgeBase
{
protected:
  /**
   * @brief Constructor.
   */
  BridgeComponent();

  /**
   * @brief Registers Component functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Component object of the currently executed DBus method call.
   *
   * @return The Component object
   */
  Dali::Accessibility::Component* FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Component::IsAccessibleContainingPoint()
   */
  DBus::ValueOrError<bool> IsAccessibleContainingPoint(int32_t x, int32_t y, uint32_t coordType);

  /**
   * @copydoc Dali::Accessibility::Component::GetAccessibleAtPoint()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetAccessibleAtPoint(int32_t x, int32_t y, uint32_t coordType);

  /**
   * @copydoc Dali::Accessibility::Component::GetExtents()
   */
  DBus::ValueOrError<std::tuple<int32_t, int32_t, int32_t, int32_t> > GetExtents(uint32_t coordType);

  /**
   * @brief Gets the position from the given coordinate.
   * @param[in] coordType The enumeration with type of coordinate systems
   * @return The X and Y position of rectangle
   */
  DBus::ValueOrError<int32_t, int32_t> GetPosition(uint32_t coordType);

  /**
   * @brief Gets the size from the given coordinate.
   * @param[in] coordType The enumeration with type of coordinate systems
   * @return The width and height of rectangle
   */
  DBus::ValueOrError<int32_t, int32_t> GetSize(uint32_t coordType);

  /**
   * @copydoc Dali::Accessibility::Component::GetLayer()
   */
  DBus::ValueOrError<Dali::Accessibility::ComponentLayer> GetLayer();

  /**
   * @copydoc Dali::Accessibility::Component::GetAlpha()
   */
  DBus::ValueOrError<double> GetAlpha();

  /**
   * @copydoc Dali::Accessibility::Component::GrabFocus()
   */
  DBus::ValueOrError<bool> GrabFocus();

  /**
   * @copydoc Dali::Accessibility::Component::GrabHighlight()
   */
  DBus::ValueOrError<bool> GrabHighlight();

  /**
   * @copydoc Dali::Accessibility::Component::ClearHighlight()
   */
  DBus::ValueOrError<bool> ClearHighlight();

  /**
   * @copydoc Dali::Accessibility::Component::GetMdiZOrder()
   */
  DBus::ValueOrError<int16_t> GetMdiZOrder();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COMPONENT_H
