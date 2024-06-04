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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>


// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>


namespace Dali::Graphics::Vulkan
{

/**
 * @brief Custom deleter for all Graphics objects created
 * with use of the Controller.
 *
 * When Graphics object dies the unique pointer (Graphics::UniquePtr)
 * doesn't destroy it directly but passes the ownership back
 * to the Controller. The VKDeleter is responsible for passing
 * the object to the discard queue (by calling Resource::DiscardResource()).
 */
template<typename T>
struct VKDeleter
{
  VKDeleter() = default;

  void operator()(T* object)
  {
    // Discard resource (add it to discard queue)
    object->DiscardResource();
  }
};

/**
 * @brief Helper function allocating graphics object
 *
 * @param[in] info Create info structure
 * @param[in] controller Controller object
 * @param[out] out Unique pointer to the return object
 */
template<class VKType, class GfxCreateInfo, class T>
auto NewObject(const GfxCreateInfo& info, VulkanGraphicsController& controller, T&& oldObject)
{
  // Use allocator
  using Type = typename T::element_type;
  using UPtr = Dali::Graphics::UniquePtr<Type>;
  if(info.allocationCallbacks)
  {
    auto* memory = info.allocationCallbacks->allocCallback(
      sizeof(VKType),
      0,
      info.allocationCallbacks->userData);
    return UPtr(new(memory) VKType(info, controller), VKDeleter<VKType>());
  }
  else // Use standard allocator
  {
    // We are given all object for recycling
    if(oldObject)
    {
      auto reusedObject = oldObject.release();
      // If succeeded, attach the object to the unique_ptr and return it back
      if(static_cast<VKType*>(reusedObject)->TryRecycle(info, controller))
      {
        return UPtr(reusedObject, VKDeleter<VKType>());
      }
      else
      {
        // can't reuse so kill object by giving it back to original
        // unique pointer.
        oldObject.reset(reusedObject);
      }
    }

    // Create brand new object
    return UPtr(new VKType(info, controller), VKDeleter<VKType>());
  }
}

template<class T0, class T1>
T0* CastObject(T1* apiObject)
{
  return static_cast<T0*>(apiObject);
}



struct VulkanGraphicsController::Impl
{
  explicit Impl(VulkanGraphicsController& controller)
  : mGraphicsController(controller),
    mGraphicsDevice(nullptr)
  {
  }

  bool Initialize(Vulkan::Device& device)
  {
    mGraphicsDevice = &device;

    // Create factories.
    // Create pipeline cache
    // Initialize thread pool
    return true;
  }

  void AcquireNextFramebuffer()
  {
    // for all swapchains acquire new framebuffer
    auto surface = mGraphicsDevice->GetSurface( 0u );

    auto swapchain = mGraphicsDevice->GetSwapchainForFramebuffer( 0u ); // @todo Rename from Framebuffer

    if ( mGraphicsDevice->IsSurfaceResized() )
    {
      swapchain->Invalidate();
    }

    swapchain->AcquireNextFramebuffer(true);

    if( !swapchain->IsValid() )
    {
      // make sure device doesn't do any work before replacing swapchain
      mGraphicsDevice->DeviceWaitIdle();

      // replace swapchain
      swapchain = mGraphicsDevice->ReplaceSwapchainForSurface( surface, std::move(swapchain) );

      // get new valid framebuffer
      swapchain->AcquireNextFramebuffer(true);
    }
  }

