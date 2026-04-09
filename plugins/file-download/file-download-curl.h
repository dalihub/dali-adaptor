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

#ifndef DALI_PLUGIN_CURL_FILE_DOWNLOADER_H
#define DALI_PLUGIN_CURL_FILE_DOWNLOADER_H

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-download-plugin.h>
#include <dali/public-api/common/dali-vector.h>
#include <cstdint>
#include <string>

namespace Dali::Plugin
{
/**
 * @brief curl-based implementation of the FileDownloadPlugin interface
 *
 * This class encapsulates all curl-specific functionality for downloading files.
 * It is designed to be loaded dynamically via dlopen by FileDownloadPluginProxy.
 */
class CurlFileDownloader : public Dali::FileDownloadPlugin
{
public:
  /**
   * Constructor
   */
  CurlFileDownloader();

  /**
   * Destructor - cleans up curl global state
   */
  virtual ~CurlFileDownloader();

  /**
   * @brief Second phase initialize - initializes curl global state
   *
   * @return True if initialize successed, or already initialized. False if initialize failed.
   */
  bool InitializePlugin() override;

  /**
   * @brief Download a requested file into a memory buffer using curl.
   *
   * @param[in] url The requested file URL
   * @param[out] dataBuffer A memory buffer object to be written with downloaded file data.
   * @param[out] dataSize The size of the memory buffer.
   * @param[in] maximumAllowedSizeBytes The maximum allowed file size in bytes to download.
   * @return true on success, false on failure
   */
  bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                    Dali::Vector<uint8_t>& dataBuffer,
                                    size_t&                dataSize,
                                    size_t                 maximumAllowedSizeBytes) override;

private:
  // Non-copyable, non-movable
  CurlFileDownloader(const CurlFileDownloader&)            = delete;
  CurlFileDownloader(CurlFileDownloader&&)                 = delete;
  CurlFileDownloader& operator=(const CurlFileDownloader&) = delete;
  CurlFileDownloader& operator=(CurlFileDownloader&&)      = delete;

private:
  class Impl;
  Impl* mImpl{nullptr};
};

} // namespace Dali::Plugin

#endif // DALI_PLUGIN_CURL_FILE_DOWNLOADER_H
