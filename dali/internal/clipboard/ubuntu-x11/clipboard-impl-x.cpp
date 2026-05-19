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

// EXTERNAL INCLUDES
#include <Ecore.h>
#include <Ecore_X.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <queue>
#include <vector>

#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/clipboard/common/clipboard-factory.h>
#include <dali/internal/window-system/ubuntu-x11/window-interface-ecore-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static Eina_Bool EcoreEventSelectionClear(void* data, int type, void* event);
static Eina_Bool EcoreEventSelectionNotify(void* data, int type, void* event);

// Keep this local until Clipboard MIME constants are made static/publicly reusable.
constexpr const char* MIME_TYPE_TEXT_PLAIN_X11 = "text/plain;charset=utf-8";
constexpr uint32_t    INVALID_DATA_ID          = 0u;

const char* ToX11Target(const std::string& mimeType)
{
  if(mimeType == MIME_TYPE_TEXT_PLAIN_X11)
  {
    return ECORE_X_SELECTION_TARGET_UTF8_STRING;
  }

  return mimeType.c_str();
}

std::string FromX11Target(const char* target)
{
  if(target == nullptr)
  {
    return std::string();
  }

  if(0 == std::strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING) ||
     0 == std::strcmp(target, ECORE_X_SELECTION_TARGET_TEXT) ||
     0 == std::strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
  {
    return MIME_TYPE_TEXT_PLAIN_X11;
  }

  return std::string(target);
}

std::string ExtractTextFromSelectionData(void* data)
{
  if(data == nullptr)
  {
    return std::string();
  }

  Ecore_X_Selection_Data* selectionData = reinterpret_cast<Ecore_X_Selection_Data*>(data);
  if(selectionData->content != Ecore_X_Selection_Data::ECORE_X_SELECTION_CONTENT_TEXT)
  {
    return std::string();
  }

  Ecore_X_Selection_Data_Text* textData = reinterpret_cast<Ecore_X_Selection_Data_Text*>(selectionData);
  if(textData->text == nullptr)
  {
    return std::string();
  }

  return std::string(textData->text);
}
} // unnamed namespace

struct Clipboard::Impl
{
  Impl(Ecore_X_Window ecoreXwin)
  : mApplicationWindow(ecoreXwin)
  {
    mSelectionClearHandler =
      ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, EcoreEventSelectionClear, this);

