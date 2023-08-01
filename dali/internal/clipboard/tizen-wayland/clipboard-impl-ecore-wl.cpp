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
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
#include <dali/integration-api/debug.h>
#include <unistd.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
static Eina_Bool EcoreEventDataSend(void* data, int type, void* event);
static Eina_Bool EcoreEventOfferDataReady(void* data, int type, void* event);

struct Clipboard::Impl
{
  Impl()
  {
    mSendHandler    = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND, EcoreEventDataSend, this);
    mReceiveHandler = ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, EcoreEventOfferDataReady, this);
  }
  ~Impl()
  {
    ecore_event_handler_del(mSendHandler);
    ecore_event_handler_del(mReceiveHandler);
  }

  bool SetData(const Dali::Clipboard::ClipData& clipData)
  {
    mMimeType = clipData.GetMimeType();
    mData     = clipData.GetData();

    if(mData.empty())
    {
      DALI_LOG_ERROR("ClipData is empty, return false.\n");
      return false;
    }

    const char* mimeTypes[2];
    mimeTypes[0] = mMimeType.c_str();
    mimeTypes[1] = nullptr;

    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    Ecore_Wl2_Input* input     = ecore_wl2_input_default_input_get(display);
    mSerial                    = ecore_wl2_dnd_selection_set(input, mimeTypes);
    DALI_LOG_RELEASE_INFO("selection_set success, serial:%u, type:%s, data:%s\n", mSerial, mMimeType.c_str(), mData.c_str());

    return true;
  }

  uint32_t GetData(const std::string &mimeType)
  {
    const char* type = mimeType.c_str();
    if(type)
    {
      Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
      Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
      Ecore_Wl2_Offer*   offer   = ecore_wl2_dnd_selection_get(input);

      if(!offer)
      {
        DALI_LOG_ERROR("selection_get fail, request type:%s\n", mimeType.c_str());
        return 0u;
      }
      else
      {
        Eina_Array* availableTypes = ecore_wl2_offer_mimes_get(offer);
        char*       selectedType = nullptr;

        for(unsigned int i = 0; i < (unsigned int)eina_array_count((Eina_Array *)availableTypes) && !selectedType; ++i)
        {
          char* availableType = (char*)eina_array_data_get((Eina_Array *)availableTypes, i);
          if(!mimeType.compare(availableType))
          {
            selectedType = availableType;
            break;
          }
        }

        if(selectedType)
        {
          mDataId++;
          mDataRequestIds.push_back(mDataId);
          mDataRequestItems[mDataId] = std::make_pair(mimeType, offer);
          ecore_wl2_offer_receive(offer, const_cast<char*>(type));
          ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
          DALI_LOG_RELEASE_INFO("offer_receive, id:%u, request type:%s\n", mDataId, mimeType.c_str());
          return mDataId;
        }
        else
        {
          DALI_LOG_ERROR("no matching type, request type:%s\n", mimeType.c_str());
          return 0u;
        }
      }
    }

    DALI_LOG_ERROR("no request type, type is null.\n");
    return 0u;
  }

  void SendData(void* event)
  {
    Ecore_Wl2_Event_Data_Source_Send* ev = reinterpret_cast<Ecore_Wl2_Event_Data_Source_Send*>(event);

    if(ev->serial != mSerial)
    {
      return;
    }

    // no matching mime type.
    if(mMimeType.compare(ev->type))
    {
      auto it = mDataRequestIds.begin();
      while(it != mDataRequestIds.end())
      {
        uint32_t dataRequestId = *it;
        auto     item          = mDataRequestItems.find(dataRequestId);
        if(item != mDataRequestItems.end())
        {
          std::string mimeType = static_cast<std::string>(item->second.first);
          if(!mimeType.compare(ev->type))
          {
            mDataRequestItems.erase(dataRequestId);
            it = mDataRequestIds.erase(it);
            DALI_LOG_ERROR("no matching type, empty signal emit, request type:%s, available type:%s\n", ev->type, mMimeType.c_str());
            mDataReceivedSignal.Emit(dataRequestId, "", "");
          }
          else
          {
            ++it;
          }
        }
      }
      return;
    }

    size_t dataLength = strlen(mData.c_str());
    size_t bufferSize = dataLength + 1u;

    char* buffer = new char[bufferSize];
    if(!buffer)
    {
      return;
    }

    memcpy(buffer, mData.c_str(), dataLength);
    buffer[dataLength] = '\0';

    auto ret = write(ev->fd, buffer, bufferSize);
    if(DALI_UNLIKELY(ret != static_cast<ssize_t>(bufferSize)))
    {
      DALI_LOG_ERROR("write(ev->fd) return %zd\n", ret);
    }

    close(ev->fd);
    delete[] buffer;

    DALI_LOG_RELEASE_INFO("send data, type:%s, data:%s \n", mMimeType.c_str(), mData.c_str());
    mDataSentSignal.Emit(ev->type, mData.c_str());
  }

  void ReceiveData(void* event)
  {
    Ecore_Wl2_Event_Offer_Data_Ready* ev = reinterpret_cast<Ecore_Wl2_Event_Offer_Data_Ready*>(event);

    size_t      dataLength = strlen(ev->data);
    size_t      bufferSize = static_cast<size_t>(ev->len);
    std::string content;

    if(dataLength < bufferSize)
    {
      content.append(ev->data, dataLength);
    }
    else
    {
      content.append(ev->data, bufferSize);
    }

    DALI_LOG_RELEASE_INFO("receive data, type:%s, data:%s\n", ev->mimetype, content.c_str());

    auto it = mDataRequestIds.begin();
    while(it != mDataRequestIds.end())
    {
      uint32_t dataRequestId = *it;
      auto     item          = mDataRequestItems.find(dataRequestId);
      if(item != mDataRequestItems.end())
      {
        Ecore_Wl2_Offer* offer = static_cast<Ecore_Wl2_Offer*>(item->second.second);
        if(offer == ev->offer)
        {
          std::string mimeType = static_cast<std::string>(item->second.first);
          mDataRequestItems.erase(dataRequestId);
          it = mDataRequestIds.erase(it);
          DALI_LOG_RELEASE_INFO("receive data, success signal emit, id:%u, type:%s\n", dataRequestId, mimeType.c_str());
          mDataReceivedSignal.Emit(dataRequestId, mimeType.c_str(), content.c_str());
        }
        else
        {
          ++it;
        }
      }
    }
  }

  uint32_t             mSerial{0u};
  std::string          mMimeType;
  std::string          mData;
  Ecore_Event_Handler* mSendHandler{nullptr};
  Ecore_Event_Handler* mReceiveHandler{nullptr};

  Dali::Clipboard::DataSentSignalType     mDataSentSignal;
  Dali::Clipboard::DataReceivedSignalType mDataReceivedSignal;

  uint32_t mDataId{0};
  std::vector<uint32_t> mDataRequestIds;
  std::unordered_map<uint32_t, std::pair<std::string, Ecore_Wl2_Offer*>> mDataRequestItems;
};

static Eina_Bool EcoreEventDataSend(void* data, int type, void* event)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(data);
  impl->SendData(event);

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventOfferDataReady(void* data, int type, void* event)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(data);
  impl->ReceiveData(event);

  return ECORE_CALLBACK_PASS_ON;
}

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
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
      Clipboard::Impl* impl(new Clipboard::Impl());
      clipboard = Dali::Clipboard(new Clipboard(impl));
      service.Register(typeid(Dali::Clipboard), clipboard);
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
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
