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
#include <dali/internal/system/common/remote-file-download-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/async-task-manager.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <mutex>
#include <utility>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

#ifdef DEBUG_ENABLED
Debug::Filter* gRemoteFileDownloadManagerLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_REMOTE_FILE_DOWNLOAD_MANAGER");
#endif

void RemoteFileDownloadStartTaskComplete(Dali::AsyncTaskPtr)
{
}

CallbackBase* CreateRemoteFileDownloadStartTaskCompleteCallback()
{
  return MakeCallback(&RemoteFileDownloadStartTaskComplete);
}

class RemoteFileDownloadStartTask : public Dali::AsyncTask
{
public:
  RemoteFileDownloadStartTask(Dali::IntrusivePtr<RemoteFileDownloadManager>       manager,
                              RemoteFileDownloadManager::RequestId                requestId,
                              std::string                                         url,
                              size_t                                              maxSize)
  : Dali::AsyncTask(CreateRemoteFileDownloadStartTaskCompleteCallback(), Dali::AsyncTask::PriorityType::LOW, Dali::AsyncTask::ThreadType::WORKER_THREAD),
    mManager(std::move(manager)),
    mRequestId(requestId),
    mUrl(std::move(url)),
    mMaxSize(maxSize)
  {
  }

  void Process() override
  {
    if(DALI_LIKELY(mManager))
    {
      mManager->RequestProviderDownload(mRequestId, mUrl, mMaxSize);
    }
  }

  Dali::StringView GetTaskName() const override
  {
    return "RemoteFileDownloadStartTask";
  }

private:
  Dali::IntrusivePtr<RemoteFileDownloadManager> mManager;
  RemoteFileDownloadManager::RequestId          mRequestId;
  std::string                                   mUrl;
  size_t                                        mMaxSize;
};

Dali::RemoteFileDownloadManager RemoteFileDownloadManager::Get()
{
  Dali::RemoteFileDownloadManager manager;

  Dali::SingletonService service(Dali::SingletonService::Get());
  if(DALI_LIKELY(service))
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::RemoteFileDownloadManager));
    if(handle)
    {
      manager = Dali::RemoteFileDownloadManager(static_cast<RemoteFileDownloadManager*>(handle.GetObjectPtr()));
    }
    else
    {
      manager = Dali::RemoteFileDownloadManager(new RemoteFileDownloadManager());
      service.Register(typeid(Dali::RemoteFileDownloadManager), manager);
    }
  }
  return manager;
}

bool RemoteFileDownloadManager::IsAsyncDownloadSupported()
{
  static bool           sAvailable = false;
  static std::once_flag sOnce;
  std::call_once(sOnce, [&]() {
    sAvailable = Dali::FileDownloadPluginProxy::IsAsyncDownloadSupported();
    DALI_LOG_RELEASE_INFO("[FileDownload][RemoteManager] async download %s\n", sAvailable ? "ENABLED (download-api path)" : "DISABLED (direct LoadingTask path)");
  });
  return sAvailable;
}

RemoteFileDownloadManager::~RemoteFileDownloadManager()
{
  {
    std::scoped_lock lock(mRequestInfosMutex);
    mShuttingDown = true;
  }

  CancelAllDownloadsInternal();
}

Dali::RemoteFileDownloadManager::RequestId RemoteFileDownloadManager::StartDownload(const std::string&                                   url,
                                                                                    size_t                                               maxSize,
                                                                                    Dali::RemoteFileDownloadManager::CompletionCallback callback)
{
  DALI_LOG_INFO(gRemoteFileDownloadManagerLogFilter, Debug::Verbose, "[FileDownload][RemoteManager] StartDownload url=%s\n", url.c_str());

  Dali::IntrusivePtr<RemoteFileDownloadManager> selfPtr(this);
  RequestId                                    requestId = ++mNextRequestId;
  if(DALI_UNLIKELY(requestId == Dali::RemoteFileDownloadManager::INVALID_REQUEST_ID))
  {
    requestId = ++mNextRequestId;
  }

  {
    std::scoped_lock lock(mRequestInfosMutex);
    mRequestInfos[requestId] = {Dali::FileDownloadPluginProxy::INVALID_DOWNLOAD_ID, std::move(callback)};
  }

  Dali::AsyncTaskManager::Get().AddTask(new RemoteFileDownloadStartTask(selfPtr, requestId, url, maxSize));
  return requestId;
}

