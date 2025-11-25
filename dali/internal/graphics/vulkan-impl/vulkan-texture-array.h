#pragma once

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
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-handle.h>

namespace Dali::Graphics::Vulkan
{
class Texture;

class TextureArray
{
public:
  TextureArray(VulkanGraphicsController& graphicsController);
  static TextureArray* New(Graphics::TextureCreateInfo createInfo, VulkanGraphicsController& graphicsController, uint32_t numLayers);

  virtual ~TextureArray() = default;

  void Initialize(Graphics::TextureCreateInfo createInfo, VulkanGraphicsController& graphicsController, uint32_t numLayers);
  void Release();
  void Retain();
  void DestroyResource();

  uint32_t GetRefCount()
  {
    return mRefCount;
  }

  Texture* GetTexture();
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

private:
  VulkanGraphicsController& mGraphicsController;
  std::unique_ptr<Texture>  mTexture;
  std::atomic_uint          mRefCount{0u};
};

using TextureArrayHandle = Handle<TextureArray>;

} // namespace Dali::Graphics::Vulkan
