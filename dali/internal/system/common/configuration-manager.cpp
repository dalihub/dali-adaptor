/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/debug.h>
#include <fstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-stream.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/thread-controller.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const char* SYSTEM_CACHE_FILE                           = "gpu-environment.conf";
const char* DALI_ENV_MULTIPLE_WINDOW_SUPPORT            = "DALI_ENV_MULTIPLE_WINDOW_SUPPORT";
const char* DALI_BLEND_EQUATION_ADVANCED_SUPPORT        = "DALI_BLEND_EQUATION_ADVANCED_SUPPORT";
const char* DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT = "DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT";
const char* DALI_GLSL_VERSION                           = "DALI_GLSL_VERSION";
const char* DALI_GRAPHICS_BACKEND_TYPE                  = "DALI_GRAPHICS_BACKEND_TYPE";

} // unnamed namespace

ConfigurationManager::ConfigurationManager(std::string systemCachePath, Graphics::GraphicsInterface* graphics, ThreadController* threadController)
: mSystemCacheFilePath(systemCachePath + SYSTEM_CACHE_FILE),
  mGraphics(graphics),
  mThreadController(threadController),
  mMaxTextureSize(0u),
  mMaxCombinedTextureUnits(0u),
  mShaderLanguageVersion(0u),
  mIsMultipleWindowSupported(true),
  mIsAdvancedBlendEquationSupported(true),
  mIsMultisampledRenderToTextureSupported(true),
  mMaxTextureSizeCached(false),
  mIsMultipleWindowSupportedCached(false),
  mIsAdvancedBlendEquationSupportedCached(false),
  mIsMultisampledRenderToTextureSupportedCached(false),
  mShaderLanguageVersionCached(false),
  mMaxCombinedTextureUnitsCached(false)
{
  // First, read the cached graphics backend from the config file
  RetrieveKeysFromConfigFile(mSystemCacheFilePath);

  // Check for backend switching and handle it during initialization
  CheckAndHandleBackendSwitch();
}

ConfigurationManager::~ConfigurationManager()
{
}

void ConfigurationManager::RetrieveKeysFromConfigFile(const std::string& configFilePath)
{
  Dali::FileStream configFile(configFilePath, Dali::FileStream::READ | Dali::FileStream::TEXT);
  std::iostream&   stream = configFile.GetStream();
  if(stream.rdbuf()->in_avail())
  {
    std::string line;
    while(std::getline(stream, line))
    {
      line.erase(line.find_last_not_of(" \t\r\n") + 1);
      line.erase(0, line.find_first_not_of(" \t\r\n"));
      if('#' == *(line.cbegin()) || line == "")
      {
        continue;
      }

      std::istringstream subStream(line);
      std::string        name;
      std::string        value;
      std::getline(subStream, name, ' ');
      if(!mMaxTextureSizeCached && name == DALI_ENV_MAX_TEXTURE_SIZE)
      {
        std::getline(subStream, value);
        mMaxTextureSize       = std::atoi(value.c_str());
        mMaxTextureSizeCached = true;
      }
      if(!mMaxCombinedTextureUnitsCached && name == DALI_ENV_MAX_COMBINED_TEXTURE_UNITS)
      {
        std::getline(subStream, value);
        mMaxCombinedTextureUnits       = std::atoi(value.c_str());
        mMaxCombinedTextureUnitsCached = true;
      }
      else if(!mIsAdvancedBlendEquationSupportedCached && name == DALI_BLEND_EQUATION_ADVANCED_SUPPORT)
      {
        std::getline(subStream, value);
        mIsAdvancedBlendEquationSupported       = std::atoi(value.c_str());
        mIsAdvancedBlendEquationSupportedCached = true;
      }
      else if(!mIsMultisampledRenderToTextureSupportedCached && name == DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT)
      {
        std::getline(subStream, value);
        mIsMultisampledRenderToTextureSupported       = std::atoi(value.c_str());
        mIsMultisampledRenderToTextureSupportedCached = true;
      }
      else if(!mShaderLanguageVersionCached && name == DALI_GLSL_VERSION)
      {
        std::getline(subStream, value);
        mShaderLanguageVersion       = std::atoi(value.c_str());
        mShaderLanguageVersionCached = true;
      }
      else if(!mIsMultipleWindowSupportedCached && name == DALI_ENV_MULTIPLE_WINDOW_SUPPORT)
      {
        std::getline(subStream, value);
        mIsMultipleWindowSupported       = std::atoi(value.c_str());
        mIsMultipleWindowSupportedCached = true;
      }
      else if(name == DALI_GRAPHICS_BACKEND_TYPE)
      {
        std::getline(subStream, value);
        mCachedGraphicsBackend = value;
      }
    }
  }
}

