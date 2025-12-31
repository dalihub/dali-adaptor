/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/system-error-print.h>
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
const char* DALI_ENV_MULTIPLE_WINDOW_SUPPORT            = "DALI_ENV_MULTIPLE_WINDOW_SUPPORT";
const char* DALI_BLEND_EQUATION_ADVANCED_SUPPORT        = "DALI_BLEND_EQUATION_ADVANCED_SUPPORT";
const char* DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT = "DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT";
const char* DALI_GLSL_VERSION                           = "DALI_GLSL_VERSION";

const char* DALI_GRAPHICS_BACKEND_NAME_GLES   = "GLES";
const char* DALI_GRAPHICS_BACKEND_NAME_VULKAN = "VULKAN";

const char* SYSTEM_CACHE_FILE_GLES   = "gpu-environment-gles.conf";
const char* SYSTEM_CACHE_FILE_VULKAN = "gpu-environment-vulkan.conf";

/**
 * @brief Get the current graphics backend type as a string
 * @return The graphics backend type as string ("GLES" or "VULKAN")
 */
inline std::string GetCurrentGraphicsBackendString()
{
  Graphics::Backend currentBackend = Graphics::GetCurrentGraphicsBackend();

  switch(currentBackend)
  {
    case Graphics::Backend::GLES:
      return DALI_GRAPHICS_BACKEND_NAME_GLES;
    case Graphics::Backend::VULKAN:
      return DALI_GRAPHICS_BACKEND_NAME_VULKAN;
    default:
      return DALI_GRAPHICS_BACKEND_NAME_GLES; // Default fallback
  }
}

/**
 * @brief Get the system cache file name as current graphics backend type.
 * @return The system cache file name as graphics backend type
 */
inline std::string GetCurrentSystemCacheFileName()
{
  Graphics::Backend currentBackend = Graphics::GetCurrentGraphicsBackend();

  switch(currentBackend)
  {
    case Graphics::Backend::GLES:
      return SYSTEM_CACHE_FILE_GLES;
    case Graphics::Backend::VULKAN:
      return SYSTEM_CACHE_FILE_VULKAN;
    default:
      return SYSTEM_CACHE_FILE_GLES; // Default fallback
  }
}

template<typename KeyType, typename ValueType>
void AppendCacheFile(const std::string& filePath, const KeyType& key, const ValueType& value)
{
  Dali::FileStream configFile(filePath, Dali::FileStream::READ | Dali::FileStream::APPEND | Dali::FileStream::TEXT);
  std::fstream&    stream = dynamic_cast<std::fstream&>(configFile.GetStream());
  if(DALI_LIKELY(stream.is_open()))
  {
    stream << key << " " << value << std::endl;
  }
  else
  {
    DALI_LOG_ERROR("Fail to write key[%s]. file open failed : %s\n", key, filePath.c_str());
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }
}

} // unnamed namespace

ConfigurationManager::ConfigurationManager(std::string systemCachePath, Graphics::GraphicsInterface* graphics, ThreadController* threadController)
: mSystemCacheFilePath(systemCachePath + GetCurrentSystemCacheFileName()),
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
  mMaxCombinedTextureUnitsCached(false),
  mEnabled(true)
{
  CheckAndHandleBackendSwitch();

  // First, read the cached graphics backend from the config file
  RetrieveKeysFromConfigFile(mSystemCacheFilePath);
}

ConfigurationManager::~ConfigurationManager()
{
}

void ConfigurationManager::ChangeGraphics(Graphics::GraphicsInterface* graphics)
{
  mGraphics = graphics;

  CheckAndHandleBackendSwitch();
}

