#ifndef DALI_WEB_ENGINE_SETTINGS_H
#define DALI_WEB_ENGINE_SETTINGS_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <cstdint>

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
   * @brief Enable/Disable cache builder
   *
   * @param[in] enabled if true, to enable cache builder
   *        otherwise to disable
   */
  virtual void EnableCacheBuilder(bool enabled) = 0;

  /**
   * @brief Used/Unused scrollbar thumb focus notifications
   *
   * @param[in] used if true, to use scrollbar thumb focus notifications
   *          otherwise to unused
   */
  virtual void UseScrollbarThumbFocusNotifications(bool used) = 0;

  /**
   * @brief Enable/Disable do not track
   *
   * @param[in] enabled if true, to enable do not track
   *        otherwise to disable
   */
  virtual void EnableDoNotTrack(bool enabled) = 0;

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
   * @brief Check if auto fitting is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool IsAutoFittingEnabled() const = 0;

  /**
   * @brief Enable/Disable auto fitting
   *
   * @param[in] enabled if true, to enable auto fitting
   *       otherwise to disable
   */
  virtual void EnableAutoFitting(bool enabled) = 0;

  /**
   * @brief Check if plugins are enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool ArePluginsEnabled() const = 0;

  /**
   * @brief Enable/Disable plugins
   *
   * @param[in] enabled if true, to enable plugins
   *       otherwise to disable
   */
  virtual void EnablePlugins(bool enabled) = 0;

  /**
   * @brief Check if private browsing is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool IsPrivateBrowsingEnabled() const = 0;

  /**
   * @brief Enable/Disable private browsing
   *
   * @param[in] enabled if true, to enable private browsing
   *       otherwise to disable
   */
  virtual void EnablePrivateBrowsing(bool enabled) = 0;

  /**
   * @brief Check if link magnifier is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool IsLinkMagnifierEnabled() const = 0;

  /**
   * @brief Enable/Disable link magnifier
   *
   * @param[in] enabled if true, to enable link magnifier
   *        otherwise to disable
   */
  virtual void EnableLinkMagnifier(bool enabled) = 0;

  /**
   * @brief Check if uses keypad without user action is used or not.
   *
   * @return true if used, false if unused.
   */
  virtual bool IsKeypadWithoutUserActionUsed() const = 0;

  /**
   * @brief Uses/Unused keypad without user action
   *
   * @param[in] used if true, to use keypad without user action
   *       otherwise to unused
   */
  virtual void UseKeypadWithoutUserAction(bool used) = 0;

  /**
   * @brief Check if autofill password form is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  virtual bool IsAutofillPasswordFormEnabled() const = 0;

  /**
   * @brief Enable/Disable autofill password form
   *
   * @param[in] enabled if true, to enable autofill_password_form
   *       otherwise to disable
   */
  virtual void EnableAutofillPasswordForm(bool enabled) = 0;

  /**
   * @brief check if form candidate data is enabled or not.
   *
   * @return true if enabled, false is disabled.
   */
  virtual bool IsFormCandidateDataEnabled() const = 0;

  /**
   * @brief Enable/Disable form candidate data
   *
   * @param[in] enabled if true, to enable form candidate data
   *        otherwise to disable
   */
  virtual void EnableFormCandidateData(bool enabled) = 0;

  /**
   * @brief check if text selection is enabled or not.
   *
   * @return true if enabled, false is disabled.
   */
  virtual bool IsTextSelectionEnabled() const = 0;

  /**
   * @brief Enable/Disable text selection
   *
   * @param[in] enabled if true, to enable text selection
   *        otherwise to disable
   */
  virtual void EnableTextSelection(bool enabled) = 0;

  /**
   * @brief check if text autosizing is enabled or not.
   *
   * @return true if enabled, false is disabled.
   */
  virtual bool IsTextAutosizingEnabled() const = 0;

  /**
   * @brief Enable/Disable text autosizing
   *
   * @param[in] enabled if true, to enable text autosizing
   *        otherwise to disable
   */
  virtual void EnableTextAutosizing(bool enabled) = 0;

  /**
   * @brief check if arrow scroll is enabled or not.
   *
   * @return true if enabled, false is disabled.
   */
  virtual bool IsArrowScrollEnabled() const = 0;

  /**
   * @brief Enable/Disable arrow scroll
   *
   * @param[in] enable if true, to enable arrow scroll
   *        otherwise to disable
   */
  virtual void EnableArrowScroll(bool enable) = 0;

  /**
   * @brief check if clipboard is enabled or not.
   *
   * @return true if enabled, false is disabled.
   */
  virtual bool IsClipboardEnabled() const = 0;

  /**
   * @brief Enable/Disable clipboard
   *
   * @param[in] enabled if true, to enable clipboard
   *          otherwise to disable
   */
  virtual void EnableClipboard(bool enabled) = 0;

  /**
   * @brief check if ime panel is enabled or not.
   *
   * @return true if enabled, false is disabled.
   */
  virtual bool IsImePanelEnabled() const = 0;

  /**
   * @brief Enable/Disable ime panel
   *
   * @param[in] enabled if true, to enable ime panel
   *          otherwise to disable
   */
  virtual void EnableImePanel(bool enabled) = 0;

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

  /**
   * @brief Enables/disables the viewport meta tag.
   *
   * By default, the viewport meta tag is enabled on mobile and wearable,
   * but it is disabled on TV.
   *
   * @param[in] enable @c true to enable the viewport meta tag
   *               @c false to disable
   *
   * @return @c true on success or @c false on failure
   */
  virtual bool SetViewportMetaTag(bool enable) = 0;

  /**
   * @brief Requests setting of force zoom.
   *
   * @param[in] enable to force zoom
   *
   * @return @c true on success or @c false on failure
   */
  virtual bool SetForceZoom(bool enable) = 0;

  /**
   * @brief Returns the force zoom status.
   *
   * @return @c true if enable force zoom or @c false.
   */
  virtual bool IsZoomForced() const = 0;

  /**
   * @brief Requests setting use of text zoom.
   *
   * @param[in] enable to text zoom.
   *
   * @return @c true on success or @c false on failure
   */
  virtual bool SetTextZoomEnabled(bool enable) = 0;

  /**
   * @brief Returns whether text zoom is enabled or not.
   *
   * @return @c true if enable text zoom or @c false.
   */
  virtual bool IsTextZoomEnabled() const = 0;

  /**
   * @brief Requests enables/disables to the specific extra feature
   *
   * @param[in] feature feature name
   * @param[in] enable @c true to enable the specific extra feature
   *        @c false to disable
   */
  virtual void SetExtraFeature(const std::string& feature, bool enable) = 0;

  /**
   * @brief Returns enable/disable to the specific extra feature
   *
   * @param[in] feature feature name
   *
   * @return @c true on enable or @c false on disable
   */
  virtual bool IsExtraFeatureEnabled(const std::string& feature) const = 0;

  /**
   * @brief Sets the style of IME.
   *
   * @param[in] style @c IME_STYLE_FULL       full IME style
   *                  @c IME_STYLE_FLOATING   floating IME style
   *                  @c IME_STYLE_DYNAMIC    dynamic IME style
   *
   */
  virtual void SetImeStyle(int style) = 0;

  /**
   * @brief Gets the style of IME.
   *
   * @return @c IME_STYLE_FULL       full IME style
   *         @c IME_STYLE_FLOATING   floating IME style
   *         @c IME_STYLE_DYNAMIC    dynamic IME style
   */
  virtual int GetImeStyle() const = 0;

  /**
   * @brief Sets default audio input device
   *
   * @param[in] deviceId default device ID
   */
  virtual void SetDefaultAudioInputDevice(const std::string& deviceId) const = 0;

  /**
   * @brief Enables Drag&Drop or not
   *
   * @param[in] enable if true, to enable Drag&Drop
   *       otherwise to disable
   */
  virtual void EnableDragAndDrop(bool enable) = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_SETTINGS_H
