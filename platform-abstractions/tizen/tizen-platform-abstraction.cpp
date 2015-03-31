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
#include "dynamics/dynamics-factory.h"

#include "tizen-font-configuration-parser.h"
#include "data-cache/metrics-cache.h"
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
const std::string DEFAULT_FONT_FAMILY( "HelveticaNeue" );                  ///< Default font family when unable to retrieve from font configuration file
const std::string DEFAULT_FONT_STYLE( "Book" );                            ///< Default font style when unable to retrieve from font configuration file
const std::string NULL_FONT_FAMILY_NAME( "" );
const unsigned int NANOSECS_TO_MICROSECS( 1000 );                          ///< 1000 nanoseconds = 1 microsecond

/// Settings to float point conversion table
const float FONT_SIZE_TABLE[5] =
{
  8.0f,  ///< Small
  10.0f, ///< Normal
  15.0f, ///< Large
  19.0f, ///< Huge
  25.0f  ///< Giant
};
}

TizenPlatformAbstraction::TizenPlatformAbstraction()
: mResourceLoader(new ResourceLoader),
  mDynamicsFactory(NULL),
  mDataStoragePath( "" ),
  mDefaultFontSize(FONT_SIZE_TABLE[1])
{
  int error = FT_Init_FreeType(&mFreeTypeHandle);
  DALI_ASSERT_ALWAYS( error == 0 && "Freetype initialization failed" );

  UpdateDefaultsFromDevice();
}

