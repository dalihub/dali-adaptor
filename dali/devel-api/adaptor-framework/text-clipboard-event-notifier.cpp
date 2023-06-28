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
#include <dali/devel-api/adaptor-framework/text-clipboard-event-notifier.h>

// INTERNAL INCLUDES
#include <dali/internal/text-clipboard/common/text-clipboard-event-notifier-impl.h>

namespace Dali
{
TextClipboardEventNotifier::TextClipboardEventNotifier()
{
}

TextClipboardEventNotifier TextClipboardEventNotifier::Get()
{
  return Internal::Adaptor::TextClipboardEventNotifier::Get();
}

TextClipboardEventNotifier::~TextClipboardEventNotifier()
{
}

const std::string& TextClipboardEventNotifier::GetContent() const
{
  return Internal::Adaptor::TextClipboardEventNotifier::GetImplementation(*this).GetContent();
}

void TextClipboardEventNotifier::SetContent(const std::string& content)
{
  Internal::Adaptor::TextClipboardEventNotifier::GetImplementation(*this).SetContent(content);
}

void TextClipboardEventNotifier::ClearContent()
{
  Internal::Adaptor::TextClipboardEventNotifier::GetImplementation(*this).ClearContent();
}

void TextClipboardEventNotifier::EmitContentSelectedSignal()
{
  Internal::Adaptor::TextClipboardEventNotifier::GetImplementation(*this).EmitContentSelectedSignal();
}

TextClipboardEventNotifier::TextClipboardEventSignalType& TextClipboardEventNotifier::ContentSelectedSignal()
{
  return Internal::Adaptor::TextClipboardEventNotifier::GetImplementation(*this).ContentSelectedSignal();
}

TextClipboardEventNotifier::TextClipboardEventNotifier(Internal::Adaptor::TextClipboardEventNotifier* notifier)
: BaseHandle(notifier)
{
}

} // namespace Dali
