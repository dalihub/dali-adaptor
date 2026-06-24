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
#include <dali/internal/clipboard/common/clipboard-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/clipboard/common/clipboard-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct Clipboard::Impl
{
  Clipboard::DataSentSignalType     mDataSentSignal{};
  Clipboard::DataReceivedSignalType mDataReceivedSignal{};
  Clipboard::DataOfferedSignalType  mDataOfferedSignal{};
  Clipboard::DataSelectedSignalType mDataSelectedSignal{};
};

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
}

Clipboard::~Clipboard()
{
  FinalizeGetDataCallbacks();
  delete mImpl;
}

class ClipboardFactoryGeneric : public ClipboardFactory
{
public:
  Dali::Clipboard CreateClipboard() override
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
};

std::unique_ptr<ClipboardFactory> GetClipboardFactory()
{
  return std::unique_ptr<ClipboardFactory>(new ClipboardFactoryGeneric());
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

Clipboard::DataSentSignalType& Clipboard::DataSentSignal()
{
  return mImpl->mDataSentSignal;
}

Clipboard::DataReceivedSignalType& Clipboard::DataReceivedSignal()
{
  return mImpl->mDataReceivedSignal;
}

Clipboard::DataOfferedSignalType& Clipboard::DataOfferedSignal()
{
  return mImpl->mDataOfferedSignal;
}

Clipboard::DataSelectedSignalType& Clipboard::DataSelectedSignal()
{
  return mImpl->mDataSelectedSignal;
}

bool Clipboard::HasType(const std::string& mimeType)
{
  return true;
}

bool Clipboard::SetData(const Dali::ClipboardData& clipboardData)
{
  return true;
}

uint32_t Clipboard::GetData(const std::string& mimeType)
{
  return 0u;
}

uint32_t Clipboard::GetItemCount()
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

bool Clipboard::OnReceiveData()
{
  return false;
}

bool Clipboard::OnMultiSelectionTimeout()
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
