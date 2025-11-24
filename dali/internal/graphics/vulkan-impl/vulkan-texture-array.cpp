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

#include <dali/internal/graphics/vulkan-impl/vulkan-texture-array.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>

namespace Dali::Graphics::Vulkan
{

TextureArray::TextureArray(VulkanGraphicsController& graphicsController)
: mGraphicsController(graphicsController),
  mTexture(nullptr),
  mRefCount(0u)
{
}

TextureArray* TextureArray::New(Graphics::TextureCreateInfo createInfo, VulkanGraphicsController& graphicsController, uint32_t numLayers)
{
  TextureArray* array = new TextureArray(graphicsController);
  array->Initialize(createInfo, graphicsController, numLayers);
  return array;
}

void TextureArray::Initialize(Graphics::TextureCreateInfo createInfo, VulkanGraphicsController& graphicsController, uint32_t numLayers)
{
  mTexture = std::make_unique<Texture>(createInfo, mGraphicsController);
  if(mTexture)
  {
    mTexture->SetFormatAndUsage();
    mTexture->Initialize(numLayers);
  }
}
void TextureArray::Release()
{
  --mRefCount;
  if(mRefCount == 0)
  {
    // Remove from controller. Should call DestroyResource.
    mGraphicsController.RemoveTextureArray(this);
  }
}
void TextureArray::Retain()
{
  ++mRefCount;
}
void TextureArray::DestroyResource()
{
  mTexture.reset();
  delete this;
}

Texture* TextureArray::GetTexture()
{
  return static_cast<Vulkan::Texture*>(mTexture.get());
}
uint32_t TextureArray::GetWidth() const
{
  return mTexture ? static_cast<Vulkan::Texture*>(mTexture.get())->GetWidth() : 0;
}
uint32_t TextureArray::GetHeight() const
{
  return mTexture ? static_cast<Vulkan::Texture*>(mTexture.get())->GetHeight() : 0;
}

} //namespace Dali::Graphics::Vulkan
