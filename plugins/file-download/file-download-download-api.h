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

#ifndef DALI_PLUGIN_DOWNLOAD_API_FILE_DOWNLOADER_H
#define DALI_PLUGIN_DOWNLOAD_API_FILE_DOWNLOADER_H

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-download-plugin.h>
#include <dali/public-api/common/dali-vector.h>
#include <download.h>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Dali::Plugin
{
/**
 * @brief download-provider (capi-web-url-download) based implementation of FileDownloadPlugin.
 *
 * Supports both synchronous (DownloadRemoteFileIntoMemory) and non-blocking async
 * download through optional C symbols loaded by FileDownloadPluginProxy.
 */
class DownloadApiFileDownloader : public Dali::FileDownloadPlugin
{
public:
  /**
   * @brief Constructor.
   */
  DownloadApiFileDownloader() = default;

  /**
   * @brief Destructor.
   */
  ~DownloadApiFileDownloader() = default;

  /**
   * @copydoc Dali::FileDownloadPlugin::InitializePlugin()
   */
  bool InitializePlugin() override;

  /**
   * @copydoc Dali::FileDownloadPlugin::DownloadRemoteFileIntoMemory()
   */
  bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                    Dali::Vector<uint8_t>& dataBuffer,
                                    size_t&                dataSize,
                                    size_t                 maximumAllowedSizeBytes) override;

  using DownloadId              = int32_t;
  using AsyncCompletionCallback = void (*)(DownloadId downloadId, bool success, const char* filePath, void* userData);

  /**
   * @brief Start a non-blocking async download.
   */
  DownloadId StartAsyncDownload(const char*             url,
                                size_t                  maxSize,
                                AsyncCompletionCallback callback,
                                void*                   userData);

  /**
   * @brief Cancel a previously started async download.
   */
  void CancelAsyncDownload(DownloadId downloadId);

private:
  // Non-copyable, non-movable
  DownloadApiFileDownloader(const DownloadApiFileDownloader&)            = delete;
  DownloadApiFileDownloader(DownloadApiFileDownloader&&)                 = delete;
  DownloadApiFileDownloader& operator=(const DownloadApiFileDownloader&) = delete;
  DownloadApiFileDownloader& operator=(DownloadApiFileDownloader&&)      = delete;

private:
  struct ActiveEntry
  {
    AsyncCompletionCallback callback;
    void*                   userData;
    size_t                  maxSize;
  };

  // Called from __dp_event_manager thread. Must not do heavy work.
  static void OnStateChanged(int id, download_state_e state, void* userData);

private:
  // downloadId to {callback, maxSize}, guarded by mMutex.
  std::unordered_map<DownloadId, ActiveEntry> mActiveDownloads;
  std::mutex                                  mMutex;
};

} // namespace Dali::Plugin

#endif // DALI_PLUGIN_DOWNLOAD_API_FILE_DOWNLOADER_H
