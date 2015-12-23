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

// CLASS HEADER
#include <imf-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <singleton-service-impl.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

BaseHandle Create()
{
  return ImfManager::Get();
}

TypeRegistration IMF_MANAGER_TYPE( typeid(Dali::ImfManager), typeid(Dali::BaseHandle), Create );

} // unnamed namespace

bool ImfManager::IsAvailable()
{
  return false;
}

Dali::ImfManager ImfManager::Get()
{
  // Return empty handle as not supported
  return Dali::ImfManager();
}

ImfManager::~ImfManager()
{
}

void ImfManager::ConnectCallbacks()
{
}

void ImfManager::DisconnectCallbacks()
{
}

void ImfManager::Activate()
{
}

void ImfManager::Deactivate()
{
}

void ImfManager::Reset()
{
}

void* ImfManager::GetContext()
{
  return NULL;
}

bool ImfManager::RestoreAfterFocusLost() const
{
  return false;
}

void ImfManager::SetRestoreAfterFocusLost( bool toggle )
{
}

void ImfManager::NotifyCursorPosition()
{
}
unsigned int ImfManager::GetCursorPosition() const
{
  return 0;
}

void ImfManager::SetCursorPosition( unsigned int cursorPosition )
{
}

void ImfManager::SetSurroundingText(  const std::string& text )
{
}

const std::string& ImfManager::GetSurroundingText() const
{
  static std::string dummyString("");
  return dummyString;
}

} // Adaptor

} // Internal

} // Dali
