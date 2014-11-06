#ifndef __DALI_STYLE_CHANGE_H__
#define __DALI_STYLE_CHANGE_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-common.h>

namespace Dali
{

/**
 * @brief Used to describe what style information has changed.
 *
 * This structure is used when any style changes occur and contains information about what exactly
 * has changed.
 */
struct DALI_IMPORT_API StyleChange
{
  // Data

  bool defaultFontChange:1;     ///< Denotes that the default font has changed.
  bool defaultFontSizeChange:1; ///< Denotes that the default font size has changed.
  bool themeChange:1;           ///< Denotes that the theme has changed.
  std::string themeFilePath;    ///< Contains the path to the new theme file.

  // Construction

  /**
   * @brief Default Constructor.
   */
  StyleChange()
  : defaultFontChange(false),
    defaultFontSizeChange(false),
    themeChange(false)
  {
  }
};

} // namespace Dali

#endif // __DALI_STYLE_CHANGE_H__
