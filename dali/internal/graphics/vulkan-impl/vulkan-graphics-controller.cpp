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
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-discard-queue.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-memory.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture-dependency-checker.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#include <queue>
#include <unordered_map>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace
{
const uint32_t INITIAL_POOL_CAPACITY = 32u;
} // Anonymous namespace

namespace Dali::Graphics::Vulkan
{
/**
 * @brief Custom deleter for all Graphics objects created
 * with use of the Controller.
 *
 * When Graphics object dies the unique pointer (Graphics::UniquePtr)
 * doesn't destroy it directly but passes the ownership back
 * to the Controller. The GraphicsDeleter is responsible for passing
 * the graphics object to the discard queue (by calling Resource::DiscardResource()).
 */
template<typename T>
struct GraphicsDeleter
{
  GraphicsDeleter() = default;

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
auto NewGraphicsObject(const GfxCreateInfo& info, VulkanGraphicsController& controller, T&& oldObject)
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
    return UPtr(new(memory) VKType(info, controller), GraphicsDeleter<VKType>());
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
        return UPtr(reusedObject, GraphicsDeleter<VKType>());
      }
      else
      {
        // can't reuse so kill object by giving it back to original
        // unique pointer.
        oldObject.reset(reusedObject);
      }
    }

    // Create brand-new object
    UPtr gfxObject(new VKType(info, controller), GraphicsDeleter<VKType>());
    static_cast<VKType*>(gfxObject.get())->InitializeResource(); // @todo Consider using create queues?
    return gfxObject;
  }
}

template<class T0, class T1>
T0* CastObject(T1* apiObject)
{
  return static_cast<T0*>(apiObject);
}

namespace DepthStencilFlagBits
{
static constexpr uint32_t DEPTH_BUFFER_BIT   = 1; // depth buffer enabled
static constexpr uint32_t STENCIL_BUFFER_BIT = 2; // stencil buffer enabled
} // namespace DepthStencilFlagBits

// State of the depth-stencil buffer
using DepthStencilFlags = uint32_t;

struct VulkanGraphicsController::Impl
{
  explicit Impl(VulkanGraphicsController& controller)
  : mGraphicsController(controller),
    mDependencyChecker(controller),
    mResourceTransfer(controller)
  {
  }

  ~Impl()
  {
    if(mSamplerImpl)
    {
      mSamplerImpl->Destroy();
      mSamplerImpl = nullptr;
    }
  }

  bool Initialize(Vulkan::Device& device)
  {
    mGraphicsDevice = &device;
    mDiscardQueues.Initialize(device);
    mResourceTransfer.Initialize();

    // @todo Create pipeline cache & descriptor set allocator here
    return true;
  }

  bool EnableDepthStencilBuffer(const RenderTarget& renderTarget, bool enableDepth, bool enableStencil)
  {
    auto surface = static_cast<const Vulkan::RenderTarget*>(&renderTarget)->GetSurface();
    if(!surface)
    {
      // Do nothing if this is not a surface.
      return false;
    }

    auto renderSurface = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface);
    auto surfaceId     = renderSurface->GetSurfaceId();

    mDepthStencilBufferRequestedState = (enableDepth ? DepthStencilFlagBits::DEPTH_BUFFER_BIT : 0u) |
                                        (enableStencil ? DepthStencilFlagBits::STENCIL_BUFFER_BIT : 0u);

    auto retval = mDepthStencilBufferRequestedState != mDepthStencilBufferCurrentState;

