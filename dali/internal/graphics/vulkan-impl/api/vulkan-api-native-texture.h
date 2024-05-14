#ifndef DALI_GRAPHICS_VULKAN_API_NATIVE_TEXTURE_H
#define DALI_GRAPHICS_VULKAN_API_NATIVE_TEXTURE_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/graphics-api/graphics-api-texture-factory.h>
#include <dali/graphics/vulkan/api/vulkan-api-texture.h>
#include <dali/graphics/vulkan/internal/vulkan-types.h>
#include <dali/public-api/object/any.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
class Graphics;
}
namespace VulkanAPI
{
class TextureFactory;
class Controller;

/**
 * NativeTexture implements native image support for Vulkan
 */
class NativeTexture : public VulkanAPI::Texture
{
public:

  explicit NativeTexture( Dali::Graphics::TextureFactory& factory );

  ~NativeTexture() override;

  bool Initialize() override;

  bool IsSamplerImmutable() const override;

  void CopyNativeImage( TextureDetails::UpdateMode updateMode );

private:

  bool InitialiseNativeImage();
  bool CreateSamplerYUVNativeImage( bool support_LinearFilter );
  void CreateImageViewYUVNativeImage();

private:

  Any                              mNativeImage;
  VkSamplerYcbcrConversionInfoKHR  mYcbcrConvInfo{};
  VkSamplerYcbcrConversionKHR      mYcbcrConv;
  void*                            mTbmSurface;
  bool                             mSamplerIsImmutable;
  bool                             mIsSupportNativeImage { false };
};

std::unique_ptr<NativeTexture> __attribute__((weak)) MakeUniqueVulkanNativeTexture( Dali::Graphics::TextureFactory& factory )
{
  return {};
}

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
#endif // DALI_GRAPHICS_VULKAN_API_NATIVE_TEXTURE_H
