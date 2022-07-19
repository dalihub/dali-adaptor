#ifndef DALI_INTERNAL_GENERIC_DRAWABLE_IMPL_GENERIC_H
#define DALI_INTERNAL_GENERIC_DRAWABLE_IMPL_GENERIC_H

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
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Drawable.
 */
class DrawableGeneric : public Dali::Internal::Adaptor::Drawable
{
public:
  /**
   * @brief Creates a Drawable object.
   * @return A pointer to a newly allocated Drawable
   */
  static DrawableGeneric* New();

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetOpacity()
   */
  bool SetOpacity(float opacity) override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::GetOpacity()
   */
  float GetOpacity() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Rotate()
   */
  bool Rotate(Degree degree) override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Scale()
   */
  bool Scale(float factor) override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Translate()
   */
  bool Translate(Vector2 translate) override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::Transform()
   */
  bool Transform(const Dali::Matrix3& matrix) override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::GetBoundingBox()
   */
  Rect<float> GetBoundingBox() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetClipPath()
   */
  bool SetClipPath(Dali::CanvasRenderer::Drawable& clip) override;

  /**
   * @copydoc Dali::CanvasRenderer::Drawable::SetMask()
   */
  bool SetMask(Dali::CanvasRenderer::Drawable& mask, Dali::CanvasRenderer::Drawable::MaskType type) override;

  /**
   * @copydoc Internal::Adaptor::Drawable::GetCompositionDrawable()
   */
  Dali::CanvasRenderer::Drawable GetCompositionDrawable() const override;

  /**
   * @copydoc Internal::Adaptor::Drawable::GetCompositionType()
   */
  CompositionType GetCompositionType() const override;

  /**
   * @copydoc Internal::Adaptor::Drawable::SetAdded()
   */
  void SetAdded(bool added) override;

  /**
   * @copydoc Internal::Adaptor::Drawable::IsAdded()
   */
  bool IsAdded() const override;

  /**
   * @copydoc Internal::Adaptor::Drawable::SetObject()
   */
  void SetObject(const void* object) override;

  /**
   * @copydoc Internal::Adaptor::Drawable::GetObject()
   */
  void* GetObject() const override;

  /**
   * @copydoc Internal::Adaptor::Drawable::SetChanged()
   */
  void SetChanged(bool changed);

  /**
   * @copydoc Internal::Adaptor::Drawable::GetChanged()
   */
  bool GetChanged() const;

  DrawableGeneric(const DrawableGeneric&) = delete;
  DrawableGeneric& operator=(DrawableGeneric&) = delete;
  DrawableGeneric(DrawableGeneric&&)           = delete;
  DrawableGeneric& operator=(DrawableGeneric&&) = delete;

protected:
  /**
   * @brief Constructor
   */
  DrawableGeneric();

  /**
   * @brief Destructor.
   */
  virtual ~DrawableGeneric() override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GENERIC_DRAWABLE_IMPL_GENERIC_H
