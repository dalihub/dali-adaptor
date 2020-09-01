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

// CLASS HEADER
#include <dali/internal/offscreen/android/offscreen-window-factory-android.h>

// INTERNAL HEADERS
#include <dali/internal/offscreen/android/offscreen-window-impl-android.h>

namespace Dali
{
namespace Internal
{

std::unique_ptr< OffscreenWindow > OffscreenWindowFactoryAndroid::CreateOffscreenWindow( uint16_t width, uint16_t height, Any surface, bool isTranslucent )
{
  return std::unique_ptr< OffscreenWindow >( OffscreenWindowAndroid::New( width, height, surface, isTranslucent ) );
}

std::unique_ptr< OffscreenWindowFactory > GetOffscreenWindowFactory()
{
  return std::unique_ptr< OffscreenWindowFactoryAndroid >( new OffscreenWindowFactoryAndroid() );
}

} // Internal
} // Dali
