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
#include <dali/internal/text-clipboard/common/text-clipboard-event-notifier-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/public-api/object/type-registry.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Dali::TextClipboardEventNotifier TextClipboardEventNotifier::New()
{
  Dali::TextClipboardEventNotifier notifier = Dali::TextClipboardEventNotifier(new TextClipboardEventNotifier());

  return notifier;
}

Dali::TextClipboardEventNotifier TextClipboardEventNotifier::Get()
{
  Dali::TextClipboardEventNotifier notifier;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TextClipboardEventNotifier));
    if(handle)
    {
      // If so, downcast the handle
      notifier = Dali::TextClipboardEventNotifier(dynamic_cast<TextClipboardEventNotifier*>(handle.GetObjectPtr()));
    }
    else
    {
      notifier = Dali::TextClipboardEventNotifier(TextClipboardEventNotifier::New());
      service.Register(typeid(notifier), notifier);
    }
  }

  return notifier;
}

const std::string& TextClipboardEventNotifier::GetContent() const
{
  return mContent;
}

void TextClipboardEventNotifier::SetContent(const std::string& content)
{
  mContent = content;
}

void TextClipboardEventNotifier::ClearContent()
{
  mContent.clear();
}

void TextClipboardEventNotifier::EmitContentSelectedSignal()
{
  if(!mContentSelectedSignal.Empty())
  {
    Dali::TextClipboardEventNotifier handle(this);
    mContentSelectedSignal.Emit(handle);
  }
}

TextClipboardEventNotifier::TextClipboardEventNotifier()
: mContent()
{
}

TextClipboardEventNotifier::~TextClipboardEventNotifier()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
