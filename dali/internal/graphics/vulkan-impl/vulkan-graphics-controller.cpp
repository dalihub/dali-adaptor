/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan/graphics-implementation.h>

namespace Dali::Graphics
{

struct VulkanGraphicsController::Impl
{
  Impl(VulkanGraphicsController& controller)
  : mGraphicsController(controller)
  {
  }

  bool Initialize()
  {
    // Create factories.
    // Create pipeline cache
    // Create DescriptorSetAllocator
    // Initialize thread pool
    return true;
  }

  VulkanGraphicsController& mGraphicsController;
  Vulkan::Device mGraphicsDevice; // @todo You don't own me!
};

VulkanGraphicsController::VulkanGraphicsController()
: mImpl(std::make_unique<Impl>(*this))
{
}

VulkanGraphicsController::~VulkanGraphicsController()
{
}

void VulkanGraphicsController::Initialize(Dali::Graphics::VulkanGraphics& graphicsImplementation)
{
  mImpl->mGraphicsDevice.Create();
  mImpl->mGraphicsDevice.CreateDevice();
  mImpl->Initialize();
}

Vulkan::Device& VulkanGraphicsController::GetGraphicsDevice() const
{
  return mImpl->mGraphicsDevice;
}

Integration::GraphicsConfig& VulkanGraphicsController::GetGraphicsConfig()
{
  return *this;
}

void VulkanGraphicsController::SubmitCommandBuffers(const SubmitInfo& submitInfo)
{
}

void VulkanGraphicsController::PresentRenderTarget(RenderTarget* renderTarget)
{
}

void VulkanGraphicsController::WaitIdle()
{
}

void VulkanGraphicsController::Pause()
{
}

void VulkanGraphicsController::Resume()
{
}

void VulkanGraphicsController::Shutdown()
{
}

void VulkanGraphicsController::Destroy()
{
}

void VulkanGraphicsController::UpdateTextures(const std::vector<TextureUpdateInfo>&       updateInfoList,
                                              const std::vector<TextureUpdateSourceInfo>& sourceList)
{
}

void VulkanGraphicsController::GenerateTextureMipmaps(const Texture& texture)
{
}

bool VulkanGraphicsController::EnableDepthStencilBuffer(bool enableDepth, bool enableStencil)
{
  return true;
}

void VulkanGraphicsController::RunGarbageCollector(size_t numberOfDiscardedRenderers)
{
}

void VulkanGraphicsController::DiscardUnusedResources()
{
}

bool VulkanGraphicsController::IsDiscardQueueEmpty()
{
  return true;
}

bool VulkanGraphicsController::IsDrawOnResumeRequired()
{
  return true;
}

UniquePtr<Buffer> VulkanGraphicsController::CreateBuffer(const BufferCreateInfo& bufferCreateInfo, UniquePtr<Buffer>&& oldBuffer)
{
  return UniquePtr<Buffer>{};
}

UniquePtr<CommandBuffer> VulkanGraphicsController::CreateCommandBuffer(const CommandBufferCreateInfo& commandBufferCreateInfo, UniquePtr<CommandBuffer>&& oldCommandBuffer)
{
  return UniquePtr<CommandBuffer>{};
}

UniquePtr<RenderPass> VulkanGraphicsController::CreateRenderPass(const RenderPassCreateInfo& renderPassCreateInfo, UniquePtr<RenderPass>&& oldRenderPass)
{
  return UniquePtr<RenderPass>{};
}

UniquePtr<Texture> VulkanGraphicsController::CreateTexture(const TextureCreateInfo& textureCreateInfo, UniquePtr<Texture>&& oldTexture)
{
  return UniquePtr<Texture>{};
}

UniquePtr<Framebuffer> VulkanGraphicsController::CreateFramebuffer(const FramebufferCreateInfo& framebufferCreateInfo, UniquePtr<Framebuffer>&& oldFramebuffer)
{
  return UniquePtr<Framebuffer>{};
}

UniquePtr<Pipeline> VulkanGraphicsController::CreatePipeline(const PipelineCreateInfo& pipelineCreateInfo, UniquePtr<Pipeline>&& oldPipeline)
{
  return UniquePtr<Pipeline>{};
}

UniquePtr<Program> VulkanGraphicsController::CreateProgram(const ProgramCreateInfo& programCreateInfo, UniquePtr<Program>&& oldProgram)
{
  return UniquePtr<Program>{};
}

UniquePtr<Shader> VulkanGraphicsController::CreateShader(const ShaderCreateInfo& shaderCreateInfo, UniquePtr<Shader>&& oldShader)
{
  return UniquePtr<Shader>{};
}

UniquePtr<Sampler> VulkanGraphicsController::CreateSampler(const SamplerCreateInfo& samplerCreateInfo, UniquePtr<Sampler>&& oldSampler)
{
  return UniquePtr<Sampler>{};
}

UniquePtr<RenderTarget> VulkanGraphicsController::CreateRenderTarget(const RenderTargetCreateInfo& renderTargetCreateInfo, UniquePtr<RenderTarget>&& oldRenderTarget)
{
  return UniquePtr<RenderTarget>{};
}

UniquePtr<SyncObject> VulkanGraphicsController::CreateSyncObject(const SyncObjectCreateInfo& syncObjectCreateInfo,
                                                                 UniquePtr<SyncObject>&&     oldSyncObject)
{
  return UniquePtr<SyncObject>{};
}

UniquePtr<Memory> VulkanGraphicsController::MapBufferRange(const MapBufferInfo& mapInfo)
{
  return UniquePtr<Memory>{};
}

UniquePtr<Memory> VulkanGraphicsController::MapTextureRange(const MapTextureInfo& mapInfo)
{
  return UniquePtr<Memory>{};
}

void VulkanGraphicsController::UnmapMemory(UniquePtr<Memory> memory)
{
}

MemoryRequirements VulkanGraphicsController::GetTextureMemoryRequirements(Texture& texture) const
{
  return MemoryRequirements{};
}

MemoryRequirements VulkanGraphicsController::GetBufferMemoryRequirements(Buffer& buffer) const
{
  return MemoryRequirements{};
}

TextureProperties VulkanGraphicsController::GetTextureProperties(const Texture& texture)
{
  return TextureProperties{};
}

const Reflection& VulkanGraphicsController::GetProgramReflection(const Program& program)
{
  return *(reinterpret_cast<Reflection*>(0));
}

bool VulkanGraphicsController::PipelineEquals(const Pipeline& pipeline0, const Pipeline& pipeline1) const
{
  return true;
}

bool VulkanGraphicsController::GetProgramParameter(Graphics::Program& program, uint32_t parameterId, void* outData)
{
  return false;
}

//bool VulkanGraphicsController::IsBlendEquationSupported(DevelBlendEquation::Type blendEquation)
//{
//  return false;
//}

uint32_t VulkanGraphicsController::GetShaderLanguageVersion()
{
  return 4;
}

std::string VulkanGraphicsController::GetShaderVersionPrefix()
{
  return "";
}

std::string VulkanGraphicsController::GetVertexShaderPrefix()
{
  return "";
}

std::string VulkanGraphicsController::GetFragmentShaderPrefix()
{
  return "";
}

} //namespace Dali::Graphics
