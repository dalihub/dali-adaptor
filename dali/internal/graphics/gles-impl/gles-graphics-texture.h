#ifndef DALI_GRAPHICS_GLES_TEXTURE_H
#define DALI_GRAPHICS_GLES_TEXTURE_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/graphics-api/graphics-command-buffer.h>
#include <dali/graphics-api/graphics-texture-create-info.h>
#include <dali/graphics-api/graphics-texture.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
using TextureResource = Resource<Graphics::Texture, Graphics::TextureCreateInfo>;

/**
 * The Texture class represents a GPU texture object. It's slightly
 * higher level than the Vulkan VkImage (more like combined image sampler).
 */
class Texture : public TextureResource
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo valid TextureCreateInfo structure
   * @param[in] controller Reference to the Controller
   */
  Texture(const Graphics::TextureCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~Texture() override = default;

  /**
   * @brief Called when GL resources are destroyed
   */
  void DestroyResource() override;

  /**
   * @brief Returns the Gl texture
   * @return GL texture id
   */
  [[nodiscard]] uint32_t GetGLTexture() const
  {
    return mTextureId;
  }

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  bool InitializeResource() override;

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;

  void Bind(const TextureBinding& binding) const;

protected:
private:
  std::vector<char> mStagingBuffer;
  uint32_t          mTextureId{0u};
  void*             mGLOwnerContext{nullptr};
};
} // namespace Dali::Graphics::GLES

#endif