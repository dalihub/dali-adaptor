#ifndef DALI_INTERNAL_PORTABLE_FILE_WRITER_H
#define DALI_INTERNAL_PORTABLE_FILE_WRITER_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-stream.h>

// EXTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Platform
{

class FileWriter : public FileStream
{
public:
  /**
   * Opens a file pointer onto the memory for writing to.
   * Note, in some implementations, the vector may be resized to be larger than dataSize.
   * @param[in,out] vector The vector to write to
   * @param[in] dataSize the amount of data to be written
   */
  FileWriter( Dali::Vector<uint8_t>& vector, size_t dataSize )
  : FileStream( vector, dataSize, FileStream::WRITE | FileStream::BINARY )
  {
  }
};

} /* namespace Platform */
} /* namespace Internal */
} /* namespace Dali */

#endif // DALI_INTERNAL_PORTABLE_FILE_WRITER_H
