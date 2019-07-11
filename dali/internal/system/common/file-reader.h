#ifndef DALI_INTERNAL_PORTABLE_FILE_READER_H
#define DALI_INTERNAL_PORTABLE_FILE_READER_H

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

class FileReader : public FileStream
{
public:
  FileReader( const std::string& filename )
  : FileStream( filename, FileStream::READ | FileStream::BINARY )
  {
  }

  FileReader( Dali::Vector<uint8_t>& vector )
  : FileStream( &vector[0], vector.Size(), FileStream::READ | FileStream::BINARY )
  {
  }

  FileReader( Dali::Vector<uint8_t>& vector, size_t dataSize )
  : FileStream( &vector[0], dataSize, FileStream::READ | FileStream::BINARY )
  {
  }

  FileReader( uint8_t* data, size_t dataSize )
  : FileStream( data, dataSize, FileStream::READ | FileStream::BINARY )
  {
  }
};

} /* namespace Platform */
} /* namespace Internal */
} /* namespace Dali */

#endif // DALI_INTERNAL_PORTABLE_FILE_READER_H
