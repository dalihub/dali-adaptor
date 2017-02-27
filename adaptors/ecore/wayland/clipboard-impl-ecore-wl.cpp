/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "clipboard-impl.h"

// EXTERNAL INCLUDES
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <unistd.h>

#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

// INTERNAL INCLUDES
#include <singleton-service-impl.h>

#define CBHM_DBUS_OBJPATH "/org/tizen/cbhm/dbus"
#ifndef CBHM_DBUS_INTERFACE
#define CBHM_DBUS_INTERFACE "org.tizen.cbhm.dbus"
#endif /* CBHM_DBUS_INTERFACE */
#define CBHM_COUNT_ALL 0    // ATOM_INDEX_CBHM_COUNT_ALL

///////////////////////////////////////////////////////////////////////////////////////////////////
// Clipboard
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

struct Clipboard::Impl
{
  Impl()
  {
    Eldbus_Object *eldbus_obj;
    cbhm_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
    eldbus_obj = eldbus_object_get(cbhm_conn, CBHM_DBUS_INTERFACE, CBHM_DBUS_OBJPATH);
    eldbus_proxy = eldbus_proxy_get(eldbus_obj, CBHM_DBUS_INTERFACE);
    eldbus_name_owner_changed_callback_add(cbhm_conn, CBHM_DBUS_INTERFACE, NULL, cbhm_conn, EINA_TRUE);
    eldbus_proxy_signal_handler_add(eldbus_proxy, "ItemClicked", _on_item_clicked, this);
    mVisible = false;
    mIsFirstTimeHidden = true;
  }

  ~Impl()
  {
    if (cbhm_conn)
      eldbus_connection_unref(cbhm_conn);
  }

  Eldbus_Proxy* cbhm_proxy_get()
  {
    return eldbus_proxy;
  }

  Eldbus_Connection* cbhm_connection_get()
  {
    return cbhm_conn;
  }

  void SetItem( const std::string &itemData )
  {
    const char *types[10] = {0, };
    int i = -1;

    if (itemData.length() == 0)
    {
      return;
    }
    mSendBuffer = itemData;

    // ELM_SEL_TYPE_CLIPBOARD - To distinguish clipboard selection in cbhm
    types[++i] = "CLIPBOARD_BEGIN";

    types[++i] = "text/plain;charset=utf-8";

    // ELM_SEL_TYPE_CLIPBOARD - To distinguish clipboard selection in cbhm
    types[++i] = "CLIPBOARD_END";
    ecore_wl_dnd_selection_set(ecore_wl_input_get(), types);
  }

  void RequestItem()
  {
    const char *types[10] = {0, };
    int i = -1;

    types[++i] = "text/plain;charset=utf-8";
    ecore_wl_dnd_selection_get(ecore_wl_input_get(), *types);
  }

  char *ExcuteSend( void *event )
  {
    Ecore_Wl_Event_Data_Source_Send *ev = (Ecore_Wl_Event_Data_Source_Send *)event;
    int len_buf = mSendBuffer.length();
    int len_remained = len_buf;
    int len_written = 0, ret;
    const char *buf = mSendBuffer.c_str();

    while (len_written < len_buf)
    {
       ret = write(ev->fd, buf, len_remained);
       if (ret == -1) break;
       buf += ret;
       len_written += ret;
       len_remained -= ret;
    }
    close(ev->fd);
    return NULL;
  }

  char *ExcuteReceive( void *event )
  {
    Ecore_Wl_Event_Selection_Data_Ready *ev = (Ecore_Wl_Event_Selection_Data_Ready *)event;

    return (char *)ev->data;
  }

  int GetCount()
  {
    Eldbus_Message *reply, *req;
    const char *errname = NULL, *errmsg = NULL;
    int count = -1;

    if (!(req = eldbus_proxy_method_call_new(eldbus_proxy, "CbhmGetCount")))
    {
      DALI_LOG_ERROR("Failed to create method call on org.freedesktop.DBus.Properties.Get");
      return -1;
    }

    eldbus_message_ref(req);
    eldbus_message_arguments_append(req, "i", CBHM_COUNT_ALL) ;
    reply = eldbus_proxy_send_and_block(eldbus_proxy, req, 100);
    if (!reply || eldbus_message_error_get(reply, &errname, &errmsg))
    {
      DALI_LOG_ERROR("Unable to call method org.freedesktop.DBus.Properties.Get: %s %s",
      errname, errmsg);
      eldbus_message_unref(req);
      return -1;
    }

    if (!eldbus_message_arguments_get(reply, "i", &count))
    {
      DALI_LOG_ERROR("Cannot get arguments from eldbus");
      eldbus_message_unref(req);
      return -1;
    }

    eldbus_message_unref(req);
    DALI_LOG_ERROR("cbhm item count(%d)", count);
    return count;
  }

  void ShowClipboard()
  {
    eldbus_proxy_call(cbhm_proxy_get(), "CbhmShow", NULL, NULL, -1, "s", "0");
    mIsFirstTimeHidden = true;
    mVisible = true;
  }

  void HideClipboard( bool skipFirstHide )
  {
    if ( skipFirstHide && mIsFirstTimeHidden )
    {
      mIsFirstTimeHidden = false;
      return;
    }
    eldbus_proxy_call(cbhm_proxy_get(), "CbhmHide", NULL, NULL, -1, "");
    mIsFirstTimeHidden = false;
    mVisible = false;
  }

  bool IsVisible() const
  {
    return mVisible;
  }

  static void _on_item_clicked(void *data, const Eldbus_Message *msg EINA_UNUSED)
  {
    static_cast<Clipboard::Impl*>(data)->RequestItem();
  }

  Eldbus_Proxy *eldbus_proxy;
  Eldbus_Connection *cbhm_conn;

  std::string mSendBuffer;
  bool mVisible;
  bool mIsFirstTimeHidden;
};

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
}

Clipboard::~Clipboard()
{
}

Dali::Clipboard Clipboard::Get()
{
  Dali::Clipboard clipboard;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::Clipboard ) );
    if(handle)
    {
      // If so, downcast the handle
      clipboard = Dali::Clipboard( dynamic_cast< Clipboard* >( handle.GetObjectPtr() ) );
    }
    else
    {
      Clipboard::Impl* impl( new Clipboard::Impl() );
      clipboard = Dali::Clipboard( new Clipboard(impl) );
      service.Register( typeid(Dali::Clipboard), clipboard );
    }
  }

  return clipboard;
}

bool Clipboard::SetItem(const std::string &itemData )
{
  mImpl->SetItem( itemData );
  return true;
}

/*
 * Request clipboard service to give an item
 */
void Clipboard::RequestItem()
{
  mImpl->RequestItem();
}

/*
 * Get number of items in clipboard
 */
unsigned int Clipboard::NumberOfItems()
{
  return mImpl->GetCount();
}

void Clipboard::ShowClipboard()
{
  mImpl->ShowClipboard();
}

void Clipboard::HideClipboard(bool skipFirstHide)
{
  mImpl->HideClipboard(skipFirstHide);
}

bool Clipboard::IsVisible() const
{
  return mImpl->IsVisible();
}

char* Clipboard::ExcuteBuffered( bool type, void *event )
{
  return (type ?  mImpl->ExcuteSend( event ) : mImpl->ExcuteReceive( event ));
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
