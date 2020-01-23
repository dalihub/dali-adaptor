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

bool RetrieveKeyFromFile( std::fstream& stream, std::string key, std::string& value )
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
  mFileStream( new Dali::FileStream( mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT ) ),
  mEglGraphics( eglGraphics ),
  mThreadController( threadController ),
  mMaxTextureSize( 0u ),
  mIsMultipleWindowSupported( true ),
  mMaxTextureSizeCached( false ) ,
  mIsMultipleWindowSupportedCached( false )
{
}

ConfigurationManager::~ConfigurationManager()
{
}

unsigned int ConfigurationManager::GetMaxTextureSize()
{
  if ( !mMaxTextureSizeCached )
  {
    std::fstream& configFile = dynamic_cast<std::fstream&>( mFileStream->GetStream() );
    if( configFile.is_open() )
    {
      std::string environmentVariableValue;
      if( RetrieveKeyFromFile( configFile, DALI_ENV_MAX_TEXTURE_SIZE, environmentVariableValue ) )
      {
        mMaxTextureSize = std::atoi( environmentVariableValue.c_str() );
      }
      else
      {
        GlImplementation& mGLES = mEglGraphics->GetGlesInterface();
        mMaxTextureSize = mGLES.GetMaxTextureSize();

        configFile.clear();
        configFile << DALI_ENV_MAX_TEXTURE_SIZE << " " << mMaxTextureSize << std::endl;
      }

      mMaxTextureSizeCached = true;

      if ( mIsMultipleWindowSupportedCached )
      {
        configFile.close();
      }
    }
    else
    {
      DALI_LOG_ERROR( "Fail to open file : %s\n", mSystemCacheFilePath.c_str() );
    }
  }

  return mMaxTextureSize;
}

bool ConfigurationManager::IsMultipleWindowSupported()
{
  if ( !mIsMultipleWindowSupportedCached )
  {
    std::fstream& configFile = dynamic_cast<std::fstream&>( mFileStream->GetStream() );
    if( configFile.is_open() )
    {
      std::string environmentVariableValue;
      if( RetrieveKeyFromFile( configFile, DALI_ENV_MULTIPLE_WINDOW_SUPPORT, environmentVariableValue ) )
      {
        mIsMultipleWindowSupported = std::atoi( environmentVariableValue.c_str() );
      }
      else
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

        configFile.clear();
        configFile << DALI_ENV_MULTIPLE_WINDOW_SUPPORT << " " << mIsMultipleWindowSupported << std::endl;
      }

      mIsMultipleWindowSupportedCached = true;

      if ( mMaxTextureSizeCached )
      {
        configFile.close();
      }
    }
    else
    {
      DALI_LOG_ERROR( "Fail to open file : %s\n", mSystemCacheFilePath.c_str() );
    }
  }

  return mIsMultipleWindowSupported;
}

} // Adaptor

} // Internal

} // Dali
