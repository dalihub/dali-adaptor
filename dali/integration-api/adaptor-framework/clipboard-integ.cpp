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
#include <dali/integration-api/adaptor-framework/clipboard-integ.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/clipboard.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>

namespace Dali
{
namespace Integration
{
namespace Clipboard
{

bool IsAvailable()
{
  return Dali::Internal::Adaptor::Clipboard::IsAvailable();
}

DataSentSignalType& DataSentSignal(Dali::Clipboard& clipboard)
{
  return GetImplementation(clipboard).DataSentSignal();
}

DataReceivedSignalType& DataReceivedSignal(Dali::Clipboard& clipboard)
{
  return GetImplementation(clipboard).DataReceivedSignal();
}

DataSelectedSignalType& DataSelectedSignal(Dali::Clipboard& clipboard)
{
  return GetImplementation(clipboard).DataSelectedSignal();
}

uint32_t GetData(Dali::Clipboard& clipboard, const Dali::String& mimeType)
{
  return GetImplementation(clipboard).GetData(mimeType.CStr());
}

uint32_t GetItemCount(Dali::Clipboard& clipboard)
{
  return GetImplementation(clipboard).GetItemCount();
}

void ShowClipboard(Dali::Clipboard& clipboard)
{
  GetImplementation(clipboard).ShowClipboard();
}

void HideClipboard(Dali::Clipboard& clipboard, bool skipFirstHide)
{
  GetImplementation(clipboard).HideClipboard(skipFirstHide);
}

bool IsVisible(const Dali::Clipboard& clipboard)
{
  return GetImplementation(clipboard).IsVisible();
}

} // namespace Clipboard
} // namespace Integration
} // namespace Dali
