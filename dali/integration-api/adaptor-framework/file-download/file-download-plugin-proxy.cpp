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
#include <dali/integration-api/adaptor-framework/file-download/file-download-plugin-proxy.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlfcn.h>
#if defined(DALI_PRIVILEGE_CHECK_AVAILABLE)
#include <cynara-client.h>
#include <cynara-creds-self.h>
#endif
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/file-download-plugin.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-error-print.h>

namespace Dali
{
namespace // unnamed namespace
{
#ifdef DEBUG_ENABLED
Debug::Filter* gFileDownloadProxyLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_FILE_DOWNLOAD_PROXY");
#endif

// Plugin library relative codes.

// Forward declarations
typedef Dali::FileDownloadPlugin* (*CreatePluginFunction)();
typedef bool (*InitializePluginFunction)(Dali::FileDownloadPlugin*);
typedef void (*DestroyPluginFunction)(Dali::FileDownloadPlugin*);
typedef bool (*IsAsyncDownloadSupportedFunction)();
typedef void (*AsyncCompletionCallbackFunction)(FileDownloadPluginProxy::DownloadId, bool, const char*, void*);
typedef FileDownloadPluginProxy::DownloadId (*StartAsyncDownloadFunction)(Dali::FileDownloadPlugin*, const char*, size_t, AsyncCompletionCallbackFunction, void*);
typedef void (*CancelAsyncDownloadFunction)(Dali::FileDownloadPlugin*, FileDownloadPluginProxy::DownloadId);

struct AsyncDownloadRequest
{
  explicit AsyncDownloadRequest(FileDownloadPluginProxy::AsyncCompletionCallback&& callback)
  : callback(std::move(callback))
  {
  }

  FileDownloadPluginProxy::AsyncCompletionCallback callback;
  std::string                                      filePath;
  bool                                             completed{false};
  bool                                             success{false};
};

struct ActiveAsyncDownload
{
  FileDownloadPluginProxy::DownloadId   downloadId;
  std::unique_ptr<AsyncDownloadRequest> request;
};

// Plugin state variables
void*                            gPluginLibraryHandle             = nullptr;
Dali::FileDownloadPlugin*        gFileDownloadPlugin              = nullptr;
CreatePluginFunction             gCreatePluginFunc                = nullptr;
InitializePluginFunction         gInitializePluginFunc            = nullptr;
DestroyPluginFunction            gDestroyPluginFunc               = nullptr;
IsAsyncDownloadSupportedFunction gIsAsyncDownloadSupportedFunc    = nullptr;
StartAsyncDownloadFunction       gStartAsyncDownloadFunc          = nullptr;
CancelAsyncDownloadFunction      gCancelAsyncDownloadFunc         = nullptr;
std::once_flag                   gInitializeOnce;

const char* PLUGIN_FACTORY_FUNCTION_NAME                 = "CreateFileDownloadPlugin";
const char* PLUGIN_INITIALIZE_FUNCTION_NAME              = "InitializeFileDownloadPlugin";
const char* PLUGIN_DESTRUCTOR_FUNCTION_NAME              = "DestroyFileDownloadPlugin";
const char* PLUGIN_IS_ASYNC_DOWNLOAD_SUPPORTED_FUNC_NAME = "FileDownloadPluginIsAsyncDownloadSupported";
const char* PLUGIN_START_ASYNC_DOWNLOAD_FUNC_NAME        = "FileDownloadPluginStartAsyncDownload";
const char* PLUGIN_CANCEL_ASYNC_DOWNLOAD_FUNC_NAME       = "FileDownloadPluginCancelAsyncDownload";
const char* DEFAULT_PLUGIN_PATH                          = "libdali2-file-download-plugin-curl.so";
const char* DOWNLOAD_API_PLUGIN_PATH                     = "libdali2-file-download-plugin-download-api.so";
#if defined(DALI_PRIVILEGE_CHECK_AVAILABLE)
const char* DOWNLOAD_API_PRIVILEGE = "http://tizen.org/privilege/download";
#endif

std::mutex                                                     gAsyncRequestMutex;
std::unordered_map<FileDownloadPluginProxy::DownloadId, std::unique_ptr<AsyncDownloadRequest>> gDownloadIdToRequest;
// Guards immediate callbacks before the plugin returns a download id.
std::unordered_set<AsyncDownloadRequest*> gStartingRequests;

void OnAsyncDownloadCompleted(FileDownloadPluginProxy::DownloadId downloadId, bool success, const char* filePath, void* userData)
{
  FileDownloadPluginProxy::AsyncCompletionCallback completionCallback;
  std::unique_ptr<AsyncDownloadRequest>            request;
  auto*                                            requestPtr = static_cast<AsyncDownloadRequest*>(userData);
  {
    std::scoped_lock lock(gAsyncRequestMutex);
    auto             it = gDownloadIdToRequest.find(downloadId);
    if(it != gDownloadIdToRequest.end() && it->second.get() == requestPtr)
    {
      request = std::move(it->second);
      gDownloadIdToRequest.erase(it);
    }
    else if(requestPtr && gStartingRequests.find(requestPtr) != gStartingRequests.end())
    {
      // Cache hits can complete during StartAsyncDownload(); deliver after return.
      requestPtr->completed = true;
      requestPtr->success   = success;
      requestPtr->filePath  = filePath ? std::string(filePath) : std::string();
    }
  }

  if(DALI_LIKELY(request))
  {
    completionCallback = std::move(request->callback);
    if(DALI_LIKELY(completionCallback))
    {
      completionCallback(success, filePath ? std::string(filePath) : std::string());
    }
  }
}

bool IsDownloadApiPrivilegeAllowed()
{
#if defined(DALI_PRIVILEGE_CHECK_AVAILABLE)
  cynara*     cynaraHandle  = nullptr;
  int         ret           = cynara_initialize(&cynaraHandle, nullptr);
  char        uid[]         = "";
  const char* clientSession = "";
  char*       cynaraClient  = nullptr;

  if(DALI_UNLIKELY(ret != CYNARA_API_SUCCESS))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] cynara_initialize failed ret[%d], falling back to curl for download-api path\n", ret);
    return false;
  }

