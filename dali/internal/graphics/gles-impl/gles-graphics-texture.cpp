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
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <vector>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"
#include "gles-graphics-types.h"

namespace Dali::Graphics::GLES
{
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
  auto gl = mController.GetGL();

  GLuint texture{0};

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
        gl->TexImage2D(GL_TEXTURE_2D,
                       0,
                       format.format,
                       mCreateInfo.size.width,
                       mCreateInfo.size.height,
                       0,
                       format.format,
                       format.type,
                       (mCreateInfo.data ? mStagingBuffer.data() : nullptr));

        // Clear staging buffer if there was any
        mStagingBuffer.clear();

        mTextureId = texture;

        // Default texture filtering (to be set later via command buffer binding)
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Graphics::GLES::GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter::LINEAR, SamplerMipmapMode::NONE));
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
  // This is a proper destructor
  if(mTextureId)
  {
    auto gl = mController.GetGL();
    gl->DeleteTextures(1, &mTextureId);
  }
}

void Texture::DiscardResource()
{
  mController.DiscardResource(this);
}

void Texture::Bind(const TextureBinding& binding) const
{
  auto gl = mController.GetGL();

  // Bind texture to shader slot
  gl->BindTexture(GL_TEXTURE_2D, mTextureId);
  gl->ActiveTexture(GL_TEXTURE0 + binding.binding);

  // For GLES2 if there is a sampler set in the binding
  //if(binding.sampler)
  //{

  //}
}

} // namespace Dali::Graphics::GLES
