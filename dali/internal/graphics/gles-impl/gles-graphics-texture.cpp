/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include "gles-graphics-texture.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <vector>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"
#include "gles-graphics-sampler.h"
#include "gles-graphics-types.h"

namespace
{
// These match the GL specification
//const int32_t GL_MINIFY_DEFAULT  = GL_NEAREST_MIPMAP_LINEAR;
//const int32_t GL_MAGNIFY_DEFAULT = GL_LINEAR;
const int32_t GL_WRAP_DEFAULT = GL_CLAMP_TO_EDGE;

// These are the Dali defaults
const int32_t DALI_MINIFY_DEFAULT  = GL_LINEAR;
const int32_t DALI_MAGNIFY_DEFAULT = GL_LINEAR;
} // namespace

namespace Dali::Graphics::GLES
{
struct ColorConversion
{
  Format srcFormat;
  Format destFormat;
  std::vector<uint8_t> (*pConversionFunc)(const void*, uint32_t, uint32_t, uint32_t, uint32_t);
  void (*pConversionWriteFunc)(const void*, uint32_t, uint32_t, uint32_t, uint32_t, void*);
};

inline void WriteRGB32ToRGBA32(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride, void* pOutput)
{
  auto inData  = reinterpret_cast<const uint8_t*>(pData);
  auto outData = reinterpret_cast<uint8_t*>(pOutput);
  auto outIdx  = 0u;
  for(auto i = 0u; i < sizeInBytes; i += 3)
  {
    outData[outIdx]     = inData[i];
    outData[outIdx + 1] = inData[i + 1];
    outData[outIdx + 2] = inData[i + 2];
    outData[outIdx + 3] = 0xff;
    outIdx += 4;
  }
}

/**
 * Converts RGB to RGBA
 */
inline std::vector<uint8_t> ConvertRGB32ToRGBA32(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride)
{
  std::vector<uint8_t> rgbaBuffer{};
  rgbaBuffer.resize(width * height * 4);
  WriteRGB32ToRGBA32(pData, sizeInBytes, width, height, rowStride, &rgbaBuffer[0]);
  return rgbaBuffer;
}

/**
 * Format conversion table
 */
static const std::vector<ColorConversion> COLOR_CONVERSION_TABLE = {
  {Format::R8G8B8_UNORM, Format::R8G8B8A8_UNORM, ConvertRGB32ToRGBA32, WriteRGB32ToRGBA32}};

/**
 * Constructor
 */
Texture::Texture(const Graphics::TextureCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: TextureResource(createInfo, controller)
{
  // If there is any data, move it into staging buffer
  if(mCreateInfo.data && mCreateInfo.dataSize)
  {
    mStagingBuffer.resize(size_t(mCreateInfo.dataSize));
    std::copy(reinterpret_cast<char*>(mCreateInfo.data),
              reinterpret_cast<char*>(mCreateInfo.data) + mCreateInfo.dataSize,
              mStagingBuffer.begin());
  }

  // Add texture to the Resource queue
  mController.AddTexture(*this);
}

bool Texture::InitializeResource()
{
  if(mCreateInfo.nativeImagePtr)
  {
    return InitializeNativeImage();
  }
  return InitializeTexture();
}

bool Texture::InitializeNativeImage()
{
  auto   gl = mController.GetGL();
  GLuint texture{0};

  if(!gl)
  {
    // Do nothing during shutdown
    return false;
  }

  NativeImageInterfacePtr nativeImage = mCreateInfo.nativeImagePtr;
  bool                    created     = nativeImage->CreateResource();
  mGlTarget                           = nativeImage->GetTextureTarget();
  if(created)
  {
    gl->GenTextures(1, &texture);
    gl->BindTexture(mGlTarget, texture);

    gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1); // We always use tightly packed data

    // Apply default sampling parameters
    gl->TexParameteri(mGlTarget, GL_TEXTURE_MIN_FILTER, DALI_MINIFY_DEFAULT);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_MAG_FILTER, DALI_MAGNIFY_DEFAULT);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_S, GL_WRAP_DEFAULT);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_T, GL_WRAP_DEFAULT);

    // platform specific implementation decides on what GL extension to use
    if(nativeImage->TargetTexture() != 0u)
    {
      gl->DeleteTextures(1, &texture);
      nativeImage->DestroyResource();
      texture = 0u;
      created = false;
    }
    else
    {
      mTextureId = texture;
    }
  }
  else
  {
    DALI_LOG_ERROR("Native Image: InitializeNativeImage, CreateResource() failed\n");
  }

  return created; // WARNING! May be false! Needs handling! (Well, initialized on bind)
}