    // @todo move state vars to surface
    if(surface && mDepthStencilBufferCurrentState != mDepthStencilBufferRequestedState)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "UpdateDepthStencilBuffer(): New state: DEPTH: %d, STENCIL: %d\n", int(mDepthStencilBufferRequestedState & 1), int((mDepthStencilBufferRequestedState >> 1) & 1));

      // Formats
      const std::array<vk::Format, 4> DEPTH_STENCIL_FORMATS = {
        vk::Format::eUndefined,     // no depth nor stencil needed
        vk::Format::eD16Unorm,      // only depth buffer
        vk::Format::eS8Uint,        // only stencil buffer
        vk::Format::eD24UnormS8Uint // depth and stencil buffers
      };

      mGraphicsDevice->DeviceWaitIdle();

      mGraphicsDevice->GetSwapchainForSurfaceId(surfaceId)->SetDepthStencil(DEPTH_STENCIL_FORMATS[mDepthStencilBufferRequestedState]);

      // make sure GPU finished any pending work
      mGraphicsDevice->DeviceWaitIdle();

      mDepthStencilBufferCurrentState = mDepthStencilBufferRequestedState;
    }
    return retval;
  }

  void GarbageCollect(bool allQueues)
  {
    if(DALI_UNLIKELY(mGraphicsDevice == nullptr))
    {
      return;
    }

    // Only garbage collect from the oldest discard queue.
    auto bufferIndex = mGraphicsDevice->GetCurrentBufferIndex() + 1;
    bufferIndex %= mGraphicsDevice->GetBufferCount();
    if(!allQueues)
    {
      mDiscardQueues.Process(bufferIndex);
    }
    else
    {
      for(uint32_t i = 0; i < mGraphicsDevice->GetBufferCount(); i++)
      {
        mDiscardQueues.Process(i);
      }
    }
  }

  void Flush()
  {
    // Flush any outstanding queues.
    GarbageCollect(false);
  }

  SamplerImpl* GetDefaultSampler()
  {
    if(!mSamplerImpl)
    {
      // Create basic sampler to reuse for any texture
      auto samplerCreateInfo = vk::SamplerCreateInfo()
                                 .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
                                 .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                                 .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                                 .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
                                 .setCompareOp(vk::CompareOp::eNever)
                                 .setMinFilter(vk::Filter::eLinear)
                                 .setMagFilter(vk::Filter::eLinear)
                                 .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                                 .setMaxAnisotropy(1.0f); // must be 1.0f when anisotropy feature isn't enabled

      samplerCreateInfo.setBorderColor(vk::BorderColor::eFloatTransparentBlack);

      mSamplerImpl.reset(SamplerImpl::New(*mGraphicsDevice, samplerCreateInfo));
    }

    return mSamplerImpl.get();
  }

  VulkanGraphicsController& mGraphicsController;
  Vulkan::Device*           mGraphicsDevice{nullptr};

  Vulkan::TextureDependencyChecker mDependencyChecker; ///< Dependencies between framebuffers/scene

  DiscardQueues<ResourceBase> mDiscardQueues;

  std::unique_ptr<SamplerImpl> mSamplerImpl{nullptr};
  DepthStencilFlags            mDepthStencilBufferCurrentState{0u};
  DepthStencilFlags            mDepthStencilBufferRequestedState{0u};

  std::unordered_map<uint32_t, Graphics::UniquePtr<Graphics::Texture>> mExternalTextureResources;        ///< Used for ResourceId.
  std::queue<const Vulkan::Texture*>                                   mTextureMipmapGenerationRequests; ///< Queue for texture mipmap generation requests
  bool                                                                 mDidPresent{false};
  ResourceTransfer                                                     mResourceTransfer;

  std::size_t mCapacity{0u}; ///< Memory Usage (of command buffers)
};

VulkanGraphicsController::VulkanGraphicsController()
: mImpl(std::make_unique<Impl>(*this))
{
}

VulkanGraphicsController::~VulkanGraphicsController()
{
  mImpl->GarbageCollect(true);
}

void VulkanGraphicsController::Initialize(Dali::Graphics::VulkanGraphics& graphicsImplementation,
                                          Vulkan::Device&                 graphicsDevice)
{
  mImpl->Initialize(graphicsDevice);
}

