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

#include "tizen-platform-abstraction.h"

#ifndef DALI_PROFILE_UBUNTU
#include <vconf.h>
#endif // DALI_PROFILE_UBUNTU
#include <dirent.h>

#include <dali/integration-api/debug.h>
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/resource-types.h>

#include "resource-loader/resource-loader.h"

#include "tizen-font-configuration-parser.h"
#include "image-loaders/image-loader.h"

namespace Dali
{

Integration::PlatformAbstraction* CreatePlatformAbstraction()
{
  return new TizenPlatform::TizenPlatformAbstraction();
}


namespace TizenPlatform
{

namespace
{
const std::string FONT_CONFIGURATION_FILE( FONT_CONFIGURATION_FILE_PATH ); ///< Default font configuration file
const unsigned int NANOSECS_TO_MICROSECS( 1000 );                          ///< 1000 nanoseconds = 1 microsecond
}

TizenPlatformAbstraction::TizenPlatformAbstraction()
: mResourceLoader(new ResourceLoader),
  mDataStoragePath( "" )
{
}

TizenPlatformAbstraction::~TizenPlatformAbstraction()
{
  delete mResourceLoader;
}

void TizenPlatformAbstraction::GetTimeMicroseconds(unsigned int &seconds, unsigned int &microSeconds)
{
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  seconds = time.tv_sec;
  microSeconds = time.tv_nsec / NANOSECS_TO_MICROSECS;
}

void TizenPlatformAbstraction::Suspend()
{
  if (mResourceLoader)
  {
    mResourceLoader->Pause();
  }
}

void TizenPlatformAbstraction::Resume()
{
  if (mResourceLoader)
  {
    mResourceLoader->Resume();
  }
}

void TizenPlatformAbstraction::GetDefaultFontDescription( std::string& fontFamily, std::string& fontStyle ) const
{
  FontConfigurationParser::Parse(FONT_CONFIGURATION_FILE, fontFamily, fontStyle);
}

int TizenPlatformAbstraction::GetDefaultFontSize() const
{
  int fontSize( -1 );

#ifndef DALI_PROFILE_UBUNTU
  vconf_get_int( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &fontSize );
#endif // DALI_PROFILE_UBUNTU

  return fontSize;
}

ImageDimensions TizenPlatformAbstraction::GetClosestImageSize( const std::string& filename,
                                                               ImageDimensions size,
                                                               FittingMode::Type fittingMode,
                                                               SamplingMode::Type samplingMode,
                                                               bool orientationCorrection )
{
  return ImageLoader::GetClosestImageSize( filename, size, fittingMode, samplingMode, orientationCorrection );
}

ImageDimensions TizenPlatformAbstraction::GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                                               ImageDimensions size,
                                                               FittingMode::Type fittingMode,
                                                               SamplingMode::Type samplingMode,
                                                               bool orientationCorrection )
{
  return ImageLoader::GetClosestImageSize( resourceBuffer, size, fittingMode, samplingMode, orientationCorrection );
}

void TizenPlatformAbstraction::LoadResource(const Integration::ResourceRequest& request)
{
  if (mResourceLoader)
  {
    mResourceLoader->LoadResource(request);
  }
}

Integration::ResourcePointer TizenPlatformAbstraction::LoadResourceSynchronously(const Integration::ResourceType& resourceType, const std::string& resourcePath)
{
  return ImageLoader::LoadResourceSynchronously( resourceType, resourcePath );
}


void TizenPlatformAbstraction::SaveResource(const Integration::ResourceRequest& request)
{
  if (mResourceLoader)
  {
    if( request.GetType()->id == Integration::ResourceShader )
    {
#ifdef SHADERBIN_CACHE_ENABLED
      std::string path = mDataStoragePath;
      path += request.GetPath();

      Integration::ResourceRequest newRequest( request.GetId(), *request.GetType(), path, request.GetResource() );
      mResourceLoader->SaveResource(newRequest);
#endif
    }
    else
    {
      mResourceLoader->SaveResource(request);
    }
  }
}

void TizenPlatformAbstraction::CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId)
{
  if (mResourceLoader)
  {
    mResourceLoader->CancelLoad(id, typeId);
  }
}

bool TizenPlatformAbstraction::IsLoading()
{
  if (mResourceLoader)
  {
    return mResourceLoader->IsLoading();
  }

  return false;
}

void TizenPlatformAbstraction::GetResources(Integration::ResourceCache& cache)
{
  if (mResourceLoader)
  {
    mResourceLoader->GetResources(cache);
  }
}

void TizenPlatformAbstraction::SetDpi(unsigned int dpiHor, unsigned int dpiVer)
{
  if (mResourceLoader)
  {
    mResourceLoader->SetDpi(dpiHor, dpiVer);
  }
}

bool TizenPlatformAbstraction::LoadFile( const std::string& filename, std::vector< unsigned char >& buffer ) const
{
  bool result = false;

  if (mResourceLoader)
  {
    result = mResourceLoader->LoadFile(filename, buffer);
  }

  return result;
}

std::string TizenPlatformAbstraction::LoadFile( const std::string& filename )
{
  std::string result;
  if (mResourceLoader)
  {
    result = mResourceLoader->LoadFile(filename);
  }

  return result;
}

bool TizenPlatformAbstraction::SaveFile(const std::string& filename, std::vector< unsigned char >& buffer) const
{
  bool result = false;

  if (mResourceLoader)
  {
    result = mResourceLoader->SaveFile(filename, buffer);
  }

  return result;
}

void TizenPlatformAbstraction::JoinLoaderThreads()
{
  delete mResourceLoader;
  mResourceLoader = NULL;
}

bool TizenPlatformAbstraction::LoadShaderBinFile( const std::string& filename, std::vector< unsigned char >& buffer ) const
{
  bool result = false;

#ifdef SHADERBIN_CACHE_ENABLED
  std::string path;

  if( mResourceLoader )
  {
    path = DALI_SHADERBIN_DIR;
    path += filename;
    result = mResourceLoader->LoadFile( path, buffer );
  }

  if( mResourceLoader && result == false )
  {
    path = mDataStoragePath;
    path += filename;
    result = mResourceLoader->LoadFile( path, buffer );
  }
#endif

  return result;
}

void TizenPlatformAbstraction::SetDataStoragePath( const std::string& path )
{
  mDataStoragePath = path;
}

}  // namespace TizenPlatform

}  // namespace Dali
