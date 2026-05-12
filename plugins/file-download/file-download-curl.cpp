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
#include "file-download-curl.h"

// EXTERNAL INCLUDES
#include <curl/curl.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/file-stream.h>
#include <dali/devel-api/adaptor-framework/thread-settings.h> ///< To check whether we call InitializePlugin() at main thread or not.
#include <dali/devel-api/threading/semaphore.h>
#include <dali/integration-api/adaptor-framework/file-download/file-download-plugin-proxy.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/dali-adaptor-version.h>

// Export factory functions for dynamic loading
extern "C" DALI_ADAPTOR_API Dali::FileDownloadPlugin* CreateFileDownloadPlugin()
{
  return new Dali::Plugin::CurlFileDownloader();
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

namespace Dali::Plugin
{
namespace // unnamed namespace
{
#ifdef DALI_PROFILE_WINDOWS
#define FUNCTION_CALL_FROM_CURL_PREFIX __cdecl
#else
#define FUNCTION_CALL_FROM_CURL_PREFIX
#endif

// Curl relative environments
const char* DALI_ENV_CURLOPT_VERBOSE_MODE = "DALI_CURLOPT_VERBOSE_MODE";
const char* DALI_ENV_CURLOPT_MAXREDIRS    = "DALI_CURLOPT_MAXREDIRS";
const char* HTTP_PROXY_ENV                = "http_proxy";

// Helper to print the curl error codes.
// Use macro, ensure to print the line number.
#define LOG_CURL_RESULT(result, errorBuffer, url, prefix)                                                                                  \
  {                                                                                                                                        \
    if(DALI_UNLIKELY(result != CURLE_OK))                                                                                                  \
    {                                                                                                                                      \
      if(errorBuffer != nullptr)                                                                                                           \
      {                                                                                                                                    \
        DALI_LOG_ERROR("%s \"%s\" with error code %d\n", std::string(prefix).c_str(), std::string(url).c_str(), result);                   \
      }                                                                                                                                    \
      else                                                                                                                                 \
      {                                                                                                                                    \
        DALI_LOG_ERROR("%s \"%s\" with error code %d (%s)\n", std::string(prefix).c_str(), std::string(url).c_str(), result, errorBuffer); \
      }                                                                                                                                    \
    }                                                                                                                                      \
  }

#define CHECK_CURL_RESULT_AND_RETURN_FALSE(curlResult, prefix) \
  {                                                            \
    if(DALI_UNLIKELY(curlResult != CURLE_OK))                  \
    {                                                          \
      LOG_CURL_RESULT(curlResult, errorBuffer, url, prefix);   \
      DALI_LOG_ERROR("CURL error occured!\n");                 \
      return false;                                            \
    }                                                          \
  }

std::string ConvertDataReadable(uint8_t* data, const size_t size, const size_t width)
{
  std::ostringstream oss;

  for(size_t i = 0u; i < size; ++i)
  {
    if(i > 0u && (i % width) == 0u)
    {
      oss << '\n';
    }
    oss << ((data[i] >= 0x20 && data[i] < 0x80) ? static_cast<char>(data[i]) : '.');
  }

  return oss.str();
}

int FUNCTION_CALL_FROM_CURL_PREFIX CurloptVerboseLogTrace(CURL* handle, curl_infotype type, char* data, size_t size, void* clientp)
{
  std::ostringstream oss;
  (void)handle; /* prevent compiler warning */
  (void)clientp;

  switch(type)
  {
    case CURLINFO_TEXT:
    {
      oss << "== Info: " << std::string(data, size) << "\n";

      DALI_FALLTHROUGH;
    }
    default: /* in case a new one is introduced to shock us */
    {
      return 0;
    }

    case CURLINFO_HEADER_OUT:
    {
      oss << "=> Send header\n";
      break;
    }
    case CURLINFO_DATA_OUT:
    {
      oss << "=> Send data\n";
      break;
    }
    case CURLINFO_SSL_DATA_OUT:
    {
      oss << "=> Send SSL data\n";
      break;
    }
    case CURLINFO_HEADER_IN:
    {
      oss << "<= Recv header\n";
      break;
    }
    case CURLINFO_DATA_IN:
    {
      oss << "<= Recv data\n";
      break;
    }
    case CURLINFO_SSL_DATA_IN:
    {
      oss << "<= Recv SSL data\n";
      break;
    }
  }

  oss << "data size : " << size << " bytes\n";
  oss << "data : \n";
  oss << ConvertDataReadable(reinterpret_cast<uint8_t*>(data), size, 0x40);

  DALI_LOG_DEBUG_INFO("Verbose curl log : %s", oss.str().c_str());

  return 0;
}

const int  CONNECTION_TIMEOUT_SECONDS(30L);
const int  TIMEOUT_SECONDS(120L);
const long CLOSE_CONNECTION_ON_ERROR = 1L; // 0 == off, 1 == on
const long EXCLUDE_HEADER            = 0L;
const long INCLUDE_HEADER            = 1L;
const long INCLUDE_BODY              = 0L;
const long EXCLUDE_BODY              = 1L;

/**
 * @brief Get the Curlopt Verbose Mode value from environment.
 *
 * @return 0 if verbose mode off. 1 if verbose mode on.
 */
long GetCurloptVerboseMode()
{
  static long verboseMode       = 0;
  static bool verboseModeSetted = false;
  if(DALI_UNLIKELY(!verboseModeSetted))
  {
    auto verboseModeString = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_CURLOPT_VERBOSE_MODE);
    verboseMode            = verboseModeString ? (std::strtol(verboseModeString, nullptr, 10) > 0 ? 1 : 0) : 0;
    verboseModeSetted      = true;
  }

  return verboseMode;
}

/**
 * @brief Get the Curlopt Maximum Redirection count value from environment.
 *
 * @return 5 if environment not defined. Otherwise, value from environment.
 */
long GetCurloptMaximumRedirectionCount()
{
  static long maxiumumRedirectionCount       = 5L;
  static bool maxiumumRedirectionCountSetted = false;
  if(DALI_UNLIKELY(!maxiumumRedirectionCountSetted))
  {
    auto maxiumumRedirectionCountString = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_CURLOPT_MAXREDIRS);
    maxiumumRedirectionCount            = maxiumumRedirectionCountString ? (std::strtol(maxiumumRedirectionCountString, nullptr, 10)) : 5L;
    maxiumumRedirectionCountSetted      = true;
  }

  return maxiumumRedirectionCount;
}

struct ChunkData
{
  std::vector<uint8_t> data;
};

// Without a write function or a buffer (file descriptor) to write to, curl will pump out
// header/body contents to stdout
size_t FUNCTION_CALL_FROM_CURL_PREFIX DummyWrite(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  return size * nmemb;
}

size_t FUNCTION_CALL_FROM_CURL_PREFIX ChunkLoader(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::vector<ChunkData>* chunks   = static_cast<std::vector<ChunkData>*>(userdata);
  int                     numBytes = size * nmemb;
  chunks->push_back(ChunkData());
  ChunkData& chunkData = (*chunks)[chunks->size() - 1];
  chunkData.data.reserve(numBytes);
  memcpy(&chunkData.data[0], ptr, numBytes);
  return numBytes;
}

#ifdef DALI_PROFILE_WINDOWS
size_t FUNCTION_CALL_FROM_CURL_PREFIX WriteFunction(void* input, size_t uSize, size_t uCount, void* avg)
{
  fwrite((const char*)input, uSize, uCount, (FILE*)avg);
  return uSize * uCount;
}
#endif

bool ConfigureCurlOptions(CURL* curlHandle, const std::string& url, const std::string& userAgent, const std::string& proxy, char* errorBuffer)
{
  const auto verboseMode = GetCurloptVerboseMode(); // 0 : off, 1 : on

  const long maximumRedirectionCounts = GetCurloptMaximumRedirectionCount(); // 5 for default

  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str()), "CURLOPT_URL");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, verboseMode), "CURLOPT_VERBOSE");

  // CURLOPT_FAILONERROR is not fail-safe especially when authentication is involved ( see manual )
  // Removed CURLOPT_FAILONERROR option
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT_SECONDS), "CURLOPT_CONNECTTIMEOUT");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, TIMEOUT_SECONDS), "CURLOPT_TIMEOUT");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_HEADER, INCLUDE_HEADER), "CURLOPT_HEADER");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOBODY, EXCLUDE_BODY), "CURLOPT_NOBODY");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1L), "CURLOPT_NOSIGNAL");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L), "CURLOPT_FOLLOWLOCATION");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, maximumRedirectionCounts), "CURLOPT_MAXREDIRS");

  if(DALI_LIKELY(!userAgent.empty()))
  {
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, userAgent.c_str()), "CURLOPT_USERAGENT");
  }

  if(verboseMode != 0)
  {
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_DEBUGFUNCTION, CurloptVerboseLogTrace), "CURLOPT_DEBUGFUNCTION");
  }

  // If the proxy variable is set, ensure it's also used.
  // In theory, this variable should be used by the curl library; however, something
  // is overriding it.
  if(!proxy.empty())
  {
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_PROXY, proxy.c_str()), "CURLOPT_PROXY");
  }

  return true;
}

