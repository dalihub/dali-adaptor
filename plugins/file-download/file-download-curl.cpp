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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <mutex>
#include <sstream>

#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/dali-adaptor-version.h>

#if defined(_WIN32)
#define DALI_FILE_DOWNLOAD_PLUGIN_EXPORT __declspec(dllexport)
#else
#define DALI_FILE_DOWNLOAD_PLUGIN_EXPORT DALI_ADAPTOR_API
#endif

// Export factory functions for dynamic loading
extern "C" DALI_FILE_DOWNLOAD_PLUGIN_EXPORT Dali::FileDownloadPlugin* CreateFileDownloadPlugin()
{
  return new Dali::Plugin::CurlFileDownloader();
}

extern "C" DALI_FILE_DOWNLOAD_PLUGIN_EXPORT bool InitializeFileDownloadPlugin(Dali::FileDownloadPlugin* plugin)
{
  if(DALI_LIKELY(plugin))
  {
    return plugin->InitializePlugin();
  }
  return false;
}

extern "C" DALI_FILE_DOWNLOAD_PLUGIN_EXPORT void DestroyFileDownloadPlugin(Dali::FileDownloadPlugin* plugin)
{
  delete plugin;
}

// Shutdown entry point looked up via dlsym by FileDownloadPluginProxy::Shutdown().
// Optional, so that older plugin SOs without it still load.
extern "C" DALI_FILE_DOWNLOAD_PLUGIN_EXPORT bool FileDownloadPluginShutdown(Dali::FileDownloadPlugin* plugin)
{
  if(DALI_LIKELY(plugin))
  {
    return static_cast<Dali::Plugin::CurlFileDownloader*>(plugin)->Shutdown();
  }
  return true;
}

#undef DALI_FILE_DOWNLOAD_PLUGIN_EXPORT

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
#define LOG_CURL_RESULT(result, errorBuffer, url, prefix)                                                                                                          \
  {                                                                                                                                                                \
    if(DALI_UNLIKELY(result != CURLE_OK))                                                                                                                          \
    {                                                                                                                                                              \
      const char* curlErrorMessage = ((errorBuffer) != nullptr && (errorBuffer)[0] != '\0') ? (errorBuffer) : curl_easy_strerror(result);                          \
      DALI_LOG_ERROR("[FileDownload][Curl] %s \"%s\" with error code %d (%s)\n", std::string(prefix).c_str(), std::string(url).c_str(), result, curlErrorMessage); \
    }                                                                                                                                                              \
  }

