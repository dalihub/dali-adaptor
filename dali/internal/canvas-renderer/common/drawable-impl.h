#ifndef DALI_INTERNAL_DRAWABLE_IMPL_H
#define DALI_INTERNAL_DRAWABLE_IMPL_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
   * @brief Constructor
   */
  Drawable();

  /**
   * @brief Destructor.
   */
  virtual ~Drawable() override;

  /**
   * @brief Create factory item(implementation) object.
   */
  void Create();

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetOpacity()
   */
  virtual bool SetOpacity(float opacity);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::GetOpacity()
   */
  virtual float GetOpacity() const;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Rotate()
   */
  virtual bool Rotate(Degree degree);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Scale()
   */
  virtual bool Scale(float factor);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Translate()
   */
  virtual bool Translate(Vector2 translate);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Transform()
   */
  virtual bool Transform(const Dali::Matrix3& matrix);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::GetBoundingBox
   */
  virtual Rect<float> GetBoundingBox() const;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetClipPath()
   */
  virtual bool SetClipPath(Dali::CanvasRenderer::Drawable& clip);

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetMask()
   */
  virtual bool SetMask(Dali::CanvasRenderer::Drawable& mask, Dali::CanvasRenderer::Drawable::MaskType type);

  /**
   * @brief Returns a composition drawble object.
   * @return Returns a composition drawble object.
   */
  virtual Dali::CanvasRenderer::Drawable GetCompositionDrawable() const;

  /**
   * @brief Returns a composition type
   * @return Returns a composition type
   */
  virtual CompositionType GetCompositionType() const;

  /**
   * @brief Set whether this drawable object was added to other object(CanvasRenderer or DrawableGroup) or not.
   * @param[in] added Ture if added, false otherwise.
   */
  virtual void SetAdded(bool added);

  /**
   * @brief Returns whether this drawable object was added to another object(CanvasRenderer or DrawableGroup).
   * @return Returns Ture if added, false otherwise.
   */
  virtual bool IsAdded() const;

  /**
   * @brief Returns a drawable object pointer.
   * @return Returns a drawable object pointer.
   */
  virtual void* GetObject() const;

  /**
   * @brief Set a drawable object
   * @param[in] object drawable object
   */
  virtual void SetObject(const void* object);

  /**
   * @brief Set a changed state.
   * @param[in] changed The state of changed.
   */
  virtual void SetChanged(bool changed);

  /**
   * @brief Get a changed state.
   * @return Returns state of changed.
   */
  virtual bool GetChanged() const;

  /**
   * @brief Set drawable's type.
   * @param[in] type Type of drawable.
   */
  virtual void SetType(Types type);

  /**
   * @brief Get drawable's type.
   * @return Returns type of drawable.
   */
  virtual Types GetType() const;

  /**
   * @brief Returns a drawable's implements object pointer.
   * @return Returns a drawable's implements object pointer.
   */
  Dali::Internal::Adaptor::Drawable* GetImplementation();

  Drawable(const Drawable&) = delete;
  Drawable& operator=(Drawable&) = delete;
  Drawable(Drawable&&)           = delete;
  Drawable& operator=(Drawable&&) = delete;

private:
  Dali::Internal::Adaptor::Drawable* mImpl = nullptr;
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