bool DownloadFileDataWithSize(CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t dataSize, const std::string& url, char* errorBuffer)
{
  // create
  Dali::FileStream fileWriter(dataBuffer, dataSize, FileStream::WRITE | FileStream::BINARY);
  FILE*            dataBufferFilePointer = fileWriter.GetFile();

  if(NULL != dataBufferFilePointer)
  {
    setbuf(dataBufferFilePointer, NULL); // Turn buffering off

    // we only want the body which contains the file data
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER), "CURLOPT_HEADER");
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOBODY, INCLUDE_BODY), "CURLOPT_NOBODY");

    // disable the write callback, and get curl to write directly into our data buffer
#ifdef DALI_PROFILE_WINDOWS
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteFunction), "CURLOPT_WRITEFUNCTION");
#else
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, nullptr), "CURLOPT_WRITEFUNCTION");
#endif

    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, dataBufferFilePointer), "CURLOPT_WRITEDATA");

    // synchronous request of the body data
    CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_perform(curlHandle), "Failed to download image file with fixed size");
  }
  else
  {
    DALI_LOG_ERROR("Fail to open buffer writter with size : %zu!\n", dataSize);
    // @todo : Need to check that is it correct error code?
    CHECK_CURL_RESULT_AND_RETURN_FALSE(CURLE_READ_ERROR, "Dali::FileStream fileWriter create failed!");
  }
  return true;
}

