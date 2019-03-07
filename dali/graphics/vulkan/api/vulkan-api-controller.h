#ifndef DALI_GRAPHICS_VULKAN_API_CONTROLLER_H
#define DALI_GRAPHICS_VULKAN_API_CONTROLLER_H

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
#include <dali/graphics-api/graphics-api-controller.h>
#include <dali/graphics/vulkan/internal/vulkan-buffer.h>
#include <dali/graphics/vulkan/internal/vulkan-image.h>
#include <dali/graphics/vulkan/internal/vulkan-types.h>
#include <dali/devel-api/threading/thread-pool.h>

namespace Dali
{
namespace Graphics
{

namespace Vulkan
{
class Graphics;
class Buffer;
class Image;
}

namespace VulkanAPI
{
class Controller;
class UboManager;
class DescriptorSetList;
class Buffer;
/**
 * Structure describes deferred memory transfer
 * Source memory is owned by the buffer and will be discarded
 * when transfer is completed
 */
struct BufferMemoryTransfer
{
  BufferMemoryTransfer() = default;

  ~BufferMemoryTransfer()
  {
    delete [] srcPtr;
  }

  char* srcPtr{ nullptr };
  uint32_t srcSize{ 0u };

  Vulkan::RefCountedBuffer dstBuffer;
  uint32_t dstOffset{ 0u };
};

enum class TransferRequestType
{
  BUFFER_TO_IMAGE,
  IMAGE_TO_IMAGE,
  BUFFER_TO_BUFFER,
  IMAGE_TO_BUFFER,
  USE_TBM_SURFACE,
  LAYOUT_TRANSITION_ONLY,
  UNDEFINED
};

/**
 * Structure describing blitting request Buffer to Image
 */
struct ResourceTransferRequest
{
  ResourceTransferRequest( TransferRequestType type )
  : requestType( type )
  {}

  TransferRequestType requestType;

  struct
  {
    Vulkan::RefCountedBuffer                    srcBuffer   { nullptr };  /// Source buffer
    Vulkan::RefCountedImage                     dstImage    { nullptr };  /// Destination image
    vk::BufferImageCopy                         copyInfo    { };          /// Vulkan specific copy info
  } bufferToImageInfo;

  struct
  {
    Vulkan::RefCountedImage                    srcImage    { nullptr };  /// Source image
    Vulkan::RefCountedImage                    dstImage    { nullptr };  /// Destination image
    vk::ImageCopy                              copyInfo    { };          /// Vulkan specific copy info
  } imageToImageInfo;

  struct
  {
    Vulkan::RefCountedImage                    image;
    vk::ImageLayout                            srcLayout;
    vk::ImageLayout                            dstLayout;
  } imageLayoutTransitionInfo;

  struct
  {
    Vulkan::RefCountedImage                    srcImage    { nullptr };  /// Source image
  } useTBMSurfaceInfo;

  bool                                         deferredTransferMode{ true }; // Vulkan implementation prefers deferred mode

  // delete copy
  ResourceTransferRequest( const ResourceTransferRequest& ) = delete;
  ResourceTransferRequest& operator=( const ResourceTransferRequest& ) = delete;
  ResourceTransferRequest& operator=( ResourceTransferRequest&& obj ) = delete;

  ResourceTransferRequest( ResourceTransferRequest&& obj )
  {
    requestType = obj.requestType;
    deferredTransferMode = obj.deferredTransferMode;

    if( requestType == TransferRequestType::BUFFER_TO_IMAGE )
    {
      bufferToImageInfo.srcBuffer = std::move( obj.bufferToImageInfo.srcBuffer );
      bufferToImageInfo.dstImage = std::move( obj.bufferToImageInfo.dstImage );
      bufferToImageInfo.copyInfo = std::move( obj.bufferToImageInfo.copyInfo );
    }
    else if( requestType == TransferRequestType::IMAGE_TO_IMAGE )
    {
      imageToImageInfo.srcImage = std::move( obj.imageToImageInfo.srcImage );
      imageToImageInfo.dstImage = std::move( obj.imageToImageInfo.dstImage );
      imageToImageInfo.copyInfo = std::move( obj.imageToImageInfo.copyInfo );
    }
  }

};

/**
 * @brief Interface class for Manager types in the graphics API.
 */
class Controller : public Dali::Graphics::Controller
{
public:
  /**
   * Constructor
   */
  Controller();

  void Initialise();

  /**
   * @brief Create a new object
   */
  std::unique_ptr< Dali::Graphics::Shader > CreateShader( const Dali::Graphics::BaseFactory< Dali::Graphics::Shader >& factory ) override;

