/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/input/common/physical-keyboard-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Dali::PhysicalKeyboard PhysicalKeyboard::New()
{
  Dali::PhysicalKeyboard keyboardHandle;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    keyboardHandle = Dali::PhysicalKeyboard(new PhysicalKeyboard());
    service.Register(typeid(keyboardHandle), keyboardHandle);
  }

  return keyboardHandle;
}

Dali::PhysicalKeyboard PhysicalKeyboard::Get()
{
  Dali::PhysicalKeyboard keyboardHandle;

  Dali::SingletonService service = SingletonService::Get();
  if(service)
  {
    BaseHandle handle = service.GetSingleton(typeid(Dali::PhysicalKeyboard));
    if(handle)
    {
      // If so, downcast the handle of singleton to focus manager
      keyboardHandle = Dali::PhysicalKeyboard(dynamic_cast<PhysicalKeyboard*>(handle.GetObjectPtr()));
    }
  }

  return keyboardHandle;
}

bool PhysicalKeyboard::IsAttached() const
{
  return mAttached;
}

void PhysicalKeyboard::KeyReceived(bool fromPhysicalKeyboard)
{
  if(mAttached != fromPhysicalKeyboard)
  {
    mAttached = fromPhysicalKeyboard;

    Dali::PhysicalKeyboard handle(this);
    mStatusChangedSignal.Emit(handle);
  }
}

PhysicalKeyboard::~PhysicalKeyboard()
{
}

PhysicalKeyboard::PhysicalKeyboard()
: mAttached(false)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
