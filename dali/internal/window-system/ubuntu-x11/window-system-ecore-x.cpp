/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-system.h>
#include <dali/devel-api/adaptor-framework/keyboard.h>

// EXTERNAL_HEADERS
#include <dali/internal/system/linux/dali-ecore-x.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace WindowSystem
{

void Initialize()
{
  ecore_x_init( NULL );
}

void Shutdown()
{
  ecore_x_shutdown();
}

void GetScreenSize( int& width, int& height )
{
  ecore_x_screen_size_get( ecore_x_default_screen_get(), &width, &height );
}

bool SetKeyboardRepeatInfo( float rate, float delay )
{
  return false;
}

bool GetKeyboardRepeatInfo( float& rate, float& delay )
{
  return false;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
