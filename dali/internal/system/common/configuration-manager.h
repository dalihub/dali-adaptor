#ifndef DALI_INTERNAL_ENVIRONMENT_CONFIGURATION_MANAGER_H
#define DALI_INTERNAL_ENVIRONMENT_CONFIGURATION_MANAGER_H

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

// EXTERNAL INCLUDES
#include <memory>
#include <string>

namespace Dali
{

class FileStream;

namespace Internal
{
namespace Adaptor
{

class EglGraphics;
class ThreadController;

/**
 * This class retrieves and caches the system configuration.
 *
 */
class ConfigurationManager
{
public:

  /**
   * @brief Constructor
   */
  ConfigurationManager( std::string systemCachePath, EglGraphics* eglGraphics, ThreadController* threadController );

  /**
   * @brief Virtual Destructor for interface cleanup
   */
  virtual ~ConfigurationManager();

  /**
   * @brief Retrieve all keys from the config file if the file exists.
   */
  void RetrieveKeysFromConfigFile( const std::string& configFilePath );

  /**
   * @brief Get the maximum texture size.
   * @return The maximum texture size
   */
  unsigned int GetMaxTextureSize();

  /**
   * @brief Check whether multiple window is supported
   * @return Whether multiple window is supported
   */
  bool IsMultipleWindowSupported();

  // Deleted copy constructor.
  ConfigurationManager( const ConfigurationManager& ) = delete;

  // Deleted move constructor.
  ConfigurationManager( const ConfigurationManager&& ) = delete;

  // Deleted assignment operator.
  ConfigurationManager& operator=( const ConfigurationManager& ) = delete;

  // Deleted move assignment operator.
  ConfigurationManager& operator=( const ConfigurationManager&& ) = delete;

private: // Data

  std::string mSystemCacheFilePath;              ///< The path of system cache file
  EglGraphics* mEglGraphics;                     ///< EGL graphics
  ThreadController* mThreadController;           ///< The thread controller
  unsigned int mMaxTextureSize;                  ///< The largest texture that the GL can handle
  bool mIsMultipleWindowSupported:1;             ///< Whether multiple window is supported by the GLES
  bool mMaxTextureSizeCached:1;                  ///< Whether we have checked the maximum texture size
  bool mIsMultipleWindowSupportedCached:1;       ///< Whether we have checked the support of multiple window
};

} // Adaptor
} // Internal
} // Dali

#endif // DALI_INTERNAL_ENVIRONMENT_CONFIGURATION_MANAGER_H
