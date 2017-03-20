#ifndef __DALI_VIRTUAL_KEYBOARD_OPTIONS_H__
#define __DALI_VIRTUAL_KEYBOARD_OPTIONS_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/property-map.h>

// INTERNAL INCLUDES
#include "input-method-devel.h"

namespace Dali
{

/**
 * Class to handle the Input Method options
 */
class DALI_IMPORT_API InputMethodOptions
{
public:

  /**
   * Constructor
   */
  InputMethodOptions();   /// Default InputMethodOptions options

  /**
   * @brief Returns whether panel layout type is password or not
   * @return true if panel layout type is password, false otherwise.
   */
  bool IsPassword();

  /**
   * @brief Apply property map to attribute class, this class will keep the virtualKeyboard settings.
   * @param[in] settings The property map to be applied
   */
  void ApplyProperty( const Property::Map& settings );

  /**
   * @brief Retrieve property map from current option
   * @param[out] settings The converted property map
   */
  void RetrieveProperty( Property::Map& settings );

public: // Intended for internal use
  /**
   * @brief Set option respectively
   * @param[in] type The type of source option will be updated
   * @param[in] options The source option to be applied
   * @param[out] index The updated index after applying source option
   * @return true if the value of this option is updated by source option
   */
  DALI_INTERNAL bool CompareAndSet( InputMethod::Category::Type type, const InputMethodOptions& options, int& index );

private:

  struct Impl;
  Impl* mImpl;
};

} // namespace Dali

#endif // __DALI_VIRTUAL_KEYBOARD_OPTIONS_H__
