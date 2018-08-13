/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <string>
#include <curl/curl.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/file-download.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/file-closer.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{

namespace TizenPlatform
{

const int CONNECTION_TIMEOUT_SECONDS( 30L );
const long VERBOSE_MODE = 0L;                // 0 == off, 1 == on
const long CLOSE_CONNECTION_ON_ERROR = 1L;   // 0 == off, 1 == on
const long EXCLUDE_HEADER = 0L;
const long INCLUDE_HEADER = 1L;
const long INCLUDE_BODY = 0L;
const long EXCLUDE_BODY = 1L;

namespace Network
{

unsigned long CurlEnvironment::GetThreadId()
{
  // If dali uses c++ thread, we may replace pthread_self() to this_thread::get_id()
  return Internal::Adaptor::WindowsPlatformImplement::GetCurrentThreadId();
}

void CurlEnvironment::ConfigureCurlOptions( void* curlHandle, const std::string& url )
{
  curl_easy_setopt( curlHandle, CURLOPT_URL, url.c_str() );
  //curl_easy_setopt( curlHandle, CURLOPT_VERBOSE, VERBOSE_MODE );
  curl_easy_setopt( curlHandle, CURLOPT_PROXY, "109.123.100.31:3128" );

  // CURLOPT_FAILONERROR is not fail-safe especially when authentication is involved ( see manual )
  // Removed CURLOPT_FAILONERROR option
  curl_easy_setopt( curlHandle, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT_SECONDS );
  curl_easy_setopt( curlHandle, CURLOPT_HEADER, INCLUDE_HEADER );
  curl_easy_setopt( curlHandle, CURLOPT_NOBODY, EXCLUDE_BODY );
}

static size_t WriteFunction( void *input, size_t uSize, size_t uCount, void *avg )
{
  Internal::Platform::InternalFile::fwrite( input, uSize, uCount, (FILE*)avg );
  return uSize * uCount;
}

void CurlEnvironment::InitWriteFunction( void* curlHandle )
{
  curl_easy_setopt( curlHandle, CURLOPT_WRITEFUNCTION, WriteFunction );
}
}
}
}