  ret = cynara_creds_self_get_client(CLIENT_METHOD_DEFAULT, &cynaraClient);
  if(DALI_UNLIKELY(ret != CYNARA_API_SUCCESS))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] cynara_creds_self_get_client failed ret[%d], falling back to curl for download-api path\n", ret);
    cynara_finish(cynaraHandle);
    return false;
  }

  ret = cynara_check(cynaraHandle, cynaraClient, clientSession, uid, DOWNLOAD_API_PRIVILEGE);
  cynara_finish(cynaraHandle);
  std::free(cynaraClient);

  if(DALI_UNLIKELY(ret != CYNARA_API_ACCESS_ALLOWED))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] download privilege check denied ret[%d], falling back to curl\n", ret);
    return false;
  }
#endif

  return true;
}

/**
 * @brief Attempt to load the file download plugin library.
 *
 * @param[in] pluginName The name or path of the plugin library to load
 * @return true if library and symbols loaded successfully, false otherwise
 */
bool InitializeFileDownloadPluginLibrary(const char* pluginName)
{
  if(DALI_UNLIKELY(pluginName == nullptr))
  {
    DALI_LOG_ERROR("Plugin name is null\n");
    return false;
  }

  DALI_LOG_INFO(gFileDownloadProxyLogFilter, Debug::General, "[FileDownload][Proxy] Try to load dali file-download plugin library: %s\n", pluginName);

  // Attempt to load the plugin library
  gPluginLibraryHandle = dlopen(pluginName, RTLD_LAZY | RTLD_GLOBAL);
  if(DALI_UNLIKELY(!gPluginLibraryHandle))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] Cannot load dali file-download plugin library: %s : %s\n", pluginName, dlerror());
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  DALI_LOG_INFO(gFileDownloadProxyLogFilter, Debug::General, "[FileDownload][Proxy] Loaded dali file-download plugin library: %s\n", pluginName);

  // Extract the factory function
  gCreatePluginFunc = reinterpret_cast<CreatePluginFunction>(dlsym(gPluginLibraryHandle, PLUGIN_FACTORY_FUNCTION_NAME));

  if(DALI_UNLIKELY(!gCreatePluginFunc))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] Cannot load symbol %s from dali file-download plugin library (%s)\n", PLUGIN_FACTORY_FUNCTION_NAME, dlerror());
    DALI_PRINT_SYSTEM_ERROR_LOG();
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle = nullptr;
    return false;
  }

  // Extract the initialize function
  gInitializePluginFunc = reinterpret_cast<InitializePluginFunction>(dlsym(gPluginLibraryHandle, PLUGIN_INITIALIZE_FUNCTION_NAME));

  if(DALI_UNLIKELY(!gInitializePluginFunc))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] Cannot load symbol %s from dali file-download plugin library (%s)\n", PLUGIN_INITIALIZE_FUNCTION_NAME, dlerror());
    DALI_PRINT_SYSTEM_ERROR_LOG();
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle = nullptr;
    gCreatePluginFunc    = nullptr;
    return false;
  }

  // Extract the destructor function
  gDestroyPluginFunc = reinterpret_cast<DestroyPluginFunction>(dlsym(gPluginLibraryHandle, PLUGIN_DESTRUCTOR_FUNCTION_NAME));

  if(DALI_UNLIKELY(!gDestroyPluginFunc))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] Cannot load symbol %s from dali file-download plugin library (%s)\n", PLUGIN_DESTRUCTOR_FUNCTION_NAME, dlerror());
    DALI_PRINT_SYSTEM_ERROR_LOG();
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle  = nullptr;
    gCreatePluginFunc     = nullptr;
    gInitializePluginFunc = nullptr;
    return false;
  }

  // Create the plugin instance
  gFileDownloadPlugin = gCreatePluginFunc();
  if(DALI_UNLIKELY(!gFileDownloadPlugin))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] Call to CreateFileDownloadPlugin() failed\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle  = nullptr;
    gCreatePluginFunc     = nullptr;
    gInitializePluginFunc = nullptr;
    gDestroyPluginFunc    = nullptr;
    return false;
  }

  // Optional capability probe. Present only in plugins that support async download.
  // dlsym-based lookup avoids vtable dispatch crashes with older plugin SOs that lack the slots.
  gIsAsyncDownloadSupportedFunc = reinterpret_cast<IsAsyncDownloadSupportedFunction>(dlsym(gPluginLibraryHandle, PLUGIN_IS_ASYNC_DOWNLOAD_SUPPORTED_FUNC_NAME));
  gStartAsyncDownloadFunc       = reinterpret_cast<StartAsyncDownloadFunction>(dlsym(gPluginLibraryHandle, PLUGIN_START_ASYNC_DOWNLOAD_FUNC_NAME));
  gCancelAsyncDownloadFunc      = reinterpret_cast<CancelAsyncDownloadFunction>(dlsym(gPluginLibraryHandle, PLUGIN_CANCEL_ASYNC_DOWNLOAD_FUNC_NAME));
  DALI_LOG_INFO(gFileDownloadProxyLogFilter, Debug::General, "[FileDownload][Proxy] Created dali file-download plugin successfully. %s: %s\n", PLUGIN_IS_ASYNC_DOWNLOAD_SUPPORTED_FUNC_NAME, gIsAsyncDownloadSupportedFunc ? "found" : "not found");

  return true;
}

