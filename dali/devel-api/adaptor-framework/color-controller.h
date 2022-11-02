#ifndef DALI_COLOR_CONTROLLER_H
#define DALI_COLOR_CONTROLLER_H

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
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/base-handle.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class ColorController;
}

} // namespace DALI_INTERNAL

/**
 * Color controller currently caches the changeable color table which updates with the theme change
 *
 * It provides the functionality of retrieving a RGBA color by passing in the color code string.
 */
class DALI_ADAPTOR_API ColorController : public BaseHandle
{
public:
  /**
   * @brief Create an uninitialized ColorController handle.
   */
  ColorController();

  /**
   * @brief Creates a copy of the handle.
   *
   * The copy will point to the same implementation as the original.
   * @param[in]  colorController  The Color Controller to copy from.
   */
  ColorController(const ColorController& colorController);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] rhs  A reference to the copied handle
   * @return A reference to this
   */
  ColorController& operator=(const ColorController& rhs);

  /**
   * @brief Creates a move of the handle.
   *
   * The move will point to the same implementation as the original.
   * @param[in]  colorController  The Color Controller to move from.
   */
  ColorController(ColorController&& colorController);

  /**
   * @brief This move assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] rhs  A reference to the moved handle
   * @return A reference to this
   */
  ColorController& operator=(ColorController&& rhs);

  /**
   * @brief Retrieve the initialized instance of the ColorController.
   *
   * @return Handle to ColorController.
   */
  static ColorController Get();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~ColorController();

  /**
   * @brief Retrieve the RGBA value by given the color code.
   *
   * @param[in] colorCode The color code string.
   * @param[out] colorValue The RGBA color
   * @return true if the color code exists, otherwise false
   */
  bool RetrieveColor(const std::string& colorCode, Vector4& colorValue);

  /**
    * @brief Retrieve the RGBA values by given the color code.
    *
    * @param[in] colorCode The color code string.
    * @param[out] textColor The text color.
    * @param[out] textOutlineColor The text outline color.
    * @param[out] textShadowColor The text shadow color.
    * @return true if the color code exists, otherwise false
    */
  bool RetrieveColor(const std::string& colorCode, Vector4& textColor, Vector4& textOutlineColor, Vector4& textShadowColor);

public: // Not intended for application developers
  /**
   * @brief This constructor is used internally to create a handle from an object pointer.
   * @param [in] colorController A pointer the internal color controller.
   */
  explicit DALI_INTERNAL ColorController(Internal::Adaptor::ColorController* colorController);
};

} //namespace Dali

#endif // DALI_COLOR_CONTROLLER_H
