#ifndef DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_NATIVE_TIZEN_H
#define DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_NATIVE_TIZEN_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/internal/vector-animation/common/vector-animation-renderer-native.h>
#include <cstdint>
#include <memory>
#include <vector>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Tizen backend for native vector animation rendering.
 *
 * Uses NativeImageQueue for zero-copy GPU rendering.
 * When buffer is unavailable, the frame is skipped.
 */
class VectorAnimationRendererNativeTizen : public VectorAnimationRendererNative
{
public:
  VectorAnimationRendererNativeTizen();
  ~VectorAnimationRendererNativeTizen() override;

  /**
   * @copydoc VectorAnimationRendererNative::Render()
   */
  bool Render(uint32_t frameNumber) override;

  /**
   * @copydoc VectorAnimationRendererNative::FreeReleasedBuffers()
   */
  void FreeReleasedBuffers() override;

protected:
  /**
   * @copydoc VectorAnimationRendererNative::OnFinalize()
   */
  void OnFinalize() override;

  /**
   * @copydoc VectorAnimationRendererNative::OnNotify()
   */
  void OnNotify() override;

  /**
   * @copydoc VectorAnimationRendererNative::CreateRenderingData()
   */
  std::shared_ptr<RenderingData> CreateRenderingData() override;

  /**
   * @copydoc VectorAnimationRendererNative::PrepareTarget()
   */
  void PrepareTarget(std::shared_ptr<RenderingData> renderingData) override;

  /**
   * @copydoc VectorAnimationRendererNative::IsTargetPrepared()
   */
  bool IsTargetPrepared() override;

  /**
   * @copydoc VectorAnimationRendererNative::GetTargetTexture()
   */
  Dali::Texture GetTargetTexture() override;

private:
  class RenderingDataImpl;

  std::vector<Dali::Texture> mPreviousTextures;  ///< Previously rendered textures awaiting GPU release
  Dali::Texture              mRenderedTexture;    ///< Currently displayed texture
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_NATIVE_TIZEN_H