Integration::GraphicsConfig& VulkanGraphicsController::GetGraphicsConfig()
{
  return *this;
}

void VulkanGraphicsController::FrameStart()
{
  mImpl->mDependencyChecker.Reset(); // Clean down the dependency graph.
  mImpl->mCapacity = 0;

  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "FrameStart: bufferIndex:%u\n", mImpl->mGraphicsDevice->GetCurrentBufferIndex());
  // Check the size of the discard queues.
  auto bufferCount = mImpl->mGraphicsDevice->GetBufferCount();
  mImpl->mDiscardQueues.Resize(bufferCount);
}

void VulkanGraphicsController::ResetDidPresent()
{
  mImpl->mDidPresent = false;
}

bool VulkanGraphicsController::DidPresent() const
{
  return mImpl->mDidPresent;
}

void VulkanGraphicsController::SetResourceBindingHints(const std::vector<SceneResourceBinding>& resourceBindings)
{
  // Check if there is some extra information about used resources
  // if so then apply optimizations

  // update programs with descriptor pools
  for(auto& binding : resourceBindings)
  {
    if(binding.type == ResourceType::PROGRAM)
    {
      auto programImpl = static_cast<Vulkan::Program*>(binding.programBinding->program)->GetImplementation();

      // Start with conservative estimate, will grow dynamically on demand
      programImpl->AddDescriptorPool(INITIAL_POOL_CAPACITY);

      // Reset for fresh frame (returns all sets to free list)
      uint32_t frameIndex = mImpl->mGraphicsDevice->GetCurrentBufferIndex();
      programImpl->ResetDescriptorSetsForFrame(frameIndex);
    }
  }
}

void VulkanGraphicsController::SubmitCommandBuffers(const SubmitInfo& submitInfo)
{
  // Ensure any outstanding image transfers are complete before submitting commands for rendering
  mImpl->mResourceTransfer.WaitOnResourceTransferFutures();

  std::vector<SubmissionData> submitData;
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "SubmitCommandBuffers() bufferIndex:%d\n", mImpl->mGraphicsDevice->GetCurrentBufferIndex());

  // Gather all command buffers targeting frame buffers into a single Submit request
  for(auto gfxCmdBuffer : submitInfo.cmdBuffer)
  {
    auto cmdBuffer    = static_cast<const CommandBuffer*>(gfxCmdBuffer);
    auto renderTarget = cmdBuffer->GetRenderTarget();
    DALI_ASSERT_DEBUG(renderTarget && "Cmd buffer has no render target set.");
    if(renderTarget && renderTarget->GetSurface() == nullptr)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateSubmissionData: FBO CmdBuffer:%p\n", cmdBuffer->GetImpl());
      renderTarget->CreateSubmissionData(cmdBuffer, submitData);
    }
  }
  if(!submitData.empty())
  {
    mImpl->mGraphicsDevice->GetGraphicsQueue(0).Submit(submitData, nullptr);
  }

  // Submit each scene's cmd buffer separately, as these use EndOfFrameFence.
  for(auto gfxCmdBuffer : submitInfo.cmdBuffer)
  {
    auto cmdBuffer    = static_cast<const CommandBuffer*>(gfxCmdBuffer);
    auto renderTarget = cmdBuffer->GetRenderTarget();
    DALI_ASSERT_DEBUG(renderTarget && "Cmd buffer has no render target set.");

    if(renderTarget && renderTarget->GetSurface() != nullptr)
    {
      auto surface   = renderTarget->GetSurface();
      auto surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
      auto swapchain = GetGraphicsDevice().GetSwapchainForSurfaceId(surfaceId);

      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateSubmissionData: Surface CmdBuffer:%p\n", cmdBuffer->GetImpl());
      renderTarget->CreateSubmissionData(cmdBuffer, submitData);
      swapchain->GetQueue()->Submit(submitData, swapchain->GetEndOfFrameFence());
    }
  }

  // If flush bit set, flush all pending tasks
  if(submitInfo.flags & (0 | SubmitFlagBits::FLUSH))
  {
    Flush();
  }
}

