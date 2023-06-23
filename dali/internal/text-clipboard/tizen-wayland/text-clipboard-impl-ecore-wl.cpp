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
#include <dali/devel-api/adaptor-framework/text-clipboard-event-notifier.h>
#include <dali/internal/text-clipboard/common/text-clipboard-impl.h>

// EXTERNAL INCLUDES
#include <dali/internal/system/linux/dali-ecore.h>

#ifdef ECORE_WAYLAND2
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
#else
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wayland.h>
#endif

#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/type-registry.h>
#include <unistd.h>

#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

#define CBHM_DBUS_OBJPATH "/org/tizen/cbhm/dbus"
#ifndef CBHM_DBUS_INTERFACE
#define CBHM_DBUS_INTERFACE "org.tizen.cbhm.dbus"
#endif                   /* CBHM_DBUS_INTERFACE */
#define CBHM_COUNT_ALL 0 // ATOM_INDEX_CBHM_COUNT_ALL

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextClipboard
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct TextClipboard::Impl
{
  Impl()
  {
#ifdef DALI_ELDBUS_AVAILABLE
    Eldbus_Object* eldbus_obj;
    eldbus_init();
    cbhm_conn    = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
    eldbus_obj   = eldbus_object_get(cbhm_conn, CBHM_DBUS_INTERFACE, CBHM_DBUS_OBJPATH);
    eldbus_proxy = eldbus_proxy_get(eldbus_obj, CBHM_DBUS_INTERFACE);
    eldbus_name_owner_changed_callback_add(cbhm_conn, CBHM_DBUS_INTERFACE, NULL, cbhm_conn, EINA_TRUE);
    eldbus_proxy_signal_handler_add(eldbus_proxy, "ItemClicked", _on_item_clicked, this);
#endif // DALI_ELDBUS_AVAILABLE
    mVisible           = false;
    mIsFirstTimeHidden = true;
  }

  ~Impl()
  {
#ifdef DALI_ELDBUS_AVAILABLE
    if(cbhm_conn)
      eldbus_connection_unref(cbhm_conn);
    eldbus_shutdown();
#endif // DALI_ELDBUS_AVAILABLE
  }

#ifdef DALI_ELDBUS_AVAILABLE
  Eldbus_Proxy* cbhm_proxy_get()
  {
    return eldbus_proxy;
  }

  Eldbus_Connection* cbhm_connection_get()
  {
    return cbhm_conn;
  }
#endif // DALI_ELDBUS_AVAILABLE

  void SetItem(const std::string& itemData)
  {
    const char* types[10] = {
      0,
    };
    int i = -1;

    if(itemData.length() == 0)
    {
      return;
    }
    mSendBuffer = itemData;

    // ELM_SEL_TYPE_CLIPBOARD - To distinguish clipboard selection in cbhm
    types[++i] = "CLIPBOARD_BEGIN";

    types[++i] = "text/plain;charset=utf-8";

    // ELM_SEL_TYPE_CLIPBOARD - To distinguish clipboard selection in cbhm
    types[++i] = "CLIPBOARD_END";

#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    mSerial                = ecore_wl2_dnd_selection_set(input, types);
#else
    mSerial                = ecore_wl_dnd_selection_set(ecore_wl_input_get(), types);
#endif
  }

  void RequestItem()
  {
    const char* types[10] = {
      0,
    };
    int i = -1;

    types[++i] = "text/plain;charset=utf-8";

#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
    Ecore_Wl2_Offer*   offer   = ecore_wl2_dnd_selection_get(input);

    ecore_wl2_offer_receive(offer, const_cast<char*>(*types));
    ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
#else
    ecore_wl_dnd_selection_get(ecore_wl_input_get(), *types);
#endif
  }

  void ExcuteSend(void* event)
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Event_Data_Source_Send* ev = reinterpret_cast<Ecore_Wl2_Event_Data_Source_Send*>(event);
#else
    Ecore_Wl_Event_Data_Source_Send*  ev = reinterpret_cast<Ecore_Wl_Event_Data_Source_Send*>(event);
#endif

    if(ev->serial != mSerial)
    {
      return;
    }

    int         len_buf      = mSendBuffer.length() + 1; // we should consider the char* buffer length
    int         len_remained = len_buf;
    int         len_written  = 0, ret;
    const char* buf          = mSendBuffer.c_str(); // last char in the buffer must be \0

    while(len_written < len_buf)
    {
      ret = write(ev->fd, buf, len_remained);
      if(ret == -1) break;
      buf += ret;
      len_written += ret;
      len_remained -= ret;
    }
    close(ev->fd);
  }

  void ExcuteReceive(void* event, char*& data, int& length)
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Event_Offer_Data_Ready* ev = reinterpret_cast<Ecore_Wl2_Event_Offer_Data_Ready*>(event);
#else
    Ecore_Wl_Event_Selection_Data_Ready* ev = reinterpret_cast<Ecore_Wl_Event_Selection_Data_Ready*>(event);