uint32_t ConfigurationManager::GetMaxTextureSize()
{
  if(!mMaxTextureSizeCached)
  {
    RetrieveKeysFromConfigFile(mSystemCacheFilePath);

    if(!mMaxTextureSizeCached)
    {
      if(!mGraphics->IsInitialized())
      {
        // Wait until Graphics Subsystem is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      mMaxTextureSize       = mGraphics->GetMaxTextureSize();
      mMaxTextureSizeCached = true;

      Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
      std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
      if(stream.is_open())
      {
        stream << DALI_ENV_MAX_TEXTURE_SIZE << " " << mMaxTextureSize << std::endl;
      }
      else
      {
        DALI_LOG_ERROR("Fail to open file : %s\n", mSystemCacheFilePath.c_str());
      }
    }
  }

  return mMaxTextureSize;
}

uint32_t ConfigurationManager::GetMaxCombinedTextureUnits()
{
  if(!mMaxCombinedTextureUnitsCached)
  {
    RetrieveKeysFromConfigFile(mSystemCacheFilePath);

    if(!mMaxCombinedTextureUnitsCached)
    {
      if(!mGraphics->IsInitialized())
      {
        // Wait until Graphics Subsystem is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      mMaxCombinedTextureUnits       = mGraphics->GetMaxCombinedTextureUnits();
      mMaxCombinedTextureUnitsCached = true;
      DALI_LOG_RENDER_INFO("MaxCombinedTextureUnits = %d\n", mMaxCombinedTextureUnits);

      Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
      std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
      if(stream.is_open())
      {
        stream << DALI_ENV_MAX_COMBINED_TEXTURE_UNITS << " " << mMaxCombinedTextureUnits << std::endl;
      }
      else
      {
        DALI_LOG_ERROR("Fail to open file : %s\n", mSystemCacheFilePath.c_str());
      }
    }
  }

  return mMaxCombinedTextureUnits;
}

uint32_t ConfigurationManager::GetShadingLanguageVersion()
{
  if(!mShaderLanguageVersionCached)
  {
    RetrieveKeysFromConfigFile(mSystemCacheFilePath);

    if(!mShaderLanguageVersionCached)
    {
      if(!mGraphics->IsInitialized())
      {
        // Wait until Graphics Subsystem is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from graphics and save the cache
      mShaderLanguageVersion       = mGraphics->GetShaderLanguageVersion();
      mShaderLanguageVersionCached = true;

      Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
      std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
      if(stream.is_open())
      {
        stream << DALI_GLSL_VERSION << " " << mShaderLanguageVersion << std::endl;
      }
      else
      {
        DALI_LOG_ERROR("Fail to open file : %s\n", mSystemCacheFilePath.c_str());
      }
    }
  }

  return mShaderLanguageVersion;
}

bool ConfigurationManager::IsMultipleWindowSupported()
{
  if(!mIsMultipleWindowSupportedCached)
  {
    RetrieveKeysFromConfigFile(mSystemCacheFilePath);

    if(!mIsMultipleWindowSupportedCached)
    {
      if(!mGraphics->IsInitialized())
      {
        // Wait until Graphics Subsystem is initialised, but this will happen once.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from Graphics Subsystem and save the cache
      mIsMultipleWindowSupported       = mGraphics->IsResourceContextSupported();
      mIsMultipleWindowSupportedCached = true;

      Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
      std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
      if(stream.is_open())
      {
        stream << DALI_ENV_MULTIPLE_WINDOW_SUPPORT << " " << mIsMultipleWindowSupported << std::endl;
      }
      else
      {
        DALI_LOG_ERROR("Fail to open file : %s\n", mSystemCacheFilePath.c_str());
      }
    }
  }

  return mIsMultipleWindowSupported;
}

bool ConfigurationManager::IsAdvancedBlendEquationSupported()
{
  if(!mIsAdvancedBlendEquationSupportedCached)
  {
    RetrieveKeysFromConfigFile(mSystemCacheFilePath);

    if(!mIsAdvancedBlendEquationSupportedCached)
    {
      if(!mGraphics->IsInitialized())
      {
        // Wait until graphics subsystem is initialised, but this will happen once per factory reset.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from Graphics Subsystem and save the cache
      mIsAdvancedBlendEquationSupported       = mGraphics->IsAdvancedBlendEquationSupported();
      mIsAdvancedBlendEquationSupportedCached = true;

      Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
      std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
      if(stream.is_open())
      {
        stream << DALI_BLEND_EQUATION_ADVANCED_SUPPORT << " " << mIsAdvancedBlendEquationSupported << std::endl;
      }
      else
      {
        DALI_LOG_ERROR("Fail to open file : %s\n", mSystemCacheFilePath.c_str());
      }
    }
  }

  return mIsAdvancedBlendEquationSupported;
}

bool ConfigurationManager::IsMultisampledRenderToTextureSupported()
{
  if(!mIsMultisampledRenderToTextureSupportedCached)
  {
    RetrieveKeysFromConfigFile(mSystemCacheFilePath);

    if(!mIsMultisampledRenderToTextureSupportedCached)
    {
      if(!mGraphics->IsInitialized())
      {
        // Wait until graphics subsystem is initialised, but this will happen once per factory reset.
        // This method blocks until the render thread has initialised the graphics.
        mThreadController->WaitForGraphicsInitialization();
      }

      // Query from Graphics Subsystem and save the cache
      mIsMultisampledRenderToTextureSupported       = mGraphics->IsMultisampledRenderToTextureSupported();
      mIsMultisampledRenderToTextureSupportedCached = true;

      Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
      std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
      if(stream.is_open())
      {
        stream << DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT << " " << mIsMultisampledRenderToTextureSupported << std::endl;
      }
      else
      {
        DALI_LOG_ERROR("Fail to open file : %s\n", mSystemCacheFilePath.c_str());
      }
    }
  }

  return mIsMultisampledRenderToTextureSupported;
}

std::string ConfigurationManager::GetCurrentGraphicsBackendString() const
{
  Graphics::Backend currentBackend = Graphics::GetCurrentGraphicsBackend();

  switch(currentBackend)
  {
    case Graphics::Backend::GLES:
      return "GLES";
    case Graphics::Backend::VULKAN:
      return "VULKAN";
    default:
      return "GLES"; // Default fallback
  }
}

void ConfigurationManager::CheckAndHandleBackendSwitch()
{
  std::string currentBackend = GetCurrentGraphicsBackendString();

  // If we have a cached backend and it's different from current, clear the cache
  if(!mCachedGraphicsBackend.empty() && mCachedGraphicsBackend != currentBackend)
  {
    DALI_LOG_WARNING("Graphics backend switched from %s to %s, clearing configuration cache\n",
                     mCachedGraphicsBackend.c_str(),
                     currentBackend.c_str());
    ClearConfigurationCache();

    // Save the new graphics backend type to the configuration file
    SaveCurrentGraphicsBackend();
  }
  else if(mCachedGraphicsBackend.empty())
  {
    // If no cached backend exists (first run), save the current backend type
    SaveCurrentGraphicsBackend();
  }

  // Update the cached backend type
  mCachedGraphicsBackend = currentBackend;
}

void ConfigurationManager::ClearConfigurationCache()
{
  // Reset all cached flags
  mMaxTextureSizeCached                         = false;
  mMaxCombinedTextureUnitsCached                = false;
  mShaderLanguageVersionCached                  = false;
  mIsMultipleWindowSupportedCached              = false;
  mIsAdvancedBlendEquationSupportedCached       = false;
  mIsMultisampledRenderToTextureSupportedCached = false;

  // Reset cached values to defaults
  mMaxTextureSize                         = 0u;
  mMaxCombinedTextureUnits                = 0u;
  mShaderLanguageVersion                  = 0u;
  mIsMultipleWindowSupported              = true;
  mIsAdvancedBlendEquationSupported       = true;
  mIsMultisampledRenderToTextureSupported = true;

  // Delete the cache file
  if(std::remove(mSystemCacheFilePath.c_str()) != 0)
  {
    DALI_LOG_WARNING("Failed to remove cache file: %s\n", mSystemCacheFilePath.c_str());
  }
}

void ConfigurationManager::SaveCurrentGraphicsBackend()
{
  std::string currentBackend = GetCurrentGraphicsBackendString();

  Dali::FileStream configFile(mSystemCacheFilePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
  std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
  if(stream.is_open())
  {
    stream << DALI_GRAPHICS_BACKEND_TYPE << " " << currentBackend << std::endl;
  }
  else
  {
    DALI_LOG_ERROR("Fail to open file for saving graphics backend: %s\n", mSystemCacheFilePath.c_str());
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
