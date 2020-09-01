#ifndef DALI_INTERNAL_OFFSCREEN_WINDOW_FACTORY_X_H
#define DALI_INTERNAL_OFFSCREEN_WINDOW_FACTORY_X_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-window-factory.h>

namespace Dali
{
namespace Internal
{

class OffscreenWindowFactoryX : public OffscreenWindowFactory
{
public:

  /**
   * @brief Create a new OffscreenWindow
   *
   * @param[in] width The initial width of the OffscreenWindow
   * @param[in] height The initial height of the OffscreenWindow
   * @param[in] surface The tbm surface queue handle
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  std::unique_ptr< OffscreenWindow > CreateOffscreenWindow( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent ) override;
};

} // Internal
} // Dali

#endif // DALI_INTERNAL_OFFSCREEN_WINDOW_FACTORY_X_H
