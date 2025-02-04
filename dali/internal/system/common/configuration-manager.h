#ifndef DALI_INTERNAL_ENVIRONMENT_CONFIGURATION_MANAGER_H
#define DALI_INTERNAL_ENVIRONMENT_CONFIGURATION_MANAGER_H

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

// EXTERNAL INCLUDES
#include <memory>
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class FileStream;

namespace Graphics
{
class GraphicsInterface;
}

namespace Internal::Adaptor
{
class ThreadController;

/**
 * This class retrieves and caches the system configuration.
 * Some of the methods in this class can block system until GL has been initialized,
 * only at the first time the DALi application is launched in the system.
 *
 * Needs exporting as it's called by the Graphics Libraries
 */
class DALI_ADAPTOR_API ConfigurationManager
{
public:
  /**
   * @brief Constructor
   */
  ConfigurationManager(std::string systemCachePath, Dali::Graphics::GraphicsInterface* graphics, ThreadController* threadController);

  /**
   * @brief Virtual Destructor for interface cleanup
   */
  virtual ~ConfigurationManager();

  /**
   * @brief Retrieve all keys from the config file if the file exists.
   */
  void RetrieveKeysFromConfigFile(const std::string& configFilePath);

  /**
   * @brief Get the maximum texture size.
   * @return The maximum texture size
   */
  uint32_t GetMaxTextureSize();

  /**
   * @brief Get the maximum number of combined texture units (across all shaders in program)
   * @return the number of combined texture units
   */
  uint32_t GetMaxCombinedTextureUnits();

  /**
   * @brief Get the shader language version that the system supports
   * @return the shader language version.
   */
  uint32_t GetShadingLanguageVersion();

  /**
   * @brief Check whether multiple window is supported
   * @return Whether multiple window is supported
   */
  bool IsMultipleWindowSupported();

  /**
   * @brief Check whether blend equation advanced (extension) is supported
   * @return Whether blend equation advanced (extension) is supported
   */
  bool IsAdvancedBlendEquationSupported();
  /**
   * @brief Check whether multisampled render to texture (extension) is supported
   * @return Whether multisampled render to texture (extension) is supported
   */
  bool IsMultisampledRenderToTextureSupported();

  // Deleted copy constructor.
  ConfigurationManager(const ConfigurationManager&) = delete;

  // Deleted move constructor.
  ConfigurationManager(const ConfigurationManager&&) = delete;

  // Deleted assignment operator.
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;

  // Deleted move assignment operator.
  ConfigurationManager& operator=(const ConfigurationManager&&) = delete;

private:                                                                                // Data
  std::string                        mSystemCacheFilePath;                              ///< The path of system cache file
  Dali::Graphics::GraphicsInterface* mGraphics;                                         ///< Graphics interface
  ThreadController*                  mThreadController;                                 ///< The thread controller
  unsigned int                       mMaxTextureSize;                                   ///< The largest texture that the GL can handle
  unsigned int                       mMaxCombinedTextureUnits;                          ///< The maximum number of combined texture units
  unsigned int                       mShaderLanguageVersion;                            ///< The shader language version that the system supports.
  bool                               mIsMultipleWindowSupported : 1;                    ///< Whether multiple window is supported by the GLES
  bool                               mIsAdvancedBlendEquationSupported : 1;             ///< Whether blend equation advanced (extension) is supported by the GLES
  bool                               mIsMultisampledRenderToTextureSupported : 1;       ///< Whether multisampled render to texture (extension) is supported by the GLES
  bool                               mMaxTextureSizeCached : 1;                         ///< Whether we have checked the maximum texture size
  bool                               mIsMultipleWindowSupportedCached : 1;              ///< Whether we have checked the support of multiple window
  bool                               mIsAdvancedBlendEquationSupportedCached : 1;       ///< Whether we have checked the support of blend equation advanced (extension)
  bool                               mIsMultisampledRenderToTextureSupportedCached : 1; ///< Whether we have checked the support of multisampled render to texture (extension)
  bool                               mShaderLanguageVersionCached : 1;                  ///< Whether we have checked the shader language version
  bool                               mMaxCombinedTextureUnitsCached : 1;                ///< Whether we have checked the maximum number of combined texture units
};

} // namespace Internal::Adaptor
} // namespace Dali

#endif // DALI_INTERNAL_ENVIRONMENT_CONFIGURATION_MANAGER_H
