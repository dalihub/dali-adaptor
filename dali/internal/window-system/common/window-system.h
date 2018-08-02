#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_SYSTEM_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_SYSTEM_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace WindowSystem
{

/**
 * @brief Initialize a window system
 */
void Initialize();

/**
 * @brief Shutdown a window system
 */
void Shutdown();

/**
 * @brief Get the screen size
 */
void GetScreenSize( int& width, int& height );

/**
 * @copydoc Dali::Keyboard::SetRepeatInfo()
 */
bool SetKeyboardRepeatInfo( float rate, float delay );

/**
 * @copydoc Dali::Keyboard::GetRepeatInfo()
 */
bool GetKeyboardRepeatInfo( float& rate, float& delay );

} // namespace WindowSystem

} // namespace Adaptor
} // namespace internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_SYSTEM_H
