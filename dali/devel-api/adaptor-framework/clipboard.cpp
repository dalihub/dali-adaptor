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
#include <dali/devel-api/adaptor-framework/clipboard.h>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-impl.h>

namespace Dali
{
Clipboard::Clipboard()
{
}

Clipboard::~Clipboard()
{
}

Clipboard::Clipboard(Internal::Adaptor::Clipboard* impl)
: BaseHandle(impl)
{
}

Clipboard Clipboard::Get()
{
  return Internal::Adaptor::Clipboard::Get();
}

bool Clipboard::IsAvailable()
{
  return Internal::Adaptor::Clipboard::IsAvailable();
}

Clipboard::DataSentSignalType& Clipboard::DataSentSignal()
{
  return GetImplementation(*this).DataSentSignal();
}

Clipboard::DataReceivedSignalType& Clipboard::DataReceivedSignal()
{
  return GetImplementation(*this).DataReceivedSignal();
}

bool Clipboard::SetData(const ClipData& clipData)
{
  return GetImplementation(*this).SetData(clipData);
}

uint32_t Clipboard::GetData(const std::string& mimeType)
{
  return GetImplementation(*this).GetData(mimeType);
}

size_t Clipboard::NumberOfItems()
{
  return GetImplementation(*this).NumberOfItems();
}

void Clipboard::ShowClipboard()
{
  GetImplementation(*this).ShowClipboard();
}

void Clipboard::HideClipboard()
{
  GetImplementation(*this).HideClipboard(false);
}

bool Clipboard::IsVisible() const
{
  return GetImplementation(*this).IsVisible();
}

} // namespace Dali
