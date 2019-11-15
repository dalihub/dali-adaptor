/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/file-stream.h>

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <dali/internal/adaptor-framework/common/file-stream-impl.h>

namespace Dali
{

FileStream::FileStream(const std::string& filename, uint8_t mode)
{
  mImpl.reset( new Impl( filename, mode ) );
}

FileStream::FileStream(uint8_t* buffer, size_t dataSize, uint8_t mode)
{
  mImpl.reset( new Impl( buffer, dataSize, mode ) );
}

FileStream::FileStream(Dali::Vector<uint8_t>& buffer, size_t dataSize, uint8_t mode)
{
  mImpl.reset( new Impl( buffer, dataSize, mode ) );
}

FileStream::FileStream(FileStream&&) = default;

FileStream& FileStream::operator=(FileStream&&) = default;

FileStream::~FileStream() = default;

std::iostream& FileStream::GetStream()
{
  return mImpl->GetStream();
}

FILE* FileStream::GetFile()
{
  return mImpl->GetFile();
}

} // Dali
