#ifndef DALI_INTEGRATION_SYSTEM_SETTINGS_H
#define DALI_INTEGRATION_SYSTEM_SETTINGS_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class SystemSettings;
}
} // namespace Internal

namespace Integration
{

/**
 * @brief Provides access to platform system settings used by integration code.
 */
class DALI_ADAPTOR_API SystemSettings : public BaseHandle
{
public:
  /**
   * @brief Platform-provided font size preference.
   */
  enum class FontSize
  {
    SMALL       = 0,
    NORMAL      = 1,
    LARGE       = 2,
    EXTRA_LARGE = 3,
    GIANT       = 4
  };

  using FontSizeChangedSignalType = Signal<void(FontSize)>;

  /**
   * @brief Retrieves the DALi SystemSettings singleton.
   *
   * @return A handle to the SystemSettings singleton, or an empty handle
   *         when SingletonService is unavailable.
   */
  static SystemSettings Get();

  /**
   * @brief Creates an uninitialized SystemSettings handle.
   */
  SystemSettings();

  /**
   * @brief Destructor.
   */
  ~SystemSettings();

  /**
   * @brief Copy constructor.
   *
   * @param[in] handle A reference to the copied handle.
   */
  SystemSettings(const SystemSettings& handle);

  /**
   * @brief Assignment operator.
   *
   * @param[in] handle A reference to the copied handle.
   * @return A reference to this.
   */
  SystemSettings& operator=(const SystemSettings& handle);

  /**
   * @brief Move constructor.
   *
   * @param[in] handle A reference to the moved handle.
   */
  SystemSettings(SystemSettings&& handle) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @param[in] handle A reference to the moved handle.
   * @return A reference to this.
   */
  SystemSettings& operator=(SystemSettings&& handle) noexcept;

  /**
   * @brief Gets the cached platform font size preference.
   *
   * This is a platform-provided user preference, not an actual point size,
   * pixel size, or UI scale value.
   *
   * @return The current platform font size preference.
   */
  FontSize GetFontSize() const;

  /**
   * @brief Gets the signal emitted when the platform font size preference changes.
   *
   * @return The font size changed signal.
   */
  FontSizeChangedSignalType& FontSizeChangedSignal();

public: // Not intended for application developers
  /**
   * @brief This constructor is used internally to create a handle from an object pointer.
   *
   * @param[in] implementation A pointer to the internal SystemSettings.
   */
  explicit DALI_INTERNAL SystemSettings(Dali::Internal::Adaptor::SystemSettings* implementation);
};

} // namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_SYSTEM_SETTINGS_H