bool DownloadFileDataByChunk(CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t& dataSize, const std::string& url, char* errorBuffer)
{
  // create
  std::vector<ChunkData> chunks;

  // we only want the body which contains the file data
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER), "CURLOPT_HEADER");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOBODY, INCLUDE_BODY), "CURLOPT_NOBODY");

  // Enable the write callback.
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, ChunkLoader), "CURLOPT_WRITEFUNCTION");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &chunks), "CURLOPT_WRITEDATA");

  // synchronous request of the body data
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_perform(curlHandle), "Failed to download image file by chunk");

  // chunks should now contain all of the chunked data. Reassemble into a single vector
  dataSize = 0;
  for(size_t i = 0; i < chunks.size(); ++i)
  {
    dataSize += chunks[i].data.capacity();
  }
  dataBuffer.ResizeUninitialized(dataSize);

  if(DALI_LIKELY(dataSize > 0))
  {
    std::uint8_t* dataBufferPtr = dataBuffer.Begin();
    for(size_t i = 0; i < chunks.size(); ++i)
    {
      memcpy(dataBufferPtr, &chunks[i].data[0], chunks[i].data.capacity());
      dataBufferPtr += chunks[i].data.capacity();
    }
  }

  return true;
}

bool DownloadFile(CURL*                  curlHandle,
                  const std::string&     url,
                  Dali::Vector<uint8_t>& dataBuffer,
                  size_t&                dataSize,
                  size_t                 maximumAllowedSizeBytes,
                  const std::string&     userAgent,
                  const std::string&     proxy,
                  char*                  errorBuffer)
{
  bool result = false;
  if(errorBuffer != nullptr)
  {
    errorBuffer[0] = 0;
  }
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, errorBuffer), "CURLOPT_ERRORBUFFER");

  // setup curl to download just the header so we can extract the content length
  if(DALI_UNLIKELY(!ConfigureCurlOptions(curlHandle, url, userAgent, proxy, errorBuffer)))
  {
    DALI_LOG_ERROR("Fail to set curlopt!\n");
    return false;
  }
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, DummyWrite), "CURLOPT_WRITEFUNCTION");

  // perform the request to get the header
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_perform(curlHandle), "Failed to download http header for");

  // get the content length, -1 == size is not known
  curl_off_t size{0};
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_getinfo(curlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size), "Fail to get size of content");

  if(size == -1)
  {
    result = DownloadFileDataByChunk(curlHandle, dataBuffer, dataSize, url, errorBuffer);
  }
  else if(size >= static_cast<curl_off_t>(maximumAllowedSizeBytes))
  {
    DALI_LOG_ERROR("File content length %" CURL_FORMAT_CURL_OFF_T " > max allowed %zu \"%s\" \n", size, maximumAllowedSizeBytes, url.c_str());
    result = false;
  }
  else
  {
    // If we know the size up front, allocate once and avoid chunk copies.
    dataSize = static_cast<size_t>(size);
    result   = DownloadFileDataWithSize(curlHandle, dataBuffer, dataSize, url, errorBuffer);
    if(!result)
    {
      DALI_LOG_DEBUG_INFO("Failed to download file, trying to load by chunk. \"%s\"\n", url.c_str());
      // In the case where the size is wrong (e.g. on a proxy server that rewrites data),
      // the data buffer will be corrupt. In this case, try again using the chunk writer.
      result = DownloadFileDataByChunk(curlHandle, dataBuffer, dataSize, url, errorBuffer);
    }
  }

  if(DALI_UNLIKELY(result && dataSize == 0u))
  {
    DALI_LOG_ERROR("Warning : Download data size is 0! url : %s\n", url.c_str());
  }
  return result;
}

} // unnamed namespace

