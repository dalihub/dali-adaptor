#ifndef DALI_WEB_ENGINE_LOAD_ERROR_H
#define DALI_WEB_ENGINE_LOAD_ERROR_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

namespace Dali
{
/**
 * @brief A class WebEngineLoadError for loading error of web engine.
 */
class WebEngineLoadError
{
public:
  /**
   * @brief Enumeration for indicating error code of page loading.
   */
  enum class ErrorCode
  {
    UNKNOWN = 0,           ///< Unknown.
    CANCELED,              ///< User canceled.
    CANT_SUPPORT_MIMETYPE, ///< Can't show the page for this MIME type.
    FAILED_FILE_IO,        ///< File IO error.
    CANT_CONNECT,          ///< Cannot connect to the network.
    CANT_LOOKUP_HOST,      ///< Fail to look up host from the DNS.
    FAILED_TLS_HANDSHAKE,  ///< Fail to SSL/TLS handshake.
    INVALID_CERTIFICATE,   ///< Received certificate is invalid.
    REQUEST_TIMEOUT,       ///< Connection timeout.
    TOO_MANY_REDIRECTS,    ///< Too many redirects.
    TOO_MANY_REQUESTS,     ///< Too many requests during this load.
    BAD_URL,               ///< Malformed URL.
    UNSUPPORTED_SCHEME,    ///< Unsupported scheme.
    AUTHENTICATION,        ///< User authentication failed on the server.
    INTERNAL_SERVER,       ///< Web server has an internal server error.
    OTHER,                 ///< other error.
  };

  /**
   * @brief Enumeration that provides type of loading error.
   */
  enum class ErrorType
  {
    NONE,
    INTERNAL,
    NETWORK,
    POLICY,
    PLUGIN,
    DOWNLOAD,
    PRINT,
  };

  /**
   * @brief Constructor.
   */
  WebEngineLoadError() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineLoadError() = default;

  /**
   * @brief Query failing URL for this error.
   * @return URL that failed to load
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Query the error code.
   * @return The error code
   */
  virtual ErrorCode GetCode() const = 0;

  /**
   * @brief Query description for this error.
   * @return SeverityLevel indicating the console message level
   */
  virtual std::string GetDescription() const = 0;

  /**
   * @brief Query type for this error.
   * @return the error type
   */
  virtual ErrorType GetType() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_LOAD_ERROR_H
