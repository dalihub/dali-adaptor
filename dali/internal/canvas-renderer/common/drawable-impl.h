#ifndef DALI_INTERNAL_DRAWABLE_IMPL_H
#define DALI_INTERNAL_DRAWABLE_IMPL_H

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
#ifdef THORVG_SUPPORT
#include <thorvg.h>
#endif
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Drawable.
 */
class Drawable : public Dali::BaseObject
{
public:
  /**
   * @brief Enumeration for type of drawable.
   */
  enum class Types
  {
    NONE = 0,       ///< Means that type is not defined.
    SHAPE,          ///< Meaning of Shape class that inherits Drawable.
    DRAWABLE_GROUP, ///< Meaning of DrawableGorup class that inherits Drawable.
    PICTURE         ///< Meaning of Picture class that inherits Drawable.
  };

  /**
   * @brief Enumeration indicating type used in the composition of two objects - the target and the source.
   */
  enum class CompositionType
  {
    NONE = 0,          ///< Means that type is not defined.
    CLIP_PATH,         ///< The intersection of the source and the target is determined and only the resulting pixels from the source are rendered.
    ALPHA_MASK,        ///< The pixels of the source and the target are alpha blended. As a result, only the part of the source, which intersects with the target is visible.
    ALPHA_MASK_INVERSE ///< The pixels of the source and the complement to the target's pixels are alpha blended. As a result, only the part of the source which is not covered by the target is visible.
  };

public:
  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetOpacity()
   */
  bool SetOpacity(float opacity);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::GetOpacity()
   */
  float GetOpacity() const;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Rotate()
   */
  bool Rotate(Degree degree);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Scale()
   */
  bool Scale(float factor);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Translate()
   */
  bool Translate(Vector2 translate);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Transform()
   */
  bool Transform(const Dali::Matrix3& matrix);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::GetBoundingBox
   */
  Rect<float> GetBoundingBox() const;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetClipPath()
   */
  bool SetClipPath(Dali::CanvasRenderer::Drawable& clip);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetMask()
   */
  bool SetMask(Dali::CanvasRenderer::Drawable& mask, Dali::CanvasRenderer::Drawable::MaskType type);

public:
  /**
   * @brief Returns a composition drawable object.
   * @return Returns a composition drawable object.
   */
  Dali::CanvasRenderer::Drawable GetCompositionDrawable() const;

  /**
   * @brief Returns a composition type
   * @return Returns a composition type
   */
  CompositionType GetCompositionType() const;

  /**
   * @brief Set whether this drawable object was added to other object(CanvasRenderer or DrawableGroup) or not.
   * @param[in] added Ture if added, false otherwise.
   */
  void SetAdded(bool added);

  /**
   * @brief Returns whether this drawable object was added to another object(CanvasRenderer or DrawableGroup).
   * @return Returns Ture if added, false otherwise.
   */
  bool IsAdded() const;

  /**
   * @brief Returns a drawable object pointer.
   * @return Returns a drawable object pointer.
   */
  void* GetObject() const;

  /**
   * @brief Set a drawable object
   * @param[in] object drawable object
   */
  void SetObject(const void* object);

  /**
   * @brief Set a changed state.
   * @param[in] changed The state of changed.
   */
  void SetChanged(bool changed);

  /**
   * @brief Get a changed state.
   * @return Returns state of changed.
   */
  bool GetChanged() const;

  /**
   * @brief Set drawable's type.
   * @param[in] type Type of drawable.
   */
  void SetType(Types type);

  /**
   * @brief Get drawable's type.
   * @return Returns type of drawable.
   */
  Types GetType() const;

private:
  Drawable(const Drawable&)       = delete;
  Drawable& operator=(Drawable&)  = delete;
  Drawable(Drawable&&)            = delete;
  Drawable& operator=(Drawable&&) = delete;

protected:
  /**
   * @brief Constructor
   */
  Drawable();

  /**
   * @brief Destructor.
   */
  virtual ~Drawable() override;

private:
  Dali::CanvasRenderer::Drawable mCompositionDrawable;

  Drawable::Types           mType : 3;
  Drawable::CompositionType mCompositionType : 3;

  bool mAdded : 1;
  bool mChanged : 1;

#ifdef THORVG_SUPPORT
  tvg::Paint* mTvgPaint;
#endif
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::Drawable& GetImplementation(Dali::CanvasRenderer::Drawable& drawable)
{
  DALI_ASSERT_ALWAYS(drawable && "Drawable handle is empty.");

  BaseObject& handle = drawable.GetBaseObject();

  return static_cast<Internal::Adaptor::Drawable&>(handle);
}

inline static const Internal::Adaptor::Drawable& GetImplementation(const Dali::CanvasRenderer::Drawable& drawable)
{
  DALI_ASSERT_ALWAYS(drawable && "Drawable handle is empty.");

  const BaseObject& handle = drawable.GetBaseObject();

  return static_cast<const Internal::Adaptor::Drawable&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_DRAWABLE_IMPL_H
