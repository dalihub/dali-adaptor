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

// HEADER
#include "file-download-download-api.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/semaphore.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>

namespace
{
constexpr Dali::Plugin::DownloadApiFileDownloader::DownloadId INVALID_DOWNLOAD_ID = -1;
}

// Export symbols looked up by FileDownloadPluginProxy via dlsym

extern "C" DALI_ADAPTOR_API Dali::FileDownloadPlugin* CreateFileDownloadPlugin()
{
  return new Dali::Plugin::DownloadApiFileDownloader();
}

extern "C" DALI_ADAPTOR_API bool InitializeFileDownloadPlugin(Dali::FileDownloadPlugin* plugin)
{
  if(DALI_LIKELY(plugin))
  {
    return plugin->InitializePlugin();
  }
  return false;
}

extern "C" DALI_ADAPTOR_API void DestroyFileDownloadPlugin(Dali::FileDownloadPlugin* plugin)
{
  delete plugin;
}

// Capability probe looked up via dlsym by FileDownloadPluginProxy::IsAsyncDownloadSupported()
// to avoid vtable dispatch crashes on older plugin SO versions.
extern "C" DALI_ADAPTOR_API bool FileDownloadPluginIsAsyncDownloadSupported()
{
  return true;
}

extern "C" DALI_ADAPTOR_API Dali::Plugin::DownloadApiFileDownloader::DownloadId FileDownloadPluginStartAsyncDownload(Dali::FileDownloadPlugin* plugin,
                                                                                                                      const char*               url,
                                                                                                                      size_t                    maxSize,
                                                                                                                      Dali::Plugin::DownloadApiFileDownloader::AsyncCompletionCallback callback,
                                                                                                                      void* userData)
{
  if(DALI_LIKELY(plugin))
  {
    return static_cast<Dali::Plugin::DownloadApiFileDownloader*>(plugin)->StartAsyncDownload(url, maxSize, callback, userData);
  }
  return INVALID_DOWNLOAD_ID;
}

extern "C" DALI_ADAPTOR_API void FileDownloadPluginCancelAsyncDownload(Dali::FileDownloadPlugin* plugin,
                                                                        Dali::Plugin::DownloadApiFileDownloader::DownloadId downloadId)
{
  if(DALI_LIKELY(plugin))
  {
    static_cast<Dali::Plugin::DownloadApiFileDownloader*>(plugin)->CancelAsyncDownload(downloadId);
  }
}

// Implementation

namespace Dali::Plugin
{
namespace
{
#ifdef DEBUG_ENABLED
Debug::Filter* gDownloadApiLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_DOWNLOAD_API");
#endif

struct SyncDownloadContext
{
  SyncDownloadContext()
  : semaphore(0)
  {
  }

  Dali::Semaphore<1> semaphore;
  download_state_e   state{DOWNLOAD_STATE_NONE};
  bool               success{false};
};

void OnSyncStateChanged(int id, download_state_e state, void* userData)
{
  (void)id;

  auto* context = static_cast<SyncDownloadContext*>(userData);
  switch(state)
  {
    case DOWNLOAD_STATE_COMPLETED:
      context->success = true;
      context->state   = state;
      context->semaphore.Release();
      break;
    case DOWNLOAD_STATE_FAILED:
    case DOWNLOAD_STATE_CANCELED:
      context->success = false;
      context->state   = state;
      context->semaphore.Release();
      break;
    default:
      break;
  }
}

void DestroyDownloadWithStateCallback(int downloadId)
{
  download_unset_state_changed_cb(downloadId);
  download_destroy(downloadId);
}

bool ReadFileIntoBuffer(const char*            filePath,
                        Dali::Vector<uint8_t>& dataBuffer,
                        size_t&                dataSize,
                        size_t                 maximumAllowedSizeBytes)
{
  struct stat fileStat;
  if(stat(filePath, &fileStat) != 0 || fileStat.st_size <= 0)
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI] stat() failed or empty file: %s\n", filePath);
    return false;
  }

  const size_t fileSize = static_cast<size_t>(fileStat.st_size);
  if(fileSize > maximumAllowedSizeBytes)
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI] file size %zu exceeds limit %zu: %s\n", fileSize, maximumAllowedSizeBytes, filePath);
    return false;
  }

  FILE* fp = fopen(filePath, "rb");
  if(!fp)
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI] fopen() failed: %s\n", filePath);
    return false;
  }

  dataBuffer.Resize(fileSize);
  const size_t bytesRead = fread(dataBuffer.Begin(), 1u, fileSize, fp);
  fclose(fp);

  if(bytesRead != fileSize)
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI] fread incomplete (%zu/%zu): %s\n", bytesRead, fileSize, filePath);
    dataBuffer.Clear();
    return false;
  }

  dataSize = fileSize;
  return true;
}

