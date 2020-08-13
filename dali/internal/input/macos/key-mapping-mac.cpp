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

// CLASS HEADER
#include <dali/internal/input/common/key-impl.h>

namespace Dali::Internal::Adaptor::KeyLookup
{

// matches a DALI_KEY enum, to key name
KeyLookup KeyLookupTable[]=
{
  { "Escape",                DALI_KEY_ESCAPE,                                            false },
  { "Command",               DALI_KEY_MENU,                                              false },
  { "Cancel",                DALI_KEY_CANCEL,                                            false },
  { "Backspace",             DALI_KEY_BACKSPACE,                                         false },
  { "Up",                    DALI_KEY_CURSOR_UP,                                         false },
  { "Left",                  DALI_KEY_CURSOR_LEFT,                                       false },
  { "Right",                 DALI_KEY_CURSOR_RIGHT,                                      false },
  { "Down",                  DALI_KEY_CURSOR_DOWN,                                       false },
  { "Shift",                 DALI_KEY_SHIFT_LEFT,                                        false },
  { "Delete",                static_cast<Dali::KEY>( DevelKey::DALI_KEY_DELETE ),        false },
  { "Control",               static_cast<Dali::KEY>( DevelKey::DALI_KEY_CONTROL_LEFT ),  false },
};

const std::size_t KEY_LOOKUP_COUNT = (sizeof( KeyLookupTable ))/ (sizeof( KeyLookup ));

}
