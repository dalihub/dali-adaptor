#ifndef DALI_ADAPTOR_ATSPI_COMPONENT_H
#define DALI_ADAPTOR_ATSPI_COMPONENT_H

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
#include <dali/public-api/math/rect.h>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-geometry.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-integ.h>

namespace Dali::Accessibility
{
class Accessible;

/**
 * @brief Interface representing objects having screen coordinates.
 */
class DALI_ADAPTOR_API Component
{
public:
  /**
   * @brief Gets rectangle describing Dali::Devel::Accessibility::Size.
   *
   * @param[in] type The enumeration with type of coordinate systems
   *
   * @return Bounds object
   *
   * @see Dali::Bounds
   */
  virtual Bounds GetExtents(Dali::Devel::Accessibility::CoordinateType type) const = 0;

  /**
   * @brief Gets layer current object is localized on.
   *
   * @return The enumeration pointing layer
   *
   * @see Dali::Devel::Accessibility::ComponentLayer
   */
  virtual Dali::Devel::Accessibility::ComponentLayer GetLayer() const = 0;

  /**
   * @brief Gets value of z-order.
   *
   * @return The value of z-order
   * @remarks MDI means "Multi Document Interface" (https://en.wikipedia.org/wiki/Multiple-document_interface)
   * which in short means that many stacked windows can be displayed within a single application.
   * In such model, the concept of z-order of UI element became important to deal with element overlapping.
   */
  virtual int16_t GetMdiZOrder() const = 0;

  /**
   * @brief Sets current object as "focused".
   *
   * @return true on success, false otherwise
   */
  virtual bool GrabFocus() = 0;

  /**
   * @brief Gets value of alpha channel.
   *
   * @return The alpha channel value in Dali::Devel::Accessibility::Range [0.0, 1.0]
   */
  virtual double GetAlpha() const = 0;

  /**
   * @brief Sets current object as "highlighted".
   *
   * The method assings "highlighted" state, simultaneously removing it
   * from currently highlighted object.
   *
   * @return true on success, false otherwise
   */
  virtual bool GrabHighlight() = 0;

  /**
   * @brief Sets current object as "unhighlighted".
   *
   * The method removes "highlighted" state from object.
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Integration::Accessibility::State
   */
  virtual bool ClearHighlight() = 0;

  /**
   * @brief Checks whether object can be scrolled.
   *
   * @return true if object is scrollable, false otherwise
   *
   * @see Dali::Integration::Accessibility::State
   */
  virtual bool IsScrollable() const = 0;

  /**
   * @brief Gets Accessible object containing given Dali::Devel::Accessibility::Point.
   *
   * @param[in] point The two-dimensional Dali::Devel::Accessibility::Point
   * @param[in] type The enumeration with type of coordinate system
   *
   * @return The handle to last child of current object which contains given Dali::Devel::Accessibility::Point
   *
   * @see Dali::Devel::Accessibility::Point
   */
  virtual Accessible* GetAccessibleAtPoint(Dali::Devel::Accessibility::Point point, Dali::Devel::Accessibility::CoordinateType type) = 0;

  /**
   * @brief Checks if the current object contains the given Dali::Devel::Accessibility::Point inside.
   *
   * @param[in] point The two-dimensional Dali::Devel::Accessibility::Point
   * @param[in] type The enumeration with type of coordinate system
   *
   * @return True if accessible contains in point, otherwise false.
   *
   * @remarks This method is `Contains` in DBus method.
   * @see Dali::Devel::Accessibility::Point
   */
  virtual bool IsAccessibleContainingPoint(Dali::Devel::Accessibility::Point point, Dali::Devel::Accessibility::CoordinateType type) const = 0;
};

} // namespace Dali::Accessibility

namespace Dali::Integration::Accessibility
{
template<>
struct AccessibilityInterfaceTypeHelper<Dali::Integration::Accessibility::AccessibilityInterface::COMPONENT>
{
  using Type = Dali::Accessibility::Component;
};
} // namespace Dali::Integration::Accessibility

#endif // DALI_ADAPTOR_ATSPI_COMPONENT_H
