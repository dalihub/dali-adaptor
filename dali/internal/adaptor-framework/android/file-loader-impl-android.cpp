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
#include <cstdio>
#include <string>
#include <fstream>

#include <android_native_app_glue.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/android/file-wrapper.h>
#include <dali/internal/adaptor/common/framework.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

int ReadFile(const std::string& filename, Dali::Vector<char>& memblock, Dali::FileLoader::FileType fileType)
{
  std::streampos size;

  return Dali::Internal::Adaptor::ReadFile( filename, size, memblock, fileType);
}

int ReadFile(const std::string& filename, std::streampos& fileSize, Dali::Vector<char>& memblock, Dali::FileLoader::FileType fileType)
{
  int errorCode = 0;
  int length = 0;
  char mode[3] = { 'r', 0, 0 };

  if( fileType == Dali::FileLoader::BINARY )
  {
    mode[1] = 'b';
  }
  else if( fileType != Dali::FileLoader::TEXT )
  {
    return errorCode;
  }

  const char* path = filename.c_str();
  const int assetsOffset = ( sizeof("assets/") - sizeof( char ) ) / sizeof( char );
  if( !strncmp( path, "assets/", assetsOffset ) )
  {
    android_app* androidApp = static_cast<android_app*>( Dali::Internal::Adaptor::Framework::GetApplicationContext() );
    AAsset* asset = AAssetManager_open( androidApp->activity->assetManager, path + assetsOffset, AASSET_MODE_BUFFER );
    if( asset )
    {
      length = AAsset_getLength( asset );
      memblock.Resize( length + 1 ); // 1 for extra zero at the end

      char* buffer = &memblock[0];
      errorCode = ( AAsset_read( asset, buffer, length ) != length ) ? 0 : 1;
      fileSize = length;

      AAsset_close( asset );
    }
    else
    {
      DALI_LOG_ERROR( "Asset not found %s\n", path );
    }
  }
  else {
    FILE* file = fopen( path,  mode );
    if( file )
    {
      fseek( file, 0, SEEK_END );
      fileSize = ftell( file );
      memblock.Resize( length + 1 ); // 1 for extra zero at the end

      char* buffer = &memblock[0];
      fseek( file, 0, SEEK_SET );
      errorCode = ( fread( buffer, 1, length, file ) != length ) ? 0 : 1;
      fileSize = length;

      fclose( file );
    }
    else
    {
      DALI_LOG_ERROR( "File not found %s\n", path );
    }
  }

  return errorCode;
}

std::streampos GetFileSize(const std::string& filename)
{
  std::streampos size = 0;

  const char* path = filename.c_str();
  const int assetsOffset = ( sizeof("assets/") - sizeof( char ) ) / sizeof( char );
  if( !strncmp( path, "assets/", assetsOffset ) )
  {
    android_app* androidApp = static_cast<android_app*>( Dali::Internal::Adaptor::Framework::GetApplicationContext() );
    AAsset* asset = AAssetManager_open( androidApp->activity->assetManager, path + assetsOffset, AASSET_MODE_BUFFER );
    if( asset )
    {
      size = AAsset_getLength( asset );
      AAsset_close( asset );
    }
    else
    {
      DALI_LOG_ERROR( "Asset not found %s\n", path );
    }
  }
  else {
    FILE* file = fopen( path, "r" );
    if( file )
    {
      fseek( file, 0, SEEK_END );
      size = ftell( file );
      fclose( file );
    }
    else
    {
      DALI_LOG_ERROR( "File not found %s\n", path );
    }
  }

  return size;
}

} // Adaptor

} // Internal

} // Dali