  VulkanGraphicsController& mGraphicsController;
  Vulkan::Device* mGraphicsDevice;
};

VulkanGraphicsController::VulkanGraphicsController()
: mImpl(std::make_unique<Impl>(*this))
{
}

VulkanGraphicsController::~VulkanGraphicsController() = default;

void VulkanGraphicsController::Initialize(Dali::Graphics::VulkanGraphics& graphicsImplementation,
                                          Vulkan::Device& graphicsDevice)
{
  mImpl->Initialize(graphicsDevice);
}

Integration::GraphicsConfig& VulkanGraphicsController::GetGraphicsConfig()
{
  return *this;
}

void VulkanGraphicsController::SubmitCommandBuffers(const SubmitInfo& submitInfo)
{
}

void VulkanGraphicsController::PresentRenderTarget(Graphics::RenderTarget* renderTarget)
{
  // Test code to create a render pass to clear the surface
  mImpl->AcquireNextFramebuffer();

  auto swapchain = mImpl->mGraphicsDevice->GetSwapchainForFramebuffer( 0u );

  CommandPool* commandPool = mImpl->mGraphicsDevice->GetCommandPool(std::this_thread::get_id());
  auto primaryCommandBuffer = commandPool->NewCommandBuffer(true);

  primaryCommandBuffer->Begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr );
  primaryCommandBuffer->BeginRenderPass( vk::RenderPassBeginInfo{}
        .setFramebuffer(swapchain->GetCurrentFramebuffer()->GetVkHandle())
        .setRenderPass(swapchain->GetCurrentFramebuffer()->GetRenderPass())
        .setRenderArea( { {0, 0}, { swapchain->GetCurrentFramebuffer()->GetWidth(),
                                    swapchain->GetCurrentFramebuffer()->GetHeight() } } )
        .setPClearValues( swapchain->GetCurrentFramebuffer()->GetClearValues().data() )
        .setClearValueCount( uint32_t(swapchain->GetCurrentFramebuffer()->GetClearValues().size()) ), vk::SubpassContents::eInline );
  primaryCommandBuffer->EndRenderPass();
  primaryCommandBuffer->End();
  swapchain->Present();
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

void VulkanGraphicsController::GenerateTextureMipmaps(const Graphics::Texture& texture)
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

UniquePtr<Graphics::RenderTarget> VulkanGraphicsController::CreateRenderTarget(const Graphics::RenderTargetCreateInfo& renderTargetCreateInfo, UniquePtr<Graphics::RenderTarget>&& oldRenderTarget)
{
  return NewObject<Vulkan::RenderTarget>(renderTargetCreateInfo, *this, std::move(oldRenderTarget));

}

UniquePtr<Graphics::CommandBuffer> VulkanGraphicsController::CreateCommandBuffer(const Graphics::CommandBufferCreateInfo& commandBufferCreateInfo, UniquePtr<Graphics::CommandBuffer>&& oldCommandBuffer)
{
  return NewObject<Vulkan::CommandBuffer>(commandBufferCreateInfo, *this, std::move(oldCommandBuffer));
}

UniquePtr<Graphics::RenderPass> VulkanGraphicsController::CreateRenderPass(const Graphics::RenderPassCreateInfo& renderPassCreateInfo, UniquePtr<Graphics::RenderPass>&& oldRenderPass)
{
  // Do new stuff?!
  return NewObject<Vulkan::RenderPass>(renderPassCreateInfo, *this, std::move(oldRenderPass));
}

UniquePtr<Graphics::Buffer> VulkanGraphicsController::CreateBuffer(const Graphics::BufferCreateInfo& bufferCreateInfo, UniquePtr<Graphics::Buffer>&& oldBuffer)
{
  return UniquePtr<Graphics::Buffer>{};
}

UniquePtr<Graphics::Texture> VulkanGraphicsController::CreateTexture(const TextureCreateInfo& textureCreateInfo, UniquePtr<Graphics::Texture>&& oldTexture)
{
  return UniquePtr<Graphics::Texture>{};
}

UniquePtr<Graphics::Framebuffer> VulkanGraphicsController::CreateFramebuffer(const Graphics::FramebufferCreateInfo& framebufferCreateInfo, UniquePtr<Graphics::Framebuffer>&& oldFramebuffer)
{
  return UniquePtr<Graphics::Framebuffer>{};
}

UniquePtr<Graphics::Pipeline> VulkanGraphicsController::CreatePipeline(const Graphics::PipelineCreateInfo& pipelineCreateInfo, UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  return UniquePtr<Graphics::Pipeline>{};
}

UniquePtr<Graphics::Program> VulkanGraphicsController::CreateProgram(const Graphics::ProgramCreateInfo& programCreateInfo, UniquePtr<Graphics::Program>&& oldProgram)
{
  return UniquePtr<Graphics::Program>{};
}

UniquePtr<Graphics::Shader> VulkanGraphicsController::CreateShader(const Graphics::ShaderCreateInfo& shaderCreateInfo, UniquePtr<Graphics::Shader>&& oldShader)
{
  return UniquePtr<Graphics::Shader>{};
}

UniquePtr<Graphics::Sampler> VulkanGraphicsController::CreateSampler(const Graphics::SamplerCreateInfo& samplerCreateInfo, UniquePtr<Graphics::Sampler>&& oldSampler)
{
  return UniquePtr<Graphics::Sampler>{};
}

UniquePtr<Graphics::SyncObject> VulkanGraphicsController::CreateSyncObject(const Graphics::SyncObjectCreateInfo& syncObjectCreateInfo,
                                                                 UniquePtr<Graphics::SyncObject>&&     oldSyncObject)
{
  return UniquePtr<Graphics::SyncObject>{};
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

MemoryRequirements VulkanGraphicsController::GetTextureMemoryRequirements(Graphics::Texture& texture) const
{
  return MemoryRequirements{};
}

MemoryRequirements VulkanGraphicsController::GetBufferMemoryRequirements(Graphics::Buffer& buffer) const
{
  return MemoryRequirements{};
}

TextureProperties VulkanGraphicsController::GetTextureProperties(const Graphics::Texture& texture)
{
  return TextureProperties{};
}

const Reflection& VulkanGraphicsController::GetProgramReflection(const Graphics::Program& program)
{
  return *(reinterpret_cast<Reflection*>(0));
}

bool VulkanGraphicsController::PipelineEquals(const Graphics::Pipeline& pipeline0, const Graphics::Pipeline& pipeline1) const
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

void VulkanGraphicsController::Add(Vulkan::RenderTarget* surface)
{

}

void VulkanGraphicsController::DiscardResource(Vulkan::RenderTarget* surface)
{
}

Vulkan::Device& VulkanGraphicsController::GetGraphicsDevice()
{
  return *mImpl->mGraphicsDevice;
}

} //namespace Dali::Graphics