#define CHECK_CURL_RESULT_AND_RETURN_FALSE(curlResult, prefix)      \
  {                                                                 \
    if(DALI_UNLIKELY(curlResult != CURLE_OK))                       \
    {                                                               \
      LOG_CURL_RESULT(curlResult, errorBuffer, url, prefix);        \
      DALI_LOG_ERROR("[FileDownload][Curl] CURL error occured!\n"); \
      return false;                                                 \
    }                                                               \
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
const long EXCLUDE_HEADER = 0L;
const long INCLUDE_HEADER = 1L;
const long INCLUDE_BODY   = 0L;
const long EXCLUDE_BODY   = 1L;

// Abort a transfer that makes no meaningful progress rather than holding a worker thread until
// TIMEOUT_SECONDS expires. Deliberately conservative : only a genuinely stalled connection is
// slower than this, so a legitimately slow network is not killed.
const long LOW_SPEED_LIMIT_BYTES_PER_SECOND(1L);
const long LOW_SPEED_TIME_SECONDS(20L);

// How long Shutdown() waits for in-flight downloads to leave libcurl. Cancellation is not
// instantaneous (see ShutdownProgressCallback), so this must be bounded : blocking forever here
// would turn a shutdown crash into a shutdown ANR.
constexpr auto DRAIN_TIMEOUT = std::chrono::seconds(3);

// Shutdown state.
//
// curl global state is process-wide, so the state guarding it is process-wide too rather than
// per CurlFileDownloader instance.
//
// gShuttingDown is atomic so that the curl callbacks can poll it without taking a lock, but it is
// only ever written while holding gActiveDownloadMutex. That makes "raise the flag, then observe
// the count" and "test the flag, then increment the count" mutually atomic, so a download can
// never slip into libcurl after Shutdown() has decided the process is quiescent.
std::atomic<bool>       gShuttingDown{false};
std::mutex              gActiveDownloadMutex;
std::condition_variable gActiveDownloadCondition;
uint32_t                gActiveDownloadCount{0u};

bool IsShuttingDown()
{
  return gShuttingDown.load(std::memory_order_relaxed);
}

/**
 * @brief RAII guard tracking one synchronous download that is inside libcurl.
 *
 * Construction fails (IsAcquired() returns false) if shutdown has already begun, in which case the
 * caller must not enter libcurl at all.
 */
class ActiveDownloadGuard
{
public:
  ActiveDownloadGuard()
  {
    std::scoped_lock lock(gActiveDownloadMutex);
    if(DALI_LIKELY(!gShuttingDown.load(std::memory_order_relaxed)))
    {
      ++gActiveDownloadCount;
      mAcquired = true;
    }
  }

  ~ActiveDownloadGuard()
  {
    if(mAcquired)
    {
      bool drained = false;
      {
        std::scoped_lock lock(gActiveDownloadMutex);
        drained = (--gActiveDownloadCount == 0u);
      }
      if(drained)
      {
        gActiveDownloadCondition.notify_all();
      }
    }
  }

  bool IsAcquired() const
  {
    return mAcquired;
  }

  ActiveDownloadGuard(const ActiveDownloadGuard&)            = delete;
  ActiveDownloadGuard& operator=(const ActiveDownloadGuard&) = delete;
  ActiveDownloadGuard(ActiveDownloadGuard&&)                 = delete;
  ActiveDownloadGuard& operator=(ActiveDownloadGuard&&)      = delete;

private:
  bool mAcquired{false};
};

/**
 * @brief Block new downloads and wait for in-flight ones to leave libcurl.
 *
 * @return The number of downloads still inside libcurl, so zero if the drain succeeded. A non-zero
 * count tells how badly it failed, which is worth knowing from a field log : one transfer stuck in
 * name resolution is a different problem from every worker being stuck.
 */
uint32_t DrainActiveDownloads()
{
  {
    std::scoped_lock lock(gActiveDownloadMutex);
    gShuttingDown.store(true, std::memory_order_relaxed);
  }

  // In-flight transfers observe the flag from the progress and write callbacks and abort.
  std::unique_lock lock(gActiveDownloadMutex);
  gActiveDownloadCondition.wait_for(lock, DRAIN_TIMEOUT, []()
  { return gActiveDownloadCount == 0u; });
  return gActiveDownloadCount;
}

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
  size_t               maximumSize{0u};
};

struct FixedBufferData
{
  uint8_t* data{nullptr};
  size_t   capacity{0u};
  size_t   written{0u};
};

// Without a write function or a buffer (file descriptor) to write to, curl will pump out
// header/body contents to stdout
/**
 * @brief Progress callback registered only so that a transfer can be aborted.
 *
 * The progress values are ignored. Returning non-zero makes curl_easy_perform() return
 * CURLE_ABORTED_BY_CALLBACK, which is how an in-flight transfer is pulled out of libcurl at
 * shutdown. curl invokes this at least once per second even while no data is arriving, and
 * between the non-blocking steps of a TLS handshake, so a hung connection is still reachable.
 *
 * Reaction is not immediate : the polling interval bounds it, and with a synchronous resolver the
 * name resolution phase is not reachable at all. Hence the bounded wait in DrainActiveDownloads().
 *
 * Must stay cheap - it runs on the transfer thread.
 */
int FUNCTION_CALL_FROM_CURL_PREFIX ShutdownProgressCallback(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
  return IsShuttingDown() ? 1 : 0;
}

size_t FUNCTION_CALL_FROM_CURL_PREFIX DummyWrite(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  // Returning a short count aborts the transfer. While data is flowing this reacts faster than
  // ShutdownProgressCallback, which curl only polls periodically.
  if(DALI_UNLIKELY(IsShuttingDown()))
  {
    return 0u;
  }
  return size * nmemb;
}