void VulkanGraphicsController::PresentRenderTarget(Graphics::RenderTarget* renderTarget)
{
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "RenderTarget:%p  Surface:%p\n", renderTarget, static_cast<Vulkan::RenderTarget*>(renderTarget)->GetSurface());

  if(auto surface = static_cast<Vulkan::RenderTarget*>(renderTarget)->GetSurface())
  {
    const auto surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto       swapchain = mImpl->mGraphicsDevice->GetSwapchainForSurfaceId(surfaceId);
    mImpl->mDidPresent   = swapchain->Present();
    surface->PostRender();
  }
  // else no presentation required for framebuffer render target.
}

void VulkanGraphicsController::WaitIdle()
{
}

void VulkanGraphicsController::Pause()
{
}

void VulkanGraphicsController::Resume()
{
  // Draw on resume - update manager could query.
  // could use this to trigger debug output for a few frames (EGL dumps each swap-buffer)
}

void VulkanGraphicsController::Shutdown()
{
}

void VulkanGraphicsController::Destroy()
{
  mImpl->Flush();
}

void VulkanGraphicsController::UpdateTextures(
  const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
  const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList)
{
  mImpl->mResourceTransfer.UpdateTextures(updateInfoList, sourceList);
}

void VulkanGraphicsController::GenerateTextureMipmaps(const Graphics::Texture& texture)
{
}

bool VulkanGraphicsController::EnableDepthStencilBuffer(
  const Graphics::RenderTarget& gfxRenderTarget,
  bool                          enableDepth,
  bool                          enableStencil)
{
  // if we enable depth/stencil dynamically we need to block and invalidate pipeline cache
  auto renderTarget = static_cast<const Vulkan::RenderTarget*>(&gfxRenderTarget);
  return mImpl->EnableDepthStencilBuffer(*renderTarget, enableDepth, enableStencil);
}

void VulkanGraphicsController::RunGarbageCollector(size_t numberOfDiscardedRenderers)
{
  mImpl->GarbageCollect(false);
}

void VulkanGraphicsController::DiscardUnusedResources()
{
}

bool VulkanGraphicsController::IsDiscardQueueEmpty()
{
  auto bufferIndex = mImpl->mGraphicsDevice->GetCurrentBufferIndex();
  return mImpl->mDiscardQueues.IsEmpty(bufferIndex);
}

bool VulkanGraphicsController::IsDrawOnResumeRequired()
{
  return true;
}

UniquePtr<Graphics::RenderTarget> VulkanGraphicsController::CreateRenderTarget(const Graphics::RenderTargetCreateInfo& renderTargetCreateInfo, UniquePtr<Graphics::RenderTarget>&& oldRenderTarget)
{
  auto renderTarget = NewGraphicsObject<Vulkan::RenderTarget>(renderTargetCreateInfo, *this, std::move(oldRenderTarget));
  //mImpl->mDependencyChecker.AddRenderTarget(CastObject<Vulkan::RenderTarget>(renderTarget.get()));
  return renderTarget;
}

UniquePtr<Graphics::CommandBuffer> VulkanGraphicsController::CreateCommandBuffer(const Graphics::CommandBufferCreateInfo& commandBufferCreateInfo, UniquePtr<Graphics::CommandBuffer>&& oldCommandBuffer)
{
  auto commandBuffer = NewGraphicsObject<Vulkan::CommandBuffer>(commandBufferCreateInfo, *this, std::move(oldCommandBuffer));
  return commandBuffer;
}

