#ifndef DALI_INTERNAL_SYSTEM_SETTINGS_IMPL_H
#define DALI_INTERNAL_SYSTEM_SETTINGS_IMPL_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/system/system-settings.h>
#include <dali/public-api/object/base-object.h>
#include <string>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Internal service for platform system settings.
 *
 * The object is owned by DALi SingletonService. Initialize() and Shutdown()
 * manage platform callback registration and cached values.
 *
 * This service caches platform preferences and does not decide UI font scale
 * policy.
 */
class SystemSettings : public BaseObject
{
public:
  /**
   * @brief Returns the singleton handle.
   *
   * If SingletonService is available and the object does not exist, this method
   * creates and registers it.
   *
   * @return The SystemSettings singleton handle, or an empty handle.
   */
  static Dali::Integration::SystemSettings Get();

  /**
   * @brief Checks whether the SystemSettings object is already registered.
   *
   * This method never creates the object.
   *
   * @return True if the singleton is registered.
   */
  static bool IsAvailable();

  /**
   *  Gets the path at which application resources are stored.
   */
  static std::string GetResourcePath();

  /**
   *  Gets the path at which application data are stored.
   */
  static std::string GetDataPath();

  /**
   * @brief Queries the current locale language setting directly from the platform.
   *
   * This does not require SingletonService or Initialize() and is used by
   * framework language fallback paths.
   */
  static std::string QueryLocaleLanguage();

  /**
   * @brief Queries the current locale country setting directly from the platform.
   *
   * This does not require SingletonService or Initialize() and is used by
   * framework region fallback paths.
   */
  static std::string QueryLocaleCountry();

  /**
   * @brief Initializes cached settings and platform callback registration.
   */
  void Initialize();

  /**
   * @brief Unregisters platform callbacks.
   */
  void Shutdown();

  /**
   * @brief Gets the cached locale language setting.
   *
   * The value is read on first request or during Initialize(), then updated by
   * the locale language changed callback.
   */
  std::string GetLocaleLanguage();

  /**
   * @brief Gets the current locale country setting.
   *
   * This value is read on demand and is used as a fallback for framework region
   * queries.
   */
  std::string GetLocaleCountry() const;

  /**
   * @brief Gets the cached platform font size preference.
   *
   * This is not a point size, pixel size, or UI scale policy.
   */
  Dali::Integration::SystemSettings::FontSize GetFontSize() const;

  /**
   * @brief Gets the signal emitted when the platform font size preference changes.
   */
  Dali::Integration::SystemSettings::FontSizeChangedSignalType& FontSizeChangedSignal();

  /**
   * @brief Updates the cached locale language after a platform notification.
   */
  void NotifyLocaleLanguageChanged();

  /**
   * @brief Updates the cached platform font size preference and emits a signal if it changed.
   */
  void NotifyFontSizeChanged();

private:
  SystemSettings();
  ~SystemSettings() override;

  SystemSettings(const SystemSettings&) = delete;
  SystemSettings& operator=(const SystemSettings&) = delete;

private:
  bool mInitialized{false};

  bool mLocaleChangedCallbackRegistered{false};
  bool mFontSizeChangedCallbackRegistered{false};

  std::string mLocaleLanguage;
  Dali::Integration::SystemSettings::FontSize mFontSize{
    Dali::Integration::SystemSettings::FontSize::NORMAL};

  Dali::Integration::SystemSettings::FontSizeChangedSignalType mFontSizeChangedSignal;
};

inline static SystemSettings& GetImplementation(Dali::Integration::SystemSettings& handle)
{
  DALI_ASSERT_ALWAYS(handle && "SystemSettings handle is empty");
  BaseObject& object = handle.GetBaseObject();
  return static_cast<SystemSettings&>(object);
}

inline static const SystemSettings& GetImplementation(const Dali::Integration::SystemSettings& handle)
{
  DALI_ASSERT_ALWAYS(handle && "SystemSettings handle is empty");
  const BaseObject& object = handle.GetBaseObject();
  return static_cast<const SystemSettings&>(object);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_SYSTEM_SETTINGS_IMPL_H