/**
 * Curl library environment. Direct initialize ensures it's constructed before this plugin
 * uses any curl functionality.
 */
class CurlFileDownloader::Impl : public Dali::FileDownloadPluginProxy::EventThreadCallbackObserver
{
public:
  Impl()
  : mUserAgent("DALi/" + std::to_string(ADAPTOR_MAJOR_VERSION) + "." + std::to_string(ADAPTOR_MINOR_VERSION) + "." + std::to_string(ADAPTOR_MICRO_VERSION)),
    mProxy(),
    mInitializationSemaphore(0),
    mTerminateSemaphore(0),
    mInitialized(false)
  {
  }

  ~Impl()
  {
    if(DALI_UNLIKELY(mInitialized))
    {
      DALI_LOG_DEBUG_INFO("Cleaning up curl library environment\n");

      if(Dali::GetThreadId() == Dali::GetMainThreadId())
      {
        curl_global_cleanup();
      }
      else
      {
        if(DALI_UNLIKELY(!Dali::FileDownloadPluginProxy::RegisterEventThreadObserver(*this)))
        {
          DALI_LOG_ERROR("Fail to register EventThreadCallback observer. Looks Adaptor is not available.\n");
        }
        else
        {
          DALI_LOG_DEBUG_INFO("Wait OnTriggered()\n");
          // Now we can assume that OnTriggered() callback will be comes. Acquire the semaphore.
          mTerminateSemaphore.Acquire();
          DALI_LOG_DEBUG_INFO("Cleaning up curl library environment done\n");
        }
      }
    }
  }

