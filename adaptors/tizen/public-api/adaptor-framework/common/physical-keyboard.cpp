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

// CLASS HEADER
#include <public-api/adaptor-framework/common/physical-keyboard.h>

// INTERNAL INCLUDES
#include <internal/common/physical-keyboard-impl.h>

namespace Dali
{

PhysicalKeyboard::PhysicalKeyboard()
{
}

PhysicalKeyboard::~PhysicalKeyboard()
{
}

PhysicalKeyboard PhysicalKeyboard::Get()
{
  // Get the physical keyboard handle
  PhysicalKeyboard handle = Internal::Adaptor::PhysicalKeyboard::Get();

  // If it's not been created then create one
  if ( !handle )
  {
    handle = Internal::Adaptor::PhysicalKeyboard::New();
  }

  return handle;
}

bool PhysicalKeyboard::IsAttached() const
{
  return GetImplementation( *this ).IsAttached();
}

PhysicalKeyboard::Signal& PhysicalKeyboard::StatusChangedSignal()
{
  return GetImplementation( *this ).StatusChangedSignal();
}

PhysicalKeyboard::PhysicalKeyboard( Internal::Adaptor::PhysicalKeyboard *impl )
: BaseHandle(impl)
{
}

} // namespace Dali