UniquePtr<Graphics::RenderPass> VulkanGraphicsController::CreateRenderPass(const Graphics::RenderPassCreateInfo& renderPassCreateInfo, UniquePtr<Graphics::RenderPass>&& oldRenderPass)
{
  auto renderPass = UniquePtr<Graphics::RenderPass>(new Vulkan::RenderPass(renderPassCreateInfo, *this));

  // Don't create actual vulkan resource here. It will instead be done on demand. (e.g. framebuffer creation, CommandBuffer::BeginRenderPass())
  return renderPass;
}

UniquePtr<Graphics::Buffer> VulkanGraphicsController::CreateBuffer(const Graphics::BufferCreateInfo& bufferCreateInfo, UniquePtr<Graphics::Buffer>&& oldBuffer)
{
  return NewGraphicsObject<Vulkan::Buffer>(bufferCreateInfo, *this, std::move(oldBuffer));
}

UniquePtr<Graphics::Texture> VulkanGraphicsController::CreateTexture(const Graphics::TextureCreateInfo& textureCreateInfo, UniquePtr<Graphics::Texture>&& oldTexture)
{
  return NewGraphicsObject<Vulkan::Texture>(textureCreateInfo, *this, std::move(oldTexture));
}

UniquePtr<Graphics::Framebuffer> VulkanGraphicsController::CreateFramebuffer(const Graphics::FramebufferCreateInfo& framebufferCreateInfo, UniquePtr<Graphics::Framebuffer>&& oldFramebuffer)
{
  return NewGraphicsObject<Vulkan::Framebuffer>(framebufferCreateInfo, *this, std::move(oldFramebuffer));
}

UniquePtr<Graphics::Pipeline> VulkanGraphicsController::CreatePipeline(const Graphics::PipelineCreateInfo& pipelineCreateInfo, UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  return UniquePtr<Graphics::Pipeline>(new Vulkan::Pipeline(pipelineCreateInfo, *this, nullptr));
}

UniquePtr<Graphics::Program> VulkanGraphicsController::CreateProgram(const Graphics::ProgramCreateInfo& programCreateInfo, UniquePtr<Graphics::Program>&& oldProgram)
{
  return UniquePtr<Graphics::Program>(new Vulkan::Program(programCreateInfo, *this));
}

UniquePtr<Graphics::Shader> VulkanGraphicsController::CreateShader(const Graphics::ShaderCreateInfo& shaderCreateInfo, UniquePtr<Graphics::Shader>&& oldShader)
{
  return UniquePtr<Graphics::Shader>(new Vulkan::Shader(shaderCreateInfo, *this));
}

UniquePtr<Graphics::Sampler> VulkanGraphicsController::CreateSampler(const Graphics::SamplerCreateInfo& samplerCreateInfo, UniquePtr<Graphics::Sampler>&& oldSampler)
{
  return NewGraphicsObject<Vulkan::Sampler>(samplerCreateInfo, *this, std::move(oldSampler));
}

UniquePtr<Graphics::SyncObject> VulkanGraphicsController::CreateSyncObject(const Graphics::SyncObjectCreateInfo& syncObjectCreateInfo,
                                                                           UniquePtr<Graphics::SyncObject>&&     oldSyncObject)
{
  return UniquePtr<Graphics::SyncObject>{};
}

void VulkanGraphicsController::DiscardResource(ResourceBase* resource)
{
  mImpl->mDiscardQueues.Discard(resource);
}

UniquePtr<Graphics::Memory> VulkanGraphicsController::MapBufferRange(const MapBufferInfo& mapInfo)
{
  // @todo: Process create queues
  auto        buffer     = static_cast<Vulkan::Buffer*>(mapInfo.buffer);
  BufferImpl* bufferImpl = buffer->GetImpl();
  DALI_ASSERT_DEBUG(bufferImpl && "Mapping CPU allocated buffer is not used in Vulkan");
  if(bufferImpl)
  {
    auto memoryImpl = bufferImpl->GetMemory();
    auto memory     = UniquePtr<Memory>(new Memory(mapInfo, *this));
    memory->Initialize(memoryImpl);
    return memory;
  }
  return nullptr;
}

