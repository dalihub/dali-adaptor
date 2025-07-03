#ifndef DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H
#define DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H

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
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <atomic>

#ifdef THORVG_SUPPORT
#include <thorvg.h>
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/devel-api/adaptor-framework/vector-image-renderer.h>

#ifndef THORVG_SUPPORT
struct NSVGrasterizer;
struct NSVGimage;
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class VectorImageRenderer;
using VectorImageRendererPtr = IntrusivePtr<VectorImageRenderer>;

/**
 * Dali internal VectorImageRenderer.
 */
class VectorImageRenderer : public BaseObject
{
public:
  /**
   * @brief Creates a VectorImageRenderer object.
   *
   * @return A handle to a newly allocated VectorImageRenderer
   */
  static VectorImageRendererPtr New();

  /**
   * @copydoc Dali::VectorImageRenderer::Load()
   */
  bool Load(const Vector<uint8_t>& data, float dpi);

  /**
   * @copydoc Dali::VectorImageRenderer::IsLoaded()
   */
  bool IsLoaded() const;

  /**
   * @copydoc Dali::VectorImageRenderer::Rasterize()
   */
  Dali::Devel::PixelBuffer Rasterize(uint32_t width, uint32_t height);

  /**
   * @copydoc Dali::VectorImageRenderer::GetDefaultSize()
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  VectorImageRenderer(const VectorImageRenderer&)       = delete;
  VectorImageRenderer& operator=(VectorImageRenderer&)  = delete;
  VectorImageRenderer(VectorImageRenderer&&)            = delete;
  VectorImageRenderer& operator=(VectorImageRenderer&&) = delete;

private:
  /**
   * @brief Constructor
   */
  VectorImageRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~VectorImageRenderer();

  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
#ifdef THORVG_SUPPORT
  std::unique_ptr<tvg::SwCanvas> mSwCanvas{nullptr};
  tvg::Picture*                  mPicture{nullptr}; ///< The pointer to the picture
#else
  NSVGimage*      mParsedImage{nullptr};
  NSVGrasterizer* mRasterizer{nullptr};
#endif
  Dali::Mutex mMutex{};          ///< The mutex
  uint32_t    mDefaultWidth{0};  ///< The default width of the file
  uint32_t    mDefaultHeight{0}; ///< The default height of the file

  std::atomic_bool mIsLoaded{false}; ///< Indicates whether the image is loaded
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::VectorImageRenderer& GetImplementation(Dali::VectorImageRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "VectorImageRenderer handle is empty.");

  BaseObject& handle = renderer.GetBaseObject();

  return static_cast<Internal::Adaptor::VectorImageRenderer&>(handle);
}

inline static const Internal::Adaptor::VectorImageRenderer& GetImplementation(const Dali::VectorImageRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "VectorImageRenderer handle is empty.");

  const BaseObject& handle = renderer.GetBaseObject();

  return static_cast<const Internal::Adaptor::VectorImageRenderer&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H
