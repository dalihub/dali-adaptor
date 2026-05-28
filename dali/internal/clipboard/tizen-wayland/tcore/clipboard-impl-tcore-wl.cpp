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
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/common/dali-utility.h>
#include <tizen_core_wl.h>
#include <unistd.h>
#include <limits>
#include <map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const char*    DALI_CLIPBOARD_MULTI_SELECTION_TIMEOUT("DALI_CLIPBOARD_MULTI_SELECTION_TIMEOUT");
const uint32_t DEFAULT_MULTI_SELECTION_TIMEOUT = 500u;
} // namespace

static void TcoreEventDataSend(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data);
static void TcoreEventOfferDataReady(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data);
static void TcoreEventSelectionOffer(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data);

struct Clipboard::Impl
{
  Impl()
  {
    // Ensure display/seat/data are prepared so we can fetch the event handle.
    if(!EnsureDisplaySeatData())
    {
      return;
    }

    tizen_core_event_h event = nullptr;
    if(tizen_core_wl_display_get_event(mDisplay, &event) != TIZEN_CORE_WL_ERROR_NONE || !event)
    {
      return;
    }

    tizen_core_wl_event_listener_h sendHandle      = nullptr;
    tizen_core_wl_event_listener_h receiveHandle   = nullptr;
    tizen_core_wl_event_listener_h selectionHandle = nullptr;

    if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_SOURCE_SEND, TcoreEventDataSend, this, &sendHandle) == TIZEN_CORE_WL_ERROR_NONE)
    {
      mSendHandler = sendHandle;
    }
    if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_READY, TcoreEventOfferDataReady, this, &receiveHandle) == TIZEN_CORE_WL_ERROR_NONE)
    {
      mReceiveHandler = receiveHandle;
    }
    if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_SEAT_SELECTION, TcoreEventSelectionOffer, this, &selectionHandle) == TIZEN_CORE_WL_ERROR_NONE)
    {
      mSelectionHanlder = selectionHandle;
    }

    mEvent = event;
  }
  ~Impl()
  {
    if(mEvent)
    {
      if(mSendHandler)
      {
        tizen_core_wl_event_remove_listener(mEvent, mSendHandler);
      }
      if(mReceiveHandler)
      {
        tizen_core_wl_event_remove_listener(mEvent, mReceiveHandler);
      }
      if(mSelectionHanlder)
      {
        tizen_core_wl_event_remove_listener(mEvent, mSelectionHanlder);
      }
    }
  }

  bool EnsureDisplaySeatData()
  {
    if(mDisplay)
    {
      return true;
    }
    if(tizen_core_wl_get_connected_display(nullptr, &mDisplay) != TIZEN_CORE_WL_ERROR_NONE || !mDisplay)
    {
      return false;
    }
    if(tizen_core_wl_display_get_default_seat(mDisplay, &mSeat) != TIZEN_CORE_WL_ERROR_NONE || !mSeat)
    {
      return false;
    }
    if(tizen_core_wl_seat_get_data(mSeat, &mData) != TIZEN_CORE_WL_ERROR_NONE)
    {
      return false;
    }
    return true;
  }

  bool HasType(const std::string& mimeType)
  {
    if(!EnsureDisplaySeatData())
    {
      return false;
    }

    tizen_core_wl_data_offer_h offer = nullptr;
    if(tizen_core_wl_data_get_selection(mData, &offer) != TIZEN_CORE_WL_ERROR_NONE || !offer)
    {
      DALI_LOG_ERROR("selection_get fail, request type:%s\n", mimeType.c_str());
      return false;
    }

    char** mimetypes = nullptr;
    int    types_num = 0;
    if(tizen_core_wl_data_get_mimes(offer, &mimetypes, &types_num) != TIZEN_CORE_WL_ERROR_NONE || !mimetypes)
    {
      return false;
    }

    for(int i = 0; i < types_num; ++i)
    {
      if(mimetypes[i] && !mimeType.compare(mimetypes[i]))
      {
        return true;
      }
    }
    return false;
  }

  bool SetData(const Dali::Clipboard::ClipData& clipData)
  {
    std::string mimeType = clipData.GetMimeType();
    std::string data     = clipData.GetData();

    if(data.empty())
    {
      DALI_LOG_ERROR("ClipData is empty, return false.\n");
      return false;
    }

    const char* mimeTypes[2];
    mimeTypes[0] = mimeType.c_str();
    mimeTypes[1] = nullptr;

    mSetDatas[mimeType] = data;

    if(!EnsureDisplaySeatData())
    {
      return false;
    }

    uint32_t serial = 0;
    if(tizen_core_wl_data_set_selection(mData, const_cast<const char**>(mimeTypes), &serial) != TIZEN_CORE_WL_ERROR_NONE)
    {
      return false;
    }
    DALI_LOG_RELEASE_INFO("selection_set success, serial:%u, type:%s, data:%s\n", serial, mimeType.c_str(), data.c_str());

    if(mSerial == serial && mLastType != mimeType && !mMultiSelectionTimeout)
    {
      bool typeExists = false;
      for(const auto& type : mSetTypes)
      {
        if(type == mimeType)
        {
          typeExists = true;
          break;
        }
      }

      if(!typeExists)
      {
        mSetTypes.push_back(mimeType);
        std::vector<const char*> types;
        for(const auto& t : mSetTypes)
        {
          types.push_back(t.c_str());
          DALI_LOG_RELEASE_INFO("selection_set multi types, serial:%u, type:%s\n", serial, t.c_str());
        }
        types.push_back(nullptr);
        tizen_core_wl_data_set_selection(mData, const_cast<const char**>(types.data()), &serial);
      }
      else
      {
        mSetTypes.clear();
        mSetTypes.push_back(mimeType);
      }
    }
    else
    {
      mSetTypes.clear();
      mSetTypes.push_back(mimeType);
    }

    mSerial   = serial;
    mLastType = mimeType;
    SetMultiSelectionTimeout();
    return true;
  }

  uint32_t GetData(const std::string& mimeType)
  {
    const char* type = mimeType.c_str();
    if(!type)
    {
      DALI_LOG_ERROR("no request type, type is null.\n");
      return 0u;
    }

    if(!EnsureDisplaySeatData())
    {
      return 0u;
    }

    tizen_core_wl_data_offer_h offer = nullptr;
    if(tizen_core_wl_data_get_selection(mData, &offer) != TIZEN_CORE_WL_ERROR_NONE || !offer)
    {
      DALI_LOG_ERROR("selection_get fail, request type:%s\n", mimeType.c_str());
      mLastOffer = nullptr;
      return 0u;
    }

    char** mimetypes = nullptr;
    int    types_num = 0;
    if(tizen_core_wl_data_get_mimes(offer, &mimetypes, &types_num) != TIZEN_CORE_WL_ERROR_NONE || !mimetypes)
    {
      return 0u;
    }

    const char* selectedType = nullptr;
    for(int i = 0; i < types_num && !selectedType; ++i)
    {
      if(mimetypes[i] && !mimeType.compare(mimetypes[i]))
      {
        selectedType = mimetypes[i];
      }
    }

    if(!selectedType)
    {
      DALI_LOG_ERROR("no matching type, num of available types:%d, request type:%s\n", types_num, mimeType.c_str());
      return 0u;
    }

    uint32_t lastDataId = mDataId;
    mDataId++;
    mDataRequestIds.push_back(mDataId);
    mDataRequestItems[mDataId] = std::make_pair(mimeType, offer);

    if(mLastOffer == offer && mDataRequestItems.count(lastDataId))
    {
      if(std::find(mGetTypes.begin(), mGetTypes.end(), mimeType) == mGetTypes.end())
      {
        mGetTypes.push_back(mimeType);
        mReservedOfferReceives[lastDataId] = mDataId;
      }
    }
    else
    {
      mGetTypes.clear();
      mGetTypes.push_back(mimeType);
      DALI_LOG_RELEASE_INFO("offer_receive, id:%u, request type:%s\n", mDataId, mimeType.c_str());
      tizen_core_wl_data_accept(offer, type);
      tizen_core_wl_data_receive(offer, const_cast<char*>(type));
      if(mDisplay)
      {
        tizen_core_wl_display_flush(mDisplay);
      }
    }

    mLastOffer = offer;
    return mDataId;
  }

  void SendData(void* event)
  {
    tizen_core_wl_event_data_source_send_h ev      = static_cast<tizen_core_wl_event_data_source_send_h>(event);
    char*                                  typeStr = nullptr;
    int                                    fd      = -1;
    uint32_t                               serial  = 0;
    if(tizen_core_wl_event_data_source_send_get_type(ev, &typeStr) != TIZEN_CORE_WL_ERROR_NONE || !typeStr)
    {
      return;
    }
    if(tizen_core_wl_event_data_source_send_get_fd(ev, &fd) != TIZEN_CORE_WL_ERROR_NONE || fd < 0)
    {
      return;
    }
    tizen_core_wl_event_data_source_base_get_serial(static_cast<tizen_core_wl_event_data_source_base_h>(event), &serial);

    if(serial != mSerial)
    {
      DALI_LOG_ERROR("ev->serial:%u, mSerial:%u, type:%s\n", serial, mSerial, typeStr);
      close(fd);
      return;
    }

    std::string type = typeStr;
    std::string data = "";
    if(mSetDatas.count(type))
    {
      data = mSetDatas[type];
    }

    size_t dataSize = data.size() + 1u;
    auto   ret      = write(fd, data.c_str(), dataSize);
    if(DALI_UNLIKELY(ret != static_cast<ssize_t>(dataSize)))
    {
      DALI_LOG_ERROR("write(ev->fd) return %zd\n", ret);
    }

    close(fd);
    DALI_LOG_RELEASE_INFO("send data, type:%s, data:%s \n", typeStr, data.c_str());
    mDataSentSignal.Emit(typeStr, data.c_str());
  }

  void ReceiveData(void* event)
  {
    tizen_core_wl_event_data_ready_h ev = static_cast<tizen_core_wl_event_data_ready_h>(event);
    if(!ev)
    {
      DALI_LOG_ERROR("ev is nullptr.\n");
      return;
    }

    void*       dataPtr  = nullptr;
    int         len      = 0;
    const char* mimetype = nullptr;
    tizen_core_wl_event_data_ready_get_data(ev, &dataPtr);
    tizen_core_wl_event_data_ready_get_len(ev, &len);
    tizen_core_wl_event_data_ready_get_mimetype(ev, &mimetype);

    if((!dataPtr || len < 1) && !mimetype)
    {
      DALI_LOG_ERROR("no selection data.\n");
      return;
    }

    std::string content;
    if(!dataPtr && mimetype)
    {
      DALI_LOG_ERROR("cbhm [%s] data is empty.\n", mimetype);
    }
    else if(dataPtr)
    {
      const char* data       = static_cast<const char*>(dataPtr);
      size_t      dataLength = strlen(data);
      size_t      bufferSize = static_cast<size_t>(len);
      content.append(data, Min(dataLength, bufferSize));
    }

    DALI_LOG_RELEASE_INFO("receive data, type:%s, data:%s\n", mimetype ? mimetype : "", content.c_str());

    tizen_core_wl_data_offer_h eventOffer = nullptr;
    tizen_core_wl_event_data_ready_get_offer(ev, &eventOffer);
    const char* evMimetype = mimetype ? mimetype : "";

    for(auto it = mDataRequestIds.begin(); it != mDataRequestIds.end();)
    {
      uint32_t dataRequestId = *it;
      if(mDataRequestItems.count(dataRequestId))
      {
        const auto&                item     = mDataRequestItems[dataRequestId];
        std::string                mimeType = item.first;
        tizen_core_wl_data_offer_h offer    = item.second;

        if(!mimeType.compare(evMimetype))
        {
          mDataRequestItems.erase(dataRequestId);
          it = mDataRequestIds.erase(it);

          if(offer && offer == eventOffer && mLastOffer == offer)
          {
            if(content.empty())
            {
              DALI_LOG_RELEASE_INFO("content is empty, id:%u\n", dataRequestId);
              mDataReceivedSignal.Emit(dataRequestId, "", "");
            }
            else
            {
              DALI_LOG_RELEASE_INFO("receive data, success signal emit, id:%u, type:%s\n", dataRequestId, mimeType.c_str());
              mDataReceivedSignal.Emit(dataRequestId, mimeType.c_str(), content.c_str());
            }

            if(mReservedOfferReceives.count(dataRequestId))
            {
              if(!content.empty())
              {
                uint32_t reservedId = mReservedOfferReceives[dataRequestId];
                if(mDataRequestItems.count(reservedId))
                {
                  const auto&                reservedItem  = mDataRequestItems[reservedId];
                  std::string                reservedType  = reservedItem.first;
                  tizen_core_wl_data_offer_h reservedOffer = reservedItem.second;

                  if(reservedOffer && mDisplay)
                  {
                    DALI_LOG_RELEASE_INFO("offer_receive, id:%u, request type:%s\n", reservedId, reservedType.c_str());
                    tizen_core_wl_data_accept(reservedOffer, reservedType.c_str());
                    tizen_core_wl_data_receive(reservedOffer, const_cast<char*>(reservedType.c_str()));
                    tizen_core_wl_display_flush(mDisplay);
                  }
                }
              }
              mReservedOfferReceives.erase(dataRequestId);
            }
          }
          else
          {
            DALI_LOG_RELEASE_INFO("invalid offer, id:%u, request type:%s\n", dataRequestId, mimeType.c_str());
            mDataReceivedSignal.Emit(dataRequestId, "", "");
            if(mReservedOfferReceives.count(dataRequestId))
            {
              mReservedOfferReceives.erase(dataRequestId);
            }
          }
        }
        else
        {
          ++it;
        }
      }
      else
      {
        it = mDataRequestIds.erase(it);
      }
    }
  }

  void SelectionOffer(void* event)
  {
    tizen_core_wl_event_data_seat_selection_h ev = static_cast<tizen_core_wl_event_data_seat_selection_h>(event);
    if(!ev)
    {
      DALI_LOG_ERROR("ev is nullptr.\n");
      return;
    }

    int          num_types = 0;
    const char** types     = nullptr;
    if(tizen_core_wl_event_data_seat_selection_get_num_types(ev, &num_types) != TIZEN_CORE_WL_ERROR_NONE || num_types < 1)
    {
      DALI_LOG_ERROR("num type is 0.\n");
      return;
    }
    if(tizen_core_wl_event_data_seat_selection_get_types(ev, &types) != TIZEN_CORE_WL_ERROR_NONE || !types)
    {
      DALI_LOG_ERROR("types is nullptr.\n");
      return;
    }

    const char* selectedType = nullptr;
    std::string formatMarkup("application/x-elementary-markup");

    for(int i = 0; i < num_types; i++)
    {
      if(types[i] && !formatMarkup.compare(types[i]))
      {
        continue;
      }
      selectedType = types[i];
      DALI_LOG_RELEASE_INFO("data selected signal emit, type:%s\n", selectedType ? selectedType : "");
      mDataSelectedSignal.Emit(selectedType ? selectedType : "");
    }

    if(!selectedType)
    {
      DALI_LOG_ERROR("mime type is invalid.\n");
    }
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
    DALI_LOG_RELEASE_INFO("multi-selection end\n");
    mMultiSelectionTimeout = true;
    return false;
  }

  uint32_t                   mSerial{std::numeric_limits<uint32_t>::max()};
  std::string                mLastType{};
  tizen_core_wl_data_offer_h mLastOffer{nullptr};

  tizen_core_wl_display_h mDisplay{nullptr};
  tizen_core_wl_seat_h    mSeat{nullptr};
  tizen_core_wl_data_h    mData{nullptr};
  tizen_core_event_h      mEvent{nullptr};

  tizen_core_wl_event_listener_h mSendHandler{nullptr};
  tizen_core_wl_event_listener_h mReceiveHandler{nullptr};
  tizen_core_wl_event_listener_h mSelectionHanlder{nullptr};

  Dali::Clipboard::DataSentSignalType     mDataSentSignal{};
  Dali::Clipboard::DataReceivedSignalType mDataReceivedSignal{};
  Dali::Clipboard::DataSelectedSignalType mDataSelectedSignal{};

  uint32_t                                                               mDataId{0};
  std::vector<uint32_t>                                                  mDataRequestIds{};
  std::map<uint32_t, std::pair<std::string, tizen_core_wl_data_offer_h>> mDataRequestItems{};

  std::vector<std::string>           mSetTypes{};
  std::map<std::string, std::string> mSetDatas{};
  std::vector<std::string>           mGetTypes{};
  std::map<uint32_t, uint32_t>       mReservedOfferReceives{};

  Dali::Timer mMultiSelectionTimeoutTimer{};
  bool        mMultiSelectionTimeout{false};
};

