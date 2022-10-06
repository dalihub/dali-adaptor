/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include "encoded-image-buffer.h"

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

EncodedImageBuffer::EncodedImageBuffer(EncodedImageBuffer&& handle) = default;

EncodedImageBuffer& EncodedImageBuffer::operator=(EncodedImageBuffer&& handle) = default;

const EncodedImageBuffer::RawBufferType& EncodedImageBuffer::GetRawBuffer() const
{
  return GetImplementation(*this).GetRawBuffer();
}

const std::size_t EncodedImageBuffer::GetHash() const
{
  return GetImplementation(*this).GetHash();
}

EncodedImageBuffer EncodedImageBuffer::New(const RawBufferType& buffer)
{
  IntrusivePtr<Internal::EncodedImageBuffer> internal = Internal::EncodedImageBuffer::New(buffer);
  return EncodedImageBuffer(internal.Get());
}

} // namespace Dali
