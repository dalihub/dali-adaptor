//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "slp-platform-abstraction.h"

#include <vconf.h>

#include <dali/integration-api/debug.h>
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/resource-types.h>

#include "resource-loader/resource-loader.h"
#include "resource-loader/loader-jpeg.h"
#include "resource-loader/loader-png.h"

#include "dynamics/dynamics-factory.h"

#include "slp-font-configuration-parser.h"
#include "data-cache/metrics-cache.h"

namespace Dali
{

DALI_IMPORT_API Integration::PlatformAbstraction* CreatePlatformAbstraction()
{
  return new SlpPlatform::SlpPlatformAbstraction();
}


namespace SlpPlatform
{

namespace
{
const std::string FONT_CONFIGURATION_FILE( FONT_CONFIGURATION_FILE_PATH ); ///< Default font configuration file
const std::string DEFAULT_FONT_FAMILY( "HelveticaNeue" );                  ///< Default font family when unable to retrieve from font configuration file
const std::string DEFAULT_FONT_STYLE( "Book" );                            ///< Default font style when unable to retrieve from font configuration file
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

SlpPlatformAbstraction::SlpPlatformAbstraction()
: mResourceLoader(new ResourceLoader),
  mDefaultFontSize(FONT_SIZE_TABLE[1]),
  mDynamicsFactory(NULL)
{
  int error = FT_Init_FreeType(&mFreeTypeHandle);
  DALI_ASSERT_ALWAYS( error == 0 && "Freetype initialization failed" );

  UpdateDefaultsFromDevice();
}

SlpPlatformAbstraction::~SlpPlatformAbstraction()
{
  delete mResourceLoader;
  delete mDynamicsFactory;

  if (mFreeTypeHandle)
  {
    FT_Done_FreeType(mFreeTypeHandle);
  }
}

void SlpPlatformAbstraction::GetTimeMicroseconds(unsigned int &seconds, unsigned int &microSeconds)
{
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  seconds = time.tv_sec;
  microSeconds = time.tv_nsec / NANOSECS_TO_MICROSECS;
}

void SlpPlatformAbstraction::Suspend()
{
  if (mResourceLoader)
  {
    mResourceLoader->Pause();
  }
}

void SlpPlatformAbstraction::Resume()
{
  if (mResourceLoader)
  {
    mResourceLoader->Resume();
  }
}

std::string SlpPlatformAbstraction::GetDefaultFontFamily() const
{
  // VCC TODO: return default font style as well.
  return mDefaultFontFamily;
}

const float SlpPlatformAbstraction::GetDefaultFontSize() const
{
  return mDefaultFontSize;
}

const PixelSize SlpPlatformAbstraction::GetFontLineHeightFromCapsHeight(const std::string fontFamily, const std::string& fontStyle, const CapsHeight& capsHeight) const
{
  PixelSize result(0);

  if (mResourceLoader)
  {
    result = mResourceLoader->GetFontLineHeightFromCapsHeight(fontFamily, fontStyle, capsHeight, mFreeTypeHandle);
  }

  return result;
}

Integration::GlyphSet* SlpPlatformAbstraction::GetGlyphData ( const Dali::Integration::TextResourceType& textRequest,
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

Integration::GlyphSet* SlpPlatformAbstraction::GetCachedGlyphData( const Integration::TextResourceType& textRequest,
                                                                   const std::string& fontFamily ) const
{
  if (mResourceLoader)
  {
    return mResourceLoader->GetCachedGlyphData( textRequest, fontFamily );
  }
  return NULL;
}


void SlpPlatformAbstraction::GetGlobalMetrics( const std::string& fontFamily, const std::string& fontStyle, Integration::GlobalMetrics& globalMetrics ) const
{
  if( mResourceLoader )
  {
    mResourceLoader->GetGlobalMetrics( mFreeTypeHandle,
                                       fontFamily,
                                       fontStyle,
                                       globalMetrics );
  }
}

void SlpPlatformAbstraction::GetClosestImageSize( const std::string& filename,
                                                  const ImageAttributes& attributes,
                                                  Vector2& closestSize )
{
  closestSize = Vector2::ZERO;
  if (mResourceLoader)
  {
    mResourceLoader->GetClosestImageSize(filename, attributes, closestSize );
  }
}

void SlpPlatformAbstraction::GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                                  const ImageAttributes& attributes,
                                                  Vector2& closestSize )
{
  closestSize = Vector2::ZERO;
  if (mResourceLoader)
  {
    mResourceLoader->GetClosestImageSize(resourceBuffer, attributes, closestSize );
  }
}


void SlpPlatformAbstraction::LoadResource(const Integration::ResourceRequest& request)
{
  if (mResourceLoader)
  {
    mResourceLoader->LoadResource(request);
  }
}

Integration::ResourcePointer SlpPlatformAbstraction::LoadResourceSynchronously(const Integration::ResourceType& resourceType, const std::string& resourcePath)
{
  Integration::ResourcePointer resource;

  if (mResourceLoader)
  {
    resource = mResourceLoader->LoadResourceSynchronously( resourceType, resourcePath );
  }
  return resource;
}


void SlpPlatformAbstraction::SaveResource(const Integration::ResourceRequest& request)
{
  if (mResourceLoader)
  {
    mResourceLoader->SaveResource(request);
  }
}

void SlpPlatformAbstraction::CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId)
{
  if (mResourceLoader)
  {
    mResourceLoader->CancelLoad(id, typeId);
  }
}

bool SlpPlatformAbstraction::IsLoading()
{
  if (mResourceLoader)
  {
    return mResourceLoader->IsLoading();
  }

  return false;
}

void SlpPlatformAbstraction::GetResources(Integration::ResourceCache& cache)
{
  if (mResourceLoader)
  {
    mResourceLoader->GetResources(cache);
  }
}

void SlpPlatformAbstraction::SetDpi(unsigned int dpiHor, unsigned int dpiVer)
{
  if (mResourceLoader)
  {
    mResourceLoader->SetDpi(dpiHor, dpiVer);
  }
}

std::string SlpPlatformAbstraction::GetFontFamilyForChars(const TextArray& charsRequested) const
{
  std::string ret;
  if (mResourceLoader)
  {
    ret = mResourceLoader->GetFontFamilyForChars(charsRequested);
  }
  return ret;
}

bool SlpPlatformAbstraction::AllGlyphsSupported(const std::string &fontFamily, const std::string& fontStyle, const TextArray& charsRequested) const
{
  bool ret = false;
  if (mResourceLoader)
  {
    ret = mResourceLoader->AllGlyphsSupported(fontFamily, fontStyle, charsRequested);
  }
  return ret;
}

bool SlpPlatformAbstraction::ValidateFontFamilyName(const std::string& fontFamily, const std::string& fontStyle, bool& isDefaultSystemFont, std::string& closestMatch, std::string& closestStyleMatch) const
{
  bool ret = false;
  if (mResourceLoader)
  {
    ret = mResourceLoader->ValidateFontFamilyName(fontFamily, fontStyle, isDefaultSystemFont, closestMatch, closestStyleMatch);
  }
  return ret;
}

std::vector<std::string> SlpPlatformAbstraction::GetFontList (  Dali::Integration::PlatformAbstraction::FontListMode mode ) const
{
  std::vector<std::string> ret;
  if (mResourceLoader)
  {
    ret = mResourceLoader->GetFontList( mode );
  }
  return ret;
}

bool SlpPlatformAbstraction::LoadFile( const std::string& filename, std::vector< unsigned char >& buffer ) const
{
  bool result = false;

  if (mResourceLoader)
  {
    result = mResourceLoader->LoadFile(filename, buffer);
  }

  return result;
}

std::string SlpPlatformAbstraction::LoadFile( const std::string& filename )
{
  std::string result;
  if (mResourceLoader)
  {
    result = mResourceLoader->LoadFile(filename);
  }

  return result;
}

bool SlpPlatformAbstraction::SaveFile(const std::string& filename, std::vector< unsigned char >& buffer) const
{
  bool result = false;

  if (mResourceLoader)
  {
    result = mResourceLoader->SaveFile(filename, buffer);
  }

  return result;
}

void SlpPlatformAbstraction::JoinLoaderThreads()
{
  delete mResourceLoader;
  mResourceLoader = NULL;
}

void SlpPlatformAbstraction::UpdateDefaultsFromDevice()
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
  vconf_get_int( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &fontSize );
  mDefaultFontSize = FONT_SIZE_TABLE[fontSize];
}

Integration::DynamicsFactory* SlpPlatformAbstraction::GetDynamicsFactory()
{
  if( NULL == mDynamicsFactory )
  {
    mDynamicsFactory = new DynamicsFactory;
  }

  return mDynamicsFactory;
}

bool SlpPlatformAbstraction::ReadGlobalMetricsFromCache( const std::string& fontFamily,
                                                         const std::string& fontStyle,
                                                         Integration::GlobalMetrics& globalMetrics )
{
  return MetricsCache::ReadGlobal( fontFamily, fontStyle, globalMetrics );
}

void SlpPlatformAbstraction::WriteGlobalMetricsToCache( const std::string& fontFamily,
                                                        const std::string& fontStyle,
                                                        const Integration::GlobalMetrics& globalMetrics )
{
  MetricsCache::WriteGlobal( fontFamily, fontStyle, globalMetrics);
}

bool SlpPlatformAbstraction::ReadMetricsFromCache( const std::string& fontFamily,
                                                   const std::string& fontStyle,
                                                   std::vector<Integration::GlyphMetrics>& glyphMetricsContainer )
{
  return MetricsCache::Read( fontFamily, fontStyle, glyphMetricsContainer );
}

void SlpPlatformAbstraction::WriteMetricsToCache( const std::string& fontFamily,
                                                  const std::string& fontStyle,
                                                  const Integration::GlyphSet& glyphSet )
{
  MetricsCache::Write( fontFamily, fontStyle, glyphSet );
}

}  // namespace SlpPlatform

}  // namespace Dali
