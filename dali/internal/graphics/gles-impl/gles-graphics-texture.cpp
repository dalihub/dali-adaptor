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
// const int32_t GL_MINIFY_DEFAULT  = GL_NEAREST_MIPMAP_LINEAR;
// const int32_t GL_MAGNIFY_DEFAULT = GL_LINEAR;
const int32_t GL_WRAP_DEFAULT = GL_CLAMP_TO_EDGE;

// These are the Dali defaults
const int32_t DALI_MINIFY_DEFAULT  = GL_LINEAR;
const int32_t DALI_MAGNIFY_DEFAULT = GL_LINEAR;

constexpr uint32_t CLEAR_CACHED_CONTEXT_THRESHOLD = 100u;
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

inline void WriteRGB32ToRGBA32(const void* __restrict__ pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride, void* __restrict__ pOutput)
{
  const uint8_t* __restrict__ inData = reinterpret_cast<const uint8_t*>(pData);
  uint8_t* __restrict__ outData      = reinterpret_cast<uint8_t*>(pOutput);
  if(rowStride == 0u)
  {
    rowStride = width;
  }
  for(auto y = 0u; y < height; ++y)
  {
    auto inIdx  = 0u;
    auto outIdx = 0u;
    for(auto x = 0u; x < width; ++x)
    {
      outData[outIdx]     = inData[inIdx];
      outData[outIdx + 1] = inData[inIdx + 1];
      outData[outIdx + 2] = inData[inIdx + 2];
      outData[outIdx + 3] = 0xff;
      outIdx += 4;
      inIdx += 3;
    }
    inData += rowStride * 3u;
    outData += width * 4u;
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
const std::vector<ColorConversion>& GetColorConversionTable()
{
  static const std::vector<ColorConversion> COLOR_CONVERSION_TABLE = {
    {Format::R8G8B8_UNORM, Format::R8G8B8A8_UNORM, ConvertRGB32ToRGBA32, WriteRGB32ToRGBA32}};
  return COLOR_CONVERSION_TABLE;
}

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
  auto*  context = mController.GetCurrentContext();
  auto*  gl      = mController.GetGL();
  GLuint texture{0};

  if(DALI_UNLIKELY(!gl || !context))
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
    context->BindTexture(mGlTarget, GetTextureTypeId(), texture);

    gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1); // We always use tightly packed data

    // Apply default sampling parameters
    SetSamplerParameter(GL_TEXTURE_MIN_FILTER, mDefaultSamplerState.minFilter, DALI_MINIFY_DEFAULT);
    SetSamplerParameter(GL_TEXTURE_MAG_FILTER, mDefaultSamplerState.magFilter, DALI_MAGNIFY_DEFAULT);
    SetSamplerParameter(GL_TEXTURE_WRAP_S, mDefaultSamplerState.wrapS, GL_WRAP_DEFAULT);
    SetSamplerParameter(GL_TEXTURE_WRAP_T, mDefaultSamplerState.wrapT, GL_WRAP_DEFAULT);

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
  auto* context = mController.GetCurrentContext();
  auto* gl      = mController.GetGL();
  if(DALI_UNLIKELY(!gl || !context))
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
      if(1) // format.format && format.type)
      {
        // Bind texture
        gl->GenTextures(1, &texture);
        context->BindTexture(GL_TEXTURE_2D, GetTextureTypeId(), texture);

        if(mCreateInfo.allocationPolicy == Graphics::TextureAllocationPolicy::CREATION || mCreateInfo.data)
        {
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
        }

        // Clear staging buffer if there was any
        mStagingBuffer.clear();
        mTextureId = texture;
        // Default texture filtering (to be set later via command buffer binding)
        SetSamplerParameter(GL_TEXTURE_MIN_FILTER, mDefaultSamplerState.minFilter, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        SetSamplerParameter(GL_TEXTURE_MAG_FILTER, mDefaultSamplerState.magFilter, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        SetSamplerParameter(GL_TEXTURE_WRAP_S, mDefaultSamplerState.wrapS, GL_WRAP_DEFAULT);
        SetSamplerParameter(GL_TEXTURE_WRAP_T, mDefaultSamplerState.wrapT, GL_WRAP_DEFAULT);
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
        context->BindTexture(GL_TEXTURE_CUBE_MAP, GetTextureTypeId(), texture);
        gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1); // We always use tightly packed data

        SetSamplerParameter(GL_TEXTURE_MIN_FILTER, mDefaultSamplerState.minFilter, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        SetSamplerParameter(GL_TEXTURE_MAG_FILTER, mDefaultSamplerState.magFilter, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        SetSamplerParameter(GL_TEXTURE_WRAP_S, mDefaultSamplerState.wrapS, GL_WRAP_DEFAULT);
        SetSamplerParameter(GL_TEXTURE_WRAP_T, mDefaultSamplerState.wrapT, GL_WRAP_DEFAULT);

        if(mCreateInfo.allocationPolicy == Graphics::TextureAllocationPolicy::CREATION || mCreateInfo.data)
        {
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
        }

        // Clear staging buffer if there was any
        mStagingBuffer.clear();

        mTextureId = texture;

        SetSamplerParameter(GL_TEXTURE_WRAP_R, mDefaultSamplerState.wrapR, GL_WRAP_DEFAULT);
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
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    auto* gl = mController.GetGL();
    if(DALI_UNLIKELY(!gl))
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
      // Invalidate given texture
      ClearCachedContext();

      // TODO : Shouldn't we call DestroyResource even if shutting down?
      // For now, we use EglExtensions API at DestroyResource. So just block for now.
      mCreateInfo.nativeImagePtr->DestroyResource();
    }
  }
}

void Texture::DiscardResource()
{
  mController.DiscardResource(this);
}

void Texture::Bind(const TextureBinding& binding) const
{
  auto* context = mController.GetCurrentContext();
  auto* gl      = mController.GetGL();
  if(DALI_UNLIKELY(!gl || !context))
  {
    // Do nothing during shutdown
    return;
  }

  context->ActiveTexture(binding.binding);
  context->BindTexture(mGlTarget, GetTextureTypeId(), mTextureId);

  // For GLES2 if there is a sampler set in the binding
  if(binding.sampler)
  {
    const auto& samplerCreateInfo = static_cast<const GLES::Sampler*>(binding.sampler)->GetCreateInfo();

    auto mipMapMode = samplerCreateInfo.mipMapMode;

    // @todo : Should we always ignore mipmap mode when it is compressed, and never bind higher level mipmap?
    if(mMaxMipMapLevel == 0u && mIsCompressed)
    {
      mipMapMode = Graphics::SamplerMipmapMode::NONE;
    }

    SetSamplerParameter(GL_TEXTURE_MIN_FILTER, mDefaultSamplerState.minFilter, GLSamplerFilterAndMipMapMode(samplerCreateInfo.minFilter, mipMapMode).glFilter);
    SetSamplerParameter(GL_TEXTURE_MAG_FILTER, mDefaultSamplerState.magFilter, GLSamplerFilter(samplerCreateInfo.magFilter).glFilter);
    SetSamplerParameter(GL_TEXTURE_WRAP_S, mDefaultSamplerState.wrapS, GLAddressMode(samplerCreateInfo.addressModeU).texParameter);
    SetSamplerParameter(GL_TEXTURE_WRAP_T, mDefaultSamplerState.wrapT, GLAddressMode(samplerCreateInfo.addressModeV).texParameter);

    if(mGlTarget == GL_TEXTURE_CUBE_MAP)
    {
      SetSamplerParameter(GL_TEXTURE_WRAP_R, mDefaultSamplerState.wrapR, GLAddressMode(samplerCreateInfo.addressModeW).texParameter);
    }
  }
  else
  {
    SetSamplerParameter(GL_TEXTURE_MIN_FILTER, mDefaultSamplerState.minFilter, DALI_MINIFY_DEFAULT);
    SetSamplerParameter(GL_TEXTURE_MAG_FILTER, mDefaultSamplerState.magFilter, DALI_MAGNIFY_DEFAULT);
    SetSamplerParameter(GL_TEXTURE_WRAP_S, mDefaultSamplerState.wrapS, GL_WRAP_DEFAULT);
    SetSamplerParameter(GL_TEXTURE_WRAP_T, mDefaultSamplerState.wrapT, GL_WRAP_DEFAULT);
    if(mGlTarget == GL_TEXTURE_CUBE_MAP)
    {
      SetSamplerParameter(GL_TEXTURE_WRAP_R, mDefaultSamplerState.wrapR, GL_WRAP_DEFAULT);
    }
  }

  if(mMaxMipMapLevel)
  {
    SetSamplerParameter(GL_TEXTURE_MAX_LEVEL, mDefaultSamplerState.maxLevel, mMaxMipMapLevel);
  }
}

bool Texture::PrepareNativeTexture(GLES::Context* prepareContext)
{
  NativeImageInterfacePtr nativeImage = mCreateInfo.nativeImagePtr;
  if(nativeImage)
  {
    if(!mIsPrepared)
    {
      mIsPrepared        = true;
      mLastPrepareResult = nativeImage->PrepareTexture();

      if(DALI_UNLIKELY(mLastPrepareResult >= Dali::NativeImageInterface::PrepareTextureResult::ERROR_MIN &&
                       mLastPrepareResult >= Dali::NativeImageInterface::PrepareTextureResult::ERROR_MAX))
      {
        DALI_LOG_ERROR("[ERROR] NativeImage::PrepareTexture failed with error code [%x]\n", mLastPrepareResult);
      }

      // Clear target called context if image changed.
      if(mLastPrepareResult == Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED)
      {
        // Remove cached info to contexts and invalidate this texture.
        ClearCachedContext();
      }
    }

    // NOTE : We should call TargetTextureKHR per each context.
    if(mTargetCalledContext.find(prepareContext) == mTargetCalledContext.end())
    {
      mTargetCalledContext.insert(prepareContext);
      nativeImage->TargetTexture();
    }
  }
  else
  {
    DALI_LOG_ERROR("Do not call PrepareNativeTexture for standard textures\n");
    mLastPrepareResult = Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;
  }

  return mLastPrepareResult >= Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR_MIN &&
         mLastPrepareResult <= Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR_MAX;
}

void Texture::ResetPrepare()
{
  mIsPrepared        = false;
  mLastPrepareResult = Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;

  // Remove context list if it stored too many items.
  if(DALI_UNLIKELY(mTargetCalledContext.size() > CLEAR_CACHED_CONTEXT_THRESHOLD))
  {
    ClearCachedContext();
  }
}

void Texture::InvalidateCachedContext(GLES::Context* invalidatedContext)
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    mTargetCalledContext.erase(invalidatedContext);
  }
}

