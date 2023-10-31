#ifndef DALI_NATIVE_BITMAP_BUFFER_H
#define DALI_NATIVE_BITMAP_BUFFER_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

// EXTERNAL HEADERS
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/lockless-buffer.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/images/pixel.h>

// INTERNAL HEADERS
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/graphics-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeBitmapBuffer;
typedef IntrusivePtr<NativeBitmapBuffer> NativeBitmapBufferPtr;

/**
 * A Bitmap-based implementation of the NativeImage interface.
 */
class NativeBitmapBuffer : public NativeImageInterface
{
public:
  /**
   * Constructor.
   * @param adaptor Adaptor used
   * @param width width of image
   * @param height height of image
   * @param pixelFormat pixel format for image
   */
  NativeBitmapBuffer(Adaptor* adaptor, uint32_t width, uint32_t height, Pixel::Format pixelFormat);

  /**
   * virtual destructor
   */
  virtual ~NativeBitmapBuffer();

  /**
   * Write to buffer. Does not block.
   * @param[in] src  data source
   * @param[in] size size of data in bytes
   * @return true if successful, false if currently reading from buffer in render thread
   */
  void Write(const uint8_t* src, size_t size);

public:
  /**
   * @copydoc Dali::NativeImageInterface::CreateResource()
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImageInterface::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  void PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  uint32_t GetWidth() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  uint32_t GetHeight() const override;

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  bool RequiresBlending() const override;

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

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

private:
  NativeBitmapBuffer(const NativeBitmapBuffer&);            ///< not defined
  NativeBitmapBuffer& operator=(const NativeBitmapBuffer&); ///< not defined
  NativeBitmapBuffer();                                     ///< not defined

private:
  Integration::GlAbstraction* mGlAbstraction; ///< GlAbstraction used

  Integration::LocklessBuffer* mBuffer;         ///< bitmap data double buffered
  uint32_t                     mWidth;          ///< Image width
  uint32_t                     mHeight;         ///< Image height
  Pixel::Format                mPixelFormat;    ///< Image pixelformat
  const uint8_t*               mLastReadBuffer; ///< last buffer that was read
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_NATIVE_BITMAP_BUFFER_H
