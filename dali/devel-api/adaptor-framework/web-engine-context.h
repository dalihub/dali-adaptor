#ifndef DALI_WEB_ENGINE_CONTEXT_H
#define DALI_WEB_ENGINE_CONTEXT_H

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

namespace Dali
{
/**
 * @brief A class WebEngineContext for context of web engine.
 */
class WebEngineContext
{
public:
  /**
   * @brief Enumeration for cache model options.
   */
  enum class CacheModel
  {
    DOCUMENT_VIEWER,     ///< Use the smallest cache capacity.
    DOCUMENT_BROWSER,    ///< Use the bigger cache capacity than DocumentBrowser.
    PRIMARY_WEB_BROWSER, ///< Use the biggest cache capacity.
  };

  /**
   * @brief Constructor.
   */
  WebEngineContext() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineContext() = default;

  /**
   * @brief Returns the cache model type.
   * @return #CacheModel
   */
  virtual CacheModel GetCacheModel() const = 0;

  /**
   * @brief Requests to set the cache model.
   * @param[in] cacheModel The cache model
   */
  virtual void SetCacheModel(CacheModel cacheModel) = 0;

  /**
   * @brief Sets the given proxy URI to network backend of specific context.
   * @param[in] uri The proxy URI to set
   */
  virtual void SetProxyUri(const std::string& uri) = 0;

  /**
   * @brief Sets a proxy auth credential to network backend of specific context.
   * @details Normally, proxy auth credential should be got from the callback
   *          set by ewk_view_authentication_callback_set, once the username in
   *          this API has been set with a non-null value, the authentication
   *          callback will never been invoked. Try to avoid using this API.
   * @param[in] username username to set
   * @param[in] password password to set
   */
  virtual void SetDefaultProxyAuth(const std::string& username, const std::string& password) = 0;

  /**
   * Adds CA certificates to persistent NSS certificate database
   * Function accepts a path to a CA certificate file, a path to a directory
   * containing CA certificate files, or a colon-seprarated list of those.
   * Certificate files should have *.crt extension.
   * Directories are traversed recursively.
   * @param[in] certificatePath path to a CA certificate file(s), see above for details
   */
  virtual void SetCertificateFilePath(const std::string& certificatePath) = 0;

  /**
   * Requests for deleting all web databases.
   */
  virtual void DeleteWebDatabase() = 0;

  /**
   * @brief Deletes web storage.
   * @details This function does not ensure that all data will be removed.
   *          Should be used to extend free physical memory.
   */
  virtual void DeleteWebStorage() = 0;

  /**
   * @brief Requests for deleting all local file systems.
   */
  virtual void DeleteLocalFileSystem() = 0;

  /**
   * Toggles the cache to be enabled or disabled
   * Function works asynchronously.
   * By default the cache is disabled resulting in not storing network data on disk.
   * @param[in] cacheDisabled enable or disable cache
   */
  virtual void DisableCache(bool cacheDisabled) = 0;

  /**
   * @brief Requests to clear cache
   */
  virtual void ClearCache() = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_CONTEXT_H