bool IsDownloadedFileWithinLimit(const std::string& filePath,
                                 size_t             maximumAllowedSizeBytes)
{
  struct stat fileStat;
  if(stat(filePath.c_str(), &fileStat) != 0 || fileStat.st_size <= 0)
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI] stat() failed or empty file: %s\n", filePath.c_str());
    return false;
  }

  const size_t fileSize = static_cast<size_t>(fileStat.st_size);
  if(fileSize > maximumAllowedSizeBytes)
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI] file size %zu exceeds limit %zu: %s\n", fileSize, maximumAllowedSizeBytes, filePath.c_str());
    return false;
  }

  return true;
}

} // unnamed namespace

bool DownloadApiFileDownloader::InitializePlugin()
{
  // download-provider self-initialises on first download_create(); no explicit init needed.
  return true;
}

// Sync fallback (Worker Thread)

bool DownloadApiFileDownloader::DownloadRemoteFileIntoMemory(
  const std::string&     url,
  Dali::Vector<uint8_t>& dataBuffer,
  size_t&                dataSize,
  size_t                 maximumAllowedSizeBytes)
{
  if(DALI_UNLIKELY(url.empty()))
  {
    return false;
  }

  DALI_LOG_INFO(gDownloadApiLogFilter, Debug::Verbose, "[FileDownload][DownloadAPI][sync] start url[%s]\n", url.c_str());

  // Per-download state on the stack. The sync path waits for a terminal state
  // before destroying the provider handle and returning.
  SyncDownloadContext context;

  int downloadId = INVALID_DOWNLOAD_ID;
  int ret        = download_create(&downloadId);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][sync] download_create() failed[0x%x] url[%s]\n", static_cast<unsigned int>(ret), url.c_str());
    return false;
  }

  if(DALI_UNLIKELY(download_set_url(downloadId, url.c_str()) != DOWNLOAD_ERROR_NONE))
  {
    download_destroy(downloadId);
    return false;
  }

  ret = download_set_cache(downloadId, true);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][DownloadAPI][sync] download_set_cache(true) failed[0x%x] id[%d] url[%s], continue\n", static_cast<unsigned int>(ret), downloadId, url.c_str());
  }

  if(DALI_UNLIKELY(download_set_state_changed_cb(downloadId, OnSyncStateChanged, &context) != DOWNLOAD_ERROR_NONE))
  {
    download_destroy(downloadId);
    return false;
  }

  ret = download_start(downloadId);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][sync] download_start() failed[0x%x] url[%s]\n", static_cast<unsigned int>(ret), url.c_str());
    DestroyDownloadWithStateCallback(downloadId);
    return false;
  }

  context.semaphore.Acquire();
  download_unset_state_changed_cb(downloadId);

  if(DALI_UNLIKELY(!context.success))
  {
    download_error_e error = DOWNLOAD_ERROR_NONE;
    download_get_error(downloadId, &error);
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][sync] FAILED url[%s] state[%d] error[%d]\n", url.c_str(), static_cast<int>(context.state), static_cast<int>(error));
    download_destroy(downloadId);
    return false;
  }

  char* downloadedFilePath = nullptr;
  ret                      = download_get_downloaded_file_path(downloadId, &downloadedFilePath);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE || !downloadedFilePath))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][sync] download_get_downloaded_file_path() failed: %d\n", ret);
    download_destroy(downloadId);
    return false;
  }

  const bool result = ReadFileIntoBuffer(downloadedFilePath, dataBuffer, dataSize, maximumAllowedSizeBytes);
  free(downloadedFilePath);
  download_destroy(downloadId);

  DALI_LOG_INFO(gDownloadApiLogFilter, Debug::Verbose, "[FileDownload][DownloadAPI][sync] %s url[%s] size[%zu]\n", result ? "done" : "FAILED", url.c_str(), dataSize);
  return result;
}

// Async path

