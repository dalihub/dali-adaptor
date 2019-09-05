#ifndef DALI_TIZEN_PLATFORM_NETWORK_FILE_DOWNLOAD_H
#define DALI_TIZEN_PLATFORM_NETWORK_FILE_DOWNLOAD_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-vector.h>
#include <string>
#include <mutex> //c++11
#include <stdint.h> // uint8

namespace Dali
{

namespace TizenPlatform
{

namespace Network
{

/**
 * Set up the cURL environment - this will ensure curl_global_init is called on startup
 * and curl_global_cleanup is called on shutdown.
 * Having this environment enables curl to be used in a single or multi-threaded
 * program safely.
 */
class CurlEnvironment
{
public:
  /**
   * Constructor calls curl_global_init()
   */
  CurlEnvironment();

  /**
   * Destructor calls curl_global_cleanup()
   */
  ~CurlEnvironment();

  // Moveable but not copyable

  CurlEnvironment( const CurlEnvironment& ) = delete;
  CurlEnvironment& operator=( const CurlEnvironment& ) = delete;
  CurlEnvironment( CurlEnvironment&& ) = default;
  CurlEnvironment& operator=( CurlEnvironment&& ) = default;
};


/**
 * Download a requested file into a memory buffer.
 *
 * @note Threading notes: This function can be called from multiple threads, however
 * we must explicitly call curl_global_init() from a single thread before using curl
 * as the global function calls are not thread safe.
 *
 * @param[in] url The requested file url
 * @param[out] dataBuffer  A memory buffer object to be written with downloaded file data.
 * @param[out] dataSize  The size of the memory buffer.
 * @param[in] maximumAllowedSize The maxmimum allowed file size in bytes to download. E.g. for an Image file this may be 50 MB
 * @return true on success, false on failure
 *
 */
bool DownloadRemoteFileIntoMemory( const std::string& url,
                                   Dali::Vector<uint8_t>& dataBuffer,
                                   size_t& dataSize,
                                   size_t maximumAllowedSizeBytes );

} // namespace Network

} // namespace TizenPlatform

} // namespace Dali

#endif // DALI_TIZEN_PLATFORM_RESOURCE_THREAD_IMAGE_H