void ConfigurationManager::RetrieveKeysFromConfigFile(const std::string& configFilePath)
{
  if(DALI_UNLIKELY(!mEnabled))
  {
    return;
  }

  std::vector<std::string> lines;

  // Copy whole string first and close file.
  {
    Dali::FileStream configFile(configFilePath, Dali::FileStream::READ | Dali::FileStream::TEXT);
    std::iostream&   stream = configFile.GetStream();
    if(DALI_LIKELY(stream.rdbuf()->in_avail()))
    {
      std::string line;
      while(std::getline(stream, line))
      {
        lines.emplace_back(line);
      }
    }
    else
    {
      DALI_LOG_ERROR("Fail to read file : %s\n", configFilePath.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }
  }

  try
  {
    for(auto& line : lines)
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
        mMaxTextureSize       = std::stoi(value);
        mMaxTextureSizeCached = true;
      }
      if(!mMaxCombinedTextureUnitsCached && name == DALI_ENV_MAX_COMBINED_TEXTURE_UNITS)
      {
        std::getline(subStream, value);
        mMaxCombinedTextureUnits       = std::stoi(value);
        mMaxCombinedTextureUnitsCached = true;
      }
      else if(!mIsAdvancedBlendEquationSupportedCached && name == DALI_BLEND_EQUATION_ADVANCED_SUPPORT)
      {
        std::getline(subStream, value);
        mIsAdvancedBlendEquationSupported       = std::stoi(value);
        mIsAdvancedBlendEquationSupportedCached = true;
      }
      else if(!mIsMultisampledRenderToTextureSupportedCached && name == DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT)
      {
        std::getline(subStream, value);
        mIsMultisampledRenderToTextureSupported       = std::stoi(value);
        mIsMultisampledRenderToTextureSupportedCached = true;
      }
      else if(!mShaderLanguageVersionCached && name == DALI_GLSL_VERSION)
      {
        std::getline(subStream, value);
        mShaderLanguageVersion       = std::stoi(value);
        mShaderLanguageVersionCached = true;
      }
      else if(!mIsMultipleWindowSupportedCached && name == DALI_ENV_MULTIPLE_WINDOW_SUPPORT)
      {
        std::getline(subStream, value);
        mIsMultipleWindowSupported       = std::stoi(value);
        mIsMultipleWindowSupportedCached = true;
      }
    }
  }
  catch(std::invalid_argument const& ex)
  {
    DALI_LOG_ERROR("std::invalid_argument! Please check the cache file. [%s]\n", ex.what());
  }
  catch(std::out_of_range const& ex)
  {
    DALI_LOG_ERROR("std::out_of_range! Please check the cache file. [%s]\n", ex.what());
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
      DALI_LOG_RENDER_INFO("MaxTextureSize = %d\n", mMaxTextureSize);

      if(DALI_LIKELY(mEnabled))
      {
        AppendCacheFile(mSystemCacheFilePath, DALI_ENV_MAX_TEXTURE_SIZE, mMaxTextureSize);
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

      if(DALI_LIKELY(mEnabled))
      {
        AppendCacheFile(mSystemCacheFilePath, DALI_ENV_MAX_COMBINED_TEXTURE_UNITS, mMaxCombinedTextureUnits);
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
      DALI_LOG_RENDER_INFO("ShaderLanguageVersion = %d\n", mShaderLanguageVersion);

      if(DALI_LIKELY(mEnabled))
      {
        AppendCacheFile(mSystemCacheFilePath, DALI_GLSL_VERSION, mShaderLanguageVersion);
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
      DALI_LOG_RENDER_INFO("IsMultipleWindowSupported = %d\n", mIsMultipleWindowSupported);

      if(DALI_LIKELY(mEnabled))
      {
        AppendCacheFile(mSystemCacheFilePath, DALI_ENV_MULTIPLE_WINDOW_SUPPORT, mIsMultipleWindowSupported);
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
      DALI_LOG_RENDER_INFO("IsAdvancedBlendEquationSupported = %d\n", mIsAdvancedBlendEquationSupported);

      if(DALI_LIKELY(mEnabled))
      {
        AppendCacheFile(mSystemCacheFilePath, DALI_BLEND_EQUATION_ADVANCED_SUPPORT, mIsAdvancedBlendEquationSupported);
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
      DALI_LOG_RENDER_INFO("IsMultisampledRenderToTextureSupported = %d\n", mIsMultisampledRenderToTextureSupported);

      if(DALI_LIKELY(mEnabled))
      {
        AppendCacheFile(mSystemCacheFilePath, DALI_MULTISAMPLED_RENDER_TO_TEXTURE_SUPPORT, mIsMultisampledRenderToTextureSupported);
      }
    }
  }

  return mIsMultisampledRenderToTextureSupported;
}

void ConfigurationManager::CheckAndHandleBackendSwitch()
{
  std::string currentBackend = GetCurrentGraphicsBackendString();

  // If we have a cached backend and it's different from current, clear the cache
  if(!mCachedGraphicsBackend.empty() && mCachedGraphicsBackend != currentBackend)
  {
    DALI_LOG_RELEASE_INFO("Graphics backend switched from %s to %s, invalidate current configuration cache\n",
                          mCachedGraphicsBackend.c_str(),
                          currentBackend.c_str());
    ClearConfigurationCache();

    // Let we don't use configure file if graphics backend changed during runtime.
    mEnabled = false;
  }

  // Update the cached backend type
  mCachedGraphicsBackend = currentBackend;
  DALI_LOG_RENDER_INFO("GraphicsBackend = %s\n", mCachedGraphicsBackend.c_str());
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
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
