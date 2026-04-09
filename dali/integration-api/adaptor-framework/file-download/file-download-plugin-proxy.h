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

#ifndef DALI_INTEGRATION_FILE_DOWNLOAD_PLUGIN_PROXY_H
#define DALI_INTEGRATION_FILE_DOWNLOAD_PLUGIN_PROXY_H

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <cstdint>
#include <string>

namespace Dali
{
/**
 * @brief A proxy loader for the file download plugin system.
 *
 * This class handles dynamic loading of file download plugins (e.g., curl-based plugin)
 * and provides a static interface that matches the public file-download API.
 *
 * Features:
 * - Lazy loading: Plugin is loaded on first use, not at startup
 * - Graceful degradation: If plugin loading fails, function returns false
 * - Environment variable override: Can specify custom plugin path
 * - Thread-safe lazy initialization
 */
class DALI_ADAPTOR_API FileDownloadPluginProxy
{
public:
  class EventThreadCallbackObserver
  {
  public:
    EventThreadCallbackObserver()          = default;
    virtual ~EventThreadCallbackObserver() = default;

    virtual void OnTriggered() = 0;
  };

public:
  // Call from EventThread - dali-adaptor side

  /**
   * @brief Register EventThreadCallback s.t. download plugin could access.
   */
  static void RegisterEventThreadCallback();

  /**
   * @brief Unregister EventThreadCallback.
   */
  static void UnregisterEventThreadCallback();

  /**
   * Explicitly unload the plugin and cleanup resources.
   *
   * This is normally not required as cleanup happens on process exit.
   * Provided for testing and explicit resource management.
   */
  static void Destroy();

public:
  // Call from WorkerThread - dali-adaptor side

  /**
   * Download a requested file into a memory buffer.
   *
   * Attempts to load and use the registered file download plugin. If the plugin
   * cannot be loaded, returns false and logs appropriate messages.
   *
   * @param[in] url The requested file URL
   * @param[out] dataBuffer A memory buffer object to be written with downloaded file data.
   * @param[out] dataSize The size of the memory buffer.
   * @param[in] maximumAllowedSizeBytes The maximum allowed file size in bytes to download.
   * @return true on success, false on failure (including plugin unavailable)
   */
  static bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                           Dali::Vector<uint8_t>& dataBuffer,
                                           size_t&                dataSize,
                                           size_t                 maximumAllowedSizeBytes);

public:
  // Call from WorkerThread - plugin side

  /**
   * @brief Register event thread callback that plugin could got callback at event thread.
   * @return True if register successed, so we can assume OnTrigger() will be invoked.
   *         False if register failed, so OnTrigger() will not be invoked.
   */
  static bool RegisterEventThreadObserver(EventThreadCallbackObserver& observer);

private:
  FileDownloadPluginProxy() = delete;
};

} // namespace Dali

#endif // DALI_INTEGRATION_FILE_DOWNLOAD_PLUGIN_PROXY_H