UniquePtr<Graphics::Memory> VulkanGraphicsController::MapTextureRange(const MapTextureInfo& mapInfo)
{
  // Not implemented (@todo Remove from Graphics API?
  return nullptr;
}

void VulkanGraphicsController::UnmapMemory(UniquePtr<Graphics::Memory> memory)
{
  // Do nothing. Let unique ptr die, and deal with it in the destructor.
}

MemoryRequirements VulkanGraphicsController::GetBufferMemoryRequirements(Graphics::Buffer& gfxBuffer) const
{
  auto        buffer     = static_cast<Vulkan::Buffer*>(&gfxBuffer);
  BufferImpl* bufferImpl = buffer->GetImpl();
  return bufferImpl->GetMemoryRequirements();
}

MemoryRequirements VulkanGraphicsController::GetTextureMemoryRequirements(Graphics::Texture& gfxTexture) const
{
  const Vulkan::Texture* texture = static_cast<const Vulkan::Texture*>(&gfxTexture);
  return texture->GetMemoryRequirements();
}

TextureProperties VulkanGraphicsController::GetTextureProperties(const Graphics::Texture& gfxTexture)
{
  auto* texture = const_cast<Vulkan::Texture*>(static_cast<const Vulkan::Texture*>(&gfxTexture));
  return texture->GetProperties();
}

const Graphics::Reflection& VulkanGraphicsController::GetProgramReflection(const Graphics::Program& program)
{
  return (static_cast<const Vulkan::Program*>(&program))->GetReflection();
}

bool VulkanGraphicsController::PipelineEquals(const Graphics::Pipeline& pipeline0, const Graphics::Pipeline& pipeline1) const
{
  return true;
}

bool VulkanGraphicsController::GetProgramParameter(Graphics::Program& program, uint32_t parameterId, void* outData)
{
  return false;
}

uint32_t VulkanGraphicsController::GetDeviceLimitation(Graphics::DeviceCapability capability)
{
  if(capability == DeviceCapability::MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT)
  {
    const auto& properties = mImpl->mGraphicsDevice->GetPhysicalDeviceProperties();
    return properties.limits.minUniformBufferOffsetAlignment;
  }
  return 0;
}

bool VulkanGraphicsController::IsBlendEquationSupported(DevelBlendEquation::Type blendEquation)
{
  switch(blendEquation)
  {
    case DevelBlendEquation::ADD:
    case DevelBlendEquation::SUBTRACT:
    case DevelBlendEquation::REVERSE_SUBTRACT:
    case DevelBlendEquation::MIN:
    case DevelBlendEquation::MAX:
    {
      return true;
    }
    case DevelBlendEquation::MULTIPLY:
    case DevelBlendEquation::SCREEN:
    case DevelBlendEquation::OVERLAY:
    case DevelBlendEquation::DARKEN:
    case DevelBlendEquation::LIGHTEN:
    case DevelBlendEquation::COLOR_DODGE:
    case DevelBlendEquation::COLOR_BURN:
    case DevelBlendEquation::HARD_LIGHT:
    case DevelBlendEquation::SOFT_LIGHT:
    case DevelBlendEquation::DIFFERENCE:
    case DevelBlendEquation::EXCLUSION:
    case DevelBlendEquation::HUE:
    case DevelBlendEquation::SATURATION:
    case DevelBlendEquation::COLOR:
    case DevelBlendEquation::LUMINOSITY:
    {
      return IsAdvancedBlendEquationSupported();
    }
    default:
    {
      return false;
    }
  }
  return false;
}

bool VulkanGraphicsController::IsAdvancedBlendEquationSupported()
{
  //@todo Implement this!
  return false;
}

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

Vulkan::Device& VulkanGraphicsController::GetGraphicsDevice()
{
  return *mImpl->mGraphicsDevice;
}

