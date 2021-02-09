#ifndef DALI_INTERNAL_COLOR_CONTROLLER_H
#define DALI_INTERNAL_COLOR_CONTROLLER_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/color-controller-plugin.h>
#include <dali/devel-api/adaptor-framework/color-controller.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of ColorController
 */
class ColorController : public BaseObject
{
public:
  /**
   * Constructor.
   */
  ColorController();

  /**
   * @copydoc Dali::ColorController::Get()
   */
  static Dali::ColorController Get();

  /**
   * @copydoc Dali::ColorController::RetrieveColor(const std::string&, Vector4&)
   */
  bool RetrieveColor(const std::string& colorCode, Vector4& colorValue);

  /**
   * @copydoc Dali::ColorController::RetrieveColor(const std::string&, Vector4&, Vector4&, Vector4&)
   */
  bool RetrieveColor(const std::string& colorCode, Vector4& textColor, Vector4& textOutlineColor, Vector4& textShadowColor);

private:
  /**
  * @brief Initialize
  */
  void Initialize();

protected:
  /**
   * @brief Destructor.
   */
  ~ColorController() override;

private:
  using CreateColorControllerFunction = Dali::ColorControllerPlugin* (*)();

  void*                        mLibHandle; ///< Handle for the loaded library
  Dali::ColorControllerPlugin* mPlugin;    ///< Plugin handle

  CreateColorControllerFunction mCreateColorControllerPtr; ///< Function pointer called in adaptor to create a plugin instance
};

} // namespace Adaptor

} // namespace Internal

// Additional Helpers for public-api forwarding methods
inline Internal::Adaptor::ColorController& GetImplementation(Dali::ColorController& controller)
{
  DALI_ASSERT_ALWAYS(controller && "ColorController handle is empty");
  BaseObject& handle = controller.GetBaseObject();
  return static_cast<Internal::Adaptor::ColorController&>(handle);
}

inline const Internal::Adaptor::ColorController& GetImplementation(const Dali::ColorController& controller)
{
  DALI_ASSERT_ALWAYS(controller && "ColorController handle is empty");
  const BaseObject& handle = controller.GetBaseObject();
  return static_cast<const Internal::Adaptor::ColorController&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_COLOR_CONTROLLER_H