/**
 * This function tests whether format is supported by the driver. If possible it applies
 * format conversion to suitable supported pixel format.
 */
bool Texture::TryConvertPixelData(const void* pData, Graphics::Format srcFormat, Graphics::Format destFormat, uint32_t sizeInBytes, uint32_t inStride, uint32_t width, uint32_t height, std::vector<uint8_t>& outputBuffer)
{
  // No need to convert
  if(srcFormat == destFormat)
  {
    return false;
  }

  auto it = std::find_if(GetColorConversionTable().begin(), GetColorConversionTable().end(), [&](auto& item)
                         { return item.srcFormat == srcFormat && item.destFormat == destFormat; });

  // No suitable format, return empty array
  if(it == GetColorConversionTable().end())
  {
    return false;
  }
  auto begin = reinterpret_cast<const uint8_t*>(pData);

  outputBuffer = std::move(it->pConversionFunc(begin, sizeInBytes, width, height, inStride));
  return !outputBuffer.empty();
}

void Texture::SetSamplerParameter(uint32_t param, uint32_t& cacheValue, uint32_t value) const
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl) && cacheValue != value)
  {
    gl->TexParameteri(mGlTarget, param, value);
    cacheValue = value;
  }
}

void Texture::ClearCachedContext()
{
  for(auto* context : mTargetCalledContext)
  {
    context->InvalidateCachedNativeTexture(this);
  }
  mTargetCalledContext.clear();
}

} // namespace Dali::Graphics::GLES
