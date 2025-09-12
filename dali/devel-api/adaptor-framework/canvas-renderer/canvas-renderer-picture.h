#ifndef DALI_CANVAS_RENDERER_PICTURE_H
#define DALI_CANVAS_RENDERER_PICTURE_H

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
class Picture;
} // namespace Adaptor
} //namespace Internal DALI_INTERNAL

/**
 * @brief A class representing an image read in one of the supported formats: raw, svg, png and etc.
 * Besides the methods inherited from the Drawable, it provides methods to load & draw images on the canvas.
 */
class DALI_ADAPTOR_API CanvasRenderer::Picture : public CanvasRenderer::Drawable
{
public:
  /**
   * @brief Creates an initialized handle to a new CanvasRenderer::Picture.
   *
   * @return A handle to a newly allocated Picture
   */
  static Picture New();

public:
  /**
   * @brief Creates an empty handle.
   * Use CanvasRenderer::Picture::New() to create an initialized object.
   */
  Picture();

  /**
   * @brief Destructor.
   */
  ~Picture();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  Picture(const Picture& handle) = default;

public:
  /**
   * @brief Loads a picture data directly from a file.
   * @param[in] url A path to the picture file.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Load(const std::string& url);

  /**
   * @brief Resize the picture content with the given size.
   *
   * Resize the picture content while keeping the default size aspect ratio.
   * The scaling factor is established for each of dimensions and the smaller value is applied to both of them.
   * @param[in] size A new size of the image in pixels.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetSize(Vector2 size);

  /**
   * @brief Gets the size of the image.
   * @return Returns The size of the image in pixels.
   */
  Vector2 GetSize() const;

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::Picture
   */
  explicit DALI_INTERNAL Picture(Internal::Adaptor::Picture* impl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_PICTURE_H