size_t FUNCTION_CALL_FROM_CURL_PREFIX ChunkLoader(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  if(DALI_UNLIKELY(IsShuttingDown()))
  {
    return 0u;
  }

  auto* chunk = static_cast<ChunkData*>(userdata);
  if(!chunk || (size != 0u && nmemb > std::numeric_limits<size_t>::max() / size))
  {
    return 0u;
  }

  const size_t numBytes = size * nmemb;
  if(chunk->data.size() > chunk->maximumSize || numBytes > chunk->maximumSize - chunk->data.size())
  {
    return 0u;
  }

  try
  {
    const size_t offset = chunk->data.size();
    chunk->data.resize(offset + numBytes);
    if(numBytes != 0u)
    {
      memcpy(chunk->data.data() + offset, ptr, numBytes);
    }
  }
  catch(...)
  {
    return 0u;
  }
  return numBytes;
}

size_t FUNCTION_CALL_FROM_CURL_PREFIX FixedBufferLoader(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  if(DALI_UNLIKELY(IsShuttingDown()))
  {
    return 0u;
  }

  auto* buffer = static_cast<FixedBufferData*>(userdata);
  if(!buffer || (size != 0u && nmemb > std::numeric_limits<size_t>::max() / size))
  {
    return 0u;
  }

  const size_t numBytes = size * nmemb;
  if(buffer->written > buffer->capacity || numBytes > buffer->capacity - buffer->written)
  {
    // Returning a short count makes curl report CURLE_WRITE_ERROR.  The
    // caller then retries through the bounded chunk path instead of silently
    // truncating a response whose Content-Length was inaccurate.
    return 0u;
  }

  if(numBytes != 0u)
  {
    memcpy(buffer->data + buffer->written, ptr, numBytes);
    buffer->written += numBytes;
  }
  return numBytes;
}

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

  // Make the transfer abortable at shutdown. CURLOPT_NOPROGRESS defaults to 1, in which case the
  // callback is registered but never invoked, so it must be cleared here.
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_XFERINFOFUNCTION, ShutdownProgressCallback), "CURLOPT_XFERINFOFUNCTION");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0L), "CURLOPT_NOPROGRESS");

  // Give up on a stalled connection well before CURLOPT_TIMEOUT would, shortening the window in
  // which a worker thread sits inside libcurl.
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT_BYTES_PER_SECOND), "CURLOPT_LOW_SPEED_LIMIT");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME_SECONDS), "CURLOPT_LOW_SPEED_TIME");

  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L), "CURLOPT_FOLLOWLOCATION");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, maximumRedirectionCounts), "CURLOPT_MAXREDIRS");

#if defined(_WIN32)
  // On Windows, curl uses the schannel (native Windows) TLS backend by default.
  // schannel tries to verify the server certificate's revocation status via CRL/OCSP,
  // which fails with CRYPT_E_NO_REVOCATION_CHECK (0x80092012) when those endpoints are
  // unreachable (e.g. behind a corporate proxy / firewall). Soften the revocation check
  // so a valid certificate whose revocation status simply couldn't be fetched is accepted,
  // while still rejecting certificates that are actually reported as revoked.
#if defined(CURLSSLOPT_REVOKE_BEST_EFFORT)
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_REVOKE_BEST_EFFORT), "CURLOPT_SSL_OPTIONS");
#else
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NO_REVOKE), "CURLOPT_SSL_OPTIONS");
#endif
#endif

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

bool DownloadFileDataWithSize(CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t& dataSize, const std::string& url, char* errorBuffer)
{
  dataBuffer.ResizeUninitialized(dataSize);
  FixedBufferData buffer{dataBuffer.Begin(), dataSize, 0u};

  // We only want the body which contains the file data.
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER), "CURLOPT_HEADER");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOBODY, INCLUDE_BODY), "CURLOPT_NOBODY");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, FixedBufferLoader), "CURLOPT_WRITEFUNCTION");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &buffer), "CURLOPT_WRITEDATA");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_perform(curlHandle), "Failed to download image file with fixed size");

  if(buffer.written != dataSize)
  {
    dataSize = buffer.written;
    dataBuffer.ResizeUninitialized(buffer.written);
  }
  return true;
}