static void TcoreEventDataSend(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(user_data);
  impl->SendData(event_data);
}

static void TcoreEventOfferDataReady(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(user_data);
  impl->ReceiveData(event_data);
}

static void TcoreEventSelectionOffer(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(user_data);
  impl->SelectionOffer(event_data);
}

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
  auto     timeoutString         = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_CLIPBOARD_MULTI_SELECTION_TIMEOUT);
  uint32_t multiSelectionTimeout = timeoutString ? static_cast<uint32_t>(std::atoi(timeoutString)) : DEFAULT_MULTI_SELECTION_TIMEOUT;

  DALI_LOG_RELEASE_INFO("multi-selection timeout set:%u\n", multiSelectionTimeout);
  mImpl->mMultiSelectionTimeoutTimer = Dali::Timer::New(multiSelectionTimeout);
  mImpl->mMultiSelectionTimeoutTimer.TickSignal().Connect(this, &Clipboard::OnMultiSelectionTimeout);
}

Clipboard::~Clipboard()
{
  delete mImpl;
}

class ClipboardFactoryTcoreWl : public ClipboardFactory
{
public:
  Dali::Clipboard CreateClipboard() override
  {
    Dali::Clipboard clipboard;

    Dali::SingletonService service(SingletonService::Get());
    if(service)
    {
      Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::Clipboard));
      if(handle)
      {
        clipboard = Dali::Clipboard(static_cast<Clipboard*>(handle.GetObjectPtr()));
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
  return std::unique_ptr<ClipboardFactory>(new ClipboardFactoryTcoreWl());
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
  return false;
}

bool Clipboard::OnMultiSelectionTimeout()
{
  return mImpl->OnMultiSelectionTimeout();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
