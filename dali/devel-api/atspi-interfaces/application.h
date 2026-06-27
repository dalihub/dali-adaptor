#ifndef DALI_ADAPTOR_ATSPI_APPLICATION_H
#define DALI_ADAPTOR_ATSPI_APPLICATION_H

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
 */

// EXTERNAL INCLUDES
#include <string>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-integ.h>
#include <dali/devel-api/atspi-interfaces/accessibility-feature.h>

namespace Dali::Accessibility
{
/**
 * @brief An interface identifying the root object
 * associated with a running application.
 *
 * @note Provides global properties describing
 * application's runtime environment.
 */
class DALI_ADAPTOR_API Application : public IAccessibilityFeature
{
public:
  /**
   * @brief Gets name of graphic user interface framework used by an application.
   *
   * @return String with name
   */
  virtual std::string GetToolkitName() const = 0;

  /**
   * @brief Gets version of graphic user interface framework used by an application.
   *
   * @return String with version
   */
  virtual std::string GetVersion() const = 0;

  /**
   * @brief Gets include_hidden flag currently set on the application.
   *
   * @return true is include_hidden is set; false otherwise.
   */
  virtual bool GetIncludeHidden() const = 0;

  /**
   * @brief Sets include_hidden flag to the application.
   *
   * @return true is include_hidden flag is updated; false otherwise.
   */
  virtual bool SetIncludeHidden(bool includeHidden) = 0;
};

} // namespace Dali::Accessibility

namespace Dali::Integration::Accessibility
{
template<>
struct AccessibilityInterfaceTypeHelper<Dali::Integration::Accessibility::AccessibilityInterface::APPLICATION>
{
  using Type = Dali::Accessibility::Application;
};
} // namespace Dali::Integration::Accessibility

#endif // DALI_ADAPTOR_ATSPI_APPLICATION_H
