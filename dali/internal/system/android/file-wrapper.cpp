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

FILE* fopen_wrapper(const char* path, const char* mode)
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
  }

  return fopen( path, mode );
}
