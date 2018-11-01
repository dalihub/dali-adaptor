/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/imaging/common/image-loader-plugin-proxy.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace ImageLoaderPluginProxy
{


static const char * DEFAULT_OBJECT_NAME( "libdali-image-loader-plugin.so" );

static bool mInitializeAttempted = false;
static void* mLibHandle = NULL;
static CreateImageLoaderPlugin* mCreatePluginFunctionPtr = NULL;
static DestroyImageLoaderPlugin* mDestroyImageLoaderPluginPtr = NULL;
static Dali::ImageLoaderPlugin* mImageLoaderPlugin = NULL;

#if defined(DEBUG_ENABLED)
/**
 * Disable logging of image loader plugin proxy or make it verbose from the commandline
 * as follows (e.g., for dali demo app):
 * <code>
 * LOG_IMAGE_LOADER_PLUGIN=0 dali-demo #< off
 * LOG_IMAGE_LOADER_PLUGIN=3 dali-demo #< on, verbose
 * </code>
 */
Debug::Filter* gImageLoaderPluginLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_IMAGE_LOADER_PLUGIN" );
#endif

void Initialize()
{
  // Only attempt to load dll once
  char* error = NULL;
  if ( !mInitializeAttempted )
  {
    mInitializeAttempted = true;
    mLibHandle = dlopen( DEFAULT_OBJECT_NAME, RTLD_LAZY );
    error = dlerror();
    if( !mLibHandle )
    {
      DALI_LOG_INFO( gImageLoaderPluginLogFilter, Dali::Integration::Log::Verbose, "Cannot load dali image loading plugin library error: %s\n", error );
      return;
    }

    // load plugin
    mCreatePluginFunctionPtr = reinterpret_cast<CreateImageLoaderPlugin*>( dlsym( mLibHandle, "CreateImageLoaderPlugin" ) );
    error = dlerror();
    if( !mCreatePluginFunctionPtr )
    {
      DALI_LOG_ERROR("Cannot load symbol CreateImageLoaderPlugin(): %s\n", error );
      return;
    }

    mDestroyImageLoaderPluginPtr = reinterpret_cast<DestroyImageLoaderPlugin*>( dlsym( mLibHandle, "DestroyImageLoaderPlugin" ) );
    error = dlerror();
    if( !mDestroyImageLoaderPluginPtr )
    {
      DALI_LOG_ERROR("Cannot load symbol DestroyImageLoaderPlugin(): %s\n", error );
      return;
    }


    mImageLoaderPlugin = mCreatePluginFunctionPtr();
    error = dlerror();
    if( !mImageLoaderPlugin )
    {
      DALI_LOG_ERROR("Call to function CreateImageLoaderPlugin() failed : %s\n", error );
      return;
    }
  }
}

void Destroy()
{
  if( mImageLoaderPlugin && mDestroyImageLoaderPluginPtr )
  {
    mDestroyImageLoaderPluginPtr( mImageLoaderPlugin );
    mImageLoaderPlugin = NULL;
  }
}

const ImageLoader::BitmapLoader* BitmapLoaderLookup( const std::string& filename )
{
  if( mImageLoaderPlugin )
  {
    const ImageLoader::BitmapLoader* data = mImageLoaderPlugin->BitmapLoaderLookup( filename );
    return data;
  }
  return NULL;
}

} // namespace ImageLoaderPluginProxy


} // namespace Adaptor

} // namespace Internal

} // namespace Dali
