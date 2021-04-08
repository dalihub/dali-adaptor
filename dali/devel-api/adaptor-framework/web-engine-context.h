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
#include <functional>
#include <memory>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine-security-origin.h>

namespace Dali
{
/**
 * @brief A class WebEngineContext for context of web engine.
 */
class WebEngineContext
{
public:
  /**
   * @brief Callback for getting web database origins.
   */
  using WebEngineSecurityOriginAcquiredCallback = std::function<void(std::vector<std::unique_ptr<Dali::WebEngineSecurityOrigin>>&)>;

  /**
   * @brief Callback for getting web storage usage.
   */
  using WebEngineStorageUsageAcquiredCallback = std::function<void(uint64_t)>;

  // forward declaration.
  struct PasswordData;

  /**
   * @brief Callback for getting form password.
   */
  using WebEngineFormPasswordAcquiredCallback = std::function<void(std::vector<std::unique_ptr<PasswordData>>&)>;

  /**
   * @brief Callback for download started with url.
   */
  using WebEngineDownloadStartedCallback = std::function<void(const std::string&)>;

  /**
   * @brief Callback for overriding default mime type.
   * @param[in] url for which the mime type can be overridden
   * @param[in] current mime type that will be overridden
   * @param[out] a new mime type for web engine.
   * @return true if mime should be overridden by new mime, false otherwise.
   */
  using WebEngineMimeOverriddenCallback = std::function<bool(const std::string&, const std::string&, std::string&)>;

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
   * @brief Struct for password data
   */
  struct PasswordData
  {
    std::string url;
    bool        useFingerprint;
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
   * @brief Return the cache model type.
   * @return #CacheModel
   */
  virtual CacheModel GetCacheModel() const = 0;

  /**
   * @brief Request to set the cache model.
   * @param[in] cacheModel The cache model
   */
  virtual void SetCacheModel(CacheModel cacheModel) = 0;

  /**
   * @brief Set the given proxy URI to network backend of specific context.
   * @param[in] uri The proxy URI to set
   */
  virtual void SetProxyUri(const std::string& uri) = 0;

  /**
   * @brief Set a proxy auth credential to network backend of specific context.
   * @details Normally, proxy auth credential should be got from the callback
   *          set by ewk_view_authentication_callback_set, once the username in
   *          this API has been set with a non-null value, the authentication
   *          callback will never been invoked. Try to avoid using this API.
   * @param[in] username username to set
   * @param[in] password password to set
   */
  virtual void SetDefaultProxyAuth(const std::string& username, const std::string& password) = 0;

  /**
   * @brief Add CA certificates to persistent NSS certificate database
   * Function accepts a path to a CA certificate file, a path to a directory
   * containing CA certificate files, or a colon-seprarated list of those.
   * Certificate files should have *.crt extension.
   * Directories are traversed recursively.
   * @param[in] certificatePath path to a CA certificate file(s), see above for details
   */
  virtual void SetCertificateFilePath(const std::string& certificatePath) = 0;

  /**
   * @brief Request for deleting all web databases.
   */
  virtual void DeleteAllWebDatabase() = 0;

  /**
   * @brief Request for getting web database origins.
   *
   * @param[in] callback callback called after getting web database origins
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool GetWebDatabaseOrigins(WebEngineSecurityOriginAcquiredCallback callback) = 0;

  /**
   * @brief Request for deleting web databases for origin.
   *
   * @param[in] origin application cache origin
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool DeleteWebDatabase(WebEngineSecurityOrigin& origin) = 0;

  /**
   * @brief Get list of origins that is stored in web storage db.
   *
   * @param[in] callback callback called after getting web storage origins
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool GetWebStorageOrigins(WebEngineSecurityOriginAcquiredCallback callback) = 0;

  /**
   * @brief Get list of origins that is stored in web storage db.
   *
   * @param[in] origin storage origin
   * @param[in] callback callback called after getting web storage origins
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, WebEngineStorageUsageAcquiredCallback callback) = 0;

  /**
   * @brief Delete all web storage.
   * @details This function does not ensure that all data will be removed.
   *          Should be used to extend free physical memory.
   */
  virtual void DeleteAllWebStorage() = 0;

  /**
   * @brief Delete origin that is stored in web storage database.
   *
   * @param[in] origin origin of database
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool DeleteWebStorageOrigin(WebEngineSecurityOrigin& origin) = 0;

  /**
   * @brief Request for deleting all local file systems.
   */
  virtual void DeleteLocalFileSystem() = 0;

  /**
   * @brief Toggle the cache to be enabled or disabled
   * Function works asynchronously.
   *
   * @param[in] cacheDisabled enable or disable cache
   */
  virtual void DisableCache(bool cacheDisabled) = 0;

  /**
   * @brief Request to clear cache
   */
  virtual void ClearCache() = 0;

  /**
   * @brief Request for deleting web application cache for origin.
   *
   * @param[in] origin application cache origin
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool DeleteApplicationCache(WebEngineSecurityOrigin& origin) = 0;

  /**
   * @brief Asynchronous request to get list of all password data.
   *
   * @param[in] callback callback called after getting form password
   */
  virtual void GetFormPasswordList(WebEngineFormPasswordAcquiredCallback callback) = 0;

  /**
   * @brief Register callback for download started.
   *
   * @param[in] callback callback for download started
   */
  virtual void RegisterDownloadStartedCallback(WebEngineDownloadStartedCallback callback) = 0;

  /**
   * @brief Register callback for mime overridden.
   *
   * @param[in] callback callback for mime overridden
   */
  virtual void RegisterMimeOverriddenCallback(WebEngineMimeOverriddenCallback callback) = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_CONTEXT_H
