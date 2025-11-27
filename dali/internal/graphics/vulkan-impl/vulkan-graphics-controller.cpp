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
#include <dali/devel-api/threading/conditional-wait.h>
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

#include "vulkan-framebuffer-impl.h"

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
    // @todo Consider using create queues?
    ResourceBase::InitializationResult result = static_cast<VKType*>(gfxObject.get())->InitializeResource();
    if(result == ResourceBase::InitializationResult::NOT_SUPPORTED)
    {
      DALI_LOG_ERROR("Vulkan resource failed to initialize.");
    }
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
    if(!mTextureArrays.empty())
    {
      for(auto textureArray : mTextureArrays)
      {
        textureArray->DestroyResource();
      }
      mTextureArrays.clear();
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

  void CreateDeferredTextures()
  {
    // Expect only color attachments. For better performance, we will batch all images with the same size
    // into a render target array.
    std::unordered_map<Uint16Pair::DataType, std::vector<Vulkan::Texture*>> texturesBySize{};
    for(auto texture : mDeferredTextures)
    {
      Uint16Pair           imgSize(texture->GetWidth(), texture->GetHeight());
      Uint16Pair::DataType data = *reinterpret_cast<Uint16Pair::DataType*>(&imgSize);
      if(auto iter = texturesBySize.find(data); iter != texturesBySize.end())
      {
        iter->second.emplace_back(texture);
      }
      else
      {
        texturesBySize.insert(std::pair<const Uint16Pair::DataType, std::vector<Texture*>>{data, {texture}});
      }
    }
    mDeferredTextures.clear();
    const auto MAX_ARRAY_LAYERS = mGraphicsDevice->GetPhysicalDeviceProperties().limits.maxImageArrayLayers;

    for(auto keyValue : texturesBySize)
    {
      uint32_t arrayLayers = keyValue.second.size();
      auto&    textures    = keyValue.second;
      if(arrayLayers > 1)
      {
        const int numTextureArrays    = 1 + arrayLayers / MAX_ARRAY_LAYERS; // Batch into numTextures.
        int       currentTextureSlice = 0;                                  // index of "child" texture in batched texture array.
        for(int i = 0; i < numTextureArrays; ++i)
        {
          uint32_t numArrayLayers = MAX_ARRAY_LAYERS;
          if(i == numTextureArrays - 1)
          {
            numArrayLayers = arrayLayers % MAX_ARRAY_LAYERS; // Number of layers in this batch.
          }

          auto          createInfo   = keyValue.second[0]->GetCreateInfo();
          TextureArray* textureArray = TextureArray::New(createInfo, mGraphicsController, numArrayLayers);
          mTextureArrays.emplace_back(textureArray); // Each Batch texture owned by controller.

          for(uint32_t layer = 0; layer < numArrayLayers; ++layer)
          {
            // Initialize each "child" texture to use batch for image/image view.
            textures[currentTextureSlice++]->InitializeFromTextureArray(textureArray, layer);
          }
          DALI_LOG_RELEASE_INFO("<=> Created texture array of size %u for %dx%d", numArrayLayers, textureArray->GetWidth(), textureArray->GetHeight());
        }
      }
      else
      {
        // Only 1 texture - initialize normally.
        textures[0]->Initialize();
        textures[0]->GetProperties();
      }
    }
  }

  void RemoveTextureArray(TextureArray* textureArray)
  {
    if(auto iter = std::find(mTextureArrays.begin(), mTextureArrays.end(), textureArray);
       iter != mTextureArrays.end())
    {
      mTextureArrays.erase(iter);
    }
    textureArray->DestroyResource();
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
  std::vector<Vulkan::Texture*>    mDeferredTextures;
  DiscardQueues<ResourceBase>      mDiscardQueues;

  std::unique_ptr<SamplerImpl> mSamplerImpl{nullptr};
  DepthStencilFlags            mDepthStencilBufferCurrentState{0u};
  DepthStencilFlags            mDepthStencilBufferRequestedState{0u};

  std::vector<TextureArray*>                                           mTextureArrays;                   ///< Batched textures (for color attachments)
  std::unordered_map<uint32_t, Graphics::UniquePtr<Graphics::Texture>> mExternalTextureResources;        ///< Used for ResourceId.
  std::queue<const Vulkan::Texture*>                                   mTextureMipmapGenerationRequests; ///< Queue for texture mipmap generation requests
  bool                                                                 mDidPresent{false};
  ResourceTransfer                                                     mResourceTransfer;

  std::size_t mCapacity{0u}; ///< Memory Usage (of command buffers)

  // Logical device creation synchronization
  Dali::ConditionalWait mLogicalDeviceCreatedWaitCondition;
  bool                  mIsLogicalDeviceCreated{false};

  bool mIsAdvancedBlendEquationSupported{false};
  bool mIsAdvancedBlendEquationCached{false};
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

void VulkanGraphicsController::RenderStart()
{
  if(!mImpl->mDeferredTextures.empty())
  {
    mImpl->CreateDeferredTextures();
  }
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

  std::vector<SubmissionData> fboSubmitData;
  std::vector<SubmissionData> surfaceSubmitData;
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "SubmitCommandBuffers() bufferIndex:%d\n", mImpl->mGraphicsDevice->GetCurrentBufferIndex());

  // Gather all command buffers targeting frame buffers into a single Submit request
  RenderTarget* currentRenderSurface = nullptr;
  for(auto gfxCmdBuffer : submitInfo.cmdBuffer)
  {
    auto cmdBuffer    = static_cast<const CommandBuffer*>(gfxCmdBuffer);
    auto renderTarget = cmdBuffer->GetRenderTarget();

    if(!renderTarget)
    {
      DALI_LOG_ERROR("Cmd buffer has no render target set.");
      continue;
    }
    if(renderTarget->GetSurface() == nullptr)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateSubmissionData: FBO CmdBuffer:%p\n", cmdBuffer->GetImpl());
      renderTarget->CreateSubmissionData(cmdBuffer, fboSubmitData);
    }
    else
    {
      if(currentRenderSurface == nullptr)
      {
        currentRenderSurface = renderTarget;
      }
      else
      {
        DALI_ASSERT_DEBUG(currentRenderSurface == renderTarget);
      }
      renderTarget->CreateSubmissionData(cmdBuffer, surfaceSubmitData);
    }
  }
  if(!fboSubmitData.empty())
  {
    mImpl->mGraphicsDevice->GetGraphicsQueue(0).Submit(fboSubmitData, nullptr);
  }

  if(!surfaceSubmitData.empty())
  {
    auto surface   = currentRenderSurface->GetSurface();
    auto surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto swapchain = mImpl->mGraphicsDevice->GetSwapchainForSurfaceId(surfaceId);

    swapchain->UpdateSubmissionData(surfaceSubmitData);

    swapchain->GetQueue()->Submit(surfaceSubmitData, swapchain->GetEndOfFrameFence());
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
  mImpl->mDependencyChecker.AddRenderTarget(CastObject<Vulkan::RenderTarget>(renderTarget.get()));
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

  // Don't create actual vulkan resource here. It will instead be done on demand. (e.g. framebuffer creation, CommandBuffer::BeginRenderPass())10
  return renderPass;
}

UniquePtr<Graphics::Buffer> VulkanGraphicsController::CreateBuffer(const Graphics::BufferCreateInfo& bufferCreateInfo, UniquePtr<Graphics::Buffer>&& oldBuffer)
{
  return NewGraphicsObject<Vulkan::Buffer>(bufferCreateInfo, *this, std::move(oldBuffer));
}

UniquePtr<Graphics::Texture> VulkanGraphicsController::CreateTexture(const Graphics::TextureCreateInfo& textureCreateInfo, UniquePtr<Graphics::Texture>&& oldTexture)
{
  auto gfxTexture = NewGraphicsObject<Vulkan::Texture>(textureCreateInfo, *this, std::move(oldTexture));
  if(auto texture = static_cast<Vulkan::Texture*>(gfxTexture.get()); texture->WasInitializationDeferred())
  {
    mImpl->mDeferredTextures.emplace_back(texture);
  }
  return gfxTexture;
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
  // If it is a texture, remove it from mDeferredTextures.
  if(auto texture = dynamic_cast<Vulkan::Texture*>(resource); texture != nullptr)
  {
    if(texture->WasInitializationDeferred())
    {
      if(auto gfxTexture = std::find(mImpl->mDeferredTextures.begin(), mImpl->mDeferredTextures.end(), resource);
         gfxTexture != mImpl->mDeferredTextures.end())
      {
        mImpl->mDeferredTextures.erase(gfxTexture);
      }
    }
  }
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

bool VulkanGraphicsController::IsCompatible(
  const Graphics::RenderTarget& gfxRenderTargetA, const Graphics::RenderTarget& gfxRenderTargetB, const Graphics::RenderPass& gfxRenderPassA, const Graphics::RenderPass& gfxRenderPassB)
{
  // If render target and render passes are compatible, we can re-use same pipeline.
  auto renderTargetA = CastObject<const Vulkan::RenderTarget>(&gfxRenderTargetA);
  auto renderTargetB = CastObject<const Vulkan::RenderTarget>(&gfxRenderTargetB);
  auto fboA          = const_cast<Vulkan::Framebuffer*>(CastObject<Vulkan::Framebuffer>(renderTargetA->GetCreateInfo().framebuffer));
  auto fboB          = const_cast<Vulkan::Framebuffer*>(CastObject<Vulkan::Framebuffer>(renderTargetB->GetCreateInfo().framebuffer));
  auto renderPassA   = const_cast<Vulkan::RenderPass*>(CastObject<const Vulkan::RenderPass>(&gfxRenderPassA));
  auto renderPassB   = const_cast<Vulkan::RenderPass*>(CastObject<const Vulkan::RenderPass>(&gfxRenderPassB));

  if(fboA != nullptr && fboB != nullptr)
  {
    auto fboImplA        = fboA->GetImpl();
    auto fboImplB        = fboB->GetImpl();
    auto renderPassImplA = fboImplA->GetImplFromRenderPass(renderPassA);
    auto renderPassImplB = fboImplB->GetImplFromRenderPass(renderPassB);
    return renderPassImplA->IsCompatible(renderPassImplB);
  }

  auto surfaceA = renderTargetA->GetCreateInfo().surface;
  auto surfaceB = renderTargetB->GetCreateInfo().surface;
  return surfaceA == surfaceB;
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
  // Wait for logical device creation if not ready
  {
    Dali::ConditionalWait::ScopedLock lock(mImpl->mLogicalDeviceCreatedWaitCondition);
    if(!mImpl->mIsLogicalDeviceCreated && !mImpl->mIsAdvancedBlendEquationCached)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "IsAdvancedBlendEquationSupported: waiting for logical device creation\n");
      mImpl->mLogicalDeviceCreatedWaitCondition.Wait(lock);
    }
  }

  if(mImpl->mIsAdvancedBlendEquationCached)
  {
    return mImpl->mIsAdvancedBlendEquationSupported;
  }
  else
  {
    auto& device = mImpl->mGraphicsDevice;
    return device && device->IsAdvancedBlendingSupported();
  }
}

