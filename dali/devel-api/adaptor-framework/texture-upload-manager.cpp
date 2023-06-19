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
 */

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/texture-upload-manager.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/texture-upload-manager-impl.h>

namespace Dali::Devel
{
// Called by main thread

TextureUploadManager::TextureUploadManager() = default;

TextureUploadManager::~TextureUploadManager() = default;

TextureUploadManager TextureUploadManager::Get()
{
  return Internal::Adaptor::TextureUploadManager::Get();
}

Dali::Texture TextureUploadManager::GenerateTexture2D()
{
  return GetImplementation(*this).GenerateTexture2D();
}

TextureUploadManager::TextureUploadManager(Internal::Adaptor::TextureUploadManager* impl)
: BaseHandle(impl)
{
}

// Called by update thread

bool TextureUploadManager::ResourceUpload()
{
  return GetImplementation(*this).ResourceUpload();
}

// Called by worker thread

void TextureUploadManager::RequestUpload(ResourceId resourceId, PixelData pixelData)
{
  GetImplementation(*this).RequestUpload(resourceId, pixelData);
}

} // namespace Dali::Devel
