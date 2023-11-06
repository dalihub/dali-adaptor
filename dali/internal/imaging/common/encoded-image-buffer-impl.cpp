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

// CLASS HEADER
#include <dali/internal/imaging/common/encoded-image-buffer-impl.h>

// EXTERNAL INCLUDE
#include <dali/devel-api/common/hash.h>

namespace Dali
{
namespace Internal
{
EncodedImageBuffer::EncodedImageBuffer(const RawBufferType& buffer, ImageType type)
: mBuffer(buffer),
  mType(type)
{
  mBufferHash = CalculateHash(mBuffer);
}

EncodedImageBuffer::EncodedImageBuffer(RawBufferType&& buffer, ImageType type)
: mBuffer(std::move(buffer)),
  mType(type)
{
  mBufferHash = CalculateHash(mBuffer);
}

EncodedImageBuffer::~EncodedImageBuffer()
{
}

IntrusivePtr<EncodedImageBuffer> EncodedImageBuffer::New(const RawBufferType& buffer, ImageType type)
{
  IntrusivePtr<EncodedImageBuffer> internal = new EncodedImageBuffer(buffer, type);

  return internal;
}

IntrusivePtr<EncodedImageBuffer> EncodedImageBuffer::New(RawBufferType&& buffer, ImageType type)
{
  IntrusivePtr<EncodedImageBuffer> internal = new EncodedImageBuffer(std::move(buffer), type);

  return internal;
}

const EncodedImageBuffer::RawBufferType& EncodedImageBuffer::GetRawBuffer() const
{
  return mBuffer;
}

std::size_t EncodedImageBuffer::GetHash() const
{
  return mBufferHash;
}

void EncodedImageBuffer::SetImageType(Dali::EncodedImageBuffer::ImageType type)
{
  mType = type;
}

Dali::EncodedImageBuffer::ImageType EncodedImageBuffer::GetImageType() const
{
  return mType;
}

} // namespace Internal

} // namespace Dali
