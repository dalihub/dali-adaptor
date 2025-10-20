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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-native-image-handler.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-utils.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <vulkan/vulkan.h>

namespace Dali::Graphics::Vulkan
{
/**
 * @brief Dummy implementation of VulkanNativeImageHandler for non-Tizen platforms
 */
class VulkanNativeImageHandlerGeneric : public VulkanNativeImageHandler
{
public:
  NativeTextureData                     SetFormatAndUsage(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device) override;
  std::unique_ptr<NativeImageResources> InitializeNativeTexture(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device, uint32_t width, uint32_t height, NativeTextureData& textureData) override;
  bool                                  AcquireCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr) override;
  bool                                  ReleaseCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr) override;
  void                                  DestroyNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources) override;
  void                                  ResetNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources) override;
};

// Factory implementation
std::unique_ptr<VulkanNativeImageHandler> VulkanNativeImageHandler::CreateHandler()
{
  return nullptr;
}

NativeTextureData VulkanNativeImageHandlerGeneric::SetFormatAndUsage(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device)
{
  return NativeTextureData();
}

std::unique_ptr<NativeImageResources> VulkanNativeImageHandlerGeneric::InitializeNativeTexture(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device, uint32_t width, uint32_t height, NativeTextureData& textureData)
{
  return nullptr;
}

bool VulkanNativeImageHandlerGeneric::AcquireCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr)
{
  return false;
}

bool VulkanNativeImageHandlerGeneric::ReleaseCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr)
{
  return false;
}

void VulkanNativeImageHandlerGeneric::DestroyNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources)
{
}

void VulkanNativeImageHandlerGeneric::ResetNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources)
{
}

} // namespace Dali::Graphics::Vulkan
