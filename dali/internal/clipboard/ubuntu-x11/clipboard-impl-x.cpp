/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <map>
#include <queue>

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

  bool HasType(const std::string& mimeType)
  {
    for(const auto& type : mMimeTypes)
    {
      if(type == mimeType)
      {
        return true;
      }
    }
    return false;
  }

  void UpdateData(std::string& mimeType, std::string& data, bool clearBuffer)
  {
    if(clearBuffer)
    {
      mMimeTypes.clear();
      mDatas.clear();
    }
    mMimeTypes.push_back(mimeType);
    mDatas[mimeType] = data;
  }

  bool SetData(const Dali::Clipboard::ClipData& clipData)
  {
    std::string mimeType = clipData.GetMimeType();
    std::string data     = clipData.GetData();

    if(mimeType.empty() || data.empty())
    {
      return false;
    }

    if(mLastType != mimeType && !mMultiSelectionTimeout)
    {
      bool clearBuffer = HasType(mimeType);
      UpdateData(mimeType, data, clearBuffer);
    }
    else
    {
      UpdateData(mimeType, data, true);
    }

    mLastType = mimeType;

    mDataSentSignal.Emit(mimeType.c_str(), data.c_str());
    mDataSelectedSignal.Emit(mimeType.c_str());

    SetMultiSelectionTimeout();

    return true;
  }

  uint32_t GetData(const std::string& mimeType)
  {
    if(mDatas.count(mimeType))
    {
      mDataId++;
      mDataReceiveQueue.push(std::make_pair(mDataId, mimeType));

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
    while(!mDataReceiveQueue.empty())
    {
      auto item = mDataReceiveQueue.front();
      mDataReceiveQueue.pop();

      uint32_t    requestId   = item.first;
      std::string requestType = item.second;
      std::string data        = "";

      if(mDatas.count(requestType))
      {
        data = mDatas[requestType];
      }

      DALI_LOG_RELEASE_INFO("receive data, success signal emit, id:%u, type:%s, data:%s\n", requestId, requestType.c_str(), data.c_str());
      mDataReceivedSignal.Emit(requestId, requestType.c_str(), data.c_str());
    }
    return false;
  }

  void SetMultiSelectionTimeout()
  {
    mMultiSelectionTimeout = false;
    if(mMultiSelectionTimeoutTimer.IsRunning())
    {
      mMultiSelectionTimeoutTimer.Stop();
    }
    mMultiSelectionTimeoutTimer.Start();
  }

  bool OnMultiSelectionTimeout()
  {
    mMultiSelectionTimeout = true;
    return false;
  }

  Ecore_X_Window mApplicationWindow{};
  uint32_t       mDataId{0};
  std::string    mLastType{};

  std::vector<std::string>                     mMimeTypes{};
  std::map<std::string, std::string>           mDatas{};            // type, data
  std::queue<std::pair<uint32_t, std::string>> mDataReceiveQueue{}; // id, type

  Dali::Clipboard::DataSentSignalType     mDataSentSignal{};
  Dali::Clipboard::DataReceivedSignalType mDataReceivedSignal{};
  Dali::Clipboard::DataSelectedSignalType mDataSelectedSignal{};

  Dali::Timer mDataReceiveTimer{};
  Dali::Timer mMultiSelectionTimeoutTimer{};
  bool        mMultiSelectionTimeout{false};
};

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
  mImpl->mDataReceiveTimer = Dali::Timer::New(10u);
  mImpl->mDataReceiveTimer.TickSignal().Connect(this, &Clipboard::OnReceiveData);

  mImpl->mMultiSelectionTimeoutTimer = Dali::Timer::New(500u);
  mImpl->mMultiSelectionTimeoutTimer.TickSignal().Connect(this, &Clipboard::OnMultiSelectionTimeout);
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

Dali::Clipboard::DataSelectedSignalType& Clipboard::DataSelectedSignal()
{
  return mImpl->mDataSelectedSignal;
}

bool Clipboard::HasType(const std::string& mimeType)
{
  return mImpl->HasType(mimeType);
}

bool Clipboard::SetData(const Dali::Clipboard::ClipData& clipData)
{
  return mImpl->SetData(clipData);
}

uint32_t Clipboard::GetData(const std::string& mimeType)
{
  return mImpl->GetData(mimeType);
}

size_t Clipboard::NumberOfItems()
{
  bool isItem = HasType(MIME_TYPE_TEXT_PLAIN) || HasType(MIME_TYPE_HTML) || HasType(MIME_TYPE_TEXT_URI);
  return isItem ? 1u : 0u;
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

bool Clipboard::OnMultiSelectionTimeout()
{
  return mImpl->OnMultiSelectionTimeout();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
