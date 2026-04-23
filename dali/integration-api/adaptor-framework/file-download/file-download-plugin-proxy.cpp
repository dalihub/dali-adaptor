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
#include <cstring>
#include <mutex>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/file-download-plugin.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-error-print.h>

using namespace Dali::Integration;

namespace Dali
{
namespace // unnamed namespace
{
/// Plugin library relative codes.

// Forward declarations
typedef Dali::FileDownloadPlugin* (*CreatePluginFunction)();
typedef bool (*InitializePluginFunction)(Dali::FileDownloadPlugin*);
typedef void (*DestroyPluginFunction)(Dali::FileDownloadPlugin*);

// Plugin state variables
void*                     gPluginLibraryHandle  = nullptr;
Dali::FileDownloadPlugin* gFileDownloadPlugin   = nullptr;
CreatePluginFunction      gCreatePluginFunc     = nullptr;
InitializePluginFunction  gInitializePluginFunc = nullptr;
DestroyPluginFunction     gDestroyPluginFunc    = nullptr;
std::once_flag            gInitializeOnce;

const char* PLUGIN_FACTORY_FUNCTION_NAME    = "CreateFileDownloadPlugin";
const char* PLUGIN_INITIALIZE_FUNCTION_NAME = "InitializeFileDownloadPlugin";
const char* PLUGIN_DESTRUCTOR_FUNCTION_NAME = "DestroyFileDownloadPlugin";
const char* DEFAULT_PLUGIN_PATH             = "libdali2-file-download-plugin-curl.so";

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

  // TODO : Could we use addon-manager here?
  DALI_LOG_DEBUG_INFO("Try to load dali file-download plugin library: %s\n", pluginName ? pluginName : "(null)");