bool Texture::InitializeTexture()
{
  auto gl = mController.GetGL();
  if(!gl)
  {
    // Do nothing during shutdown
    return false;
  }

  GLuint texture{0};

  mGlTarget     = GLTextureTarget(mCreateInfo.textureType).target;
  mIsCompressed = Graphics::GLES::FormatCompression(mCreateInfo.format).compressed;

  switch(mCreateInfo.textureType)
  {
    // Texture 2D
    case Graphics::TextureType::TEXTURE_2D:
    {
      Graphics::GLES::GLTextureFormatType format(mCreateInfo.format);

      // TODO: find better condition, with this test the L8 doesn't work
      if(1) //format.format && format.type)
      {
        // Bind texture
        gl->GenTextures(1, &texture);
        gl->BindTexture(GL_TEXTURE_2D, texture);

        // Allocate memory for the texture
        if(!mIsCompressed)
        {
          gl->TexImage2D(GL_TEXTURE_2D,
                         0,
                         format.internalFormat,
                         mCreateInfo.size.width,
                         mCreateInfo.size.height,
                         0,
                         format.format,
                         format.type,
                         (mCreateInfo.data ? mStagingBuffer.data() : nullptr));
        }
        else
        {
          gl->CompressedTexImage2D(GL_TEXTURE_2D,
                                   0,
                                   format.internalFormat,
                                   mCreateInfo.size.width,
                                   mCreateInfo.size.height,
                                   0,
                                   mCreateInfo.dataSize,
                                   (mCreateInfo.data ? mStagingBuffer.data() : nullptr));
        }

        // Clear staging buffer if there was any
        mStagingBuffer.clear();
        mTextureId = texture;

        // Default texture filtering (to be set later via command buffer binding)
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_WRAP_DEFAULT);
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_WRAP_DEFAULT);
      }
      break;
    }
    // Texture Cubemap
    case Graphics::TextureType::TEXTURE_CUBEMAP:
    {
      Graphics::GLES::GLTextureFormatType format(mCreateInfo.format);

      if(format.format && format.type)
      {
        // Bind texture
        gl->GenTextures(1, &texture);
        gl->BindTexture(GL_TEXTURE_CUBE_MAP, texture);
        gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1); // We always use tightly packed data

        gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_WRAP_DEFAULT);
        gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_WRAP_DEFAULT);

        // Allocate memory for the texture
        for(uint32_t i = 0; i < 6; ++i)
        {
          if(!mIsCompressed)
          {
            gl->TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           0,
                           format.internalFormat,
                           mCreateInfo.size.width,
                           mCreateInfo.size.height,
                           0,
                           format.format,
                           format.type,
                           (mCreateInfo.data ? mStagingBuffer.data() : nullptr));
          }
          else
          {
            gl->CompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                     0,
                                     format.internalFormat,
                                     mCreateInfo.size.width,
                                     mCreateInfo.size.height,
                                     0,
                                     mCreateInfo.dataSize,
                                     (mCreateInfo.data ? mStagingBuffer.data() : nullptr));
          }
        }

        // Clear staging buffer if there was any
        mStagingBuffer.clear();

        mTextureId = texture;

        gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_WRAP_DEFAULT);
      }
      break;
    }
    default:
    {
      // nothing?
    }
  }
  return true;
}

