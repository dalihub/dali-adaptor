/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/configuration-manager.h>

// EXTERNAL INCLUDES
#include <fstream>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-stream.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/thread-controller.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

const std::string SYSTEM_CACHE_FILE = "gpu-environment.conf";
const std::string DALI_ENV_MULTIPLE_WINDOW_SUPPORT = "DALI_ENV_MULTIPLE_WINDOW_SUPPORT";
const std::string DALI_BLEND_EQUATION_ADVANCED_SUPPORT = "DALI_BLEND_EQUATION_ADVANCED_SUPPORT";
const std::string DALI_GLSL_VERSION = "DALI_GLSL_VERSION";

bool RetrieveKeyFromConfigFile( std::iostream& stream, const std::string& key, std::string& value )
{
  bool keyFound = false;

  std::string line;
  while( std::getline( stream, line ) )
  {
    line.erase( line.find_last_not_of( " \t\r\n" ) + 1 );
    line.erase( 0, line.find_first_not_of( " \t\r\n" ) );
    if( '#' == *( line.cbegin() ) || line == "" )
    {
      continue;
    }

    std::istringstream stream( line );
    std::string name;
    std::getline(stream, name, ' ');
    if( name == key )
    {
      std::getline(stream, value);
      keyFound = true;
      break;
    }
  }

  return keyFound;
}


} // unnamed namespace

ConfigurationManager::ConfigurationManager( std::string systemCachePath, EglGraphics* eglGraphics, ThreadController* threadController )
: mSystemCacheFilePath( systemCachePath + SYSTEM_CACHE_FILE ),
  mEglGraphics( eglGraphics ),
  mThreadController( threadController ),
  mMaxTextureSize( 0u ),
  mIsMultipleWindowSupported( true ),
  mIsAdvancedBlendEquationSupported( true ),
  mMaxTextureSizeCached( false ),
  mIsMultipleWindowSupportedCached( false ),
  mIsAdvancedBlendEquationSupportedCached( false ),
  mGlslVersionCached( false )
{
}

ConfigurationManager::~ConfigurationManager()
{
}

void ConfigurationManager::RetrieveKeysFromConfigFile( const std::string& configFilePath )
{
  Dali::FileStream configFile( configFilePath, Dali::FileStream::READ | Dali::FileStream::TEXT );
  std::iostream& stream = configFile.GetStream();
  if( stream.rdbuf()->in_avail() )
  {
    std::string value;
    if( !mMaxTextureSizeCached &&
        RetrieveKeyFromConfigFile( stream, DALI_ENV_MAX_TEXTURE_SIZE, value ) )
    {
      mMaxTextureSize = std::atoi( value.c_str() );
      mMaxTextureSizeCached = true;
    }

    if( !mGlslVersionCached &&
        RetrieveKeyFromConfigFile( stream, DALI_GLSL_VERSION, value ) )
    {
      mGlslVersion = std::atoi( value.c_str() );
      mGlslVersionCached = true;
    }

    if( !mIsMultipleWindowSupportedCached &&
        RetrieveKeyFromConfigFile( stream, DALI_ENV_MULTIPLE_WINDOW_SUPPORT, value ) )
    {
      mIsMultipleWindowSupported = std::atoi( value.c_str() );
      mIsMultipleWindowSupportedCached = true;
    }

    if( !mIsAdvancedBlendEquationSupportedCached &&
        RetrieveKeyFromConfigFile( stream, DALI_BLEND_EQUATION_ADVANCED_SUPPORT, value ) )
    {
      mIsAdvancedBlendEquationSupported = std::atoi( value.c_str() );
      mIsAdvancedBlendEquationSupportedCached = true;
    }
  }
}

