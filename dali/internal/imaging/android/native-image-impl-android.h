#ifndef DALI_INTERNAL_NATIVE_IMAGE_H
#define DALI_INTERNAL_NATIVE_IMAGE_H

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

// EXTERNAL INCLUDES
#include <hardware_buffer.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>

#include <dali/public-api/adaptor-framework/native-image.h>

#include <dali/internal/imaging/common/native-image-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EglGraphics;
class EglImageExtensions;

/**
 * Dali internal NativeImage.
 */
class NativeImageAndroid : public Internal::Adaptor::NativeImage
{
public:
  /**
   * Create a new NativeImage internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImage contains either: Android hardware buffer or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageAndroid* New(uint32_t                      width,
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
  bool GetPixels(Dali::Vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const override;

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
  ~NativeImageAndroid() override;

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
  uint32_t GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImage::GetHeight()
   */
  uint32_t GetHeight() const override
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImage::RequiresBlending()
   */
  bool RequiresBlending() const override
  {
    return mBlendingRequired;
  }

  /**
   * @copydoc Dali::NativeImage::GetTargetTexture()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImage::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader, int mask) override;

  /**
   * @copydoc Dali::NativeImage::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

  /**
   * @copydoc Dali::NativeImage::GetNativeImageHandle()
   */
  Any GetNativeImageHandle() const override
  {
    return GetNativeImage();
  }

  /**
   * @copydoc Dali::NativeImage::SourceChanged()
   */
  bool SourceChanged() const override
  {
    return true;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  Rect<uint32_t> GetUpdatedArea() override
  {
    return Rect<uint32_t>{0, 0, mWidth, mHeight};
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
   * @param[in] nativeImage contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   */
  NativeImageAndroid(uint32_t                      width,
                     uint32_t                      height,
                     Dali::NativeImage::ColorDepth depth,
                     Any                           nativeImage);

  /**
   * 2nd phase construction.
   */
  void Initialize();

  /**
   * Gets the pixmap from the Any parameter
   * @param pixmap contains either: pixmap of type Android hardware buffer or empty
   * @return pixmap pixmap
   */
  void* GetPixmapFromAny(Any pixmap) const;

  /**
   * Given an existing pixmap, the function uses Android hardware buffer API to find
   * the width, heigth and depth of that pixmap.
   */
  void GetPixmapDetails();

private:
  uint32_t                      mWidth;                       ///< image width
  uint32_t                      mHeight;                      ///< image heights
  bool                          mOwnPixmap;                   ///< Whether we created pixmap or not
  AHardwareBuffer*              mPixmap;                      ///<
  bool                          mBlendingRequired;            ///< Whether blending is required
  Dali::NativeImage::ColorDepth mColorDepth;                  ///< color depth of image
  bool                          mEglImageChanged;             ///< Whether EGLImage changed or not. Reset flag as false at PrepareTexture().
  void*                         mEglImageKHR;                 ///< From EGL extension
  EglGraphics*                  mEglGraphics;                 ///< EGL Graphics
  EglImageExtensions*           mEglImageExtensions;          ///< The EGL Image Extensions
  EventThreadCallback*          mResourceDestructionCallback; ///< The Resource Destruction Callback
  bool                          mOwnResourceDestructionCallback;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_H
