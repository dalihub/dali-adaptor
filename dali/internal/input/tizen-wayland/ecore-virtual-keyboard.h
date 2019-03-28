#ifndef DALI_INTERNAL_ECORE_VIRTUAL_KEYBOARD_H
#define DALI_INTERNAL_ECORE_VIRTUAL_KEYBOARD_H

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

// EXTERNAL INCLUDES
#include <dali/internal/input/linux/dali-ecore-imf.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/virtual-keyboard.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Implementation of the virtual keyboard namespace
 */
namespace VirtualKeyboard
{

/**
 * Connect the virtual keyboard callbacks.
 * To get the virtual keyboard callbacks then you have to connect these callback.
 * If you don't connect callbacks, you can't get virtual keyboard signals.
 * The signals are StatusChangedSignal, ResizedSignal and LanguageChangedSignal.
 */
void ConnectCallbacks( Ecore_IMF_Context *imfContext );

/**
 * Disconnect the virtual keyboard callbacks.
 * The signals are StatusChangedSignal, ResizedSignal and LanguageChangedSignal.
 */
void DisconnectCallbacks( Ecore_IMF_Context *imfContext );

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ECORE_VIRTUAL_KEYBOARD_H