  /**
   * @brief Create a new object
   */
  std::unique_ptr< Dali::Graphics::Texture > CreateTexture( const Dali::Graphics::BaseFactory< Dali::Graphics::Texture >& factory ) override;

  /**
 * @brief Create a new object
 */
  std::unique_ptr< Dali::Graphics::Buffer > CreateBuffer( const Dali::Graphics::BaseFactory< Dali::Graphics::Buffer >& factory ) override;

  /**
   * @brief Create a new object
   */
  std::unique_ptr< Dali::Graphics::Sampler > CreateSampler( const Dali::Graphics::BaseFactory< Dali::Graphics::Sampler >& factory ) override;

  /**
   * @brief Create a new object
   */
  std::unique_ptr< Dali::Graphics::Pipeline > CreatePipeline( const Dali::Graphics::PipelineFactory& factory ) override;

  /**
   * @brief Create a new object
   */
  std::unique_ptr< Dali::Graphics::Framebuffer > CreateFramebuffer( const Dali::Graphics::BaseFactory< Dali::Graphics::Framebuffer >& factory ) override;

  void SubmitCommands( std::vector< Dali::Graphics::RenderCommand* > commands ) override;

  std::unique_ptr< Dali::Graphics::RenderCommand > AllocateRenderCommand() override;

  virtual void BeginFrame() override;

  void EndFrame() override;

  void Pause() override;

  void Resume() override;

  bool EnableDepthStencilBuffer( bool enableDepth, bool enableStencil ) override;

  void RunGarbageCollector( size_t numberOfDiscardedRenderers ) override;

  void DiscardUnusedResources() override;

  bool IsDiscardQueueEmpty() override;

  bool IsDrawOnResumeRequired() override;

  void WaitIdle() override;

  void SwapBuffers() override;

  uint32_t GetSwapchainBufferCount() override;

  // VULKAN only

  uint32_t GetCurrentBufferIndex();

public:

  Vulkan::Graphics& GetGraphics() const;

  void ScheduleBufferMemoryTransfer( std::unique_ptr< VulkanAPI::BufferMemoryTransfer > transferRequest );

  void ScheduleResourceTransfer( VulkanAPI::ResourceTransferRequest&& transferRequest );

  void PushDescriptorWrite( const vk::WriteDescriptorSet& write );

  /**
   * Pushes descriptorsets to be freed by the allocator.
   * The descriptor sets must not be used any more by the renderer
   * @param descriptorSets
   */
  void FreeDescriptorSets( VulkanAPI::DescriptorSetList&& descriptorSetList );

  bool TestDescriptorSetsValid( VulkanAPI::DescriptorSetList& descriptorSetList, std::vector<bool>& results ) const;

  bool HasPendingResourceTransfers() const;

public:

  Dali::Graphics::TextureFactory& GetTextureFactory() const override;

  Dali::Graphics::ShaderFactory& GetShaderFactory() const override;

  Dali::Graphics::BufferFactory& GetBufferFactory() const override;

  Dali::Graphics::PipelineFactory& GetPipelineFactory() override;

  Dali::Graphics::FramebufferFactory& GetFramebufferFactory() const override;

  Dali::Graphics::SamplerFactory& GetSamplerFactory() override;

  void UpdateTextures( const std::vector<Dali::Graphics::TextureUpdateInfo>& updateInfoList, const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList ) override;

public:
  // not copyable
  Controller( const Controller& ) = delete;

  Controller& operator=( const Controller& ) = delete;

  ~Controller() override;

protected:
  // derived types should not be moved direcly to prevent slicing
  Controller( Controller&& ) noexcept = default;

  Controller& operator=( Controller&& ) noexcept;

private:

  /**
   * Initializes texture staging buffer of given size. May be delegated to the worker thread.
   * @param size size of the buffer
   * @param useWorkerThread if true, it will run on the worker thread
   * @return Returns Future if running parallel or empty shared_ptr
   */
  Dali::SharedFuture InitializeTextureStagingBuffer( uint32_t size, bool useWorkerThread );

private:
  struct Impl;
  std::unique_ptr< Impl > mImpl;

public:
  struct Stats
  {
    uint32_t frame { 0u };
    uint32_t uniformBufferBindings { 0u };
    uint32_t samplerTextureBindings { 0u };
  };

  void PrintStats();

  Stats mStats;
};

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali

#endif // DALI_GRAPHICS_VULKAN_API_CONTROLLER_H
