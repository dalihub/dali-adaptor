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
#include <dali/public-api/adaptor-framework/encoded-image-buffer.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/encoded-image-buffer-impl.h>

namespace Dali
{
EncodedImageBuffer::EncodedImageBuffer()
{
}

EncodedImageBuffer::~EncodedImageBuffer()
{
}

EncodedImageBuffer::EncodedImageBuffer(Internal::EncodedImageBuffer* internal)
: BaseHandle(internal)
{
}

EncodedImageBuffer::EncodedImageBuffer(const EncodedImageBuffer& handle) = default;

EncodedImageBuffer& EncodedImageBuffer::operator=(const EncodedImageBuffer& handle) = default;

EncodedImageBuffer::EncodedImageBuffer(EncodedImageBuffer&& handle) noexcept = default;

EncodedImageBuffer& EncodedImageBuffer::operator=(EncodedImageBuffer&& handle) noexcept = default;

const EncodedImageBuffer::RawBufferType& EncodedImageBuffer::GetRawBuffer() const
{
  return GetImplementation(*this).GetRawBuffer();
}

std::size_t EncodedImageBuffer::GetHash() const
{
  return GetImplementation(*this).GetHash();
}

void EncodedImageBuffer::SetImageType(ImageType type)
{
  GetImplementation(*this).SetImageType(type);
}

EncodedImageBuffer::ImageType EncodedImageBuffer::GetImageType() const
{
  return GetImplementation(*this).GetImageType();
}

EncodedImageBuffer EncodedImageBuffer::New(const RawBufferType& buffer)
{
  return EncodedImageBuffer::New(buffer, ImageType::DEFAULT);
}

EncodedImageBuffer EncodedImageBuffer::New(const RawBufferType& buffer, ImageType type)
{
  IntrusivePtr<Internal::EncodedImageBuffer> internal = Internal::EncodedImageBuffer::New(buffer, type);
  return EncodedImageBuffer(internal.Get());
}

} // namespace Dali
