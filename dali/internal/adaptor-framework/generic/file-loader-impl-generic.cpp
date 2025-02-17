/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <cstring>
#include <fstream>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/system-error-print.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
int ReadFile(const std::string& filename, Dali::Vector<char>& memblock, Dali::FileLoader::FileType fileType)
{
  std::streampos size;

  return Dali::Internal::Adaptor::ReadFile(filename, size, memblock, fileType);
}

int ReadFile(const std::string& filename, Dali::Vector<uint8_t>& memblock, Dali::FileLoader::FileType fileType)
{
  std::streampos size;

  return Dali::Internal::Adaptor::ReadFile(filename, size, memblock, fileType);
}

template<typename T>
int ReadFile(const std::string& filename, std::streampos& fileSize, Dali::Vector<T>& memblock, Dali::FileLoader::FileType fileType)
{
  int errorCode = 0;

  if(fileType != Dali::FileLoader::BINARY && fileType != Dali::FileLoader::TEXT)
  {
    DALI_LOG_ERROR("Invaild fileType! input fileType:%d file: \"%s\"\n", static_cast<int>(fileType), filename.c_str());
    return errorCode;
  }

  auto iosFlags = std::ios::in | std::ios::ate;
  if(fileType == Dali::FileLoader::BINARY)
  {
    iosFlags |= std::ios::binary;
  }

  std::ifstream file(filename.c_str(), iosFlags);

  if(file.is_open())
  {
    fileSize = file.tellg();

    if(file.seekg(0, std::ios::beg).good() == false)
    {
      DALI_LOG_ERROR("Failed to seek the beginning of the file: \"%s\"\n", filename.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return errorCode;
    }

    memblock.ResizeUninitialized(fileSize);

    if(file.read(reinterpret_cast<char*>(memblock.Begin()), fileSize).good() == false)
    {
      DALI_LOG_ERROR("Failed to read the file: \"%s\"\n", filename.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return errorCode;
    }
    file.close();

    errorCode = 1;
  }
  else
  {
    DALI_LOG_ERROR("file open failed for: \"%s\"\n", filename.c_str());
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }

  return errorCode;
}

std::streampos GetFileSize(const std::string& filename)
{
  std::streampos size = 0;

  std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  if(file.is_open())
  {
    size = file.tellg();
    file.close();
  }
  else
  {
    DALI_LOG_ERROR("file open failed for: \"%s\"\n", filename.c_str());
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }

  return size;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
