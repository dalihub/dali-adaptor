#ifndef DALI_INTERNAL_REMOTE_FILE_DOWNLOAD_MANAGER_H
#define DALI_INTERNAL_REMOTE_FILE_DOWNLOAD_MANAGER_H

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
#include <dali/integration-api/adaptor-framework/file-download/file-download-plugin-proxy.h>
#include <dali/integration-api/adaptor-framework/file-download/remote-file-download-manager.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * The manager for remote file download.
 */
class RemoteFileDownloadManager : public Dali::BaseObject
{
public:
  /**
   * Constructor.
   */
  RemoteFileDownloadManager() = default;

  /**
   * Destructor.
   */
  ~RemoteFileDownloadManager() override;

  RemoteFileDownloadManager(const RemoteFileDownloadManager&)            = delete;
  RemoteFileDownloadManager& operator=(const RemoteFileDownloadManager&) = delete;

  /**
   * @copydoc Dali::RemoteFileDownloadManager::Get()
   */
  static Dali::RemoteFileDownloadManager Get();

  /**
   * @copydoc Dali::RemoteFileDownloadManager::IsAsyncDownloadSupported()
   */
  static bool IsAsyncDownloadSupported();

  /**
   * @copydoc Dali::RemoteFileDownloadManager::StartDownload()
   */
  Dali::RemoteFileDownloadManager::RequestId StartDownload(const std::string&                                   url,
                                                           size_t                                               maxSize,
                                                           Dali::RemoteFileDownloadManager::CompletionCallback callback);

  /**
   * @copydoc Dali::RemoteFileDownloadManager::CancelDownload()
   */
  void CancelDownload(Dali::RemoteFileDownloadManager::RequestId requestId);

  using RequestId = Dali::RemoteFileDownloadManager::RequestId;

  void RequestProviderDownload(RequestId          requestId,
                               const std::string& url,
                               size_t             maxSize);

private:
  struct RequestInfo
  {
    Dali::FileDownloadPluginProxy::DownloadId           downloadId; ///< Provider/plugin id used for cancel.
    Dali::RemoteFileDownloadManager::CompletionCallback callback;   ///< Caller callback for this request.
  };

  /**
   * One active entry per manager-owned request id.
   */
  using RequestInfoContainer = std::unordered_map<RequestId, RequestInfo>;

  void NotifyCompletion(RequestId   requestId,
                        bool        success,
                        std::string filePath);

  void CancelAllDownloadsInternal();

  RequestInfoContainer mRequestInfos;
  std::mutex           mRequestInfosMutex;

  bool mShuttingDown{false};

  std::atomic<RequestId> mNextRequestId{Dali::RemoteFileDownloadManager::INVALID_REQUEST_ID};
};

} // namespace Adaptor

} // namespace Internal

inline Internal::Adaptor::RemoteFileDownloadManager& GetImplementation(Dali::RemoteFileDownloadManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "RemoteFileDownloadManager is empty");

  Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<Internal::Adaptor::RemoteFileDownloadManager&>(handle);
}

inline const Internal::Adaptor::RemoteFileDownloadManager& GetImplementation(const Dali::RemoteFileDownloadManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "RemoteFileDownloadManager is empty");

  const Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<const Internal::Adaptor::RemoteFileDownloadManager&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_REMOTE_FILE_DOWNLOAD_MANAGER_H
