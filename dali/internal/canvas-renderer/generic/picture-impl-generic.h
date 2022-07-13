#ifndef DALI_INTERNAL_GENERIC_PICTURE_IMPL_GENERIC_H
#define DALI_INTERNAL_GENERIC_PICTURE_IMPL_GENERIC_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-picture.h>
#include <dali/internal/canvas-renderer/common/picture-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Picture.
 */
class PictureGeneric : public Dali::Internal::Adaptor::Picture
{
public:
  /**
   * @brief Creates a Picture object.
   * @return A pointer to a newly allocated picture
   */
  static PictureGeneric* New();

  /**
   * @copydoc Dali::CanvasRenderer::Picture::Load()
   */
  bool Load(const std::string& url) override;

  /**
   * @copydoc Dali::CanvasRenderer::Picture::SetSize()
   */
  bool SetSize(Vector2 size) override;

  /**
   * @copydoc Dali::CanvasRenderer::Picture::GetSize()
   */
  Vector2 GetSize() const override;

private:
  PictureGeneric(const PictureGeneric&) = delete;
  PictureGeneric& operator=(PictureGeneric&) = delete;
  PictureGeneric(PictureGeneric&&)           = delete;
  PictureGeneric& operator=(PictureGeneric&&) = delete;

  /**
   * @brief Constructor
   */
  PictureGeneric();

  /**
   * @brief Destructor.
   */
  ~PictureGeneric() override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GENERIC_PICTURE_IMPL_GENERIC_H