/**
 * @brief Select and initialize the file download plugin library.
 *
 * Searches for the library in this order:
 * 1. DALI_FILE_DOWNLOAD_PLUGIN_NAME environment variable (explicit path)
 * 2. DALI_USE_CAPI_DOWNLOAD_PROVIDER_API=1: download-api plugin (falls back to curl if unavailable)
 * 3. DEFAULT_PLUGIN_PATH (curl) as the default
 *
 * @return true if plugin initialized successfully, false otherwise
 */
bool InitializeFileDownloadPlugin()
{
  const char* pluginName = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_FILE_DOWNLOAD_PLUGIN_NAME);

  if(pluginName && InitializeFileDownloadPluginLibrary(pluginName))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] Using plugin from DALI_FILE_DOWNLOAD_PLUGIN_NAME: %s\n", pluginName);
    return true;
  }

  const char* useDownloadApi = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_USE_CAPI_DOWNLOAD_PROVIDER_API);
  if(useDownloadApi && std::strcmp(useDownloadApi, "1") == 0)
  {
    if(IsDownloadApiPrivilegeAllowed())
    {
      if(InitializeFileDownloadPluginLibrary(DOWNLOAD_API_PLUGIN_PATH))
      {
        DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] Using download-api plugin\n");
        return true;
      }
      DALI_LOG_ERROR("[FileDownload][Proxy] download-api plugin unavailable, falling back to curl\n");
    }
    else
    {
      DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] download privilege is not allowed, falling back to curl\n");
    }
  }

  // Try by default plugin
  bool result = InitializeFileDownloadPluginLibrary(DEFAULT_PLUGIN_PATH);
  if(result)
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Proxy] Using default curl plugin\n");
  }
  return result;
}

/**
 * @brief Internal cleanup function.
 */