void Texture::DestroyResource()
{
  auto gl = mController.GetGL();
  if(!gl)
  {
    return;
  }

  // This is a proper destructor
  if(mTextureId)
  {
    gl->DeleteTextures(1, &mTextureId);
  }
  if(mCreateInfo.nativeImagePtr)
  {
    mCreateInfo.nativeImagePtr->DestroyResource();
  }
}

void Texture::DiscardResource()
{
  mController.DiscardResource(this);
}

void Texture::Bind(const TextureBinding& binding) const
{
  auto gl = mController.GetGL();
  if(!gl)
  {
    // Do nothing during shutdown
    return;
  }

  gl->ActiveTexture(GL_TEXTURE0 + binding.binding);
  gl->BindTexture(mGlTarget, mTextureId);

  // For GLES2 if there is a sampler set in the binding
  if(binding.sampler)
  {
    // Non-default.
    auto*       sampler           = static_cast<const GLES::Sampler*>(binding.sampler);
    const auto& samplerCreateInfo = sampler->GetCreateInfo();

    auto mipMapMode = samplerCreateInfo.mipMapMode;

    gl->TexParameteri(mGlTarget, GL_TEXTURE_MIN_FILTER, GLSamplerFilterAndMipMapMode(samplerCreateInfo.minFilter, mipMapMode).glFilter);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_MAG_FILTER, GLSamplerFilter(samplerCreateInfo.magFilter).glFilter);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_S, GLAddressMode(samplerCreateInfo.addressModeU).texParameter);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_T, GLAddressMode(samplerCreateInfo.addressModeV).texParameter);
    if(mGlTarget == GL_TEXTURE_CUBE_MAP)
    {
      gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_R, GLAddressMode(samplerCreateInfo.addressModeW).texParameter);
    }
  }
  else
  {
    gl->TexParameteri(mGlTarget, GL_TEXTURE_MIN_FILTER, DALI_MINIFY_DEFAULT);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_MAG_FILTER, DALI_MAGNIFY_DEFAULT);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_S, GL_WRAP_DEFAULT);
    gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_T, GL_WRAP_DEFAULT);
    if(mGlTarget == GL_TEXTURE_CUBE_MAP)
    {
      gl->TexParameteri(mGlTarget, GL_TEXTURE_WRAP_R, GL_WRAP_DEFAULT);
    }
  }

  if(mMaxMipMapLevel)
  {
    gl->TexParameteri(mGlTarget, GL_TEXTURE_MAX_LEVEL, mMaxMipMapLevel);
  }
}

void Texture::Prepare()
{
  NativeImageInterfacePtr nativeImage = mCreateInfo.nativeImagePtr;
  if(nativeImage)
  {
    if(nativeImage->SourceChanged())
    {
      // Update size
      uint32_t width  = mCreateInfo.nativeImagePtr->GetWidth();
      uint32_t height = mCreateInfo.nativeImagePtr->GetHeight();
      mCreateInfo.SetSize({width, height}); // Size may change
    }

    nativeImage->PrepareTexture();
  }
}

/**
 * This function tests whether format is supported by the driver. If possible it applies
 * format conversion to suitable supported pixel format.
 */
bool Texture::TryConvertPixelData(const void* pData, Graphics::Format srcFormat, Graphics::Format destFormat, uint32_t sizeInBytes, uint32_t width, uint32_t height, std::vector<uint8_t>& outputBuffer)
{
  // No need to convert
  if(srcFormat == destFormat)
  {
    return false;
  }

  auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item) {
    return item.srcFormat == srcFormat && item.destFormat == destFormat;
  });

  // No suitable format, return empty array
  if(it == COLOR_CONVERSION_TABLE.end())
  {
    return false;
  }
  auto begin = reinterpret_cast<const uint8_t*>(pData);

  outputBuffer = std::move(it->pConversionFunc(begin, sizeInBytes, width, height, 0u));
  return !outputBuffer.empty();
}

} // namespace Dali::Graphics::GLES