    mSelectionNotifyHandler =
      ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, EcoreEventSelectionNotify, this);
  }

  ~Impl()
  {
    if(mSelectionClearHandler)
    {
      ecore_event_handler_del(mSelectionClearHandler);
      mSelectionClearHandler = nullptr;
    }

    if(mSelectionNotifyHandler)
    {
      ecore_event_handler_del(mSelectionNotifyHandler);
      mSelectionNotifyHandler = nullptr;
    }
  }

  uint32_t GenerateDataId()
  {
    ++mDataId;

    if(DALI_UNLIKELY(mDataId == INVALID_DATA_ID))
    {
      ++mDataId;
    }

    return mDataId;
  }

  bool HasLocalType(const std::string& mimeType) const
  {
    return std::find(mMimeTypes.begin(), mMimeTypes.end(), mimeType) != mMimeTypes.end();
  }

  bool HasType(const std::string& mimeType)
  {
    if(HasLocalType(mimeType))
    {
      return true;
    }

    // This is an availability check only. It does not query TARGETS.
    // The actual text request result is validated in ReceiveSelectionData().
    if(mimeType == MIME_TYPE_TEXT_PLAIN_X11)
    {
      Ecore_X_Atom clipboardAtom = ecore_x_atom_get("CLIPBOARD");
      return ecore_x_selection_owner_get(clipboardAtom) != 0u;
    }

    return false;
  }

  void UpdateData(const std::string& mimeType, const std::string& data, bool clearBuffer)
  {
    if(clearBuffer)
    {
      mMimeTypes.clear();
      mDatas.clear();
    }

    if(!HasLocalType(mimeType))
    {
      mMimeTypes.push_back(mimeType);
    }

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
      bool clearBuffer = HasLocalType(mimeType);
      UpdateData(mimeType, data, clearBuffer);
    }
    else
    {
      UpdateData(mimeType, data, true);
    }

    if(mimeType == MIME_TYPE_TEXT_PLAIN_X11)
    {
      const int dataSize = static_cast<int>(data.size() + 1u);
      if(!ecore_x_selection_clipboard_set(mApplicationWindow, data.c_str(), dataSize))
      {
        DALI_LOG_ERROR("clipboard set failed, type:%s\n", mimeType.c_str());
        return false;
      }

      mOwnClipboard = true;
      DALI_LOG_RELEASE_INFO("clipboard set, type:%s, data:%s\n", mimeType.c_str(), data.c_str());
    }

    mLastType = mimeType;

    mDataSentSignal.Emit(mimeType.c_str(), data.c_str());
    mDataSelectedSignal.Emit(mimeType.c_str());

    SetMultiSelectionTimeout();

    return true;
  }

  uint32_t GetData(const std::string& mimeType)
  {
    if(mimeType.empty())
    {
      return INVALID_DATA_ID;
    }

    if(mOwnClipboard && mDatas.count(mimeType))
    {
      const uint32_t requestId = GenerateDataId();

      mDataReceiveQueue.push(std::make_pair(requestId, mimeType));

      // For consistency of operation with tizen Wl2, a fake callback occurs using a timer.
      if(mDataReceiveTimer.IsRunning())
      {
        mDataReceiveTimer.Stop();
      }
      mDataReceiveTimer.Start();

      DALI_LOG_RELEASE_INFO("request local data, id:%u, request type:%s\n", requestId, mimeType.c_str());
      return requestId;
    }

    const char* target = ToX11Target(mimeType);
    if(target == nullptr || target[0] == '\0')
    {
      DALI_LOG_ERROR("invalid request type:%s\n", mimeType.c_str());
      return INVALID_DATA_ID;
    }

    const uint32_t requestId = GenerateDataId();

    mDataRequestIds.push_back(requestId);
    mDataRequestItems[requestId] = mimeType;

    DALI_LOG_RELEASE_INFO("request clipboard data, id:%u, request type:%s, target:%s\n", requestId, mimeType.c_str(), target);
    ecore_x_selection_clipboard_request(mApplicationWindow, target);
    ecore_x_flush();

    return requestId;
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

      DALI_LOG_RELEASE_INFO("receive local data, success signal emit, id:%u, type:%s, data:%s\n", requestId, requestType.c_str(), data.c_str());
      mDataReceivedSignal.Emit(requestId, requestType.c_str(), data.c_str());
    }

    return false;
  }

  void ReceiveSelectionData(void* event)
  {
    Ecore_X_Event_Selection_Notify* ev = reinterpret_cast<Ecore_X_Event_Selection_Notify*>(event);
    if(ev == nullptr)
    {
      DALI_LOG_ERROR("selection notify event is null.\n");
      return;
    }

    if(ev->selection != ECORE_X_SELECTION_CLIPBOARD)
    {
      return;
    }

    std::string mimeType = FromX11Target(ev->target);
    std::string content  = ExtractTextFromSelectionData(ev->data);

    DALI_LOG_RELEASE_INFO("receive clipboard data, target:%s, mimeType:%s, data:%s\n",
                          ev->target ? ev->target : "",
                          mimeType.c_str(),
                          content.c_str());

    if(mimeType.empty())
    {
      mimeType = MIME_TYPE_TEXT_PLAIN_X11;
    }

    for(auto it = mDataRequestIds.begin(); it != mDataRequestIds.end();)
    {
      const uint32_t requestId = *it;

      auto itemIt = mDataRequestItems.find(requestId);
      if(itemIt == mDataRequestItems.end())
      {
        it = mDataRequestIds.erase(it);
        continue;
      }

      const std::string requestType = itemIt->second;
      if(requestType == mimeType || requestType == MIME_TYPE_TEXT_PLAIN_X11)
      {
        mDataRequestItems.erase(itemIt);
        it = mDataRequestIds.erase(it);

        if(content.empty())
        {
          DALI_LOG_RELEASE_INFO("receive clipboard data failed or empty, id:%u, request type:%s\n", requestId, requestType.c_str());
          mDataReceivedSignal.Emit(requestId, "", "");
        }
        else
        {
          DALI_LOG_RELEASE_INFO("receive clipboard data, success signal emit, id:%u, type:%s\n", requestId, requestType.c_str());
          mDataReceivedSignal.Emit(requestId, requestType.c_str(), content.c_str());
        }
      }
      else
      {
        ++it;
      }
    }
  }

  void OnSelectionClear(void* event)
  {
    Ecore_X_Event_Selection_Clear* ev = reinterpret_cast<Ecore_X_Event_Selection_Clear*>(event);
    if(ev == nullptr)
    {
      DALI_LOG_ERROR("selection clear event is null.\n");
      return;
    }

    if(ev->selection != ECORE_X_SELECTION_CLIPBOARD)
    {
      return;
    }

    DALI_LOG_RELEASE_INFO("clipboard selection clear\n");

    mOwnClipboard = false;
    mLastType.clear();
    mMimeTypes.clear();
    mDatas.clear();
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
  uint32_t       mDataId{INVALID_DATA_ID};
  std::string    mLastType{};
  bool           mOwnClipboard{false};

  std::vector<std::string>                     mMimeTypes{};
  std::map<std::string, std::string>           mDatas{};            // type, data
  std::queue<std::pair<uint32_t, std::string>> mDataReceiveQueue{}; // id, type

  std::vector<uint32_t>           mDataRequestIds{};
  std::map<uint32_t, std::string> mDataRequestItems{}; // id, type

  Ecore_Event_Handler* mSelectionClearHandler{nullptr};
  Ecore_Event_Handler* mSelectionNotifyHandler{nullptr};

  Dali::Clipboard::DataSentSignalType     mDataSentSignal{};
  Dali::Clipboard::DataReceivedSignalType mDataReceivedSignal{};
  Dali::Clipboard::DataSelectedSignalType mDataSelectedSignal{};

  Dali::Timer mDataReceiveTimer{};
  Dali::Timer mMultiSelectionTimeoutTimer{};
  bool        mMultiSelectionTimeout{false};
};