  // Attempt to load the plugin library
  gPluginLibraryHandle = dlopen(pluginName, RTLD_LAZY | RTLD_GLOBAL);
  if(DALI_UNLIKELY(!gPluginLibraryHandle))
  {
    DALI_LOG_ERROR("Cannot load dali file-download plugin library: %s : %s\n", pluginName ? pluginName : "(null)", dlerror());
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  DALI_LOG_DEBUG_INFO("Loaded dali file-download plugin library: %s\n", pluginName ? pluginName : "(null)");

  // Extract the factory function
  gCreatePluginFunc = reinterpret_cast<CreatePluginFunction>(dlsym(gPluginLibraryHandle, PLUGIN_FACTORY_FUNCTION_NAME));

  if(DALI_UNLIKELY(!gCreatePluginFunc))
  {
    DALI_LOG_ERROR("Cannot load symbol %s from dali file-download plugin library (%s)\n", PLUGIN_FACTORY_FUNCTION_NAME, dlerror());
    DALI_PRINT_SYSTEM_ERROR_LOG();
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle = nullptr;
    return false;
  }

  // Extract the factory function
  gInitializePluginFunc = reinterpret_cast<InitializePluginFunction>(dlsym(gPluginLibraryHandle, PLUGIN_INITIALIZE_FUNCTION_NAME));

  if(DALI_UNLIKELY(!gInitializePluginFunc))
  {
    DALI_LOG_ERROR("Cannot load symbol %s from dali file-download plugin library (%s)\n", PLUGIN_INITIALIZE_FUNCTION_NAME, dlerror());
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
    DALI_LOG_ERROR("Cannot load symbol %s from dali file-download plugin library (%s)\n", PLUGIN_DESTRUCTOR_FUNCTION_NAME, dlerror());
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
    DALI_LOG_ERROR("Call to CreateFileDownloadPlugin() failed\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle  = nullptr;
    gCreatePluginFunc     = nullptr;
    gInitializePluginFunc = nullptr;
    gDestroyPluginFunc    = nullptr;
    return false;
  }

  DALI_LOG_DEBUG_INFO("Created dali file-download plugin successfully\n");

  return true;
}

/**
 * @brief Attempt to load the file download plugin library.
 *
 * Searches for the library in this order:
 * 1. DALI_FILE_DOWNLOAD_PLUGIN_NAME environment variable
 * 2. DEFAULT_PLUGIN_PATH constant
 *
 * @return true if plugin initialized successfully, false otherwise
 */
bool InitializeFileDownloadPlugin()
{
  const char* pluginName = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_FILE_DOWNLOAD_PLUGIN_NAME);

  if(pluginName && InitializeFileDownloadPluginLibrary(pluginName))
  {
    return true;
  }

  // Try by default plugin
  return InitializeFileDownloadPluginLibrary(DEFAULT_PLUGIN_PATH);
}

/**
 * @brief Internal cleanup function.
 */
void InternalDestroy()
{
  DALI_LOG_DEBUG_INFO("Request to destroy file-download plugin\n");
  if(gFileDownloadPlugin)
  {
    if(gDestroyPluginFunc)
    {
      gDestroyPluginFunc(gFileDownloadPlugin);
      DALI_LOG_DEBUG_INFO("Destroyed dali file-download plugin\n");
    }
    gFileDownloadPlugin = nullptr;
  }

  if(gPluginLibraryHandle)
  {
    dlclose(gPluginLibraryHandle);
    gPluginLibraryHandle = nullptr;
    DALI_LOG_DEBUG_INFO("Closed dali file-download plugin library\n");
  }

  gCreatePluginFunc     = nullptr;
  gInitializePluginFunc = nullptr;
  gDestroyPluginFunc    = nullptr;
}

/// EventThreadCallback relative codes.

Dali::EventThreadCallback* gEventThreadCallback = nullptr;
std::mutex                 gEventThreadCallbackMutex;

std::vector<FileDownloadPluginProxy::EventThreadCallbackObserver*> gEventThreadCallbackObserverList;

// Called from event thread.
void OnEventThreadCallbackTriggered()
{
  DALI_LOG_DEBUG_INFO("OnEventThreadCallbackTriggered\n");
  decltype(gEventThreadCallbackObserverList) observerList;
  {
    std::scoped_lock lock(gEventThreadCallbackMutex);
    observerList.swap(gEventThreadCallbackObserverList);
  }

  for(auto& observer : observerList)
  {
    DALI_LOG_DEBUG_INFO("Trigger FileDownloadPluginProxy::EventThreadCallbackObserver : %p\n", observer);
    observer->OnTriggered();
  }
}

// Called from event thread.
void CreateEventThreadCallback()
{
  std::scoped_lock lock(gEventThreadCallbackMutex);
  gEventThreadCallback = new Dali::EventThreadCallback(MakeCallback(&OnEventThreadCallbackTriggered));
  DALI_LOG_DEBUG_INFO("FileDownloadPluginProxy::EventThreadCallback [%p] Trigger Id(%u)\n", gEventThreadCallback, gEventThreadCallback->GetId());
}

// Called from event thread.
Dali::EventThreadCallback* DestroyEventThreadCallback()
{
  std::scoped_lock           lock(gEventThreadCallbackMutex);
  Dali::EventThreadCallback* eventThreadCallback = gEventThreadCallback;
  gEventThreadCallback                           = nullptr;
  DALI_LOG_DEBUG_INFO("FileDownloadPluginProxy::DestroyEventThreadCallback [%p]\n", eventThreadCallback);
  return eventThreadCallback;
}

// Called from worker thread.
bool InternalRegisterEventThreadObserver(FileDownloadPluginProxy::EventThreadCallbackObserver& observer)
{
  {
    std::scoped_lock lock(gEventThreadCallbackMutex);
    DALI_LOG_DEBUG_INFO("FileDownloadPluginProxy::InternalRegisterEventThreadObserver [%p]\n", gEventThreadCallback);
    if(DALI_LIKELY(gEventThreadCallback))
    {
      DALI_LOG_DEBUG_INFO("FileDownloadPluginProxy::InternalRegisterEventThreadObserver : %p\n", &observer);
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
    DALI_LOG_DEBUG_INFO("Try to call Trigger during adaptor terminated.\n");
    // Forcibly run trigger event once, to avoid dead-lock
    OnEventThreadCallbackTriggered();
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
      DALI_LOG_ERROR("File download plugin initialize failed!\n");
      return false;
    }
    return gFileDownloadPlugin->DownloadRemoteFileIntoMemory(url, dataBuffer, dataSize, maximumAllowedSizeBytes);
  }

  // Plugin not available, return false
  DALI_LOG_ERROR("File download plugin not available!\n");
  return false;
}

bool FileDownloadPluginProxy::RegisterEventThreadObserver(FileDownloadPluginProxy::EventThreadCallbackObserver& observer)
{
  return InternalRegisterEventThreadObserver(observer);
}

} // namespace Dali