TizenPlatformAbstraction::~TizenPlatformAbstraction()
{
  delete mResourceLoader;
  delete mDynamicsFactory;

  if (mFreeTypeHandle)
  {
    FT_Done_FreeType(mFreeTypeHandle);
  }
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

const std::string& TizenPlatformAbstraction::GetDefaultFontFamily() const
{
  // VCC TODO: return default font style as well.
  return mDefaultFontFamily;
}

float TizenPlatformAbstraction::GetDefaultFontSize() const
{
  return mDefaultFontSize;
}

PixelSize TizenPlatformAbstraction::GetFontLineHeightFromCapsHeight(const std::string& fontFamily, const std::string& fontStyle, CapsHeight capsHeight) const
{
  PixelSize result(0);

  if (mResourceLoader)
  {
    result = mResourceLoader->GetFontLineHeightFromCapsHeight(fontFamily, fontStyle, capsHeight, mFreeTypeHandle);
  }

  return result;
}

Integration::GlyphSet* TizenPlatformAbstraction::GetGlyphData ( const Dali::Integration::TextResourceType& textRequest,
                                                              const std::string& fontFamily,
                                                              bool getBitmap) const
{
  if (mResourceLoader)
  {
    return mResourceLoader->GetGlyphData(textRequest,
                                         mFreeTypeHandle,
                                         fontFamily,
                                         getBitmap);
  }
  return NULL;
}

Integration::GlyphSet* TizenPlatformAbstraction::GetCachedGlyphData( const Integration::TextResourceType& textRequest,
                                                                   const std::string& fontFamily ) const
{
  if (mResourceLoader)
  {
    return mResourceLoader->GetCachedGlyphData( textRequest, fontFamily );
  }
  return NULL;
}


void TizenPlatformAbstraction::GetGlobalMetrics( const std::string& fontFamily, const std::string& fontStyle, Integration::GlobalMetrics& globalMetrics ) const
{
  if( mResourceLoader )
  {
    mResourceLoader->GetGlobalMetrics( mFreeTypeHandle,
                                       fontFamily,
                                       fontStyle,
                                       globalMetrics );
  }
}

void TizenPlatformAbstraction::GetClosestImageSize( const std::string& filename,
                                                  const ImageAttributes& attributes,
                                                  Vector2& closestSize )
{
  closestSize = Vector2::ZERO;
  ImageLoader::GetClosestImageSize(filename, attributes, closestSize );
}

void TizenPlatformAbstraction::GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                                  const ImageAttributes& attributes,
                                                  Vector2& closestSize )
{
  closestSize = Vector2::ZERO;
  ImageLoader::GetClosestImageSize(resourceBuffer, attributes, closestSize );
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

const std::string& TizenPlatformAbstraction::GetFontFamilyForChars(const Integration::TextArray& charsRequested) const
{
  if( mResourceLoader )
  {
    return mResourceLoader->GetFontFamilyForChars(charsRequested);
  }

  return NULL_FONT_FAMILY_NAME;
}

bool TizenPlatformAbstraction::AllGlyphsSupported(const std::string &fontFamily, const std::string& fontStyle, const Integration::TextArray& charsRequested) const
{
  bool ret = false;
  if (mResourceLoader)
  {
    ret = mResourceLoader->AllGlyphsSupported(fontFamily, fontStyle, charsRequested);
  }
  return ret;
}

bool TizenPlatformAbstraction::ValidateFontFamilyName(const std::string& fontFamily, const std::string& fontStyle, bool& isDefaultSystemFont, std::string& closestMatch, std::string& closestStyleMatch) const
{
  bool ret = false;
  if( mResourceLoader )
  {
    // TODO: Consider retrieve both isDefaultSystemFontFamily and isDefaultSystemFontStyle.
    bool isDefaultFamily = false;
    bool isDefaultStyle = false;
    ret = mResourceLoader->ValidateFontFamilyName( fontFamily, fontStyle, isDefaultFamily, isDefaultStyle, closestMatch, closestStyleMatch );
    isDefaultSystemFont = isDefaultFamily && isDefaultStyle;
  }
  return ret;
}

void TizenPlatformAbstraction::GetFontList(  Dali::Integration::PlatformAbstraction::FontListMode mode, std::vector<std::string>& fontList ) const
{
  if( mResourceLoader )
  {
    mResourceLoader->GetFontList( mode, fontList );
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

void TizenPlatformAbstraction::UpdateDefaultsFromDevice()
{
  // FontConfigurationParser::Parse sets the default font family and the default font style.
  // If the isn't a configuration file or is invalid, or it doesn't have any tag with the default
  // font family nor font style then default values set by the application are used.
  mDefaultFontFamily = DEFAULT_FONT_FAMILY;
  mDefaultFontStyle = DEFAULT_FONT_STYLE;

  // The initialized values above are not used to parse the configuration file. These values
  // are set just in case FontConfigurationParser::Parse is not able to set default values.
  FontConfigurationParser::Parse(FONT_CONFIGURATION_FILE, mDefaultFontFamily, mDefaultFontStyle);

  if ( mResourceLoader )
  {
    mResourceLoader->SetDefaultFontFamily( mDefaultFontFamily, mDefaultFontStyle );
  }

  int fontSize(0);
#ifndef DALI_PROFILE_UBUNTU
  vconf_get_int( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &fontSize );
#endif // DALI_PROFILE_UBUNTU
  mDefaultFontSize = FONT_SIZE_TABLE[fontSize];
}

Integration::DynamicsFactory* TizenPlatformAbstraction::GetDynamicsFactory()
{
  if( NULL == mDynamicsFactory )
  {
    mDynamicsFactory = new DynamicsFactory;
  }

  return mDynamicsFactory;
}

bool TizenPlatformAbstraction::ReadGlobalMetricsFromCache( const std::string& fontFamily,
                                                         const std::string& fontStyle,
                                                         Integration::GlobalMetrics& globalMetrics )
{
  return MetricsCache::ReadGlobal( fontFamily, fontStyle, globalMetrics );
}

void TizenPlatformAbstraction::WriteGlobalMetricsToCache( const std::string& fontFamily,
                                                        const std::string& fontStyle,
                                                        const Integration::GlobalMetrics& globalMetrics )
{
  MetricsCache::WriteGlobal( fontFamily, fontStyle, globalMetrics);
}

bool TizenPlatformAbstraction::ReadMetricsFromCache( const std::string& fontFamily,
                                                   const std::string& fontStyle,
                                                   std::vector<Integration::GlyphMetrics>& glyphMetricsContainer )
{
  return MetricsCache::Read( fontFamily, fontStyle, glyphMetricsContainer );
}

void TizenPlatformAbstraction::WriteMetricsToCache( const std::string& fontFamily,
                                                  const std::string& fontStyle,
                                                  const Integration::GlyphSet& glyphSet )
{
  MetricsCache::Write( fontFamily, fontStyle, glyphSet );
}

void TizenPlatformAbstraction::GetFileNamesFromDirectory( const std::string& directoryName,
                                                        std::vector<std::string>& fileNames )
{
  dirent* de = NULL;
  DIR* dp;
  dp = opendir( directoryName.c_str() );
  if( dp )
  {
    const std::string dot( "." );
    const std::string dotDot( ".." );
    while( true )
    {
      de = readdir( dp );
      if( de == NULL )
      {
        break;
      }
      const std::string fileName( de->d_name );
      if( ( fileName != dot ) &&
          ( fileName != dotDot ) )
      {
        fileNames.push_back( fileName );
      }
    }
    closedir( dp );
  }
}

Integration::BitmapPtr TizenPlatformAbstraction::GetGlyphImage( const std::string& fontFamily, const std::string& fontStyle, const float fontSize, const uint32_t character ) const
{
  Integration::BitmapPtr glyphImage;

  if( mResourceLoader )
  {
    glyphImage = mResourceLoader->GetGlyphImage( mFreeTypeHandle, fontFamily, fontStyle, fontSize, character );
  }

  return glyphImage;
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
