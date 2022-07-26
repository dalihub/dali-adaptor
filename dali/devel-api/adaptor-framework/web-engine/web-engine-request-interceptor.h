#ifndef DALI_WEB_ENGINE_REQUEST_INTERCEPTOR_H
#define DALI_WEB_ENGINE_REQUEST_INTERCEPTOR_H

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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/property-map.h>
#include <dali/public-api/object/ref-object.h>

#include <string>

namespace Dali
{
/**
 * @brief A class WebEngineRequestInterceptor for intercepting http request.
 */
class WebEngineRequestInterceptor : public RefObject
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
   * @brief Gets web engine in which http request is intercepted.
   *
   * @return web engine if succeeded or null otherwise
   */
  virtual Dali::WebEnginePlugin* GetWebEngine() const = 0;

  /**
   * @brief Returns request url.
   *
   * @return url if succeeded or empty otherwise
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Returns http headers.
   *
   * @return headers if succeeded or empty otherwise
   */
  virtual Dali::Property::Map GetHeaders() const = 0;

  /**
   * @brief Returns http method.
   *
   * @return method if succeeded or empty otherwise
   */
  virtual std::string GetMethod() const = 0;

  /**
   * @brief Ignores request.
   * @note After this call, any further calls result in undefined behavior.
   *       This function can be called only INSIDE Dali::WebEngineContext::WebEngineRequestInterceptedCallback.
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool Ignore() = 0;

  /**
   * @brief Sets status code and status text of response for intercepted request.
   *
   * @param[in] statusCode Status code of response
   * @param[in] customStatusText Status code of response
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool SetResponseStatus(int statusCode, const std::string& customStatusText) = 0;

  /**
   * @brief Adds HTTP header to response for intercepted request.
   *
   * @param[in] fieldName Key of response header
   * @param[in] fieldValue Value of response header
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool AddResponseHeader(const std::string& fieldName, const std::string& fieldValue) = 0;

  /**
   * @brief Adds HTTP headers to response.
   *
   * @param[in] headers Headers of response
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool AddResponseHeaders(const Dali::Property::Map& headers) = 0;

  /**
   * @brief Writes whole response body at once.
   *
   * @param[in] body Contents of response
   * @param[in] length Length of contents of response
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool AddResponseBody(const int8_t* body, uint32_t length) = 0;

  /**
   * @brief Writes whole response at once.
   *
   * @param[in] headers Headers of response
   * @param[in] body Contents of response
   * @param[in] length Length of contents of response
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool AddResponse(const std::string& headers, const int8_t* body, uint32_t length) = 0;

  /**
   * @brief Writes a part of response body.
   * @note If this function returns false, handling the request is done.
   *       Any further calls result in undefined behavior.
   *       User should always check return value, because response to this request might not be needed any more,
   *       and function can return false even though user still has data to write.
   *       This function can be called only OUTSIDE Dali::WebEngineContext::WebEngineRequestInterceptedCallback.
   *
   * @param[in] chunk Chunks of response
   * @param[in] length Length of chunks of response
   *
   * @return true if succeeded or false otherwise
   */
  virtual bool WriteResponseChunk(const int8_t* chunk, uint32_t length) = 0;
};

using WebEngineRequestInterceptorPtr = Dali::IntrusivePtr<WebEngineRequestInterceptor>;

} // namespace Dali

#endif // DALI_WEB_ENGINE_REQUEST_INTERCEPTOR_H
