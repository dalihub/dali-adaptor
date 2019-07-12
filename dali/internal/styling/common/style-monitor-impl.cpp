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

// CLASS HEADER
#include <dali/internal/styling/common/style-monitor-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-loader.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/singleton-service-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_STYLE_MONITOR");
#endif

/**
 * Use font client to get the system default font family
 * @param[in] fontClient handle to font client
 * @param[out] fontFamily string representing font family
 */
void GetSystemDefaultFontFamily( TextAbstraction::FontClient& fontClient, std::string& fontFamily )
{
  TextAbstraction::FontDescription defaultFontDescription;
  if ( fontClient )
  {
    fontClient.GetDefaultPlatformFontDescription( defaultFontDescription );
    fontFamily = defaultFontDescription.family;
  }
}

} // unnamed namespace

Dali::StyleMonitor StyleMonitor::Get()
{
  Dali::StyleMonitor styleMonitor;

  Dali::SingletonService service( SingletonService::Get() );
  if( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::StyleMonitor ) );
    if( handle )
    {
      // If so, downcast the handle
      styleMonitor = Dali::StyleMonitor( dynamic_cast< StyleMonitor* >( handle.GetObjectPtr() ) );
    }
    else
    {
      styleMonitor = Dali::StyleMonitor( new StyleMonitor() );
      service.Register( typeid( styleMonitor ), styleMonitor );
    }
  }

  return styleMonitor;
}

StyleMonitor::StyleMonitor()
: mDefaultFontSize(-1)
{
  mFontClient = TextAbstraction::FontClient::Get();
  GetSystemDefaultFontFamily( mFontClient, mDefaultFontFamily );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "StyleMonitor::StyleMonitor::DefaultFontFamily(%s)\n", mDefaultFontFamily.c_str() );
  mDefaultFontSize = mFontClient.GetDefaultFontSize();
}

StyleMonitor::~StyleMonitor()
{
}

void StyleMonitor::StyleChanged( StyleChange::Type styleChange )
{
  switch ( styleChange )
  {
    case StyleChange::DEFAULT_FONT_CHANGE:
    {
      if ( mFontClient )
      {
        mFontClient.ResetSystemDefaults();
        GetSystemDefaultFontFamily( mFontClient, mDefaultFontFamily );
      }
      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "StyleMonitor::StyleChanged::DefaultFontFamily(%s)\n", mDefaultFontFamily.c_str() );
      break;
    }

    case StyleChange::DEFAULT_FONT_SIZE_CHANGE:
    {
      mDefaultFontSize = mFontClient.GetDefaultFontSize();
      break;
    }

    case StyleChange::THEME_CHANGE:
    {
      break;
    }
  }

  EmitStyleChangeSignal(styleChange);
}

std::string StyleMonitor::GetDefaultFontFamily() const
{
  return mDefaultFontFamily;
}

std::string StyleMonitor::GetDefaultFontStyle() const
{
  return mDefaultFontStyle;
}

int StyleMonitor::GetDefaultFontSize() const
{
  return mDefaultFontSize;
}

const std::string& StyleMonitor::GetTheme() const
{
  return mUserDefinedThemeFilePath;
}

void StyleMonitor::SetTheme(const std::string& path)
{
  mUserDefinedThemeFilePath = path;
  EmitStyleChangeSignal( StyleChange::THEME_CHANGE );
}

bool StyleMonitor::LoadThemeFile( const std::string& filename, std::string& output )
{
  bool retval( false );

  std::streampos bufferSize = 0;
  Dali::Vector<char> fileBuffer;
  if( Dali::FileLoader::ReadFile( filename, bufferSize, fileBuffer, FileLoader::FileType::TEXT ) )
  {
    output.assign( &fileBuffer[0], bufferSize );
    retval = true;
  }

  return retval;
}

Dali::StyleMonitor::StyleChangeSignalType& StyleMonitor::StyleChangeSignal()
{
  return mStyleChangeSignal;
}

void StyleMonitor::EmitStyleChangeSignal( StyleChange::Type styleChange )
{
  if( !mStyleChangeSignal.Empty() )
  {
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "StyleMonitor::EmitStyleChangeSignal\n" );
    Dali::StyleMonitor handle( this );
    mStyleChangeSignal.Emit( handle, styleChange );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