Graphics::Texture* VulkanGraphicsController::CreateTextureByResourceId(uint32_t resourceId, const Graphics::TextureCreateInfo& createInfo)
{
  Graphics::UniquePtr<Graphics::Texture> texture;

  // Check that this resource id hasn't been used previously
  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  DALI_ASSERT_ALWAYS(iter == mImpl->mExternalTextureResources.end());

  texture = CreateTexture(createInfo, std::move(texture));

  auto gfxTexture = texture.get();
  mImpl->mExternalTextureResources.insert(std::make_pair(resourceId, std::move(texture)));

  return gfxTexture;
}

void VulkanGraphicsController::DiscardTextureFromResourceId(uint32_t resourceId)
{
  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  if(iter != mImpl->mExternalTextureResources.end())
  {
    mImpl->mExternalTextureResources.erase(iter);
  }
}

Graphics::Texture* VulkanGraphicsController::GetTextureFromResourceId(uint32_t resourceId)
{
  Graphics::Texture* gfxTexture = nullptr;

  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  if(iter != mImpl->mExternalTextureResources.end())
  {
    gfxTexture = iter->second.get();
  }

  return gfxTexture;
}

Graphics::UniquePtr<Graphics::Texture> VulkanGraphicsController::ReleaseTextureFromResourceId(uint32_t resourceId)
{
  Graphics::UniquePtr<Graphics::Texture> gfxTexture;

  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  if(iter != mImpl->mExternalTextureResources.end())
  {
    gfxTexture = std::move(iter->second);
    mImpl->mExternalTextureResources.erase(iter);
  }

  return gfxTexture;
}

void VulkanGraphicsController::Flush()
{
  mImpl->Flush();
}

std::size_t VulkanGraphicsController::GetCapacity() const
{
  return mImpl->mCapacity;
}

bool VulkanGraphicsController::HasClipMatrix() const
{
  return true;
}

const Matrix& VulkanGraphicsController::GetClipMatrix() const
{
  constexpr float CLIP_MATRIX_DATA[] = {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};
  static const Matrix CLIP_MATRIX(CLIP_MATRIX_DATA);
  return CLIP_MATRIX;
}

void VulkanGraphicsController::AddTextureDependencies(RenderTarget* renderTarget)
{
  auto framebuffer = renderTarget->GetFramebuffer();
  DALI_ASSERT_DEBUG(framebuffer);

  for(auto attachment : framebuffer->GetCreateInfo().colorAttachments)
  {
    auto texture = CastObject<Vulkan::Texture>(attachment.texture);
    mImpl->mDependencyChecker.AddTexture(texture, renderTarget);
  }
  auto depthAttachment = framebuffer->GetCreateInfo().depthStencilAttachment;
  if(depthAttachment.depthTexture)
  {
    mImpl->mDependencyChecker.AddTexture(CastObject<Vulkan::Texture>(depthAttachment.depthTexture), renderTarget);
  }
  if(depthAttachment.stencilTexture)
  {
    mImpl->mDependencyChecker.AddTexture(CastObject<Vulkan::Texture>(depthAttachment.stencilTexture), renderTarget);
  }
}

void VulkanGraphicsController::CheckTextureDependencies(
  const std::vector<Graphics::TextureBinding>& textureBindings,
  RenderTarget*                                renderTarget)
{
  for(auto& binding : textureBindings)
  {
    if(binding.texture)
    {
      auto texture = CastObject<const Vulkan::Texture>(binding.texture);
      mImpl->mDependencyChecker.CheckNeedsSync(const_cast<Texture*>(texture), renderTarget);
    }
  }
}

void VulkanGraphicsController::RemoveRenderTarget(RenderTarget* renderTarget)
{
  mImpl->mDependencyChecker.RemoveRenderTarget(renderTarget);
}

SamplerImpl* VulkanGraphicsController::GetDefaultSampler()
{
  return mImpl->GetDefaultSampler();
}

} // namespace Dali::Graphics::Vulkan
