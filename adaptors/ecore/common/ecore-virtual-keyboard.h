#ifndef __DALI_INTERNAL_ECORE_VIRTUAL_KEYBOARD_H__
#define __DALI_INTERNAL_ECORE_VIRTUAL_KEYBOARD_H__

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
#include <Ecore_IMF.h>

// INTERNAL INCLUDES
#include <virtual-keyboard.h>

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

#endif // __DALI_INTERNAL_ECORE_VIRTUAL_KEYBOARD_H__
