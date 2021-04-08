#ifndef DALI_WEB_ENGINE_REQUEST_INTERCEPTOR_H
#define DALI_WEB_ENGINE_REQUEST_INTERCEPTOR_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
 * @brief A class WebEngineRequestInterceptor for intercepting http request.
 */
class WebEngineRequestInterceptor
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineRequestInterceptor() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineRequestInterceptor() = default;

  /**
   * @brief Returns request url.
   *
   * @return url on success or empty on failure
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Ignores request.
   *
   * @return true on success or false on failure
   */
  virtual bool Ignore() = 0;

  /**
   * @brief Sets status code and status text of response for intercepted request.
   *
   * @param[in] statusCode Status code of response
   * @param[in] customStatusText Status code of response
   *
   * @return true if succeeded or false if failed
   */
  virtual bool SetResponseStatus(int statusCode, const std::string& customStatusText) = 0;

  /**
   * @brief Adds HTTP header to response for intercepted request.
   *
   * @param[in] fieldName Key of response header
   * @param[in] fieldValue Value of response header
   *
   * @return true if succeeded or false if failed
   */
  virtual bool AddResponseHeader(const std::string& fieldName, const std::string& fieldValue) = 0;

  /**
   * @brief Writes whole response body at once.
   *
   * @param[in] body Contents of response
   * @param[in] length Length of Contents of response
   *
   * @return true if succeeded or false if failed
   */
  virtual bool AddResponseBody(const std::string& body, uint32_t length) = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_REQUEST_INTERCEPTOR_H
