/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/common/file-download.h>

// EXTERNAL INCLUDES
#include <curl/curl.h>
#include <dali/integration-api/debug.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/file-writer.h>
#include <dali/public-api/common/dali-common.h>

using namespace Dali::Integration;

namespace Dali
{
namespace TizenPlatform
{
namespace // unnamed namespace
{
const char* HTTP_PROXY_ENV = "http_proxy";

inline void LogCurlResult(CURLcode result, char* errorBuffer, std::string url, std::string prefix)
{
  if(result != CURLE_OK)
  {
    if(errorBuffer != nullptr)
    {
      DALI_LOG_ERROR("%s \"%s\" with error code %d\n", prefix.c_str(), url.c_str(), result);
    }
    else
    {
      DALI_LOG_ERROR("%s \"%s\" with error code %d (%s)\n", prefix.c_str(), url.c_str(), result, errorBuffer);
    }
  }
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

int CurloptVerboseLogTrace(CURL* handle, curl_infotype type, char* data, size_t size, void* clientp)
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

    // TODO : Until resolve some thread issue, let we don't mark it as true.
    // mean, always ask from environment every time.
    // verboseModeSetted = true;
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

    // TODO : Until resolve some thread issue, let we don't mark it as true.
    // mean, always ask from environment every time.
    // maxiumumRedirectionCountSetted = true;
  }

  return maxiumumRedirectionCount;
}

/**
 * Curl library environment. Direct initialize ensures it's constructed before adaptor
 * or application creates any threads.
 */
static Dali::TizenPlatform::Network::CurlEnvironment gCurlEnvironment;

void ConfigureCurlOptions(CURL* curlHandle, const std::string& url)
{
  const auto verboseMode = GetCurloptVerboseMode(); // 0 : off, 1 : on

  const long maximumRedirectionCounts = GetCurloptMaximumRedirectionCount(); // 5 for default

  curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, verboseMode);

  // CURLOPT_FAILONERROR is not fail-safe especially when authentication is involved ( see manual )
  // Removed CURLOPT_FAILONERROR option
  curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT_SECONDS);
  curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);
  curl_easy_setopt(curlHandle, CURLOPT_HEADER, INCLUDE_HEADER);
  curl_easy_setopt(curlHandle, CURLOPT_NOBODY, EXCLUDE_BODY);
  curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, maximumRedirectionCounts);

  if(verboseMode != 0)
  {
    curl_easy_setopt(curlHandle, CURLOPT_DEBUGFUNCTION, CurloptVerboseLogTrace);
  }

  // If the proxy variable is set, ensure it's also used.
  // In theory, this variable should be used by the curl library; however, something
  // is overriding it.
  const char* proxy = Dali::EnvironmentVariable::GetEnvironmentVariable(HTTP_PROXY_ENV);
  if(proxy != nullptr)
  {
    curl_easy_setopt(curlHandle, CURLOPT_PROXY, proxy);
  }
}

// Without a write function or a buffer (file descriptor) to write to, curl will pump out
// header/body contents to stdout
size_t DummyWrite(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  return size * nmemb;
}

struct ChunkData
{
  std::vector<uint8_t> data;
};

size_t ChunkLoader(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::vector<ChunkData>* chunks   = static_cast<std::vector<ChunkData>*>(userdata);
  int                     numBytes = size * nmemb;
  chunks->push_back(ChunkData());
  ChunkData& chunkData = (*chunks)[chunks->size() - 1];
  chunkData.data.reserve(numBytes);
  memcpy(&chunkData.data[0], ptr, numBytes);
  return numBytes;
}

CURLcode DownloadFileDataWithSize(CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t dataSize)
{
  CURLcode result(CURLE_OK);

  // create
  Dali::Internal::Platform::FileWriter fileWriter(dataBuffer, dataSize);
  FILE*                                dataBufferFilePointer = fileWriter.GetFile();

  if(NULL != dataBufferFilePointer)
  {
    setbuf(dataBufferFilePointer, NULL); // Turn buffering off

    // we only want the body which contains the file data
    curl_easy_setopt(curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER);
    curl_easy_setopt(curlHandle, CURLOPT_NOBODY, INCLUDE_BODY);

    // disable the write callback, and get curl to write directly into our data buffer
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, dataBufferFilePointer);

    // synchronous request of the body data
    result = curl_easy_perform(curlHandle);
  }
  else
  {
    DALI_LOG_ERROR("Fail to open buffer writter with size : %zu!\n", dataSize);
    // @todo : Need to check that is it correct error code?
    result = CURLE_READ_ERROR;
  }
  return result;
}

