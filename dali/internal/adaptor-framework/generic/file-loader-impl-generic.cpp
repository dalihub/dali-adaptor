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
#include <dali/internal/adaptor-framework/common/file-loader-impl.h>

// EXTERNAL INCLUDES
#include <string>
#include <fstream>

#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

int ReadFile(const std::string& filename, Dali::Vector<char> & memblock, Dali::FileLoader::FileType fileType)
{
  std::streampos size;

  return Dali::Internal::Adaptor::ReadFile( filename, size, memblock, fileType);
}

int ReadFile(const std::string& filename, std::streampos& fileSize, Dali::Vector<char> & memblock, Dali::FileLoader::FileType fileType)
{
  int errorCode = 0;
  std::ifstream * file;

  if( fileType == Dali::FileLoader::BINARY )
  {
    file = new std::ifstream (filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
  }
  else if( fileType == Dali::FileLoader::TEXT )
  {
    file = new std::ifstream (filename.c_str(), std::ios::in|std::ios::ate);
  }
  else
  {
    return errorCode;
  }

  if( file->is_open() )
  {
    fileSize = file->tellg();

    memblock.Resize( fileSize );

    file->seekg (0, std::ios::beg);
    file->read( memblock.Begin(), fileSize );
    file->close();

    delete file;

    errorCode = 1;
  }
  else
  {
    DALI_LOG_WARNING( "file open failed for: \"%s\"", filename );
  }

  return errorCode;
}

std::streampos GetFileSize(const std::string& filename)
{
  std::streampos size = 0;

  std::ifstream file( filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate );
  if( file.is_open() )
  {
    size = file.tellg();
    file.close();
  }

  return size;
}

} // Adaptor

} // Internal

} // Dali
