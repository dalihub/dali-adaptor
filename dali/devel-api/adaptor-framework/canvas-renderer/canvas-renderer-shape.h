#ifndef DALI_CANVAS_RENDERER_SHAPE_H
#define DALI_CANVAS_RENDERER_SHAPE_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class CanvasRenderer;
class Shape;
} // namespace Adaptor
} //namespace Internal DALI_INTERNAL

/**
 * @brief Shape is a command list for drawing one shape groups
 * It has own path data & properties for sync/asynchronous drawing
 */
class DALI_ADAPTOR_API CanvasRenderer::Shape : public CanvasRenderer::Drawable
{
public:
  /**
   * @brief Creates an initialized handle to a new CanvasRenderer::Shape.
   *
   * @return A handle to a newly allocated Shape
   */
  static Shape New();

public:
  /**
   * @brief Creates an empty handle.
   * Use CanvasRenderer::Shape::New() to create an initialized object.
   */
  Shape();

  /**
   * @brief Destructor.
   */
  ~Shape();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  Shape(const Shape& handle) = default;

public:
  /**
   * @brief Enumeration for The cap style to be used for stroking the path.
   */
  enum class StrokeCap
  {
    SQUARE = 0, ///< The end of lines is rendered as a square around the last point.
    ROUND,      ///< The end of lines is rendered as a half-circle around the last point.
    BUTT        ///< The end of lines is rendered as a full stop on the last point itself.
  };

  /**
   * @brief Enumeration for The join style to be used for stroking the path.
   */
  enum class StrokeJoin
  {
    BEVEL = 0, ///< Used to render beveled line joins. The outer corner of the joined lines is filled by enclosing the triangular region of the corner with a straight line between the outer corners of each stroke.
    ROUND,     ///< Used to render rounded line joins. Circular arcs are used to join two lines smoothly.
    MITER      ///< Used to render mitered line joins. The intersection of the strokes is clipped at a line perpendicular to the bisector of the angle between the strokes, at the distance from the intersection of the segments equal to the product of the miter limit value and the border radius.  This prevents long spikes being created.
  };

  /**
   * @brief Enumeration for The fill rule of shape.
   */
  enum class FillRule
  {
    WINDING = 0, ///< Draw a horizontal line from the point to a location outside the shape. Determine whether the direction of the line at each intersection point is up or down. The winding number is determined by summing the direction of each intersection. If the number is non zero, the point is inside the shape.
    EVEN_ODD     ///< Draw a horizontal line from the point to a location outside the shape, and count the number of intersections. If the number of intersections is an odd number, the point is inside the shape.
  };

  /**
   * @brief Enumeration specifying the values of the path commands.
   * Not to be confused with the path commands from the svg path element (like M, L, Q, H and many others).
   */
  enum class PathCommandType
  {
    CLOSE = 0, ///< Ends the current sub-path and connects it with its initial point. This command doesn't expect any points.
    MOVE_TO,   ///< Sets a new initial point of the sub-path and a new current point. This command expects 1 point: the starting position.
    LINE_TO,   ///< Draws a line from the current point to the given point and sets a new value of the current point. This command expects 1 point: the end-position of the line.
    CUBIC_TO   ///< Draws a cubic Bezier curve from the current point to the given point using two given control points and sets a new value of the current point. This command expects 3 points: the 1st control-point, the 2nd control-point, the end-point of the curve.
  };

  /**
   * @brief Structure that contains information about a list of path commands.
   * For each command from the mCommands array, an appropriate number of points in mPoints array should be specified.
   */
  struct PathCommands
  {
    PathCommandType* mCommands;     ///< Set of each PathComand.
    uint32_t         mCommandCount; ///< The number of command array.
    float*           mPoints;       ///< Set of each Point
    uint32_t         mPointCount;   ///< The number of point array.
  };

public:
  /**
   * @brief Append the given rectangle with rounded corner to the path.
   * The roundedCorner arguments specify the radii of the ellipses defining the
   * corners of the rounded rectangle.
   *
   * roundedCorner are specified in terms of width and height respectively.
   *
   * If roundedCorner's values are 0, then it will draw a rectangle without rounded corner.
   *
   * @param[in] rect size of the rectangle.
   * @param[in] roundedCorner The radius of the rounded corner and should be in range [ 0 to w/2 ]
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddRect(Rect<float> rect, Vector2 roundedCorner);

  /**
   * @brief Append a circle with given center and x,y-axis radius.
   * @param[in] center X and Y co-ordinate of the center of the circle.
   * @param[in] radius X and Y co-ordinate of radius of the circle.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddCircle(Vector2 center, Vector2 radius);

  /**
   * @brief Append the arcs .
   * @param[in] center X and Y co-ordinate of the center of the arc.
   * @param[in] radius Radius of the arc.
   * @param[in] startAngle Start angle (in degrees) where the arc begins.
   * @param[in] sweep The Angle measures how long the arc will be drawn.
   * @param[in] pie If True, the area is created by connecting start angle point and sweep angle point of the drawn arc. If false, it doesn't.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie);

  /**
   * @brief Add a point that sets the given point as the current point,
   * implicitly starting a new subpath and closing the previous one.
   * @param[in] point X and Y co-ordinate of the current point.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddMoveTo(Vector2 point);

  /**
   * @brief Adds a straight line from the current position to the given end point.
   * After the line is drawn, the current position is updated to be at the
   * end point of the line.
   * If no current position present, it draws a line to itself, basically * a point.
   * @param[in] line X and Y co-ordinate of end point of the line.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddLineTo(Vector2 line);

  /**
   * @brief Adds a cubic Bezier curve between the current position and the
   * given end point (lineEndPoint) using the control points specified by
   * (controlPoint1), and (controlPoint2). After the path is drawn,
   * the current position is updated to be at the end point of the path.
   * @param[in] controlPoint1 X and Y co-ordinate of 1st control point.
   * @param[in] controlPoint2 X and Y co-ordinate of 2nd control point.
   * @param[in] endPoint X and Y co-ordinate of end point of the line.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint);

  /**
   * @brief Appends a given sub-path to the path.
   * The current point value is set to the last point from the sub-path.
   * @param[in] pathCommand The command object that contain sub-path information. (This command information is copied internally.)
   * @return Returns True when it's successful. False otherwise.
   * @note The interface is designed for optimal path setting if the caller has a completed path commands already.
   *
   */
  bool AddPath(PathCommands& pathCommand);