#endif
    data   = reinterpret_cast<char*>(ev->data);
    length = ev->len;
  }

  int GetCount()
  {
#ifdef DALI_ELDBUS_AVAILABLE
    Eldbus_Message *reply, *req;
    const char *    errname = NULL, *errmsg = NULL;
#endif // DALI_ELDBUS_AVAILABLE
    int count = -1;

#ifdef DALI_ELDBUS_AVAILABLE
    if(!(req = eldbus_proxy_method_call_new(eldbus_proxy, "CbhmGetCount")))
    {
      DALI_LOG_ERROR("Failed to create method call on org.freedesktop.DBus.Properties.Get");
      return -1;
    }

    eldbus_message_ref(req);
    eldbus_message_arguments_append(req, "i", CBHM_COUNT_ALL);
    reply = eldbus_proxy_send_and_block(eldbus_proxy, req, 100);
    if(!reply || eldbus_message_error_get(reply, &errname, &errmsg))
    {
      DALI_LOG_ERROR("Unable to call method org.freedesktop.DBus.Properties.Get: %s %s",
                     errname,
                     errmsg);
      eldbus_message_unref(req);
      if(reply)
      {
        eldbus_message_unref(reply);
      }
      return -1;
    }

    if(!eldbus_message_arguments_get(reply, "i", &count))
    {
      DALI_LOG_ERROR("Cannot get arguments from eldbus");
      eldbus_message_unref(req);
      eldbus_message_unref(reply);
      return -1;
    }

    eldbus_message_unref(req);
    eldbus_message_unref(reply);
    DALI_LOG_ERROR("cbhm item count(%d)", count);
#endif // DALI_ELDBUS_AVAILABLE
    return count;
  }

  void ShowClipboard()
  {
#ifdef DALI_ELDBUS_AVAILABLE
    eldbus_proxy_call(cbhm_proxy_get(), "CbhmShow", NULL, NULL, -1, "s", "0");
#endif // DALI_ELDBUS_AVAILABLE
    mIsFirstTimeHidden = true;
    mVisible           = true;
  }

  void HideClipboard(bool skipFirstHide)
  {
    if(skipFirstHide && mIsFirstTimeHidden)
    {
      mIsFirstTimeHidden = false;
      return;
    }
#ifdef DALI_ELDBUS_AVAILABLE
    eldbus_proxy_call(cbhm_proxy_get(), "CbhmHide", NULL, NULL, -1, "");
#endif // DALI_ELDBUS_AVAILABLE
    mIsFirstTimeHidden = false;
    mVisible           = false;
  }

  bool IsVisible() const
  {
    return mVisible;
  }

#ifdef DALI_ELDBUS_AVAILABLE
  static void _on_item_clicked(void* data, const Eldbus_Message* msg EINA_UNUSED)
  {
    static_cast<TextClipboard::Impl*>(data)->RequestItem();
  }

  Eldbus_Proxy*      eldbus_proxy;
  Eldbus_Connection* cbhm_conn;
#endif // DALI_ELDBUS_AVAILABLE

  std::string mSendBuffer;
  bool        mVisible;
  bool        mIsFirstTimeHidden;
  uint32_t    mSerial{0u};
};

TextClipboard::TextClipboard(Impl* impl)
: mImpl(impl)
{
}

TextClipboard::~TextClipboard()
{
  delete mImpl;
}

Dali::TextClipboard TextClipboard::Get()
{
  Dali::TextClipboard clipboard;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TextClipboard));
    if(handle)
    {
      // If so, downcast the handle
      clipboard = Dali::TextClipboard(dynamic_cast<TextClipboard*>(handle.GetObjectPtr()));
    }
    else
    {
      TextClipboard::Impl* impl(new TextClipboard::Impl());
      clipboard = Dali::TextClipboard(new TextClipboard(impl));
      service.Register(typeid(Dali::TextClipboard), clipboard);
    }
  }

  return clipboard;
}

bool TextClipboard::IsAvailable()
{
  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TextClipboard));
    if(handle)
    {
      return true;
    }
  }
  return false;
}

bool TextClipboard::SetItem(const std::string& itemData)
{
  mImpl->SetItem(itemData);
  return true;
}

/*
 * Request clipboard service to give an item
 */
void TextClipboard::RequestItem()
{
  mImpl->RequestItem();
}

/*
 * Get number of items in clipboard
 */
unsigned int TextClipboard::NumberOfItems()
{
  int count = mImpl->GetCount();
  return (count < 0 ? 0 : count);
}

void TextClipboard::ShowClipboard()
{
  mImpl->ShowClipboard();
}

void TextClipboard::HideClipboard(bool skipFirstHide)
{
  mImpl->HideClipboard(skipFirstHide);
}

bool TextClipboard::IsVisible() const
{
  return mImpl->IsVisible();
}

void TextClipboard::ExcuteSend(void* event)
{
  mImpl->ExcuteSend(event);
}

void TextClipboard::ExcuteReceive(void* event, char*& data, int& length)
{
  mImpl->ExcuteReceive(event, data, length);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali