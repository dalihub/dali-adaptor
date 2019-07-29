/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/debug.h>
#include <pthread.h>
#include <curl/curl.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/internal/system/common/file-writer.h>

#ifdef TPK_CURL_ENABLED
#include <tpkp_curl.h>
#endif // TPK_CURL_ENABLED

using namespace Dali::Integration;

namespace Dali
{

namespace TizenPlatform
{

namespace // unnamed namespace
{

const int CONNECTION_TIMEOUT_SECONDS( 30L );
const int TIMEOUT_SECONDS( 120L );
const long VERBOSE_MODE = 0L;                // 0 == off, 1 == on
const long CLOSE_CONNECTION_ON_ERROR = 1L;   // 0 == off, 1 == on
const long EXCLUDE_HEADER = 0L;
const long INCLUDE_HEADER = 1L;
const long INCLUDE_BODY = 0L;
const long EXCLUDE_BODY = 1L;

/**
 * Curl library environment. Direct initialize ensures it's constructed before adaptor
 * or application creates any threads.
 */
static Dali::TizenPlatform::Network::CurlEnvironment gCurlEnvironment;

void ConfigureCurlOptions( CURL* curlHandle, const std::string& url )
{
  curl_easy_setopt( curlHandle, CURLOPT_URL, url.c_str() );
  curl_easy_setopt( curlHandle, CURLOPT_VERBOSE, VERBOSE_MODE );

  // CURLOPT_FAILONERROR is not fail-safe especially when authentication is involved ( see manual )
  // Removed CURLOPT_FAILONERROR option
  curl_easy_setopt( curlHandle, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT_SECONDS );
  curl_easy_setopt( curlHandle, CURLOPT_TIMEOUT, TIMEOUT_SECONDS );
  curl_easy_setopt( curlHandle, CURLOPT_HEADER, INCLUDE_HEADER );
  curl_easy_setopt( curlHandle, CURLOPT_NOBODY, EXCLUDE_BODY );

#ifdef TPK_CURL_ENABLED
  // Apply certificate pinning on Tizen
  curl_easy_setopt( curlHandle, CURLOPT_SSL_CTX_FUNCTION, tpkp_curl_ssl_ctx_callback );
#endif // TPK_CURL_ENABLED
}

// Without a write function or a buffer (file descriptor) to write to, curl will pump out
// header/body contents to stdout
size_t DummyWrite(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  return size * nmemb;
}

struct ChunkData
{
  std::vector< uint8_t > data;
};

size_t ChunkLoader(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  std::vector<ChunkData>* chunks = static_cast<std::vector<ChunkData>*>( userdata );
  int numBytes = size*nmemb;
  chunks->push_back( ChunkData() );
  ChunkData& chunkData = (*chunks)[chunks->size()-1];
  chunkData.data.reserve( numBytes );
  memcpy( &chunkData.data[0], ptr, numBytes );
  return numBytes;
}


CURLcode DownloadFileDataWithSize( CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t dataSize )
{
  CURLcode result( CURLE_OK );

  // create
  Dali::Internal::Platform::FileWriter fileWriter( dataBuffer, dataSize );
  FILE* dataBufferFilePointer = fileWriter.GetFile();
  if( NULL != dataBufferFilePointer )
  {
    // we only want the body which contains the file data
    curl_easy_setopt( curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER );
    curl_easy_setopt( curlHandle, CURLOPT_NOBODY, INCLUDE_BODY );

    // disable the write callback, and get curl to write directly into our data buffer
    curl_easy_setopt( curlHandle, CURLOPT_WRITEFUNCTION, NULL );
    curl_easy_setopt( curlHandle, CURLOPT_WRITEDATA, dataBufferFilePointer );

    // synchronous request of the body data
    result = curl_easy_perform( curlHandle );
  }
  return result;
}

CURLcode DownloadFileDataByChunk( CURL* curlHandle, Dali::Vector<uint8_t>& dataBuffer, size_t& dataSize )
{
  // create
  std::vector< ChunkData > chunks;

  // we only want the body which contains the file data
  curl_easy_setopt( curlHandle, CURLOPT_HEADER, EXCLUDE_HEADER );
  curl_easy_setopt( curlHandle, CURLOPT_NOBODY, INCLUDE_BODY );

  // Enable the write callback.
  curl_easy_setopt( curlHandle, CURLOPT_WRITEFUNCTION, ChunkLoader );
  curl_easy_setopt( curlHandle, CURLOPT_WRITEDATA, &chunks );

  // synchronous request of the body data
  CURLcode result = curl_easy_perform( curlHandle );

  // chunks should now contain all of the chunked data. Reassemble into a single vector
  dataSize = 0;
  for( size_t i=0; i<chunks.size() ; ++i )
  {
    dataSize += chunks[i].data.capacity();
  }
  dataBuffer.Resize(dataSize);

  size_t offset = 0;
  for( size_t i=0; i<chunks.size() ; ++i )
  {
    memcpy( &dataBuffer[offset], &chunks[i].data[0], chunks[i].data.capacity() );
    offset += chunks[i].data.capacity();
  }

  return result;
}

bool DownloadFile( CURL* curlHandle,
                   const std::string& url,
                   Dali::Vector<uint8_t>& dataBuffer,
                   size_t& dataSize,
                   size_t maximumAllowedSizeBytes )
{
  CURLcode result( CURLE_OK );
  double size(0);

  // setup curl to download just the header so we can extract the content length
  ConfigureCurlOptions( curlHandle, url );

  curl_easy_setopt( curlHandle, CURLOPT_WRITEFUNCTION, DummyWrite);

  // perform the request to get the header
  result = curl_easy_perform( curlHandle );

  if( result != CURLE_OK)
  {
    DALI_LOG_ERROR( "Failed to download http header for \"%s\" with error code %d\n", url.c_str(), result );
    return false;
  }

  // get the content length, -1 == size is not known
  curl_easy_getinfo( curlHandle,CURLINFO_CONTENT_LENGTH_DOWNLOAD , &size );


  if( size >= maximumAllowedSizeBytes )
  {
    DALI_LOG_ERROR( "File content length %f > max allowed %zu \"%s\" \n", size, maximumAllowedSizeBytes, url.c_str() );
    return false;
  }
  else if( size > 0 )
  {
    // If we know the size up front, allocate once and avoid chunk copies.
    dataSize = static_cast<size_t>( size );
    result = DownloadFileDataWithSize( curlHandle, dataBuffer, dataSize );
  }
  else
  {
    result = DownloadFileDataByChunk( curlHandle, dataBuffer, dataSize );
  }

  if( result != CURLE_OK )
  {
    DALI_LOG_ERROR( "Failed to download image file \"%s\" with error code %d\n", url.c_str(), result );
    return false;
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

bool DownloadRemoteFileIntoMemory( const std::string& url,
                                   Dali::Vector<uint8_t>& dataBuffer,
                                   size_t& dataSize,
                                   size_t maximumAllowedSizeBytes )
{
  bool result = false;

  if( url.empty() )
  {
    DALI_LOG_WARNING("empty url requested \n");
    return false;
  }

  // start a libcurl easy session, this internally calls curl_global_init, if we ever have more than one download
  // thread we need to explicity call curl_global_init() on startup from a single thread.

  CURL* curlHandle = curl_easy_init();
  if ( curlHandle )
  {
    result = DownloadFile( curlHandle, url, dataBuffer,  dataSize, maximumAllowedSizeBytes);

    // clean up session
    curl_easy_cleanup( curlHandle );

#ifdef TPK_CURL_ENABLED
    // Clean up tpkp(the module for certificate pinning) resources on Tizen
    tpkp_curl_cleanup();
#endif // TPK_CURL_ENABLED
  }
  return result;
}

} // namespace Network

} // namespace TizenPlatform

} // namespace Dali
