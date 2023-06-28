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
#include <dali/devel-api/adaptor-framework/text-clipboard.h>

// INTERNAL INCLUDES
#include <dali/internal/text-clipboard/common/text-clipboard-impl.h>

namespace Dali
{
TextClipboard::TextClipboard()
{
}
TextClipboard::~TextClipboard()
{
}
TextClipboard::TextClipboard(Internal::Adaptor::TextClipboard* impl)
: BaseHandle(impl)
{
}

TextClipboard TextClipboard::Get()
{
  return Internal::Adaptor::TextClipboard::Get();
}

bool TextClipboard::IsAvailable()
{
  return Internal::Adaptor::TextClipboard::IsAvailable();
}

bool TextClipboard::SetItem(const std::string& itemData)
{
  return GetImplementation(*this).SetItem(itemData);
}

void TextClipboard::RequestItem()
{
  GetImplementation(*this).RequestItem();
}

unsigned int TextClipboard::NumberOfItems()
{
  return GetImplementation(*this).NumberOfItems();
}

void TextClipboard::ShowClipboard()
{
  GetImplementation(*this).ShowClipboard();
}

void TextClipboard::HideClipboard()
{
  GetImplementation(*this).HideClipboard(false);
}

bool TextClipboard::IsVisible() const
{
  return GetImplementation(*this).IsVisible();
}

} // namespace Dali