bool DownloadFileDataByChunk(CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t& dataSize, size_t maximumAllowedSizeBytes, const std::string& url, char* errorBuffer)
{
  ChunkData chunk;
  chunk.maximumSize = maximumAllowedSizeBytes;

  // we only want the body which contains the file data
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER), "CURLOPT_HEADER");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_NOBODY, INCLUDE_BODY), "CURLOPT_NOBODY");

  // Enable the write callback.
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, ChunkLoader), "CURLOPT_WRITEFUNCTION");
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &chunk), "CURLOPT_WRITEDATA");

  // synchronous request of the body data
  CHECK_CURL_RESULT_AND_RETURN_FALSE(curl_easy_perform(curlHandle), "Failed to download image file by chunk");

  dataSize = chunk.data.size();
  dataBuffer.ResizeUninitialized(dataSize);

  if(DALI_LIKELY(dataSize > 0))
  {
    memcpy(dataBuffer.Begin(), chunk.data.data(), dataSize);
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
    DALI_LOG_ERROR("[FileDownload][Curl] Fail to set curlopt!\n");
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
    result = DownloadFileDataByChunk(curlHandle, dataBuffer, dataSize, maximumAllowedSizeBytes, url, errorBuffer);
  }
  else if(size == 0)
  {
    DALI_LOG_ERROR("[FileDownload][Curl] File content length is 0 \"%s\"\n", url.c_str());
    result = false;
  }
  else if(size > static_cast<curl_off_t>(maximumAllowedSizeBytes))
  {
    DALI_LOG_ERROR("[FileDownload][Curl] File content length %" CURL_FORMAT_CURL_OFF_T " > max allowed %zu \"%s\" \n", size, maximumAllowedSizeBytes, url.c_str());
    result = false;
  }
  else
  {
    // If we know the size up front, allocate once and avoid chunk copies.
    dataSize = static_cast<size_t>(size);
    result   = DownloadFileDataWithSize(curlHandle, dataBuffer, dataSize, url, errorBuffer);
    // DevNote : A cancelled transfer also reports failure, and FixedBufferLoader returns a short
    // count in both cases, so the curl error code cannot tell the two apart. Test the shutdown
    // flag directly - retrying here would start another transfer just after we asked to stop.
    if(!result && !IsShuttingDown())
    {
      DALI_LOG_DEBUG_INFO("[FileDownload][Curl] Failed to download file, trying to load by chunk. \"%s\"\n", url.c_str());
      // In the case where the size is wrong (e.g. on a proxy server that rewrites data),
      // the data buffer will be corrupt. In this case, try again using the chunk writer.
      result = DownloadFileDataByChunk(curlHandle, dataBuffer, dataSize, maximumAllowedSizeBytes, url, errorBuffer);
    }
  }

  if(DALI_UNLIKELY(result && dataSize == 0u))
  {
    DALI_LOG_ERROR("[FileDownload][Curl] Download data size is 0! url : %s\n", url.c_str());
    result = false;
  }
  return result;
}

} // unnamed namespace

/**
 * Curl library environment. Direct initialize ensures it's constructed before this plugin
 * uses any curl functionality.
 */
class CurlFileDownloader::Impl
{
public:
  Impl()
  : mUserAgent("DALi/" + std::to_string(ADAPTOR_MAJOR_VERSION) + "." + std::to_string(ADAPTOR_MINOR_VERSION) + "." + std::to_string(ADAPTOR_MICRO_VERSION)),
    mProxy(),
    mInitialized(false)
  {
  }

  ~Impl()
  {
    // Reached only through DestroyFileDownloadPlugin(), which by contract runs after the worker
    // threads have been joined, so the process is quiescent with respect to libcurl here.
    Cleanup();
  }

