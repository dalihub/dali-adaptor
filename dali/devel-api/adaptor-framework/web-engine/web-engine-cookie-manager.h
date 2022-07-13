#ifndef DALI_WEB_ENGINE_COOKIE_MANAGER_H
#define DALI_WEB_ENGINE_COOKIE_MANAGER_H

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
#include <functional>
#include <string>

namespace Dali
{
/**
 * @brief A class WebEngineCookieManager for cookie manager of web engine.
 */
class WebEngineCookieManager
{
public:
  /**
   * @brief Enumeration for the cookies accept policies.
   */
  enum class CookieAcceptPolicy
  {
    ALWAYS,         ///< Accepts every cookie sent from any page.
    NEVER,          ///< Rejects all the cookies.
    NO_THIRD_PARTY, ///< Accepts only cookies set by the main document that is loaded.
  };

  /**
   * @brief Enumeration for the cookie persistent storage type.
   */
  enum class CookiePersistentStorage
  {
    TEXT,   ///< Cookies are stored in a text file in the Mozilla "cookies.txt" format.
    SQLITE, ///< Cookies are stored in a SQLite file in the current Mozilla format.
  };

  /**
   * @brief Callback for changing watch.
   */
  using WebEngineCookieManagerChangesWatchCallback = std::function<void()>;

  /**
   * @brief Constructor.
   */
  WebEngineCookieManager() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineCookieManager() = default;

  /**
   * @brief Sets @a policy as the cookie acceptance policy for @a manager.
   * @details By default, only cookies set by the main document loaded are
   *          accepted.
   *
   * @param[in] policy A #Dali::WebEngineCookieManager::CookieAcceptPolicy
   */
  virtual void SetCookieAcceptPolicy(CookieAcceptPolicy policy) = 0;

  /**
   * @brief Gets the cookie acceptance policy.
   * The default is Toolkit::WebEngineCookieManager::CookieAcceptPolicy::NO_THIRD_PARTY.
   * @see Toolkit::WebEngineCookieManager::CookieAcceptPolicy::Type
   */
  virtual CookieAcceptPolicy GetCookieAcceptPolicy() const = 0;

  /**
   * @brief Deletes all the cookies of @a manager.
   */
  virtual void ClearCookies() = 0;

  /**
   * @brief Sets the @a path where non-session cookies are stored persistently using
   *        @a storage as the format to read/write the cookies.
   * @details Cookies are initially read from @a path/Cookies to create an initial
   *          set of cookies. Then, non-session cookies will be written to @a path/Cookies.
   *          By default, @a manager doesn't store the cookies persistently, so you need to
   *          call this method to keep cookies saved across sessions.
   *          If @a path does not exist it will be created.
   * @param[in] path The path where to read/write Cookies
   * @param[in] storage The type of storage
   */
  virtual void SetPersistentStorage(const std::string& path, CookiePersistentStorage storage) = 0;

  /**
   * @brief Watch for cookies' changes in @a manager.
   *
   * @param[in] callback function that will be called every time cookies are added, removed or modified.
   */
  virtual void ChangesWatch(WebEngineCookieManagerChangesWatchCallback callback) = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_COOKIE_MANAGER_H
