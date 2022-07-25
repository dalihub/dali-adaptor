#ifndef DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_IMPL_H
#define DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_IMPL_H

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
#include <dali/devel-api/adaptor-framework/vector-animation-renderer.h>
#include <dali/internal/vector-animation/common/vector-animation-renderer-plugin-proxy.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class VectorAnimationRenderer;
using VectorAnimationRendererPtr = IntrusivePtr<VectorAnimationRenderer>;

/**
 * Dali internal VectorAnimationRenderer.
 */
class VectorAnimationRenderer : public BaseObject, public ConnectionTracker
{
public:
  /**
   * @brief Creates a VectorAnimationRenderer object.
   *
   * @param[in] url The url of the vector animation file
   */
  static VectorAnimationRendererPtr New();

  /**
   * @copydoc Dali::VectorAnimationRenderer::Finalize()
   */
  void Finalize();

  /**
   * @copydoc Dali::VectorAnimationRenderer::Load()
   */
  bool Load(const std::string& url);

  /**
   * @copydoc Dali::VectorAnimationRenderer::SetRenderer()
   */
  void SetRenderer(Dali::Renderer renderer);

  /**
   * @copydoc Dali::VectorAnimationRenderer::SetSize()
   */
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @copydoc Dali::VectorAnimationRenderer::Render()
   */
  bool Render(uint32_t frameNumber);

  /**
   * @copydoc Dali::VectorAnimationRenderer::GetTotalFrameNumber()
   */
  uint32_t GetTotalFrameNumber() const;

  /**
   * @copydoc Dali::VectorAnimationRenderer::GetFrameRate()
   */
  float GetFrameRate() const;

  /**
   * @copydoc Dali::VectorAnimationRenderer::GetDefaultSize()
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  /**
   * @copydoc Dali::VectorAnimationRenderer::GetLayerInfo()
   */
  void GetLayerInfo(Property::Map& map) const;

  /**
   * @copydoc Dali::VectorAnimationRenderer::GetMarkerInfo()
   */
  bool GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const;

  /**
   * @copydoc Dali::VectorAnimationRenderer::InvalidateBuffer()
   */
  void InvalidateBuffer();

  /**
   * @copydoc Dali::VectorAnimationRenderer::AddPropertyValueCallback()
   */
  void AddPropertyValueCallback(const std::string& keyPath, Dali::VectorAnimationRenderer::VectorProperty property, CallbackBase* callback, int32_t id);

  /**
   * @copydoc Dali::VectorAnimationRenderer::UploadCompletedSignal()
   */
  Dali::VectorAnimationRenderer::UploadCompletedSignalType& UploadCompletedSignal();

private:
  /**
   * @brief Constructor
   */
  VectorAnimationRenderer();

  /**
   * @brief Destructor.
   */
  ~VectorAnimationRenderer();

private:
  VectorAnimationRenderer(const VectorAnimationRenderer&) = delete;
  VectorAnimationRenderer& operator=(VectorAnimationRenderer&) = delete;

private:
  VectorAnimationRendererPluginProxy mPlugin;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::VectorAnimationRenderer& GetImplementation(Dali::VectorAnimationRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "VectorAnimationRenderer handle is empty.");

  BaseObject& handle = renderer.GetBaseObject();

  return static_cast<Internal::Adaptor::VectorAnimationRenderer&>(handle);
}

inline static const Internal::Adaptor::VectorAnimationRenderer& GetImplementation(const Dali::VectorAnimationRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "VectorAnimationRenderer handle is empty.");

  const BaseObject& handle = renderer.GetBaseObject();

  return static_cast<const Internal::Adaptor::VectorAnimationRenderer&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_IMPL_H
