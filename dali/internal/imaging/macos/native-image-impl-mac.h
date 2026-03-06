#pragma once

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
#include <dali/public-api/adaptor-framework/native-image.h>

#include <CoreGraphics/CoreGraphics.h>
#include <dali/internal/imaging/common/native-image-impl.h>
#include <extern-definitions.h>

namespace Dali::Internal::Adaptor
{
/**
 * Dali internal NativeImage.
 */
class NativeImageCocoa : public Internal::Adaptor::NativeImage
{
public:
  /**
   * Create a new NativeImage internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImage contains either: pixmap of type Win32 Pixmap , a WinPixmap or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageCocoa* New(
    uint32_t                      width,
    uint32_t                      height,
    Dali::NativeImage::ColorDepth depth,
    Any                           nativeImage);

  /**
   * @copydoc Dali::NativeImage::GetNativeImage()
   */
  Any GetNativeImage() const override;

  /**
   * @copydoc Dali::NativeImage::GetPixels()
   */
  bool GetPixels(
    Dali::Vector<uint8_t>& pixbuf,
    uint32_t&              width,
    uint32_t&              height,
    Pixel::Format&         pixelFormat) const override;

  /**
   * @copydoc Dali::NativeImage::SetPixels()
   */
  bool SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat) override;

  /**
   * @copydoc Dali::NativeImage::SetSource( Any source )
   */
  void SetSource(Any source) override;

  /**
   * @copydoc Dali::NativeImage::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported(Dali::NativeImage::ColorDepth colorDepth) override;

  /**
   * destructor
   */
  ~NativeImageCocoa() override;

  /**
   * @copydoc Dali::NativeImage::CreateResource()
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImage::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImage::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImage::PrepareTexture()
   */
  Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImage::GetWidth()
   */
  uint32_t GetWidth() const override;

  /**
   * @copydoc Dali::NativeImage::GetHeight()
   */
  uint32_t GetHeight() const override;

  /**
   * @copydoc Dali::NativeImage::RequiresBlending()
   */
  bool RequiresBlending() const override;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader, int mask) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetNativeImageHandle()
   */
  Any GetNativeImageHandle() const override;

  /**
   * @copydoc Dali::NativeImageInterface::SourceChanged()
   */
  bool SourceChanged() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  Rect<uint32_t> GetUpdatedArea() override
  {
    return Rect<uint32_t>{0, 0, static_cast<uint32_t>(CGImageGetWidth(mImage.get())), static_cast<uint32_t>(CGImageGetHeight(mImage.get()))};
  }

  /**
   * @copydoc Dali::NativeImageInterface::PostRender()
   */
  void PostRender() override
  {
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImage::AcquireBuffer()
   */
  uint8_t* AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride) override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImage::ReleaseBuffer()
   */
  bool ReleaseBuffer(const Rect<uint32_t>& updatedArea) override;

  /**
   * @copydoc Dali::NativeImage::SetResourceDestructionCallback()
   */
  void SetResourceDestructionCallback(EventThreadCallback* callback, bool ownedCallback) override;

  /**
   * @copydoc Dali::DevelNativeImage::EnableBackBuffer()
   */
  void EnableBackBuffer(bool enable) override;

private:
  /**
   * Private constructor; @see NativeImage::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImage contains either: pixmap of type Win32 Pixmap , a WinPixmap or is empty
   */
  NativeImageCocoa(
    uint32_t                      width,
    uint32_t                      height,
    Dali::NativeImage::ColorDepth depth,
    Any                           nativeImage);

private:
  CFRef<CGImageRef>    mImage;
  EventThreadCallback* mResourceDestructionCallback; ///< The Resource Destruction Callback
  bool                 mOwnResourceDestructionCallback;
};

} // namespace Dali::Internal::Adaptor