  bool Initialize()
  {
    // Serialize initialization so that curl_global_init() is never entered concurrently,
    // and so that every caller observes a completed initialization.
    std::scoped_lock lock(sImplMutex);
    if(DALI_UNLIKELY(mInitialized))
    {
      return true;
    }

    // curl does not support re-initialization after curl_global_cleanup(). Fail cleanly instead,
    // which is what a download requested after shutdown should do anyway.
    if(DALI_UNLIKELY(mCleanedUp))
    {
      DALI_LOG_ERROR("curl library environment is already cleaned up. Cannot initialize again.\n");
      return false;
    }

    DALI_LOG_DEBUG_INFO("Initializing curl library environment\n");

    // DevNote : curl_global_init() has no thread affinity requirement. What it does require is that
    // it completes before any other libcurl call, and that it is not entered concurrently.
    // sImplMutex serializes it, and callers always run it before curl_easy_init(), so it is safe
    // to run here regardless of which thread we are on.
    const CURLcode initializationResult = curl_global_init(CURL_GLOBAL_ALL);

    if(DALI_UNLIKELY(initializationResult != CURLE_OK))
    {
      DALI_LOG_ERROR("curl_global_init failed with error %d (%s)\n",
                     static_cast<int>(initializationResult),
                     curl_easy_strerror(initializationResult));
      return false;
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

  /**
   * @brief Release the curl library environment.
   *
   * @note The caller must guarantee that no thread is inside libcurl. curl_global_cleanup() is not
   * thread-safe, and tearing down the SSL back-end underneath a transfer is exactly the crash this
   * shutdown path exists to prevent.
   */
  void Cleanup()
  {
    std::scoped_lock lock(sImplMutex);
    if(DALI_UNLIKELY(mInitialized))
    {
      DALI_LOG_DEBUG_INFO("Cleaning up curl library environment\n");
      curl_global_cleanup();
      DALI_LOG_DEBUG_INFO("Cleaning up curl library environment done\n");
      mInitialized = false;
    }
    mCleanedUp = true;
  }

  const std::string& GetUserAgent() const
  {
    return mUserAgent;
  }

  const std::string& GetProxy() const
  {
    return mProxy;
  }

private:
  Impl(const Impl&)            = delete;
  Impl& operator=(const Impl&) = delete;
  Impl(Impl&&)                 = delete;
  Impl& operator=(Impl&&)      = delete;

private:
  const std::string mUserAgent;
  std::string       mProxy;

  bool mInitialized;
  bool mCleanedUp{false}; ///< True once curl_global_cleanup() ran, after which we must not initialize again.

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

bool CurlFileDownloader::Shutdown()
{
  const uint32_t remainingDownloads = DrainActiveDownloads();

  if(DALI_LIKELY(remainingDownloads == 0u))
  {
    // The invariant holds : nothing is inside libcurl, so releasing its global state is safe.
    mImpl->Cleanup();
  }
  else
  {
    // A worker is stuck somewhere cancellation cannot reach it, most likely name resolution.
    // Skip the cleanup rather than pull the SSL back-end out from under it. The process is about
    // to exit anyway, so leaking curl's global state costs nothing; crashing here would not.
    DALI_LOG_ERROR("[FileDownload][Curl] Timed out waiting for %u download(s) to finish. Skipping curl_global_cleanup().\n", remainingDownloads);
  }

  return remainingDownloads == 0u;
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

  // Refuse new work once shutdown has begun, and register this download so that Shutdown() waits
  // for it. Note this must happen before curl_easy_init() : that implicitly calls
  // curl_global_init(), which must never run after curl_global_cleanup().
  ActiveDownloadGuard activeDownloadGuard;
  if(DALI_UNLIKELY(!activeDownloadGuard.IsAcquired()))
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Curl][sync] rejected during shutdown url[%s]\n", url.c_str());
    return false;
  }

  DALI_LOG_RELEASE_INFO("[FileDownload][Curl][sync] start url[%s]\n", url.c_str());

  CURL* curlHandle = curl_easy_init();
  if(curlHandle)
  {
    std::vector<char> errorBuffer(CURL_ERROR_SIZE);
    result = DownloadFile(curlHandle, url, dataBuffer, dataSize, maximumAllowedSizeBytes, mImpl->GetUserAgent(), mImpl->GetProxy(), &errorBuffer[0]);
    DALI_LOG_RELEASE_INFO("[FileDownload][Curl][sync] finish result[%s] url[%s] size[%zu]\n", result ? "true" : "false", url.c_str(), dataSize);

    // clean up session
    curl_easy_cleanup(curlHandle);
  }
  else
  {
    DALI_LOG_RELEASE_INFO("[FileDownload][Curl][sync] finish result[false] url[%s] size[%zu]\n", url.c_str(), dataSize);
  }
  return result;
}

} // namespace Dali::Plugin
