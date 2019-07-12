#ifndef DALI_FILE_STREAM_IMPL_GENERIC_H
#define DALI_FILE_STREAM_IMPL_GENERIC_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-stream.h>
#include <fstream>
#include <sstream>

// INTERNAL INCLUDES

namespace Dali
{

class FileStream::Impl
{
public:
  Impl(const std::string& filename, uint8_t mode);

  Impl(uint8_t* buffer, size_t dataSize, uint8_t mode);

  Impl(Dali::Vector<uint8_t>& buffer, size_t dataSize, uint8_t mode);

  ~Impl();

  std::iostream& GetStream();

  FILE* GetFile();

private:
  std::string mFileName;
  uint8_t mMode;
  uint8_t* mBuffer; // external buffer, not owned
  size_t mDataSize;

  Dali::Vector<char> mFileBuffer; // for internal usage only
  FILE* mFile;
  std::fstream mFileStream;
  std::stringstream mBufferStream;
};

} // Dali

#endif // DALI_FILE_STREAM_IMPL_GENERIC_H
