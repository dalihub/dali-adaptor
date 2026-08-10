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
#include <dali/devel-api/adaptor-framework/file-download-plugin.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <cstdint>
#include <functional>
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
  using AsyncCompletionCallback = std::function<void(bool success, std::string filePath)>;
  using DownloadId              = int32_t;

  static constexpr DownloadId INVALID_DOWNLOAD_ID = -1; ///< Invalid async download id.

public: // Event Thread (dali-adaptor side)
  /**
   * @brief Block new downloads, cancel in-flight ones, and wait for them to leave the plugin.
   *
   * Call this while the worker threads that may be downloading are still alive, and before
   * joining them : the point is to shorten that join, not to replace it.
   *
   * A no-op if no plugin was ever loaded. Never unloads the library - see Destroy() for that,
   * which must not run until the worker threads have been joined.
   *
   * @return True if no thread is inside the plugin any more. False if the wait timed out, or if
   * the loaded plugin does not support draining, which is normal for plugins that delegate
   * transfers to an out-of-process downloader.
   */
  static bool Shutdown();

  /**
   * @brief Explicitly unload the plugin and cleanup resources.
   *
   * This is normally not required as cleanup happens on process exit.
   * Provided for testing and explicit resource management.
   */
  static void Destroy();

  /**
   * @brief Returns true if the loaded plugin supports non-blocking async download.
   *
   * @return false when no plugin is loaded or plugin does not support async download.
   */
  static bool IsAsyncDownloadSupported();

  /**
   * @brief Start a non-blocking async download via the loaded plugin.
   *
   * @param[in] url      The URL to download.
   * @param[in] maxSize  Maximum allowed file size in bytes.
   * @param[in] callback Lightweight callback invoked on download completion (from daemon thread).
   * @return A non-negative download id on success, INVALID_DOWNLOAD_ID on failure.
   * @note The plugin must not invoke the completion callback before this function returns.
   */
  static DownloadId StartAsyncDownload(const std::string&      url,
                                       size_t                  maxSize,
                                       AsyncCompletionCallback callback);

  /**
   * @brief Cancel a previously started async download.
   *
   * @param[in] downloadId Id returned by StartAsyncDownload().
   */
  static void CancelAsyncDownload(DownloadId downloadId);

public: // Worker Thread (dali-adaptor side)
  /**
   * @brief Download a requested file into a memory buffer.
   *
   * Attempts to load and use the registered file download plugin. If the plugin
   * cannot be loaded, returns false and logs appropriate messages.
   *
   * @param[in] url The requested file URL
   * @param[out] dataBuffer A memory buffer object to be written with downloaded file data.
   * @param[out] dataSize Populated with the number of bytes downloaded.
   * @param[in] maximumAllowedSizeBytes The maximum allowed file size in bytes to download.
   * @return true on success, false on failure (including plugin unavailable)
   * @note Threading: Can be called from multiple threads after initialization.
   */
  static bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                           Dali::Vector<uint8_t>& dataBuffer,
                                           size_t&                dataSize,
                                           size_t                 maximumAllowedSizeBytes);

private:
  FileDownloadPluginProxy() = delete;
};

} // namespace Dali

#endif // DALI_INTEGRATION_FILE_DOWNLOAD_PLUGIN_PROXY_H
