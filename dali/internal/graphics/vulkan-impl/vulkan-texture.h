#ifndef DALI_GRAPHICS_VULKAN_TEXTURE_IMPL_H
#define DALI_GRAPHICS_VULKAN_TEXTURE_IMPL_H

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

#include <dali/graphics-api/graphics-texture-create-info.h>
#include <dali/graphics-api/graphics-texture.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-native-image-handler.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture-array.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class Memory;
class Image;
class ImageView;
class SamplerImpl;

/**
 * @brief Abstract interface for surface reference management
 *
 * This interface allows the Vulkan texture to manage surface lifetimes.
 * The texture uses this to acquire/release references to surfaces it's using.
 */
class SurfaceReferenceManager
{
public:
  virtual ~SurfaceReferenceManager() = default;

  /**
   * @brief Acquire a reference to a surface
   * @param surface The surface handle (opaque pointer)
   */
  virtual void AcquireSurfaceReference(void* surface) = 0;

  /**
   * @brief Release a reference to a surface
   * @param surface The surface handle (opaque pointer)
   */
  virtual void ReleaseSurfaceReference(void* surface) = 0;
};

using TextureResource = Resource<Graphics::Texture, Graphics::TextureCreateInfo>;

/**
 * Texture wraps Vulkan Image and ImageView.
 * Texture can be used for any kind of texture that Dali needs, e.g. images for sampling,
 * cubemaps, color/depth/stencil attachments for writing / reading etc.
 *
 * In the case of color attachments created in the same frame, we batch up all such attachments
 * into a single texture array, which is much more memory efficient. In this case, the Texture
 * object will own the image array and all the image views; it will get dropped when the last
 * framebuffer is destroyed.
 * In this case, a "sub" texture can be created from this texture that references the main image
 * and a single imageview, this doesn't own either resource.
 */
class Texture : public TextureResource
{
public:
  Texture(const Graphics::TextureCreateInfo& createInfo, VulkanGraphicsController& controller);
  ~Texture();

  /**
   * @copydoc Graphics::Vulkan::Resource::InitializeResource();
   */
  InitializationResult InitializeResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::DiscardResource();
   */
  void DiscardResource() override;
  /**
   * @copydoc Graphics::Vulkan::Resource::DestroyResource();
   */
  void DestroyResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::GetAllocationCallbacks()
   */
  [[nodiscard]] const Graphics::AllocationCallbacks* GetAllocationCallbacks() const override
  {
    return mCreateInfo.allocationCallbacks;
  }

  /**
   * @copydoc Graphics::Vulkan::Resource::InvokeDeleter()
   * Only intended for use by discard queue.
   */
  void InvokeDeleter() override
  {
    this->~Texture();
  }

  bool IsSamplerImmutable() const;

  [[nodiscard]] Image* GetImage() const;

  ImageView* GetImageView() const;
  ImageView* GetImageView(uint32_t layer) const;

  SamplerImpl* GetDefaultSampler() const;

  // @todo Temporary. We need to use the newer APIs
  enum UpdateMode
  {
    UNDEFINED,
    DEFERRED,
    IMMEDIATE
  };

  void SetFormatAndUsage();

  Dali::Graphics::MemoryRequirements GetMemoryRequirements() const;

  bool InitializeNativeTexture();

  bool Initialize(int numLayers = 1);

  void PrepareTexture();

  bool InitializeTextureArray(uint32_t arrayLayers);

  bool InitializeFromTextureArray(TextureArray* texture, uint32_t layer);

  /**
   * Returns structure with texture properties
   * @return The reference to immutable TextureProperties object
   */
  const Dali::Graphics::TextureProperties& GetProperties();

  /**
   * Initialises image views for the image. If there are multiple layers
   * and this is a color attachment type, then generate an image view per
   * layer, as we are batching FBOs.
   */
  void InitializeImageViews();