DownloadApiFileDownloader::DownloadId DownloadApiFileDownloader::StartAsyncDownload(const char*             url,
                                                                                    size_t                  maxSize,
                                                                                    AsyncCompletionCallback callback,
                                                                                    void*                   userData)
{
  if(DALI_UNLIKELY(!url || *url == '\0' || !callback))
  {
    return INVALID_DOWNLOAD_ID;
  }

  DALI_LOG_INFO(gDownloadApiLogFilter, Debug::Verbose, "[FileDownload][DownloadAPI][async] start url[%s]\n", url);

  int rawDownloadId = INVALID_DOWNLOAD_ID;
  int ret           = download_create(&rawDownloadId);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][async] download_create() FAILED ret[0x%x] url[%s]\n", static_cast<unsigned int>(ret), url);
    return INVALID_DOWNLOAD_ID;
  }
  const DownloadId downloadId = static_cast<DownloadId>(rawDownloadId);

  ret = download_set_url(downloadId, url);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][async] download_set_url() FAILED ret[0x%x] id[%d]\n", static_cast<unsigned int>(ret), downloadId);
    download_destroy(downloadId);
    return INVALID_DOWNLOAD_ID;
  }

  ret = download_set_cache(downloadId, true);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][DownloadAPI][async] download_set_cache(true) failed[0x%x] id[%d] url[%s], continue\n", static_cast<unsigned int>(ret), downloadId, url);
  }

  // Store callback+maxSize before registering the state callback to avoid a race
  // where OnStateChanged fires before we've stored the callback.
  {
    std::scoped_lock lock(mMutex);
    mActiveDownloads[downloadId] = {callback, userData, maxSize};
  }

  ret = download_set_state_changed_cb(downloadId, OnStateChanged, this);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][async] download_set_state_changed_cb() FAILED ret[0x%x] id[%d]\n", static_cast<unsigned int>(ret), downloadId);
    {
      std::scoped_lock lock(mMutex);
      mActiveDownloads.erase(downloadId);
    }
    download_destroy(downloadId);
    return INVALID_DOWNLOAD_ID;
  }

  ret = download_start(downloadId);
  if(DALI_UNLIKELY(ret != DOWNLOAD_ERROR_NONE))
  {
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][async] download_start() FAILED ret[0x%x] id[%d] url[%s]\n", static_cast<unsigned int>(ret), downloadId, url);
    {
      std::scoped_lock lock(mMutex);
      mActiveDownloads.erase(downloadId);
    }
    DestroyDownloadWithStateCallback(downloadId);
    return INVALID_DOWNLOAD_ID;
  }

  DALI_LOG_INFO(gDownloadApiLogFilter, Debug::Verbose, "[FileDownload][DownloadAPI][async] started id[%d] url[%s]\n", downloadId, url);
  return downloadId;
}

void DownloadApiFileDownloader::CancelAsyncDownload(DownloadId downloadId)
{
  if(downloadId < 0)
  {
    return;
  }

  DALI_LOG_INFO(gDownloadApiLogFilter, Debug::Verbose, "[FileDownload][DownloadAPI][async] cancel downloadId[%d]\n", downloadId);

  {
    std::scoped_lock lock(mMutex);
    auto             it = mActiveDownloads.find(downloadId);
    if(it == mActiveDownloads.end())
    {
      return;
    }
    mActiveDownloads.erase(it);
  }

  download_unset_state_changed_cb(downloadId);
  download_cancel(downloadId);
  download_destroy(downloadId);
}

// Called from __dp_event_manager thread. Must NOT do heavy work here.
void DownloadApiFileDownloader::OnStateChanged(int id, download_state_e state, void* userData)
{
  if(state != DOWNLOAD_STATE_COMPLETED &&
     state != DOWNLOAD_STATE_FAILED &&
     state != DOWNLOAD_STATE_CANCELED)
  {
    return; // Non-terminal state, ignore.
  }

  auto* self = static_cast<DownloadApiFileDownloader*>(userData);

  AsyncCompletionCallback completionCallback;
  void*                   callbackUserData = nullptr;
  bool                    success = (state == DOWNLOAD_STATE_COMPLETED);
  std::string             filePath;
  size_t                  maxSize = 0u;

  {
    std::scoped_lock lock(self->mMutex);
    auto             it = self->mActiveDownloads.find(id);
    if(it == self->mActiveDownloads.end())
    {
      // Duplicate or late callback. Download already processed and removed.
      return;
    }
    completionCallback = std::move(it->second.callback);
    callbackUserData   = it->second.userData;
    maxSize            = it->second.maxSize;
    self->mActiveDownloads.erase(it);
  }

  if(success)
  {
    char* downloadedFilePath = nullptr;
    if(download_get_downloaded_file_path(id, &downloadedFilePath) == DOWNLOAD_ERROR_NONE && downloadedFilePath)
    {
      filePath = downloadedFilePath;
      free(downloadedFilePath);
      DALI_LOG_INFO(gDownloadApiLogFilter, Debug::Verbose, "[FileDownload][DownloadAPI][async] completed id[%d] filePath[%s]\n", id, filePath.c_str());
      if(!IsDownloadedFileWithinLimit(filePath, maxSize))
      {
        success = false;
        filePath.clear();
      }
    }
    else
    {
      DALI_LOG_ERROR("[FileDownload][DownloadAPI][async] download_get_downloaded_file_path() failed for id[%d]\n", id);
      success = false;
    }
  }
  else
  {
    download_error_e error = DOWNLOAD_ERROR_NONE;
    download_get_error(id, &error);
    DALI_LOG_ERROR("[FileDownload][DownloadAPI][async] FAILED state[%d] error[%d] id[%d]\n", static_cast<int>(state), static_cast<int>(error), id);
  }

  DestroyDownloadWithStateCallback(id);

  // Invoke the lightweight callback provided by RemoteImageDownloadManager.
  // This runs on __dp_event_manager, so it must only queue state and trigger Event Thread.
  if(DALI_LIKELY(completionCallback))
  {
    completionCallback(static_cast<DownloadId>(id), success, filePath.c_str(), callbackUserData);
  }
}

} // namespace Dali::Plugin
