#ifndef DALI_INTERNAL_NATIVE_IMAGE_IMPL_H
#define DALI_INTERNAL_NATIVE_IMAGE_IMPL_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/public-api/adaptor-framework/native-image.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal NativeImage.
 */
class NativeImage
{
public:
  static constexpr uint32_t DEFAULT_QUALITY = 100;

  /**
   * Create a new NativeImage internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImage contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImage* New(uint32_t                      width,
                          uint32_t                      height,
                          Dali::NativeImage::ColorDepth depth,
                          Any                           nativeImage);
  /**
   * @copydoc Dali::NativeImage::GetNativeImage()
   */
  virtual Any GetNativeImage() const = 0;

  /**
   * @copydoc Dali::NativeImage::GetPixels()
   */
  virtual bool GetPixels(Dali::Vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const = 0;

  /**
   * @copydoc Dali::NativeImage::SetPixels()
   */
  virtual bool SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat) = 0;

  /**
   * @copydoc Dali::NativeImage::SetSource( Any source )
   */
  virtual void SetSource(Any source) = 0;

  /**
   * @copydoc Dali::NativeImage::IsColorDepthSupported( ColorDepth colorDepth )
   */
  virtual bool IsColorDepthSupported(Dali::NativeImage::ColorDepth colorDepth) = 0;

  /**
   * destructor
   */
  virtual ~NativeImage() = default;

  /**
   * @copydoc Dali::NativeImage::CreateResource()
   */
  virtual bool CreateResource() = 0;

  /**
   * @copydoc Dali::NativeImage::DestroyResource()
   */
  virtual void DestroyResource() = 0;

  /**
   * @copydoc Dali::NativeImage::TargetTexture()
   */
  virtual uint32_t TargetTexture() = 0;

  /**
   * @copydoc Dali::NativeImage::PrepareTexture()
   */
  virtual Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() = 0;

  /**
   * @copydoc Dali::NativeImage::GetWidth()
   */
  virtual uint32_t GetWidth() const = 0;

  /**
   * @copydoc Dali::NativeImage::GetHeight()
   */
  virtual uint32_t GetHeight() const = 0;

  /**
   * @copydoc Dali::NativeImage::RequiresBlending()
   */
  virtual bool RequiresBlending() const = 0;

  /**
   * @copydoc Dali::NativeImage::GetTextureTarget()
   */
  virtual int GetTextureTarget() const = 0;

  /**
   * @copydoc Dali::NativeImage::ApplyNativeFragmentShader()
   */
  virtual bool ApplyNativeFragmentShader(std::string& shader, int mask) = 0;

  /**
   * @copydoc Dali::NativeImage::GetCustomSamplerTypename()
   */
  virtual const char* GetCustomSamplerTypename() const = 0;

  /**
   * @copydoc Dali::NativeImage::GetNativeImageHandle()
   */
  virtual Any GetNativeImageHandle() const = 0;

  /**
   * @copydoc Dali::NativeImage::SourceChanged()
   */
  virtual bool SourceChanged() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  virtual Rect<uint32_t> GetUpdatedArea() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::PostRender()
   */
  virtual void PostRender() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  virtual NativeImageInterface::Extension* GetNativeImageInterfaceExtension() = 0;

  /**
   * @copydoc Dali::DevelNativeImage::AcquireBuffer()
   */
  virtual uint8_t* AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride) = 0;

  /**
   * @copydoc Dali::DevelNativeImage::ReleaseBuffer()
   */
  virtual bool ReleaseBuffer(const Rect<uint32_t>& updatedArea) = 0;

  /**
   * @copydoc Dali::DevelNativeImage::SetResourceDestructionCallback()
   */
  virtual void SetResourceDestructionCallback(EventThreadCallback* callback, bool ownedCallback) = 0;

  /**
   * @copydoc Dali::DevelNativeImage::EnableBackBuffer()
   */
  virtual void EnableBackBuffer(bool enable) = 0;

  /**
   * @copydoc Dali::NativeImage::EncodeToFile(const std::string& )
   */
  inline bool EncodeToFile(const std::string& filename) const
  {
    return EncodeToFile(filename, DEFAULT_QUALITY);
  }

  /**
   * @brief Converts the current pixel contents to either a JPEG or PNG format
   * and write that to the filesystem.
   *
   * @param[in] filename Identify the filesystem location at which to write the encoded image.
   *                     The extension determines the encoding used.
   *                     The two valid encoding are (".jpeg"|".jpg") and ".png".
   * @param[in] quality The quality of encoded jpeg image
   * @return    @c true if the pixels were written, and @c false otherwise
   */
  inline bool EncodeToFile(const std::string& filename, const uint32_t quality) const
  {
    Dali::Vector<uint8_t> pixbuf;
    uint32_t              width(0), height(0);
    Pixel::Format         pixelFormat;

    if(GetPixels(pixbuf, width, height, pixelFormat))
    {
      return Dali::EncodeToFile(&pixbuf[0], filename, pixelFormat, width, height, quality);
    }
    return false;
  }

public:
  inline static Internal::Adaptor::NativeImage& GetImplementation(Dali::NativeImage& image)
  {
    return *image.mImpl;
  }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_IMPL_H