void VulkanGraphicsController::SetIsAdvancedBlendEquationSupported(bool isSupported)
{
  mImpl->mIsAdvancedBlendEquationSupported = isSupported;
  mImpl->mIsAdvancedBlendEquationCached    = true;
}

uint32_t VulkanGraphicsController::GetMaxTextureSize()
{
  // Wait for logical device creation if not ready
  {
    Dali::ConditionalWait::ScopedLock lock(mImpl->mLogicalDeviceCreatedWaitCondition);
    if(!mImpl->mIsLogicalDeviceCreated)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "GetMaxTextureSize: waiting for logical device creation\n");
      mImpl->mLogicalDeviceCreatedWaitCondition.Wait(lock);
    }
  }

  // Query max texture size from Vulkan physical device properties
  const auto& properties = mImpl->mGraphicsDevice->GetPhysicalDeviceProperties();
  return properties.limits.maxImageDimension2D;
}

uint32_t VulkanGraphicsController::GetMaxCombinedTextureUnits()
{
  // Wait for logical device creation if not ready
  {
    Dali::ConditionalWait::ScopedLock lock(mImpl->mLogicalDeviceCreatedWaitCondition);
    if(!mImpl->mIsLogicalDeviceCreated)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "GetMaxCombinedTextureUnits: waiting for logical device creation\n");
      mImpl->mLogicalDeviceCreatedWaitCondition.Wait(lock);
    }
  }

  // Query max combined texture units from Vulkan physical device properties
  const auto& properties = mImpl->mGraphicsDevice->GetPhysicalDeviceProperties();
  return properties.limits.maxPerStageDescriptorSamplers;
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

void VulkanGraphicsController::NotifyLogicalDeviceCreated()
{
  // Signal that logical device is created
  {
    Dali::ConditionalWait::ScopedLock lock(mImpl->mLogicalDeviceCreatedWaitCondition);
    mImpl->mIsLogicalDeviceCreated = true;
    mImpl->mLogicalDeviceCreatedWaitCondition.Notify(lock);
  }
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

void VulkanGraphicsController::RemoveTextureArray(TextureArray* textureArray)
{
  mImpl->RemoveTextureArray(textureArray);
}

} // namespace Dali::Graphics::Vulkan
