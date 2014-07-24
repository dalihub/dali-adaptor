/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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


#include "assimp-loader.h"
#include <assimp/assimp.hpp>
#include <stdio.h>


namespace Dali
{
namespace SlpPlatform
{

AssimpIOStream::AssimpIOStream()
: Assimp::IOStream(),
  mLoader(NULL)
{
}

AssimpIOStream::AssimpIOStream(FILE* loader)
: Assimp::IOStream(),
  mLoader(loader)
{
}

AssimpIOStream::~AssimpIOStream()
{
  if (NULL != mLoader)
  {
    fclose(mLoader);
    mLoader = NULL;
  }
}

size_t AssimpIOStream::Read(void *pvBuffer, size_t pSize, size_t pCount)
{
  int elementCount = 0;

  if (NULL != mLoader && 0 != pSize && 0 != pCount)
  {
    elementCount = fread(pvBuffer, pSize, pCount, mLoader);
  }

  return elementCount;
}

size_t AssimpIOStream::Write(const void *pvBuffer, size_t pSize, size_t pCount)
{
  return 0;
}

aiReturn AssimpIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
  aiReturn status = aiReturn_FAILURE;
  if (NULL != mLoader)
  {
    if( 0 <= fseek(mLoader, pOffset, aiOrigin_SET == pOrigin ? SEEK_SET :
                                    (aiOrigin_CUR == pOrigin ? SEEK_CUR : SEEK_END)) )
    {
      status = aiReturn_SUCCESS;
    }
  }
  return status;
}

size_t AssimpIOStream::Tell() const
{
  size_t filePosition = 0;

  if (NULL != mLoader)
  {
    long positionIndicator = ftell( mLoader );
    if( positionIndicator > -1L )
    {
      filePosition = static_cast<unsigned int>(positionIndicator);
    }
    else
    {
      DALI_LOG_ERROR("Error finding the current file offset\n");
    }
  }

  return filePosition;
}

size_t AssimpIOStream::FileSize() const
{
  size_t fileSize = 0;

  if (NULL != mLoader)
  {
    size_t pos = 0u;

    long positionIndicator = ftell( mLoader );
    if( positionIndicator > -1L )
    {
      pos = static_cast<unsigned int>(positionIndicator);
    }
    else
    {
      DALI_LOG_ERROR("Error finding the current file offset\n");
    }

    if (0 == fseek(mLoader, 0, SEEK_END) )
    {
      positionIndicator = ftell( mLoader );
      if ( positionIndicator > -1L )
      {
        fileSize = static_cast<unsigned int>(positionIndicator);
      }
      else
      {
        DALI_LOG_ERROR("Error finding file size\n");
      }
    }

    if( fseek(mLoader, pos, SEEK_SET) )
    {
      DALI_LOG_ERROR("Error seeking to previous position\n");
    }
  }

  return fileSize;
}

void AssimpIOStream::Flush()
{
  fflush(mLoader);
}

AssimpIOSystem::AssimpIOSystem()
: Assimp::IOSystem()
{
}

AssimpIOSystem::~AssimpIOSystem()
{
}

bool AssimpIOSystem::Exists(const std::string& pFile) const
{
  return Exists(pFile.c_str());
}

bool AssimpIOSystem::Exists(const char* pFile) const
{
  FILE* fp;

  fp = fopen(pFile, "rb");
  if (fp)
  {
    fclose(fp);
  }

  return NULL != fp;
}

char AssimpIOSystem::getOsSeparator() const
{
  return '/';
}

Assimp::IOStream* AssimpIOSystem::Open(const char* pFile, const char* pMode)
{
  Assimp::IOStream* stream = NULL;
  FILE* fp = NULL;

  fp = fopen(pFile, pMode);
  if (NULL != fp)
  {
    stream = new AssimpIOStream(fp);
  }

  return stream;
}

Assimp::IOStream* AssimpIOSystem::Open(const std::string& pFile, const std::string& pMode)
{
  return Open(pFile.c_str(), pMode);
}

void AssimpIOSystem::Close( Assimp::IOStream* pFile)
{
  delete pFile;
}

} // namespace SlpPlatform
} // namespace Dali