CURLcode DownloadFileDataByChunk(CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t& dataSize)
{
  // create
  std::vector<ChunkData> chunks;

  // we only want the body which contains the file data
  curl_easy_setopt(curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER);
  curl_easy_setopt(curlHandle, CURLOPT_NOBODY, INCLUDE_BODY);

  // Enable the write callback.
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, ChunkLoader);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &chunks);

  // synchronous request of the body data
  CURLcode result = curl_easy_perform(curlHandle);

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

  return result;
}

bool DownloadFile(CURL*                  curlHandle,
                  const std::string&     url,
                  Dali::Vector<uint8_t>& dataBuffer,
                  size_t&                dataSize,
                  size_t                 maximumAllowedSizeBytes,
                  char*                  errorBuffer)
{
  CURLcode result(CURLE_OK);

  // setup curl to download just the header so we can extract the content length
  ConfigureCurlOptions(curlHandle, url);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, DummyWrite);
  if(errorBuffer != nullptr)
  {
    errorBuffer[0] = 0;
  }

  // perform the request to get the header
  result = curl_easy_perform(curlHandle);

  if(result != CURLE_OK)
  {
    if(errorBuffer != nullptr)
    {
      DALI_LOG_ERROR("Failed to download http header for \"%s\" with error code %d (%s)\n", url.c_str(), result, &errorBuffer[0]);
    }
    else
    {
      DALI_LOG_ERROR("Failed to download http header for \"%s\" with error code %d\n", url.c_str(), result);
    }
    return false;
  }

  // get the content length, -1 == size is not known
  curl_off_t size{0};
  curl_easy_getinfo(curlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);

  if(size == -1)
  {
    result = DownloadFileDataByChunk(curlHandle, dataBuffer, dataSize);
  }
  else if(size >= static_cast<curl_off_t>(maximumAllowedSizeBytes))
  {
    DALI_LOG_ERROR("File content length %" CURL_FORMAT_CURL_OFF_T " > max allowed %zu \"%s\" \n", size, maximumAllowedSizeBytes, url.c_str());
    return false;
  }
  else
  {
    // If we know the size up front, allocate once and avoid chunk copies.
    dataSize = static_cast<size_t>(size);
    result   = DownloadFileDataWithSize(curlHandle, dataBuffer, dataSize);
    if(result != CURLE_OK)
    {
      LogCurlResult(result, errorBuffer, url, "Failed to download file, trying to load by chunk");
      // In the case where the size is wrong (e.g. on a proxy server that rewrites data),
      // the data buffer will be corrupt. In this case, try again using the chunk writer.
      result = DownloadFileDataByChunk(curlHandle, dataBuffer, dataSize);
    }
  }

  LogCurlResult(result, errorBuffer, url, "Failed to download image file");

  if(result != CURLE_OK)
  {
    return false;
  }
  else if(DALI_UNLIKELY(dataSize == 0u))
  {
    DALI_LOG_WARNING("Warning : Download data size is 0! url : %s\n", url.c_str());
  }
  return true;
}

} // unnamed namespace

namespace Network
{
CurlEnvironment::CurlEnvironment()
{
  // Must be called before we attempt any loads. e.g. by using curl_easy_init()
  // and before we start any threads.
  curl_global_init(CURL_GLOBAL_ALL);
}

CurlEnvironment::~CurlEnvironment()
{
  curl_global_cleanup();
}

bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                  Dali::Vector<uint8_t>& dataBuffer,
                                  size_t&                dataSize,
                                  size_t                 maximumAllowedSizeBytes)
{
  bool result = false;

  if(url.empty())
  {
    DALI_LOG_WARNING("empty url requested \n");
    return false;
  }

  // start a libcurl easy session, this internally calls curl_global_init, if we ever have more than one download
  // thread we need to explicity call curl_global_init() on startup from a single thread.

  CURL* curlHandle = curl_easy_init();
  if(curlHandle)
  {
    std::vector<char> errorBuffer(CURL_ERROR_SIZE);
    curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, &errorBuffer[0]);
    result = DownloadFile(curlHandle, url, dataBuffer, dataSize, maximumAllowedSizeBytes, &errorBuffer[0]);

    // clean up session
    curl_easy_cleanup(curlHandle);
  }
  return result;
}

} // namespace Network

} // namespace TizenPlatform

} // namespace Dali