  bool Initialize()
  {
    // Lock and wait until curl_global_init completed.
    std::scoped_lock lock(sImplMutex);
    if(DALI_UNLIKELY(mInitialized))
    {
      return true;
    }

    DALI_LOG_DEBUG_INFO("Initializing curl library environment\n");

    if(Dali::GetThreadId() == Dali::GetMainThreadId())
    {
      curl_global_init(CURL_GLOBAL_ALL);
    }
    else
    {
      if(DALI_UNLIKELY(!Dali::FileDownloadPluginProxy::RegisterEventThreadObserver(*this)))
      {
        DALI_LOG_ERROR("Fail to register EventThreadCallback observer. Looks Adaptor is not available.\n");
        return false;
      }

      DALI_LOG_DEBUG_INFO("Wait OnTriggered()\n");
      // Now we can assume that OnTriggered() callback will be comes. Acquire the semaphore.
      mInitializationSemaphore.Acquire();
    }

    DALI_LOG_DEBUG_INFO("Initializing curl library environment done\n");

    GetCurloptVerboseMode();             // Ensure verbose mode is read from environment early so it's ready for any curl calls.
    GetCurloptMaximumRedirectionCount(); // Ensure maximum redirection count is read from environment early so it's ready for any curl calls.

    const char* proxy = Dali::EnvironmentVariable::GetEnvironmentVariable(HTTP_PROXY_ENV);
    if(proxy != nullptr)
    {
      mProxy = std::string(proxy); // Ensure proxy is read from environment early so it's ready for any curl calls.
    }

    DALI_LOG_DEBUG_INFO("CurlEnvironment initialize completed\n");

    mInitialized = true;

    return true;
  }

  const std::string& GetUserAgent() const
  {
    return mUserAgent;
  }

  const std::string& GetProxy() const
  {
    return mProxy;
  }

  void OnTriggered() override
  {
    DALI_LOG_DEBUG_INFO("OnTriggered\n");
    if(!mInitialized)
    {
      // Call curl_global_init and release initialization semaphore, to complete initialization.
      DALI_LOG_DEBUG_INFO("curl_global_init(CURL_GLOBAL_ALL)\n");
      curl_global_init(CURL_GLOBAL_ALL);
      DALI_LOG_DEBUG_INFO("curl_global_init(CURL_GLOBAL_ALL) done\n");
      mInitializationSemaphore.Release();
    }
    else
    {
      DALI_LOG_DEBUG_INFO("curl_global_cleanup()\n");
      curl_global_cleanup();
      DALI_LOG_DEBUG_INFO("curl_global_cleanup() done\n");
      mTerminateSemaphore.Release();
    }
    DALI_LOG_DEBUG_INFO("OnTriggered done\n");
  }

private:
  Impl(const Impl&)            = delete;
  Impl& operator=(const Impl&) = delete;
  Impl(Impl&&)                 = delete;
  Impl& operator=(Impl&&)      = delete;

private:
  const std::string mUserAgent;
  std::string       mProxy;

  Dali::Semaphore<1> mInitializationSemaphore; // Used for testing to wait until curl_global_init is complete at event thread.
  Dali::Semaphore<1> mTerminateSemaphore;      // Used for testing to wait until curl_global_cleanup is complete at event thread.

  bool mInitialized;

  static std::mutex sImplMutex;
};

std::mutex CurlFileDownloader::Impl::sImplMutex;

CurlFileDownloader::CurlFileDownloader()
: mImpl(new Impl())
{
}

CurlFileDownloader::~CurlFileDownloader()
{
  delete mImpl;
}

bool CurlFileDownloader::InitializePlugin()
{
  return mImpl->Initialize();
}

bool CurlFileDownloader::DownloadRemoteFileIntoMemory(const std::string&     url,
                                                      Dali::Vector<uint8_t>& dataBuffer,
                                                      size_t&                dataSize,
                                                      size_t                 maximumAllowedSizeBytes)
{
  bool result = false;

  if(url.empty())
  {
    DALI_LOG_ERROR("empty url requested \n");
    return false;
  }

  // start a libcurl easy session, this internally calls curl_global_init, if we ever have more than one download
  // thread we need to explicity call curl_global_init() on startup from a single thread.

  CURL* curlHandle = curl_easy_init();
  if(curlHandle)
  {
    std::vector<char> errorBuffer(CURL_ERROR_SIZE);
    result = DownloadFile(curlHandle, url, dataBuffer, dataSize, maximumAllowedSizeBytes, mImpl->GetUserAgent(), mImpl->GetProxy(), &errorBuffer[0]);

    // clean up session
    curl_easy_cleanup(curlHandle);
  }
  return result;
}

} // namespace Dali::Plugin
