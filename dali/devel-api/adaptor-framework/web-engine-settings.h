#ifndef DALI_WEB_ENGINE_SETTINGS_H
#define DALI_WEB_ENGINE_SETTINGS_H

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
 * @brief A class WebEngineSettings for settings of web engine.
 */
class WebEngineSettings
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineSettings() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineSettings() = default;

  /**
   *@brief Allow running mixed contents or not.
   *
   * @param[in] allowed if true, allow to run mixed contents,
   *      otherwise not allow
   */
  virtual void AllowMixedContents(bool allowed) = 0;

  /**
   * @brief Enable the spatial navigation or not.
   *
   * @param[in] enabled if true, use spatial navigation,
   *       otherwise to disable
   */
  virtual void EnableSpatialNavigation(bool enabled) = 0;

  /**
   * @brief Get the default font size.
   *
   * @return defaut font size.
   */
  virtual uint32_t GetDefaultFontSize() const = 0;

  /**
   * @brief Set the default font size.
   *
   * @param[in] size a new default font size to set
   */
  virtual void SetDefaultFontSize(uint32_t size) = 0;

  /**
   * @brief Enables/disables web security.
   *
   * @param[in] enabled if true, to enable the web security
   *       otherwise to disable
   */
  virtual void EnableWebSecurity(bool enabled) = 0;

  /**
   * @brief Allow/Disallow file access from external url
   *
   * @param[in] allowed if true, to allow file access from external url
   *       otherwise to disallow
   */
  virtual void AllowFileAccessFromExternalUrl(bool allowed) = 0;

  /**
   * @brief Check if javascript is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool IsJavaScriptEnabled() const = 0;

  /**
   * @brief Enable/Disable javascript
   *
   * @param[in] enabled if true, to enable javascript
   *       otherwise to disable
   */
  virtual void EnableJavaScript(bool enabled) = 0;

  /**
   * @brief Allow if the scripts can open new windows.
   *
   * @param[in] allowed if true, the scripts can open new windows,
   *       otherwise not
   */
  virtual void AllowScriptsOpenWindows(bool allowed) = 0;

  /**
   * @brief Check if images are loaded automatically or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool AreImagesLoadedAutomatically() const = 0;

  /**
   * @brief Allow to load images automatically
   *
   * @param[in] automatic if true, to load images automatically,
   *       otherwise not
   */
  virtual void AllowImagesLoadAutomatically(bool automatic) = 0;

  /**
   * @brief Get the default encoding name.
   *
   * @return defaut encoding name.
   */
  virtual std::string GetDefaultTextEncodingName() const = 0;

  /**
   * @brief Set the default encoding name.
   *
   * @param[in] defaultTextEncodingName a default encoding name to set
   */
  virtual void SetDefaultTextEncodingName(const std::string& defaultTextEncodingName) = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_SETTINGS_H
