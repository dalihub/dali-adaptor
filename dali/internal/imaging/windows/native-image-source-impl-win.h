#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>

#include <dali/internal/imaging/common/native-image-source-impl.h>
#include <cstdint>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EglGraphics;
class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceWin : public Internal::Adaptor::NativeImageSource
{
public:
  /**
   * Create a new NativeImageSource internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type Win32 Pixmap , a WinPixmap or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceWin* New(uint32_t                            width,
                                   uint32_t                            height,
                                   Dali::NativeImageSource::ColorDepth depth,
                                   Any                                 nativeImageSource);
  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  Any GetNativeImageSource() const override;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  bool GetPixels(std::vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const override;

  /**
   * @copydoc Dali::NativeImageSource::SetPixels()
   */
  bool SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat) override;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  void SetSource(Any source) override;

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth) override;

  /**
   * destructor
   */
  ~NativeImageSourceWin() override;

  /**
   * @copydoc Dali::NativeImageSource::CreateResource()
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImageSource::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImageSource::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  uint32_t GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  uint32_t GetHeight() const override
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImageSource::RequiresBlending()
   */
  bool RequiresBlending() const override
  {
    return mBlendingRequired;
  }

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader, int count) override;

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
    return Rect<uint32_t>{0, 0, mWidth, mHeight};
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageSource::AcquireBuffer()
   */
  uint8_t* AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride) override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageSource::ReleaseBuffer()
   */
  bool ReleaseBuffer(const Rect<uint32_t>& updatedArea) override;

  /**
   * @copydoc Dali::NativeImageSource::SetResourceDestructionCallback()
   */
  void SetResourceDestructionCallback(EventThreadCallback* callback) override;

  /**
   * @copydoc Dali::DevelNativeImageSource::EnableBackBuffer()
   */
  void EnableBackBuffer(bool enable) override;

private:
  /**
   * Private constructor; @see NativeImageSource::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type Win32 Pixmap , a WinPixmap or is empty
   */
  NativeImageSourceWin(uint32_t                            width,
                       uint32_t                            height,
                       Dali::NativeImageSource::ColorDepth depth,
                       Any                                 nativeImageSource);

  /**
   * 2nd phase construction.
   */
  void Initialize();

  /**
   * Uses X11 to get the default depth.
   * @param depth the PixelImage depth enum
   * @return default win32 pixel depth
   */
  int GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const;

  /**
   * Gets the pixmap from the Any parameter
   * @param pixmap contains either: pixmap of type Win32 Pixmap , a WinPixmap or is empty
   * @return pixmap x11 pixmap
   */
  unsigned int GetPixmapFromAny(Any pixmap) const;

  /**
   * Given an existing pixmap, the function uses X to find out
   * the width, heigth and depth of that pixmap.
   */
  void GetPixmapDetails();

private:
  uint32_t                             mWidth;                       ///< image width
  uint32_t                             mHeight;                      ///< image heights
  bool                                 mOwnPixmap;                   ///< Whether we created pixmap or not
  unsigned int                         mPixmap;                      ///< From Windows
  bool                                 mBlendingRequired;            ///< Whether blending is required
  Dali::NativeImageSource::ColorDepth  mColorDepth;                  ///< color depth of image
  bool                                 mEglImageChanged;             ///< Whether EGLImage changed or not. Reset flag as false at PrepareTexture().
  void*                                mEglImageKHR;                 ///< From EGL extension
  EglGraphics*                         mEglGraphics;                 ///< EGL Graphics
  EglImageExtensions*                  mEglImageExtensions;          ///< The EGL Image Extensions
  std::unique_ptr<EventThreadCallback> mResourceDestructionCallback; ///< The Resource Destruction Callback
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H
