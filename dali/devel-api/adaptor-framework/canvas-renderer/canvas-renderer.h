#ifndef DALI_CANVAS_RENDERER_H
#define DALI_CANVAS_RENDERER_H

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
#include <dali/public-api/rendering/texture.h>

// INTERNAL INCLUDES
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
}
} //namespace Internal DALI_INTERNAL

/**
 * @brief Used for rendering a vector primitives
 */
class DALI_ADAPTOR_API CanvasRenderer : public BaseHandle
{
public:
  /**
   * @brief Creates an initialized handle to a new CanvasRenderer.
   * @param[in] viewBox The viewBox of canvas.
   * @return A handle to a newly allocated CanvasRenderer
   */
  static CanvasRenderer New(const Vector2& viewBox);

public:
  /**
   * @brief Creates an empty handle.
   * Use CanvasRenderer::New() to create an initialized object.
   */
  CanvasRenderer();

  /**
   * @brief Destructor.
   */
  ~CanvasRenderer();

public:
  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  CanvasRenderer(const CanvasRenderer& handle) = default;

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this
   */
  CanvasRenderer& operator=(const CanvasRenderer& rhs) = default;

public:
  class Drawable;
  class Shape;
  class DrawableGroup;
  class Picture;

  class Gradient;
  class LinearGradient;
  class RadialGradient;

public:
  /**
   * @brief Prepare for drawing drawables added to CanvasRenderer on inner canvas.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Commit();

  /**
   * @brief Add drawable object to the Canvas. This method is similar to registration.
   * The added drawable object(shape) is drawn on the inner canvas using @ref Commit().
   * @param[in] drawable The drawable object.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddDrawable(Drawable& drawable);

  /**
   * @brief Remove drawable object to the CanvasView.
   * This method is similar to deregistration. Freeing memory is not concerned for drawables being removed.
   * @param[in] drawable the drawable object.
   * @return Returns True when it's successful. False otherwise.
   */
  bool RemoveDrawable(Drawable& drawable);

  /**
   * @brief Remove all drawable objects added to the CanvasRenderer.
   * @return Returns True when it's successful. False otherwise.
   */
  bool RemoveAllDrawables();

  /**
   * @brief Returns the rasterized texture.
   *
   * @return Returns the texture.
   */
  Dali::Texture GetRasterizedTexture();

  /**
   * @brief Draw drawables added to CanvasRenderer to inner buffer.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Rasterize();

  /**
   * @brief Returns whether the drawables added to the Canvas are changed.
   * @return Returns True when drawables added to the Canvas are changed, False otherwise.
   */
  bool IsCanvasChanged() const;

  /**
   * @brief This is the size of the buffer in the Canvas.
   * @param[in] size The size of canvas buffer.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetSize(Vector2 size);

  /**
   * @brief This is the size of the Buffer in the Canvas.
   * @return Returns The size of canvas buffer.
   */
  Vector2 GetSize() const;

  /**
   * @brief This is the viewbox of the Canvas.
   * @param[in] viewBox The size of viewbox.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetViewBox(const Vector2& viewBox);

  /**
   * @brief This is the viewbox of the Canvas.
   * @return Returns The size of viewbox.
   */
  const Vector2& GetViewBox();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer
   */
  explicit DALI_INTERNAL CanvasRenderer(Internal::Adaptor::CanvasRenderer* internal);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_H
