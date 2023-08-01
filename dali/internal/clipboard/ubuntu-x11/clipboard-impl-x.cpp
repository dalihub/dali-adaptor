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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/system/linux/dali-ecore-x.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/ubuntu-x11/window-interface-ecore-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct Clipboard::Impl
{
  Impl(Ecore_X_Window ecoreXwin)
  {
    mApplicationWindow = ecoreXwin;
  }

  bool SetData(const Dali::Clipboard::ClipData& clipData)
  {
    mMimeType = clipData.GetMimeType();
    mData     = clipData.GetData();

    if(mData.empty())
    {
      return false;
    }

    mDataSentSignal.Emit(mMimeType.c_str(), mData.c_str());

    return true;
  }

  uint32_t GetData(const std::string &mimeType)
  {
    if(!mMimeType.compare(mimeType.c_str()))
    {
      mDataId++;
      // For consistency of operation with tizen Wl2, a fake callback is occurs using a timer.
      if(mDataReceiveTimer.IsRunning())
      {
        mDataReceiveTimer.Stop();
      }
      mDataReceiveTimer.Start();
      DALI_LOG_RELEASE_INFO("request data, id:%u, request type:%s\n", mDataId, mimeType.c_str());
      return mDataId;
    }
    return 0u;
  }

  bool OnReceiveData()
  {
    DALI_LOG_RELEASE_INFO("receive data, success signal emit, id:%u, type:%s, data:%s\n", mDataId, mMimeType.c_str(), mData.c_str());
    mDataReceivedSignal.Emit(mDataId, mMimeType.c_str(), mData.c_str());
    return false;
  }

  Ecore_X_Window mApplicationWindow;
  std::string    mMimeType;
  std::string    mData;
  uint32_t       mDataId{0};

  Dali::Clipboard::DataSentSignalType     mDataSentSignal;
  Dali::Clipboard::DataReceivedSignalType mDataReceivedSignal;

  Dali::Timer mDataReceiveTimer;
};

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
  mImpl->mDataReceiveTimer = Dali::Timer::New(10);
  mImpl->mDataReceiveTimer.TickSignal().Connect(this, &Clipboard::OnReceiveData);
}

Clipboard::~Clipboard()
{
  delete mImpl;
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
      Adaptor& adaptorImpl(Adaptor::GetImplementation(Adaptor::Get()));
      Any      nativewindow = adaptorImpl.GetNativeWindowHandle();

      // The Ecore_X_Window needs to use the Clipboard.
      // Only when the render surface is window, we can get the Ecore_X_Window.
      Ecore_X_Window ecoreXwin(AnyCast<Ecore_X_Window>(nativewindow));
      if(ecoreXwin)
      {
        // If we fail to get Ecore_X_Window, we can't use the Clipboard correctly.
        // Thus you have to call "ecore_imf_context_client_window_set" somewhere.
        // In EvasPlugIn, this function is called in EvasPlugin::ConnectEcoreEvent().
        Clipboard::Impl* impl(new Clipboard::Impl(ecoreXwin));
        clipboard = Dali::Clipboard(new Clipboard(impl));
        service.Register(typeid(clipboard), clipboard);
      }
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

Dali::Clipboard::DataSentSignalType& Clipboard::DataSentSignal()
{
  return mImpl->mDataSentSignal;
}

Dali::Clipboard::DataReceivedSignalType& Clipboard::DataReceivedSignal()
{
  return mImpl->mDataReceivedSignal;
}

bool Clipboard::SetData(const Dali::Clipboard::ClipData& clipData)
{
  return mImpl->SetData(clipData);
}

uint32_t Clipboard::GetData(const std::string &mimeType)
{
  return mImpl->GetData(mimeType);
}

size_t Clipboard::NumberOfItems()
{
  // TODO: We should to check if the data is empty in the clipboard service.
  return 1u;
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
  return mImpl->OnReceiveData();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
