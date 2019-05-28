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
 *
 */
// EXTERNAL INCLUDES
#include <android_native_app_glue.h>
#include <cstdio>
#include <string>

// INTERNAL INCLUDES
#include <dali/internal/system/android/file-wrapper.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/integration-api/debug.h>

FILE* openFile(const char* path, const char* mode)
{
  const int assetsOffset = ( sizeof("assets/") - sizeof( char ) ) / sizeof( char );
  if( !strncmp( path, "assets/", assetsOffset ) )
  {
    android_app* androidApp = static_cast<android_app*>( Dali::Internal::Adaptor::Framework::GetApplicationContext() );
    AAsset* asset = AAssetManager_open( androidApp->activity->assetManager, path + assetsOffset, AASSET_MODE_BUFFER );
    if( asset )
    {
      size_t length = AAsset_getLength( asset ) + 1;
      FILE* file = fmemopen( nullptr, length, "wb+" ); // has to be append otherwise seek is inverted
      char* buffer = new char[ length ];
      length = AAsset_read( asset, buffer, length );

      fwrite( buffer, 1, length, file );
      fflush( file );
      fseek( file, 0, SEEK_SET );

      delete[] buffer;
      AAsset_close( asset );

      return file;
    }
    else
    {
      DALI_LOG_ERROR( "File not found %s\n", path );
    }
  }

  FILE* file = fopen( path, mode );
  if( !file )
  {
    DALI_LOG_ERROR( "File not found %s\n", path );
  }

  return file;
}

long getFileSize(const std::string& filename, int& errorCode)
{
  const char* path = filename.c_str();
  const int assetsOffset = ( sizeof("assets/") - sizeof( char ) ) / sizeof( char );
  long size = 0;
  errorCode = 0;

  if( !strncmp( path, "assets/", assetsOffset ) )
  {
    android_app* androidApp = static_cast<android_app*>( Dali::Internal::Adaptor::Framework::GetApplicationContext() );
    AAsset* asset = AAssetManager_open( androidApp->activity->assetManager, path + assetsOffset, AASSET_MODE_BUFFER );
    if( asset )
    {
      errorCode = 1;
      size = AAsset_getLength( asset );
      AAsset_close( asset );
    }
    else
    {
      DALI_LOG_ERROR( "File not found %s\n", path );
    }
  }
  else
  {
    FILE* file = fopen( path, "rb" );
    if( file )
    {
      errorCode = 1;
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

int readFile(const std::string& filename, char* output, long size)
{
  const char* path = filename.c_str();
  const int assetsOffset = ( sizeof("assets/") - sizeof( char ) ) / sizeof( char );
  long read = 0;

  if( !strncmp( path, "assets/", assetsOffset ) )
  {
    android_app* androidApp = static_cast<android_app*>( Dali::Internal::Adaptor::Framework::GetApplicationContext() );
    AAsset* asset = AAssetManager_open( androidApp->activity->assetManager, path + assetsOffset, AASSET_MODE_BUFFER );
    if( asset )
    {
      read = AAsset_read( asset, output, size );
      AAsset_close( asset );
    }
  }
  else
  {
    FILE* file = fopen( path, "rb" );
    if( file )
    {
      read = fread( output, 1, size, file );
      fclose( file );
    }
  }

  return read;
}

int readFile(const std::string& filename, Dali::Vector<char>& memblock)
{
  int errorCode = 0;
  long size = getFileSize( filename, errorCode );

  if( size )
  {
    memblock.Resize( size );
    size = readFile( filename, memblock.Begin(), size );
  }

  return errorCode;
}

int readFile(const std::string& filename, std::string& output)
{
  int errorCode = 0;
  long size = getFileSize( filename, errorCode );

  if( size )
  {
    char* buffer = new char[ size ];
    size = readFile( filename, buffer, size );
    output.assign( buffer, buffer+size );
    delete[] buffer;
  }

  return errorCode;
}

