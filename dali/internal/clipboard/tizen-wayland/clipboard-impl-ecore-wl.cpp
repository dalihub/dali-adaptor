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
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <unistd.h>
#include <map>

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

static Eina_Bool EcoreEventDataSend(void* data, int type, void* event);
static Eina_Bool EcoreEventOfferDataReady(void* data, int type, void* event);
static Eina_Bool EcoreEventSelectionOffer(void* data, int type, void* event);

struct Clipboard::Impl
{
  Impl()
  {
    mSendHandler      = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND, EcoreEventDataSend, this);
    mReceiveHandler   = ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, EcoreEventOfferDataReady, this);
    mSelectionHanlder = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_SELECTION, EcoreEventSelectionOffer, this);
  }
  ~Impl()
  {
    ecore_event_handler_del(mSendHandler);
    ecore_event_handler_del(mReceiveHandler);
    ecore_event_handler_del(mSelectionHanlder);
  }

  bool HasType(const std::string& mimeType)
  {
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
    Ecore_Wl2_Offer*   offer   = ecore_wl2_dnd_selection_get(input);

    if(!offer)
    {
      DALI_LOG_ERROR("selection_get fail, request type:%s\n", mimeType.c_str());
      return false;
    }

    Eina_Array*  availableTypes = ecore_wl2_offer_mimes_get(offer);
    unsigned int typeCount      = (unsigned int)eina_array_count((Eina_Array*)availableTypes);

    for(unsigned int i = 0; i < typeCount; ++i)
    {
      char* availableType = (char*)eina_array_data_get((Eina_Array*)availableTypes, i);
      if(!mimeType.compare(availableType))
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

    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);

    uint32_t serial = ecore_wl2_dnd_selection_set(input, mimeTypes);
    DALI_LOG_RELEASE_INFO("selection_set success, serial:%u, type:%s, data:%s\n", serial, mimeType.c_str(), data.c_str());

    // If the serial is the same, it is the same source.
    // If the type is the same, it is a separate copy.
    if(mSerial == serial && mLastType != mimeType && !mMultiSelectionTimeout)
    {
      // Checks whether there is an identical type requested from one source.
      bool typeExists = false;
      for(const auto& type : mSetTypes)
      {
        if(type == mimeType)
        {
          typeExists = true;
          break;
        }
      }

      if(!typeExists) // Same copy.
      {
        // It requests all types of copies requested from one source at once.
        // EcoreEventDataSend callback is called as many as the number of requested types.
        mSetTypes.push_back(mimeType);

        size_t      typeCount = mSetTypes.size();
        const char* types[typeCount + 1];
        for(size_t i = 0; i < typeCount; i++)
        {
          types[i] = mSetTypes[i].c_str();
          DALI_LOG_RELEASE_INFO("selection_set multi types, serial:%u, type:%s\n", serial, types[i]);
        }
        types[typeCount] = nullptr;

        // TODO : At this point, it is impossible to avoid duplicate calls,
        // because we cannot know how many more times the copy will be called for the same source.
        serial = ecore_wl2_dnd_selection_set(input, types);
      }
      else // Separate copy.
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

    // Store the last serial and type.
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

    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
    Ecore_Wl2_Offer*   offer   = ecore_wl2_dnd_selection_get(input);

    if(!offer)
    {
      DALI_LOG_ERROR("selection_get fail, request type:%s\n", mimeType.c_str());
      mLastOffer = nullptr;
      return 0u;
    }

    Eina_Array*  availableTypes = ecore_wl2_offer_mimes_get(offer);
    char*        selectedType   = nullptr;
    unsigned int typeCount      = (unsigned int)eina_array_count((Eina_Array*)availableTypes);

    for(unsigned int i = 0; i < typeCount && !selectedType; ++i)
    {
      char* availableType = (char*)eina_array_data_get((Eina_Array*)availableTypes, i);
      if(!mimeType.compare(availableType))
      {
        selectedType = availableType;
        break;
      }
    }

    if(!selectedType)
    {
      DALI_LOG_ERROR("no matching type, num of available types:%u, request type:%s\n", typeCount, mimeType.c_str());
      for(unsigned int i = 0; i < typeCount && !selectedType; ++i)
      {
        DALI_LOG_ERROR("available type[%u]:%s\n", i, (char*)eina_array_data_get((Eina_Array*)availableTypes, i));
      }
      return 0u;
    }

    uint32_t lastDataId = mDataId;
    mDataId++;
    mDataRequestIds.push_back(mDataId);
    mDataRequestItems[mDataId] = std::make_pair(mimeType, offer);

    // Not yet received a callback for the recent offer receive.
    if(mLastOffer == offer && mDataRequestItems.count(lastDataId))
    {
      // A receive request for the same offer and type is made only once.
      if(std::find(mGetTypes.begin(), mGetTypes.end(), mimeType) == mGetTypes.end())
      {
        mGetTypes.push_back(mimeType);
        mReservedOfferReceives[lastDataId] = mDataId;
      } // else do nothing.
    }
    else
    {
      mGetTypes.clear();
      mGetTypes.push_back(mimeType);

      DALI_LOG_RELEASE_INFO("offer_receive, id:%u, request type:%s\n", mDataId, mimeType.c_str());
      ecore_wl2_offer_receive(offer, const_cast<char*>(type));
      ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
    }

    mLastOffer = offer;

    return mDataId;
  }

  void SendData(void* event)
  {
    Ecore_Wl2_Event_Data_Source_Send* ev = reinterpret_cast<Ecore_Wl2_Event_Data_Source_Send*>(event);

    if(ev->serial != mSerial)
    {
      DALI_LOG_ERROR("ev->serial:%u, mSerial:%u, type:%s\n", ev->serial, mSerial, ev->type);
      return;
    }

    // Check whether the hash has data of the requested type.
    // If there is no data of the requested type, something has already gone wrong.
    std::string type = ev->type;
    std::string data = "";
    if(mSetDatas.count(type))
    {
      data = mSetDatas[type];
    }

    size_t dataLength = strlen(data.c_str());
    size_t bufferSize = dataLength + 1u;

    char* buffer = new char[bufferSize];
    if(!buffer)
    {
      return;
    }

    memcpy(buffer, data.c_str(), dataLength);
    buffer[dataLength] = '\0';

    auto ret = write(ev->fd, buffer, bufferSize);
    if(DALI_UNLIKELY(ret != static_cast<ssize_t>(bufferSize)))
    {
      DALI_LOG_ERROR("write(ev->fd) return %zd\n", ret);
    }

    close(ev->fd);
    delete[] buffer;

    DALI_LOG_RELEASE_INFO("send data, type:%s, data:%s \n", ev->type, data.c_str());
    mDataSentSignal.Emit(ev->type, data.c_str());
  }

  void ReceiveData(void* event)
  {
    Ecore_Wl2_Event_Offer_Data_Ready* ev = reinterpret_cast<Ecore_Wl2_Event_Offer_Data_Ready*>(event);

    if(ev == nullptr)
    {
      DALI_LOG_ERROR("ev is nullptr.\n");
      return;
    }

    if(ev->data == nullptr || ev->len < 1)
    {
      DALI_LOG_ERROR("no selection data.\n");
      return;
    }

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

    // Retrieve request id list.
    for(auto it = mDataRequestIds.begin(); it != mDataRequestIds.end();)
    {
      uint32_t dataRequestId = *it;
      if(mDataRequestItems.count(dataRequestId))
      {
        const auto&      item     = mDataRequestItems[dataRequestId];
        std::string      mimeType = static_cast<std::string>(item.first);
        Ecore_Wl2_Offer* offer    = static_cast<Ecore_Wl2_Offer*>(item.second);

        // Processes all the same types stored in the request list.
        if(!mimeType.compare(ev->mimetype))
        {
          mDataRequestItems.erase(dataRequestId);
          it = mDataRequestIds.erase(it);

          // A change in an offer means a change in the clipboard's data.
          // Old offers are not always invalid, but at least in Dali it is unknown whether they are valid or not.
          // For safe processing, old offers are considered invalid offers.
          if(offer && offer == ev->offer && mLastOffer == offer)
          {
            DALI_LOG_RELEASE_INFO("receive data, success signal emit, id:%u, type:%s\n", dataRequestId, mimeType.c_str());
            mDataReceivedSignal.Emit(dataRequestId, mimeType.c_str(), content.c_str());

            if(mReservedOfferReceives.count(dataRequestId))
            {
              uint32_t reservedId = mReservedOfferReceives[dataRequestId];
              if(mDataRequestItems.count(reservedId))
              {
                const auto&      reservedItem  = mDataRequestItems[reservedId];
                std::string      reservedType  = static_cast<std::string>(reservedItem.first);
                Ecore_Wl2_Offer* reservedOffer = static_cast<Ecore_Wl2_Offer*>(reservedItem.second);

                if(reservedOffer)
                {
                  Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
                  Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);

                  DALI_LOG_RELEASE_INFO("offer_receive, id:%u, request type:%s\n", reservedId, reservedType.c_str());
                  ecore_wl2_offer_receive(reservedOffer, const_cast<char*>(reservedType.c_str()));
                  ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
                }
              }
              mReservedOfferReceives.erase(dataRequestId);
            }
          }
          else // null or invalid offer.
          {
            DALI_LOG_RELEASE_INFO("invalid offer, id:%u, request type:%s\n", dataRequestId, mimeType.c_str());
            mDataReceivedSignal.Emit(dataRequestId, "", "");

            if(mReservedOfferReceives.count(dataRequestId))
            {
              mReservedOfferReceives.erase(dataRequestId);
            }
          }
        }
        else // item's type and event data's type are different.
        {
          ++it;
        }
      }
      else // There is no id in request items.
      {
        it = mDataRequestIds.erase(it);
      }
    }
  }

  void SelectionOffer(void* event)
  {
    Ecore_Wl2_Event_Seat_Selection* ev = reinterpret_cast<Ecore_Wl2_Event_Seat_Selection*>(event);

    if(ev == nullptr)
    {
      DALI_LOG_ERROR("ev is nullptr.\n");
      return;
    }

    if(ev->num_types < 1)
    {
      DALI_LOG_ERROR("num type is 0.\n");
      return;
    }

    if(ev->types == nullptr)
    {
      DALI_LOG_ERROR("types is nullptr.\n");
      return;
    }

    const char* selectedType = nullptr;
    std::string formatMarkup("application/x-elementary-markup");

    for(int i = 0; i < ev->num_types; i++)
    {
      if(!formatMarkup.compare(ev->types[i]))
      {
        // Ignore elementary markup from efl.
        continue;
      }

      selectedType = ev->types[i];
      DALI_LOG_RELEASE_INFO("data selected signal emit, type:%s\n", selectedType);
      mDataSelectedSignal.Emit(selectedType);
    }

    if(!selectedType)
    {
      DALI_LOG_ERROR("mime type is invalid.\n");
      return;
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

  uint32_t         mSerial{std::numeric_limits<uint32_t>::max()};
  std::string      mLastType{};         // mime type used in last copy.
  Ecore_Wl2_Offer* mLastOffer{nullptr}; // offer used in last paste.

  Ecore_Event_Handler* mSendHandler{nullptr};
  Ecore_Event_Handler* mReceiveHandler{nullptr};
  Ecore_Event_Handler* mSelectionHanlder{nullptr};

  Dali::Clipboard::DataSentSignalType     mDataSentSignal{};
  Dali::Clipboard::DataReceivedSignalType mDataReceivedSignal{};
  Dali::Clipboard::DataSelectedSignalType mDataSelectedSignal{};

  uint32_t                                                     mDataId{0};
  std::vector<uint32_t>                                        mDataRequestIds{};
  std::map<uint32_t, std::pair<std::string, Ecore_Wl2_Offer*>> mDataRequestItems{};

  std::vector<std::string>           mSetTypes{};              // types for the same source (one user copy).
  std::map<std::string, std::string> mSetDatas{};              // datas for the same source (one user copy), key is mime type, value is data.
  std::vector<std::string>           mGetTypes{};              // types requested to receive for the same offer.
  std::map<uint32_t, uint32_t>       mReservedOfferReceives{}; // in order to process offer receive sequentially, key is current id, value is reserved id.

  Dali::Timer mMultiSelectionTimeoutTimer{};
  bool        mMultiSelectionTimeout{false};
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

static Eina_Bool EcoreEventSelectionOffer(void* data, int type, void* event)
{
  Clipboard::Impl* impl = reinterpret_cast<Clipboard::Impl*>(data);
  impl->SelectionOffer(event);

  return ECORE_CALLBACK_PASS_ON;
}

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
  // Check environment variable for DALI_CLIPBOARD_MULTI_SELECTION_TIMEOUT
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