void InternalDestroy()
{
  std::vector<ActiveAsyncDownload> activeDownloads;
  {
    std::scoped_lock lock(gAsyncRequestMutex);
    for(auto& entry : gDownloadIdToRequest)
    {
      activeDownloads.push_back({entry.first, std::move(entry.second)});
    }
    gDownloadIdToRequest.clear();
  }

  if(gFileDownloadPlugin && gCancelAsyncDownloadFunc)
  {
    for(auto& download : activeDownloads)
    {
      gCancelAsyncDownloadFunc(gFileDownloadPlugin, download.downloadId);
    }
  }

  if(gFileDownloadPlugin)
  {
    if(gDestroyPluginFunc)
    {
      gDestroyPluginFunc(gFileDownloadPlugin);
    }
    gFileDownloadPlugin = nullptr;
  }

  if(gPluginLibraryHandle)
  {
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle = nullptr;
  }

  gCreatePluginFunc             = nullptr;
  gInitializePluginFunc         = nullptr;
  gDestroyPluginFunc            = nullptr;
  gIsAsyncDownloadSupportedFunc = nullptr;
  gStartAsyncDownloadFunc       = nullptr;
  gCancelAsyncDownloadFunc      = nullptr;
}

// EventThreadCallback relative codes.

Dali::EventThreadCallback* gEventThreadCallback = nullptr;
std::mutex                 gEventThreadCallbackMutex;

std::vector<FileDownloadPluginProxy::EventThreadCallbackObserver*> gEventThreadCallbackObserverList;

// Called from event thread.
void OnEventThreadCallbackTriggered()
{
  decltype(gEventThreadCallbackObserverList) observerList;
  {
    std::scoped_lock lock(gEventThreadCallbackMutex);
    observerList.swap(gEventThreadCallbackObserverList);
  }

  for(auto& observer : observerList)
  {
    observer->OnTriggered();
  }
}

// Called from event thread.
void CreateEventThreadCallback()
{
  std::scoped_lock lock(gEventThreadCallbackMutex);
  gEventThreadCallback = new Dali::EventThreadCallback(MakeCallback(&OnEventThreadCallbackTriggered));
}

// Called from event thread.
Dali::EventThreadCallback* DestroyEventThreadCallback()
{
  std::scoped_lock           lock(gEventThreadCallbackMutex);
  Dali::EventThreadCallback* eventThreadCallback = gEventThreadCallback;
  gEventThreadCallback                           = nullptr;
  return eventThreadCallback;
}

// Called from worker thread.
bool InternalRegisterEventThreadObserver(FileDownloadPluginProxy::EventThreadCallbackObserver& observer)
{
  {
    std::scoped_lock lock(gEventThreadCallbackMutex);
    if(DALI_LIKELY(gEventThreadCallback))
    {
      gEventThreadCallbackObserverList.emplace_back(&observer);
      if(gEventThreadCallbackObserverList.size() == 1u)
      {
        gEventThreadCallback->Trigger();
      }
      return true;
    }
  }
  return false;
}

} // unnamed namespace

void FileDownloadPluginProxy::RegisterEventThreadCallback()
{
  CreateEventThreadCallback();
}

void FileDownloadPluginProxy::UnregisterEventThreadCallback()
{
  auto* eventThreadCallback = DestroyEventThreadCallback();
  if(DALI_LIKELY(eventThreadCallback))
  {
    // Forcibly run trigger event once, to avoid dead-lock
    OnEventThreadCallbackTriggered();

    delete eventThreadCallback;
  }
}

void FileDownloadPluginProxy::Destroy()
{
  InternalDestroy();
}

bool FileDownloadPluginProxy::DownloadRemoteFileIntoMemory(const std::string&     url,
                                                           Dali::Vector<uint8_t>& dataBuffer,
                                                           size_t&                dataSize,
                                                           size_t                 maximumAllowedSizeBytes)
{
  // Thread-safe lazy initialization: attempt to load plugin on first call
  std::call_once(gInitializeOnce, InitializeFileDownloadPlugin);

  // Note : std::call_once will block other threads until initialization is complete, so we can ensure that gFileDownloadPlugin is valid after this point if initialization succeeded.
  if(DALI_LIKELY(gFileDownloadPlugin))
  {
    // DevNote : Must call InitializePluginFunc out of gInitializeOnce mutex, to avoid dead-lock case.
    if(DALI_UNLIKELY(gInitializePluginFunc == nullptr || !gInitializePluginFunc(gFileDownloadPlugin)))
    {
      DALI_LOG_ERROR("[FileDownload][Proxy] plugin initialize failed for sync download url[%s]\n", url.c_str());
      return false;
    }
    return gFileDownloadPlugin->DownloadRemoteFileIntoMemory(url, dataBuffer, dataSize, maximumAllowedSizeBytes);
  }

  DALI_LOG_ERROR("[FileDownload][Proxy] plugin not available for sync download url[%s]\n", url.c_str());
  return false;
}

