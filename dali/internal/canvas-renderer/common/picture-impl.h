#ifndef DALI_INTERNAL_PICTURE_IMPL_H
#define DALI_INTERNAL_PICTURE_IMPL_H

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
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Picture.
 */
class Picture : public Internal::Adaptor::Drawable
{
public:
  /**
   * @brief Constructor
   */
  Picture();

  /**
   * @brief Destructor.
   */
  ~Picture() override;

  /**
   * @copydoc Dali::CanvasRenderer::Picture::Load()
   */
  virtual bool Load(const std::string& url);

  /**
   * @copydoc Dali::CanvasRenderer::Picture::SetSize()
   */
  virtual bool SetSize(Vector2 size);

  /**
   * @copydoc Dali::CanvasRenderer::Picture::GetSize()
   */
  virtual Vector2 GetSize() const;

  Picture(const Picture&) = delete;
  Picture& operator=(Picture&) = delete;
  Picture(Picture&&)           = delete;
  Picture& operator=(Picture&&) = delete;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::Picture& GetImplementation(Dali::CanvasRenderer::Picture& picture)
{
  DALI_ASSERT_ALWAYS(picture && "Picture handle is empty.");

  BaseObject& handle = picture.GetBaseObject();

  return static_cast<Internal::Adaptor::Picture&>(handle);
}

inline static const Internal::Adaptor::Picture& GetImplementation(const Dali::CanvasRenderer::Picture& picture)
{
  DALI_ASSERT_ALWAYS(picture && "Picture handle is empty.");

  const BaseObject& handle = picture.GetBaseObject();

  return static_cast<const Internal::Adaptor::Picture&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_PICTURE_IMPL_H
