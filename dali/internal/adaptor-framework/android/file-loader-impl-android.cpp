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
#include <dali/integration-api/debug.h>
#include <cstdio>
#include <fstream>
#include <string>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/android/android-framework.h>
#include <dali/internal/adaptor/common/framework.h>
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

inline bool hasPrefix(const std::string& prefix, const std::string& path)
{
  return std::mismatch(prefix.begin(), prefix.end(), path.begin()).first == prefix.end();
}

inline std::string ConvertToAssetsInternalPath(const std::string& path, int offset)
{
  std::string internalPath = std::string(path.c_str() + offset);

  int i = 0;
  while((i = internalPath.find("//", i)) != std::string::npos)
  {
    internalPath.replace(i, 2, "/");
  }

  return internalPath;
}

template<typename T>
int ReadFile(const std::string& filename, std::streampos& fileSize, Dali::Vector<T>& memblock, Dali::FileLoader::FileType fileType)
{
  int  errorCode = 0;
  int  length    = 0;
  char mode[3]   = {'r', 0, 0};

  if(fileType == Dali::FileLoader::BINARY)
  {
    mode[1] = 'b';
  }
  else if(fileType != Dali::FileLoader::TEXT)
  {
    return errorCode;
  }

  const std::string assetsPrefix = "assets/";
  if(hasPrefix(assetsPrefix, filename))
  {
    std::string    internalPath = ConvertToAssetsInternalPath(filename, assetsPrefix.length());
    AAssetManager* assetManager = Dali::Integration::AndroidFramework::Get().GetApplicationAssets();
    AAsset*        asset        = AAssetManager_open(assetManager, internalPath.c_str(), AASSET_MODE_BUFFER);
    if(asset)
    {
      length = AAsset_getLength(asset);
      memblock.ResizeUninitialized(length + 1);
      memblock[length] = 0; // 1 for extra zero at the end

      char* buffer = reinterpret_cast<char*>(memblock.Begin());
      errorCode    = (AAsset_read(asset, buffer, length) != length) ? 0 : 1;
      fileSize     = length;

      AAsset_close(asset);
    }
    else
    {
      DALI_LOG_ERROR("Asset not found %s\n", internalPath.c_str());
    }
  }
  else
  {
    FILE* file = fopen(filename.c_str(), mode);
    if(file)
    {
      fseek(file, 0, SEEK_END);
      length = ftell(file);
      memblock.ResizeUninitialized(length + 1);
      //put last byte as 0, in case this is a text file without null-terminator
      memblock[length] = 0;

      char* buffer = reinterpret_cast<char*>(memblock.Begin());
      fseek(file, 0, SEEK_SET);
      errorCode = (fread(buffer, 1, length, file) != length) ? 0 : 1;
      fileSize  = length;

      fclose(file);
    }
    else
    {
      DALI_LOG_ERROR("File not found %s\n", filename.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }
  }

  return errorCode;
}

std::streampos GetFileSize(const std::string& filename)
{
  std::streampos size = 0;

  const std::string assetsPrefix = "assets/";
  if(hasPrefix(assetsPrefix, filename))
  {
    std::string    internalPath = ConvertToAssetsInternalPath(filename, assetsPrefix.length());
    AAssetManager* assetManager = Dali::Integration::AndroidFramework::Get().GetApplicationAssets();
    AAsset*        asset        = AAssetManager_open(assetManager, internalPath.c_str(), AASSET_MODE_BUFFER);
    if(asset)
    {
      size = AAsset_getLength(asset);
      AAsset_close(asset);
    }
    else
    {
      DALI_LOG_ERROR("Asset not found %s\n", internalPath.c_str());
    }
  }
  else
  {
    FILE* file = fopen(filename.c_str(), "r");
    if(file)
    {
      fseek(file, 0, SEEK_END);
      size = ftell(file);
      fclose(file);
    }
    else
    {
      DALI_LOG_ERROR("File not found %s\n", filename.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }
  }

  return size;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
