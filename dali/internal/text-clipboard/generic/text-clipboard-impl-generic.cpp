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
#include <dali/internal/text-clipboard/common/text-clipboard-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct TextClipboard::Impl
{
};

TextClipboard::TextClipboard(Impl* impl)
: mImpl(impl)
{
}

TextClipboard::~TextClipboard()
{
}

Dali::TextClipboard TextClipboard::Get()
{
  Dali::TextClipboard clipboard;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TextClipboard));
    if(handle)
    {
      // If so, downcast the handle
      clipboard = Dali::TextClipboard(dynamic_cast<TextClipboard*>(handle.GetObjectPtr()));
    }
    else
    {
      TextClipboard::Impl* impl(new TextClipboard::Impl());
      clipboard = Dali::TextClipboard(new TextClipboard(impl));
      service.Register(typeid(Dali::TextClipboard), clipboard);
    }
  }

  return clipboard;
}

bool TextClipboard::IsAvailable()
{
  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TextClipboard));
    if(handle)
    {
      return true;
    }
  }
  return false;
}

bool TextClipboard::SetItem(const std::string& itemData)
{
  return true;
}

void TextClipboard::RequestItem()
{
}

unsigned int TextClipboard::NumberOfItems()
{
  return 0u;
}

void TextClipboard::ShowClipboard()
{
}

void TextClipboard::HideClipboard(bool skipFirstHide)
{
}

bool TextClipboard::IsVisible() const
{
  return false;
}

void TextClipboard::ExcuteSend(void* event)
{
}

void TextClipboard::ExcuteReceive(void* event, char*& data, int& length)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
