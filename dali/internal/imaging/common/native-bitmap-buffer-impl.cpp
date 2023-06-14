/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/imaging/common/native-bitmap-buffer-impl.h>

// EXTERNAL HEADERS
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/gl-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeBitmapBuffer::NativeBitmapBuffer(Adaptor* adaptor, unsigned int width, unsigned int height, Pixel::Format pFormat)
: mGlAbstraction(nullptr),
  mWidth(width),
  mHeight(height),
  mPixelFormat(pFormat),
  mLastReadBuffer(NULL)
{
  DALI_ASSERT_ALWAYS(adaptor);
  mBuffer = new Integration::LocklessBuffer(width * height * Pixel::GetBytesPerPixel(pFormat));

  GraphicsInterface* graphics    = &(adaptor->GetGraphicsInterface());
  auto               eglGraphics = static_cast<EglGraphics*>(graphics);
  mGlAbstraction                 = &(eglGraphics->GetGlAbstraction());
}

NativeBitmapBuffer::~NativeBitmapBuffer()
{
  delete mBuffer;
}

void NativeBitmapBuffer::PrepareTexture()
{
  DALI_ASSERT_ALWAYS(mBuffer);
  GLenum pixelFormat   = GL_RGBA;
  GLenum pixelDataType = GL_UNSIGNED_BYTE;

  Integration::ConvertToGlFormat(mPixelFormat, pixelDataType, pixelFormat);

  const unsigned char* buf = mBuffer->Read();

  if(buf && buf != mLastReadBuffer) // Prevent same buffer being uploaded multiple times
  {
    mLastReadBuffer = buf;

    // The active texture has already been set to a sampler and bound.
    mGlAbstraction->TexImage2D(GL_TEXTURE_2D, 0, pixelFormat, mWidth, mHeight, 0, pixelFormat, pixelDataType, buf);
  }
}

void NativeBitmapBuffer::Write(const unsigned char* src, size_t size)
{
  mBuffer->Write(src, size); // Write will cause LocklessBuffer to switch to the other buffer
}

bool NativeBitmapBuffer::CreateResource()
{
  return true;
}

void NativeBitmapBuffer::DestroyResource()
{
}

unsigned int NativeBitmapBuffer::TargetTexture()
{
  return 0;
}

unsigned int NativeBitmapBuffer::GetWidth() const
{
  return mWidth;
}

unsigned int NativeBitmapBuffer::GetHeight() const
{
  return mHeight;
}

bool NativeBitmapBuffer::RequiresBlending() const
{
  return Pixel::HasAlpha(mPixelFormat);
}

int NativeBitmapBuffer::GetTextureTarget() const
{
  return 0;
}

bool NativeBitmapBuffer::ApplyNativeFragmentShader(std::string& shader)
{
  return false;
}

const char* NativeBitmapBuffer::GetCustomSamplerTypename() const
{
  return nullptr;
}

Any NativeBitmapBuffer::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeBitmapBuffer::SourceChanged() const
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