namespace
{
static Eina_Bool EcoreEventSelectionClear(void* data, int type, void* event)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(data);
  if(impl)
  {
    impl->OnSelectionClear(event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventSelectionNotify(void* data, int type, void* event)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(data);
  if(impl)
  {
    impl->ReceiveSelectionData(event);
  }

  return ECORE_CALLBACK_PASS_ON;
}
} // unnamed namespace

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

class ClipboardFactoryUbuntuX : public ClipboardFactory
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
        Adaptor& adaptorImpl(Adaptor::GetImplementation(Adaptor::Get()));
        Any      nativewindow = adaptorImpl.GetNativeWindowHandle();

        // The Ecore_X_Window needs to use the Clipboard.
        // Only when the render surface is window, we can get the Ecore_X_Window.
        Ecore_X_Window ecoreXwin(AnyCast<Ecore_X_Window>(nativewindow));
        if(ecoreXwin)
        {
          // If we fail to get Ecore_X_Window, we can't use the Clipboard correctly.
          // Thus you have to call \"ecore_imf_context_client_window_set\" somewhere.
          // In EvasPlugIn, this function is called in EvasPlugin::ConnectEcoreEvent().
          Clipboard::Impl* impl(new Clipboard::Impl(ecoreXwin));
          clipboard = Dali::Clipboard(new Clipboard(impl));
          service.Register(typeid(clipboard), clipboard);
        }
      }
    }

    return clipboard;
  }
};

std::unique_ptr<ClipboardFactory> GetClipboardFactory()
{
  return std::unique_ptr<ClipboardFactory>(new ClipboardFactoryUbuntuX());
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
