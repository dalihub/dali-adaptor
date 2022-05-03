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
#include "encoded-image-buffer-impl.h"

// EXTERNAL INCLUDE
#include <dali/devel-api/common/hash.h>

namespace Dali
{
namespace Internal
{
EncodedImageBuffer::EncodedImageBuffer(const RawBufferType& buffer)
: mBuffer(buffer)
{
  mBufferHash = CalculateHash(mBuffer);
}

EncodedImageBuffer::~EncodedImageBuffer()
{
}

IntrusivePtr<EncodedImageBuffer> EncodedImageBuffer::New(const RawBufferType& buffer)
{
  IntrusivePtr<EncodedImageBuffer> internal = new EncodedImageBuffer(buffer);

  return internal;
}

const EncodedImageBuffer::RawBufferType& EncodedImageBuffer::GetRawBuffer() const
{
  return mBuffer;
}

const std::size_t EncodedImageBuffer::GetHash() const
{
  return mBufferHash;
}

} // namespace Internal

} // namespace Dali
