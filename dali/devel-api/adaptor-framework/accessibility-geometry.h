#ifndef DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_GEOMETRY_H
#define DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_GEOMETRY_H
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

namespace Dali
{
namespace Devel
{
namespace Accessibility
{

/**
 * @brief Enumeration describing if coordinates are relative to screen or window.
 */
enum class CoordinateType
{
  SCREEN, ///< Screen.
  WINDOW  ///< Window.
};

/**
 * @brief Enumeration indicating relative stacking order.
 */
enum class ComponentLayer
{
  INVALID,    ///< Invalid.
  BACKGROUND, ///< Background.
  CANVAS,     ///< Canvas.
  WIDGET,     ///< Widget.
  MDI,        ///< MDI.
  POPUP,      ///< Popup.
  OVERLAY,    ///< Overlay.
  WINDOW,     ///< Window.
  MAX_COUNT
};

/**
 * @brief Helper class representing two dimensional point with integer coordinates.
 */
struct DALI_ADAPTOR_API Point
{
  int x = 0;
  int y = 0;

  Point() = default;

  Point(int x, int y)
  : x(x),
    y(y)
  {
  }

  bool operator==(Point p) const
  {
    return x == p.x && y == p.y;
  }
  bool operator!=(Point p) const
  {
    return !(*this == p);
  }
};

/**
 * @brief Helper class representing size of rectangle object with usage of two integer values.
 */
struct DALI_ADAPTOR_API Size
{
  int width  = 0;
  int height = 0;

  Size() = default;

  Size(int w, int h)
  : width(w),
    height(h)
  {
  }

  bool operator==(Size p) const
  {
    return width == p.width && height == p.height;
  }

  bool operator!=(Size p) const
  {
    return !(*this == p);
  }
};

} // namespace Accessibility
} // namespace Devel
} // namespace Dali

#endif // DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_GEOMETRY_H