void RemoteFileDownloadManager::RequestProviderDownload(RequestId          requestId,
                                                        const std::string& url,
                                                        size_t             maxSize)
{
  Dali::IntrusivePtr<RemoteFileDownloadManager> selfPtr(this);

  {
    std::scoped_lock lock(mRequestInfosMutex);
    auto             requestInfoIter = mRequestInfos.find(requestId);
    if(requestInfoIter == mRequestInfos.end())
    {
      DALI_LOG_INFO(gRemoteFileDownloadManagerLogFilter, Debug::Verbose, "[FileDownload][RemoteManager] skip cancelled start requestId=%llu\n", static_cast<unsigned long long>(requestId));
      return;
    }
  }

  auto pluginCallback = [this, selfPtr, requestId](bool success, std::string filePath) {
    NotifyCompletion(requestId, success, std::move(filePath));
  };

  Dali::FileDownloadPluginProxy::DownloadId downloadId = Dali::FileDownloadPluginProxy::StartAsyncDownload(url, maxSize, std::move(pluginCallback));

  if(DALI_UNLIKELY(downloadId == Dali::FileDownloadPluginProxy::INVALID_DOWNLOAD_ID))
  {
    DALI_LOG_ERROR("[FileDownload][RemoteManager] StartAsyncDownload() failed for url[%s]\n", url.c_str());
    NotifyCompletion(requestId, false, {});
    return;
  }

  bool shouldCancelStartedDownload = false;
  {
    std::scoped_lock lock(mRequestInfosMutex);
    auto             requestInfoIter = mRequestInfos.find(requestId);
    if(requestInfoIter != mRequestInfos.end())
    {
      requestInfoIter->second.downloadId = downloadId;
    }
    else
    {
      shouldCancelStartedDownload = true;
    }
  }

  if(shouldCancelStartedDownload)
  {
    DALI_LOG_INFO(gRemoteFileDownloadManagerLogFilter, Debug::Verbose, "[FileDownload][RemoteManager] cancel stale started download requestId=%llu downloadId=%d\n", static_cast<unsigned long long>(requestId), downloadId);
    Dali::FileDownloadPluginProxy::CancelAsyncDownload(downloadId);
  }
}

void RemoteFileDownloadManager::NotifyCompletion(RequestId   requestId,
                                                 bool        success,
                                                 std::string filePath)
{
  RequestInfo completedRequestInfo;
  {
    std::scoped_lock lock(mRequestInfosMutex);
    if(mShuttingDown)
    {
      DALI_LOG_ERROR("[FileDownload][RemoteManager] Ignore completion during shutdown requestId[%llu]\n", static_cast<unsigned long long>(requestId));
      return;
    }

    auto             requestInfoIter = mRequestInfos.find(requestId);
    if(requestInfoIter == mRequestInfos.end())
    {
      return;
    }

    completedRequestInfo = std::move(requestInfoIter->second);
    mRequestInfos.erase(requestInfoIter);
  }

  DALI_LOG_INFO(gRemoteFileDownloadManagerLogFilter, Debug::Verbose, "[FileDownload][RemoteManager] requestId=%llu completion success=%d filePath=%s\n", static_cast<unsigned long long>(requestId), success, filePath.c_str());

  if(DALI_LIKELY(completedRequestInfo.callback))
  {
    completedRequestInfo.callback(success, filePath);
  }
}

void RemoteFileDownloadManager::CancelDownload(Dali::RemoteFileDownloadManager::RequestId requestId)
{
  RequestInfo cancelledRequestInfo;
  {
    std::scoped_lock lock(mRequestInfosMutex);
    auto             requestInfoIter = mRequestInfos.find(requestId);
    if(requestInfoIter == mRequestInfos.end())
    {
      return;
    }

    DALI_LOG_INFO(gRemoteFileDownloadManagerLogFilter, Debug::Verbose, "[FileDownload][RemoteManager] CancelDownload requestId=%llu downloadId=%d\n", static_cast<unsigned long long>(requestId), requestInfoIter->second.downloadId);

    cancelledRequestInfo = std::move(requestInfoIter->second);
    mRequestInfos.erase(requestInfoIter);
  }

  Dali::FileDownloadPluginProxy::CancelAsyncDownload(cancelledRequestInfo.downloadId);
  if(cancelledRequestInfo.callback)
  {
    cancelledRequestInfo.callback(false, {});
  }
}

void RemoteFileDownloadManager::CancelAllDownloadsInternal()
{
  RequestInfoContainer requestInfos;
  {
    std::scoped_lock lock(mRequestInfosMutex);
    requestInfos = std::move(mRequestInfos);
    mRequestInfos.clear();
  }

  for(auto& requestInfo : requestInfos)
  {
    Dali::FileDownloadPluginProxy::CancelAsyncDownload(requestInfo.second.downloadId);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
