#ifndef DALI_INTEGRATION_REMOTE_FILE_DOWNLOAD_MANAGER_H
#define DALI_INTEGRATION_REMOTE_FILE_DOWNLOAD_MANAGER_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class RemoteFileDownloadManager;
}
} // namespace Internal DALI_INTERNAL

/**
 * @brief Handle to the process-singleton remote file download manager.
 *
 * Manages non-blocking async downloads for remote URLs.
 */
class DALI_ADAPTOR_API RemoteFileDownloadManager : public Dali::BaseHandle
{
public:
  /**
   * @brief Manager-owned request key used to identify one download request.
   */
  using RequestId = uint64_t;

  /**
   * @brief Invalid request id.
   */
  static constexpr RequestId INVALID_REQUEST_ID = 0u;

  /**
   * @brief Callback invoked on the download completion thread when a download finishes.
   * @param[in] success  True if the file was downloaded successfully.
   * @param[in] filePath Local path to the downloaded file (valid when success == true).
   */
  using CompletionCallback = std::function<void(bool success, const std::string& filePath)>;

  /**
   * @brief Returns (or creates) the singleton instance via SingletonService.
   */
  static RemoteFileDownloadManager Get();

  /**
   * @brief Returns true if the loaded file-download plugin supports async download.
   */
  static bool IsAsyncDownloadSupported();

  /**
   * @brief Start an async download.
   *
   * @param[in] url        Remote URL to download.
   * @param[in] maxSize    Maximum allowed file size in bytes.
   * @param[in] callback   Invoked from the download completion thread on completion/failure.
   * @return Manager-owned request id used for cancellation, or INVALID_REQUEST_ID on failure.
   */
  RequestId StartDownload(const std::string& url,
                          size_t             maxSize,
                          CompletionCallback callback);

  /**
   * @brief Cancel a pending download for the given request key.
   *
   * Safe to call even if no download is in flight for @p requestId.
   *
   * @param[in] requestId The manager-owned request id returned by StartDownload().
   */
  void CancelDownload(RequestId requestId);

public: // Not intended for application developers
  RemoteFileDownloadManager();
  explicit DALI_INTERNAL RemoteFileDownloadManager(Internal::Adaptor::RemoteFileDownloadManager* impl);
};

} // namespace Dali

#endif // DALI_INTEGRATION_REMOTE_FILE_DOWNLOAD_MANAGER_H
