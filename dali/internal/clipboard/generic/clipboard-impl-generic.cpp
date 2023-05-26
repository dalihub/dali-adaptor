/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/clipboard/common/clipboard-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct Clipboard::Impl
{
};

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
}

Clipboard::~Clipboard()
{
}

Dali::Clipboard Clipboard::Get()
{
  Dali::Clipboard clipboard;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::Clipboard));
    if(handle)
    {
      // If so, downcast the handle
      clipboard = Dali::Clipboard(dynamic_cast<Clipboard*>(handle.GetObjectPtr()));
    }
    else
    {
      Clipboard::Impl* impl(new Clipboard::Impl());
      clipboard = Dali::Clipboard(new Clipboard(impl));
      service.Register(typeid(Dali::Clipboard), clipboard);
    }
  }

  return clipboard;
}

bool Clipboard::IsAvailable()
{
  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::Clipboard));
    if(handle)
    {
      return true;
    }
  }
  return false;
}

bool Clipboard::SetItem(const std::string& itemData)
{
  return true;
}

void Clipboard::RequestItem()
{
}

unsigned int Clipboard::NumberOfItems()
{
  return 0u;
}

void Clipboard::ShowClipboard()
{
}

void Clipboard::HideClipboard(bool skipFirstHide)
{
}

bool Clipboard::IsVisible() const
{
  return false;
}

char* Clipboard::ExcuteBuffered(bool type, void* event)
{
  return NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