  /**
   * @brief Closes the current subpath by drawing a line to the beginning of the
   * subpath, automatically starting a new path. The current point of the
   * new path is (0, 0).
   * If the subpath does not contain any points, this function does nothing.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Close();

  /**
   * @brief Reset the added path(rect, circle, path, etc...) information.
   * Color and Stroke information are keeped.
   * @return Returns True when it's successful. False otherwise.
   */
  bool ResetPath();

  /**
   * @brief Set the color to use for filling the path.
   * @param[in] color The color value.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetFillColor(Vector4 color);

  /**
   * @brief Get the color to use for filling the path.
   * @return Returns The color value.
   */
  Vector4 GetFillColor() const;

  /**
   * @brief Set the gradient to use for filling the path.
   * @param[in] gradient The gradient object.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetFillGradient(CanvasRenderer::Gradient& gradient);

  /**
   * @brief Get the gradient to use for filling the path.
   * @return Returns The gradient object.
   */
  CanvasRenderer::Gradient GetFillGradient() const;

  /**
   * @brief Set the fill rule.
   * @param[in] rule The current fill rule of the shape.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetFillRule(CanvasRenderer::Shape::FillRule rule);

  /**
   * @brief Get the fill rule.
   * @return Returns the current fill rule of the shape.
   */
  CanvasRenderer::Shape::FillRule GetFillRule() const;

  /**
   * @brief Set the stroke width to use for stroking the path.
   * @param[in] width Stroke width to be used.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetStrokeWidth(float width);

  /**
   * @brief Get the stroke width to use for stroking the path.
   * @return Returns stroke width to be used.
   */
  float GetStrokeWidth() const;

  /**
   * @brief Set the color to use for stroking the path.
   * @param[in] color The stroking color.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetStrokeColor(Vector4 color);

  /**
   * @brief Get the color to use for stroking the path.
   * @return Returns the stroking color.
   */
  Vector4 GetStrokeColor() const;

  /**
   * @brief Set the gradient to use for stroking the path.
   * @param[in] gradient The gradient object.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetStrokeGradient(CanvasRenderer::Gradient& gradient);

  /**
   * @brief Get the gradient to use for stroking the path.
   * @return Returns The gradient object.
   */
  CanvasRenderer::Gradient GetStrokeGradient() const;

  /**
   * @brief Sets the stroke dash pattern. The dash pattern is specified dash pattern.
   * @param[in] dashPattern Lenght and a gap list.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetStrokeDash(const Dali::Vector<float>& dashPattern);

  /**
   * @brief Gets the stroke dash pattern.
   * @return Returns the stroke dash pattern. The dash pattern is specified dash pattern.
   */
  Dali::Vector<float> GetStrokeDash() const;

  /**
   * @brief Set the cap style to use for stroking the path. The cap will be used for capping the end point of a open subpath.
   * @param[in] cap Cap style to use.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetStrokeCap(CanvasRenderer::Shape::StrokeCap cap);

  /**
   * @brief Get the cap style to use for stroking the path.
   * @return Returns the cap style.
   */
  CanvasRenderer::Shape::StrokeCap GetStrokeCap() const;

  /**
   * @brief Set the join style to use for stroking the path.
   * The join style will be used for joining the two line segment while stroking the path.
   * @param[in] join Join style to use.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetStrokeJoin(CanvasRenderer::Shape::StrokeJoin join);

  /**
   * @brief Get the join style to use for stroking the path.
   * @return Returns join style to use.
   */
  CanvasRenderer::Shape::StrokeJoin GetStrokeJoin() const;

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::Shape
   */
  explicit DALI_INTERNAL Shape(Internal::Adaptor::Shape* impl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_SHAPE_H