uint32_t ConfigurationManager::GetMaxTextureSize()
{
  if( !mMaxTextureSizeCached )
  {
    RetrieveKeysFromConfigFile( mSystemCacheFilePath );

    if( !mMaxTextureSizeCached )
    {
      GlImplementation& mGLES = mEglGraphics->GetGlesInterface();
      mMaxTextureSize = mGLES.GetMaxTextureSize();
      mMaxTextureSizeCached = true;

      Dali::FileStream configFile( mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT );
      std::fstream& stream = dynamic_cast<std::fstream&>( configFile.GetStream() );
      if( stream.is_open() )
      {
        stream << DALI_ENV_MAX_TEXTURE_SIZE << " " << mMaxTextureSize << std::endl;
      }
      else
      {
        DALI_LOG_ERROR( "Fail to open file : %s\n", mSystemCacheFilePath.c_str() );
      }
    }
  }

  return mMaxTextureSize;
}

uint32_t ConfigurationManager::GetShadingLanguageVersion()
{
  if ( !mGlslVersionCached )
  {
    RetrieveKeysFromConfigFile( mSystemCacheFilePath );

    if ( !mGlslVersionCached )
    {
      EglImplementation& eglImpl = mEglGraphics->GetEglImplementation();
      if ( !eglImpl.IsGlesInitialized() )
      {
        // Wait until GLES is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from GLES and save the cache
      mGlslVersion = mEglGraphics->GetGlesInterface().GetShadingLanguageVersion();
      DALI_LOG_ERROR("mGlslVersion : %d\n", mGlslVersion);
      mGlslVersionCached = true;

      Dali::FileStream configFile( mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT );
      std::fstream& stream = dynamic_cast<std::fstream&>( configFile.GetStream() );
      if ( stream.is_open() )
      {
        stream << DALI_GLSL_VERSION << " " << mGlslVersion << std::endl;
      }
      else
      {
        DALI_LOG_ERROR( "Fail to open file : %s\n", mSystemCacheFilePath.c_str() );
      }
    }
  }

  return mGlslVersion;
}

bool ConfigurationManager::IsMultipleWindowSupported()
{
  if ( !mIsMultipleWindowSupportedCached )
  {
    RetrieveKeysFromConfigFile( mSystemCacheFilePath );

    if ( !mIsMultipleWindowSupportedCached )
    {
      EglImplementation& eglImpl = mEglGraphics->GetEglImplementation();
      if ( !eglImpl.IsGlesInitialized() )
      {
        // Wait until GLES is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from GLES and save the cache
      mIsMultipleWindowSupported = eglImpl.IsSurfacelessContextSupported();
      mIsMultipleWindowSupportedCached = true;

      Dali::FileStream configFile( mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT );
      std::fstream& stream = dynamic_cast<std::fstream&>( configFile.GetStream() );
      if ( stream.is_open() )
      {
        stream << DALI_ENV_MULTIPLE_WINDOW_SUPPORT << " " << mIsMultipleWindowSupported << std::endl;
      }
      else
      {
        DALI_LOG_ERROR( "Fail to open file : %s\n", mSystemCacheFilePath.c_str() );
      }
    }
  }

  return mIsMultipleWindowSupported;
}

bool ConfigurationManager::IsAdvancedBlendEquationSupported()
{
  if ( !mIsAdvancedBlendEquationSupportedCached )
  {
    RetrieveKeysFromConfigFile( mSystemCacheFilePath );

    if ( !mIsAdvancedBlendEquationSupportedCached )
    {
      EglImplementation& eglImpl = mEglGraphics->GetEglImplementation();
      if ( !eglImpl.IsGlesInitialized() )
      {
        // Wait until GLES is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from GLES and save the cache
      mIsAdvancedBlendEquationSupported = mEglGraphics->GetGlesInterface().IsAdvancedBlendEquationSupported();
      mIsAdvancedBlendEquationSupportedCached = true;

      Dali::FileStream configFile( mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT );
      std::fstream& stream = dynamic_cast<std::fstream&>( configFile.GetStream() );
      if ( stream.is_open() )
      {
        stream << DALI_BLEND_EQUATION_ADVANCED_SUPPORT << " " << mIsAdvancedBlendEquationSupported << std::endl;
      }
      else
      {
        DALI_LOG_ERROR( "Fail to open file : %s\n", mSystemCacheFilePath.c_str() );
      }
    }
  }

  return mIsAdvancedBlendEquationSupported;
}

} // Adaptor

} // Internal

} // Dali
