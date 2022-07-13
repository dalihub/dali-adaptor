#ifndef DALI_WEB_ENGINE_HTTP_AUTH_HANDLER_H
#define DALI_WEB_ENGINE_HTTP_AUTH_HANDLER_H

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
 * @brief A class WebEngineHttpAuthHandler for HTTP authentication handler of web engine.
 */
class WebEngineHttpAuthHandler
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineHttpAuthHandler() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineHttpAuthHandler() = default;

  /**
   * @brief Gets the realm string of authentication challenge received.
   *
   * @return the realm of authentication challenge
   */
  virtual std::string GetRealm() const = 0;

  /**
   * @brief Suspend the operation for authentication challenge.
   */
  virtual void Suspend() = 0;

  /**
   * @brief Send credential for authentication challenge.
   *
   * @param[in] user user id from user input.
   * @param[in] password user password from user input.
   */
  virtual void UseCredential(const std::string& user, const std::string& password) = 0;

  /**
   * @brief Send cancellation notification for authentication challenge.
   */
  virtual void CancelCredential() = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_HTTP_AUTH_HANDLER_H