  /**
   * Tries to convert pixel data to the compatible format. As result it returns new buffer.
   * @param pData source data
   * @param sizeInBytes size of source data in bytes
   * @param width width in pixels
   * @param height height in pixels
   * @param outputBuffer reference to an output buffer
   * @return True if conversion was successful
   */
  bool TryConvertPixelData(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, std::vector<uint8_t>& outputBuffer);

  /**
   * Tries to convert pixel data to the compatible format. The result is written into the specified memory area.
   * The memory must be allocated and large enough to accomodate output data.
   * @param pData  source data
   * @param sizeInBytes  size of source data in bytes
   * @param width width in pixels
   * @param height height in pixels
   * @param pOutputBuffer pointer to a valid output buffer
   * @return True if conversion was successful
   */
  bool TryConvertPixelData(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, void* pOutputBuffer);

  /**
   * Tries to convert pixel data to the compatible format. The result is written into the specified memory area.
   * The memory must be allocated and large enough to accomodate output data.
   * @param pData  source data
   * @param srcFormat The source format
   * @param sizeInBytes  size of source data in bytes
   * @param width width in pixels
   * @param height height in pixels
   * @param pOutputBuffer pointer to a valid output buffer
   * @return True if conversion was successful
   */
  bool TryConvertPixelData(const void* pData, Graphics::Format srcFormat, uint32_t sizeInBytes, uint32_t width, uint32_t height, void* pOutputBuffer);

  TextureTiling GetTiling() const
  {
    return mTiling;
  }

  bool WasInitializationDeferred() const
  {
    return mInitializationDeferred;
  }
  uint32_t GetWidth() const
  {
    return mWidth;
  }
  uint32_t GetHeight() const
  {
    return mHeight;
  }
  uint32_t GetMipLevelCount()
  {
    return mMaxMipMapLevel;
  }

private:
  /**
   * Validates initial format
   * @return if valid, returns existing format
   *         if possible conversion, returns new converted format
   *         if not supported returns vk::Format::eUndefined
   */
  vk::Format ValidateFormat(vk::Format sourceFormat);
  vk::Format ValidateCompressedFormat(vk::Format sourceFormat);

  vk::Format ConvertApiToVk(Dali::Graphics::Format format);

  vk::ComponentMapping GetVkComponentMapping(Dali::Graphics::Format format);

private:
  Vulkan::Device&         mDevice;
  TextureArrayHandle      mTextureArray;
  Image*                  mImage;
  std::vector<ImageView*> mImageViews;
  SamplerImpl*            mSampler{nullptr};

  uint32_t             mWidth{0u};
  uint32_t             mHeight{0u};
  uint32_t             mArrayLayers{1};
  uint32_t             mMaxMipMapLevel{1u};
  vk::Format           mFormat{vk::Format::eUndefined};
  vk::Format           mConvertFromFormat{vk::Format::eUndefined};
  vk::ImageUsageFlags  mUsage{};
  vk::ImageLayout      mLayout{vk::ImageLayout::eUndefined};
  vk::ComponentMapping mComponentMapping{};

  uint32_t                      refcount{0};
  bool                          mInitializationDeferred{false};
  bool                          mDisableStagingBuffer{false};
  Dali::Graphics::TextureTiling mTiling{Dali::Graphics::TextureTiling::OPTIMAL};

  std::unique_ptr<Dali::Graphics::TextureProperties> mProperties;

  bool mIsNativeImage{false};
  bool mIsYUVFormat{false};

  enum class NativeImageType
  {
    NONE,
    NATIVE_IMAGE_SOURCE,
    NATIVE_IMAGE_SOURCE_QUEUE
  };
  NativeImageType mNativeImageType{NativeImageType::NONE};

  // Native image handler and resources
  std::unique_ptr<VulkanNativeImageHandler> mNativeImageHandler;         ///< Handler for native image operations
  std::unique_ptr<NativeImageResources>     mNativeResources;            ///< Native image resources
  void*                                     mCurrentSurface{nullptr};    ///< Currently referenced surface
  bool                                      mHasSurfaceReference{false}; ///< Whether we have acquired a surface reference
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_TEXTURE_IMPL_H
