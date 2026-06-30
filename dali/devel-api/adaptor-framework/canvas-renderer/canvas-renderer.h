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
#include <dali/public-api/math/vector4.h>
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

  /**
   * @brief Applies a drop shadow effect to the whole canvas content.
   *
   * Backed by the ThorVG scene effect (tvg::SceneEffect::DropShadow). The effect takes effect
   * from the next Commit()/Rasterize().
   *
   * @param[in] color      Shadow color (RGBA, each channel 0.0 ~ 1.0).
   * @param[in] offsetX    Shadow X offset in pixels (positive X is right).
   * @param[in] offsetY    Shadow Y offset in pixels (positive Y is down).
   * @param[in] blurRadius Blur radius in pixels. Internally converted to a Gaussian
   *                       sigma = blurRadius * 0.5.
   * @return Returns True when it's successful. False otherwise.
   *
   * @note ThorVG supports isotropic blur only, so separate X/Y blur radii are not supported.
   *       The shadow is clipped to the canvas buffer (the layout size) unless effect auto-padding
   *       is enabled (see SetEffectAutoPaddingEnabled()).
   */
  bool SetDropShadow(const Vector4& color, float offsetX, float offsetY, float blurRadius);

  /**
   * @brief Removes the drop shadow effect set by SetDropShadow().
   * @return Returns True when it's successful. False otherwise.
   */
  bool ClearDropShadow();

  /**
   * @brief Returns whether a drop shadow effect is currently set.
   * @return Returns True when a drop shadow is set, False otherwise.
   */
  bool HasDropShadow() const;

  /**
   * @brief Applies a Gaussian blur effect to the whole canvas content.
   *
   * Scene effects are mutually exclusive: setting a Gaussian blur replaces a drop shadow and
   * vice versa.
   *
   * @param[in] blurRadius Blur radius in pixels (internally converted to sigma = blurRadius * 0.5).
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetGaussianBlur(float blurRadius);

  /**
   * @brief Removes the Gaussian blur effect set by SetGaussianBlur().
   * @return Returns True when it's successful. False otherwise.
   */
  bool ClearGaussianBlur();

  /**
   * @brief Returns whether a Gaussian blur effect is currently set.
   * @return Returns True when a Gaussian blur is set, False otherwise.
   */
  bool HasGaussianBlur() const;

  /**
   * @brief Enables or disables automatic padding for scene effects (enabled by default).
   *
   * When enabled, the canvas is rendered into a buffer enlarged by the effect's extent so the
   * blurred/offset result is not clipped at the canvas edges; the result is scaled back into the
   * view, so the content appears slightly smaller. When disabled, the caller must size the
   * CanvasView large enough (and inset the content) to leave room for the effect.
   *
   * @param[in] enable True to auto-pad, false to render into the exact layout-sized buffer.
   */
  void SetEffectAutoPaddingEnabled(bool enable);

  /**
   * @brief Returns whether automatic effect padding is enabled.
   * @return Returns True when auto-padding is enabled, False otherwise.
   */
  bool IsEffectAutoPaddingEnabled() const;

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
