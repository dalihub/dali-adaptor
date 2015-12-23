#ifndef __DALI_WAYLAND_WINDOW_H__
#define __DALI_WAYLAND_WINDOW_H__
/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/rect.h>
#include <string>

namespace Dali
{

typedef Dali::Rect<int> PositionSize;

namespace Wayland
{

/**
 * @brief Ecapsulates data for a wayland window.
 */
class Window
{
  public:

  /**
   * @brief constructor
   */
  Window();

  /**
   * @brief destructor. Non-virtual not intended as a base class.
   */
  ~Window();

private:

  // @brief Undefined copy constructor.
  Window( const Window& );

  // @brief Undefined assignment operator.
  Window& operator=( const Window& );

public:

  std::string mTitle;                   ///< Window title
  PositionSize mPosition;               ///< Position
  int mSurfaceId;                       ///< wayland surface id

};


} // namespace Wayland

} // namespace Dali

#endif // __DALI_WAYLAND_WINDOW_H__