bool FileDownloadPluginProxy::RegisterEventThreadObserver(FileDownloadPluginProxy::EventThreadCallbackObserver& observer)
{
  return InternalRegisterEventThreadObserver(observer);
}

bool FileDownloadPluginProxy::IsAsyncDownloadSupported()
{
  std::call_once(gInitializeOnce, InitializeFileDownloadPlugin);

  // Use a standalone C symbol rather than virtual dispatch so that old plugin SOs
  // that predate the IsAsyncDownloadSupported vtable slot do not crash.
  bool result = (gIsAsyncDownloadSupportedFunc != nullptr && gStartAsyncDownloadFunc != nullptr && gCancelAsyncDownloadFunc != nullptr) ? gIsAsyncDownloadSupportedFunc() : false;
  return result;
}

FileDownloadPluginProxy::DownloadId FileDownloadPluginProxy::StartAsyncDownload(const std::string&      url,
                                                                                size_t                  maxSize,
                                                                                AsyncCompletionCallback callback)
{
  std::call_once(gInitializeOnce, InitializeFileDownloadPlugin);

  if(DALI_UNLIKELY(!gFileDownloadPlugin || !gStartAsyncDownloadFunc))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] plugin not available for async download url[%s]\n", url.c_str());
    return INVALID_DOWNLOAD_ID;
  }

  if(DALI_UNLIKELY(gInitializePluginFunc == nullptr || !gInitializePluginFunc(gFileDownloadPlugin)))
  {
    DALI_LOG_ERROR("[FileDownload][Proxy] plugin initialize failed for async download url[%s]\n", url.c_str());
    return INVALID_DOWNLOAD_ID;
  }
  DALI_LOG_INFO(gFileDownloadProxyLogFilter, Debug::Verbose, "[FileDownload][Proxy] StartAsyncDownload url[%s]\n", url.c_str());

  auto  request    = std::make_unique<AsyncDownloadRequest>(std::move(callback));
  auto* requestPtr = request.get();

  // The provider may call back before returning downloadId.
  {
    std::scoped_lock lock(gAsyncRequestMutex);
    gStartingRequests.insert(requestPtr);
  }

  DownloadId downloadId = gStartAsyncDownloadFunc(gFileDownloadPlugin, url.c_str(), maxSize, OnAsyncDownloadCompleted, requestPtr);
  if(DALI_UNLIKELY(downloadId == INVALID_DOWNLOAD_ID))
  {
    std::scoped_lock lock(gAsyncRequestMutex);
    gStartingRequests.erase(requestPtr);
    return INVALID_DOWNLOAD_ID;
  }

  AsyncCompletionCallback completionCallback;
  bool                    completed = false;
  bool                    success   = false;
  std::string             filePath;
  {
    std::scoped_lock lock(gAsyncRequestMutex);
    gStartingRequests.erase(requestPtr);
    if(request->completed)
    {
      completed          = true;
      success            = request->success;
      filePath           = std::move(request->filePath);
      completionCallback = std::move(request->callback);
    }
    else
    {
      gDownloadIdToRequest[downloadId] = std::move(request);
    }
  }

  if(completed && completionCallback)
  {
    completionCallback(success, std::move(filePath));
  }
  return downloadId;
}

void FileDownloadPluginProxy::CancelAsyncDownload(DownloadId downloadId)
{
  if(DALI_UNLIKELY(downloadId == INVALID_DOWNLOAD_ID))
  {
    return;
  }

  std::unique_ptr<AsyncDownloadRequest> requestToDelete;
  {
    std::scoped_lock lock(gAsyncRequestMutex);
    auto             it = gDownloadIdToRequest.find(downloadId);
    if(it != gDownloadIdToRequest.end())
    {
      it->second->callback = {};
      requestToDelete      = std::move(it->second);
      gDownloadIdToRequest.erase(it);
    }
  }

  if(DALI_LIKELY(gFileDownloadPlugin && gCancelAsyncDownloadFunc))
  {
    gCancelAsyncDownloadFunc(gFileDownloadPlugin, downloadId);
  }
}

} // namespace Dali
