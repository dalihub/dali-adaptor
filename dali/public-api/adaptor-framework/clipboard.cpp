/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/clipboard.h>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-factory.h>
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
  return Internal::Adaptor::GetClipboardFactory()->CreateClipboard();
}

bool Clipboard::SetData(const ClipboardData& data)
{
  return GetImplementation(*this).SetData(data);
}

bool Clipboard::HasType(const Dali::String& mimeType)
{
  return GetImplementation(*this).HasType(mimeType.CStr());
}

void Clipboard::DoGetData(const Dali::String& mimeType,
                          Dali::ConnectionTrackerInterface* connectionTracker,
                          Dali::CallbackBase* callback)
{
  GetImplementation(*this).DoGetData(mimeType, connectionTracker, callback);
}

Clipboard::DataOfferedSignalType& Clipboard::DataOfferedSignal()
{
  return GetImplementation(*this).DataOfferedSignal();
}

} // namespace Dali
