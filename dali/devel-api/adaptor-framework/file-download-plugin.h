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

#ifndef DALI_FILE_DOWNLOAD_PLUGIN_H
#define DALI_FILE_DOWNLOAD_PLUGIN_H

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <stdint.h>
#include <string>

namespace Dali
{
/**
 * @brief Abstract interface for file download implementations (e.g., curl-based, libfetch, etc.)
 *
 * Plugin implementations must provide a factory function pair that follows this interface.
 * The plugin must export:
 *   extern "C" FileDownloadPlugin* CreateFileDownloadPlugin()
 *   extern "C" bool InitializePluginFunction(Dali::FileDownloadPlugin*)
 *   extern "C" void DestroyFileDownloadPlugin(FileDownloadPlugin* plugin)
 */
class FileDownloadPlugin
{
public:
  /**
   * Virtual destructor for safe polymorphic deletion
   */
  virtual ~FileDownloadPlugin() = default;

  /**
   * @brief Second phase initialization for the plugin.
   *
   * @note Threading: Can be called from multiple threads.
   *
   * @return True if initialize successed, or already initialized. False if initialize failed.
   */
  virtual bool InitializePlugin() = 0;

  /**
   * Download a requested file into a memory buffer.
   *
   * @note Threading: Can be called from multiple threads after initialization.
   *
   * @pre Must ensure that InitializePlugin() return true before call this function.
   *
   * @param[in] url The requested file URL
   * @param[out] dataBuffer A memory buffer object to be written with downloaded file data.
   * @param[out] dataSize The size of the memory buffer.
   * @param[in] maximumAllowedSizeBytes The maximum allowed file size in bytes to download.
   * @return true on success, false on failure
   */
  virtual bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                            Dali::Vector<uint8_t>& dataBuffer,
                                            size_t&                dataSize,
                                            size_t                 maximumAllowedSizeBytes) = 0;
};

} // namespace Dali

#endif // DALI_FILE_DOWNLOAD_PLUGIN_H
