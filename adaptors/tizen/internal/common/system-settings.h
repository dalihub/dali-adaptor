#ifndef __DALI_INTERNAL_SYSTEM_SETTINGS_H___
#define __DALI_INTERNAL_SYSTEM_SETTINGS_H___

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

// Get SYSTEM_SETTINGS_KEY_TAP_AND_HOLD_DELAY from system setting if available
int GetLongPressTime( int defaultTime );

// Get ELM_ACCESS_ACTION_OVER from Elementary if available
int GetElmAccessActionOver();

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_SYSTEM_SETTINGS_H___
