#pragma once

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <extern-definitions.h>
#include <CoreGraphics/CoreGraphics.h>

namespace Dali::Internal::Adaptor
{

class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceCocoa : public Internal::Adaptor::NativeImageSource
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
  static NativeImageSourceCocoa* New(
    unsigned int width,
    unsigned int height,
    Dali::NativeImageSource::ColorDepth depth,
    Any nativeImageSource
  );

  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  Any GetNativeImageSource() const override;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  bool GetPixels(
    std::vector<unsigned char> &pixbuf,
    unsigned int &width,
    unsigned int &height,
    Pixel::Format& pixelFormat
  ) const override;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  void SetSource( Any source ) override;

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth ) override;

  /**
   * destructor
   */
  ~NativeImageSourceCocoa() override;

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
  unsigned int TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  void PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  unsigned int GetWidth() const override;

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  unsigned int GetHeight() const override;

  /**
   * @copydoc Dali::NativeImageSource::RequiresBlending()
   */
  bool RequiresBlending() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomFragmentPrefix()
   */
  const char* GetCustomFragmentPrefix() const override;

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
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageSource::AcquireBuffer()
   */
  uint8_t* AcquireBuffer( uint16_t& width, uint16_t& height, uint16_t& stride ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageSource::ReleaseBuffer()
   */
  bool ReleaseBuffer() override;

private:

  /**
   * Private constructor; @see NativeImageSource::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type Win32 Pixmap , a WinPixmap or is empty
   */
  NativeImageSourceCocoa(
    unsigned int width,
    unsigned  int height,
    Dali::NativeImageSource::ColorDepth depth,
    Any nativeImageSource
  );

private:
  CFRef<CGImageRef> mImage;
};

} // namespace Dali::Internal::Adaptor
