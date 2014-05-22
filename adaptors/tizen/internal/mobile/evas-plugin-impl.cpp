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
#include "evas-plugin-impl.h"

// EXTERNAL HEADERS
#include <dali/public-api/dali-core.h>
#include <dali/integration-api/debug.h>

#include <Ecore_IMF_Evas.h>

// INTERNAL HEADERS
#include <dali/public-api/adaptor-framework/common/accessibility-manager.h>
#include <dali/public-api/adaptor-framework/common/clipboard-event-notifier.h>
#include <dali/public-api/adaptor-framework/common/imf-manager.h>

#include <internal/common/adaptor-impl.h>
#include "mobile-render-surface-factory.h"
#include <internal/common/ecore-x/pixmap-render-surface.h>
#include <internal/common/trigger-event.h>

namespace Dali
{

namespace SlpPlatform
{
class SlpPlatformAbstraction;
}

namespace Integration
{
class Core;
}

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gEvasPluginLogFilter = Debug::Filter::New(Debug::Verbose, true, "LOG_EVAS_PLUGIN");
#endif

const char * CLIPBOARD_ATOM                = "CBHM_MSG";
const char * CLIPBOARD_SET_OWNER_MESSAGE   = "SET_OWNER";

/**
 * Evas_Modifier enums in Ecore_Input.h do not match Ecore_Event_Modifier in Ecore_Input.h.
 * This function converts from Evas_Modifier to Ecore_Event_Modifier enums.
 * @param[in] evasModifier the Evas_Modifier input.
 * @return the Ecore_Event_Modifier output.
 */
unsigned int EvasModifierToEcoreModifier(Evas_Modifier* evasModifier)
{
   Eina_Bool control, alt, shift, altGr, win;

   control = evas_key_modifier_is_set(evasModifier, "Control");
   alt = evas_key_modifier_is_set(evasModifier, "Alt");
   shift = evas_key_modifier_is_set(evasModifier, "Shift");
   altGr = evas_key_modifier_is_set(evasModifier, "AltGr");
   win = evas_key_modifier_is_set(evasModifier, "Win");
   win = evas_key_modifier_is_set(evasModifier, "Super");
   win = evas_key_modifier_is_set(evasModifier, "Hyper");

   unsigned int modifier( 0 );  // If no other matches returns NONE.

   if ( shift )
   {
     modifier |= ECORE_EVENT_MODIFIER_SHIFT;  // enums from ecore_imf/ecore_imf.h
   }

   if ( alt )
   {
     modifier |= ECORE_EVENT_MODIFIER_ALT;
   }

   if ( control )
   {
     modifier |= ECORE_EVENT_MODIFIER_CTRL;
   }

   if ( win )
   {
     modifier |= ECORE_EVENT_MODIFIER_WIN;
   }

   if ( altGr )
   {
     modifier |= ECORE_EVENT_MODIFIER_ALTGR;
   }

   return modifier;
}

static void _evas_object_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Mouse_Down* ev;
    ev = (Evas_Event_Mouse_Down *)event_info;

    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->canvas.x - obj_x;
    rel_y = ev->canvas.y - obj_y;

    // create dali TouchEvent & SendEvent
    TouchPoint point(0, TouchPoint::Down, rel_x, rel_y);
    ep->OnTouchEvent(point, ev->timestamp);
  }
}

static void _evas_object_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Mouse_Move *ev;
    ev = (Evas_Event_Mouse_Move *)event_info;

    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->cur.canvas.x - obj_x;
    rel_y = ev->cur.canvas.y - obj_y;

    // create dali TouchEvent & SendEvent
    TouchPoint point(0, TouchPoint::Motion, rel_x, rel_y);
    ep->OnTouchEvent(point, ev->timestamp);
  }
}

static void _evas_object_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Mouse_Up *ev;
    ev = (Evas_Event_Mouse_Up *)event_info;

    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->canvas.x - obj_x;
    rel_y = ev->canvas.y - obj_y;

    // create dali TouchEvent & SendEvent
    TouchPoint point(0, TouchPoint::Up, rel_x, rel_y);
    ep->OnTouchEvent(point, ev->timestamp);
  }
}

static void _evas_object_mouse_wheel_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Mouse_Wheel *ev = (Evas_Event_Mouse_Wheel*)event_info;
    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->canvas.x - obj_x;
    rel_y = ev->canvas.y - obj_y;

    MouseWheelEvent wheelEvent(ev->direction, -1 /*Need to check evas modifier*/, Vector2(rel_x, rel_y), ev->z, ev->timestamp);
    ep->OnMouseWheelEvent(wheelEvent);
  }
}

static void _evas_object_multi_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Multi_Down *ev = (Evas_Event_Multi_Down*)event_info;

    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->canvas.x - obj_x;
    rel_y = ev->canvas.y - obj_y;

    // create dali TouchEvent & SendEvent
    TouchPoint point(ev->device, TouchPoint::Down, rel_x, rel_y);
    ep->OnTouchEvent(point, ev->timestamp);
  }
}

static void _evas_object_multi_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Multi_Up *ev = (Evas_Event_Multi_Up*)event_info;

    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->canvas.x - obj_x;
    rel_y = ev->canvas.y - obj_y;

    // create dali TouchEvent & SendEvent
    TouchPoint point(ev->device, TouchPoint::Up, rel_x, rel_y);
    ep->OnTouchEvent(point, ev->timestamp);
  }
}
static void _evas_object_multi_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    Evas_Event_Multi_Move *ev = (Evas_Event_Multi_Move*)event_info;

    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    evas_object_geometry_get(obj, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = ev->cur.canvas.x - obj_x;
    rel_y = ev->cur.canvas.y - obj_y;

    // create dali TouchEvent & SendEvent
    TouchPoint point(ev->device, TouchPoint::Motion, rel_x, rel_y);
    ep->OnTouchEvent(point, ev->timestamp);
  }
}

static void _evas_object_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;

  if(ep)
  {
    Evas_Event_Key_Down* keyEvent( (Evas_Event_Key_Down*)event_info );
    bool eventHandled( false );

    if(!keyEvent->keyname)
    {
      return;
    }

    Ecore_IMF_Context* imfContext = NULL;
    if ( Dali::Adaptor::IsAvailable() )
    {
      Dali::ImfManager imfManager = Dali::ImfManager::Get();
      if ( imfManager )
      {
        imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );
      }
    }

    // XF86Stop and XF86Send must skip ecore_imf_context_filter_event.
    if ( imfContext && strcmp( keyEvent->keyname, "XF86Send" ) && strcmp( keyEvent->keyname, "XF86Phone" ) && strcmp( keyEvent->keyname, "XF86Stop" ) )
    {
      Ecore_IMF_Event_Key_Down ecoreKeyDownEvent;
      ecore_imf_evas_event_key_down_wrap(keyEvent, &ecoreKeyDownEvent);

      eventHandled = ecore_imf_context_filter_event(imfContext,
                                                      ECORE_IMF_EVENT_KEY_DOWN,
                                                     (Ecore_IMF_Event *) &ecoreKeyDownEvent );

      // If the event has not been handled by IMF then check if we should reset our IMF context
      if( !eventHandled )
      {
        if ( !strcmp( keyEvent->keyname, "Escape"   ) ||
             !strcmp( keyEvent->keyname, "Return"   ) ||
             !strcmp( keyEvent->keyname, "KP_Enter" ) )
        {
          ecore_imf_context_reset( imfContext );
       }
      }
    }

    // If the event wasn't handled then we should send a key event.
    if ( !eventHandled )
    {
      std::string keyName( keyEvent->keyname );
      std::string keyString( "" );
      int keyCode = ecore_x_keysym_keycode_get(keyEvent->keyname);
      int modifier( EvasModifierToEcoreModifier ( keyEvent->modifiers ) );
      unsigned long time( keyEvent->timestamp );

      // Ensure key event string is not NULL as keys like SHIFT have a null string.
      if ( keyEvent->string )
      {
        keyString = keyEvent->string;
      }

      KeyEvent daliKeyEvent(keyName, keyString, keyCode, modifier, time, KeyEvent::Down);
      ep->OnKeyEvent( daliKeyEvent );
    }
  }
}

static void _evas_object_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    // We're consuming key up event so we have to pass to IMF so that it can parse it as well.
    Evas_Event_Key_Up* keyEvent( (Evas_Event_Key_Up*)event_info );
    bool eventHandled( false );

    Ecore_IMF_Context* imfContext = NULL;
    if ( Dali::Adaptor::IsAvailable() )
    {
      Dali::ImfManager imfManager = Dali::ImfManager::Get();
      if ( imfManager )
      {
        imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );
      }
    }

    if ( imfContext && strcmp( keyEvent->keyname, "XF86Send" ) && strcmp( keyEvent->keyname, "XF86Phone" ) && strcmp( keyEvent->keyname, "XF86Stop" ))
    {
      Ecore_IMF_Event_Key_Up ecoreKeyUpEvent;
      ecore_imf_evas_event_key_up_wrap(keyEvent, &ecoreKeyUpEvent);

      eventHandled = ecore_imf_context_filter_event( imfContext,
                                      ECORE_IMF_EVENT_KEY_UP,
                                      (Ecore_IMF_Event *) &ecoreKeyUpEvent );
    }

    if ( !eventHandled )
    {
      std::string keyName( keyEvent->keyname );
      std::string keyString( "" );
      int keyCode = ecore_x_keysym_keycode_get(keyEvent->keyname);
      int modifier( EvasModifierToEcoreModifier ( keyEvent->modifiers ) );
      unsigned long time( keyEvent->timestamp );

      // Ensure key event string is not NULL as keys like SHIFT have a null string.
      if ( keyEvent->string )
      {
        keyString = keyEvent->string;
      }

      KeyEvent daliKeyEvent(keyName, keyString, keyCode, modifier, time, KeyEvent::Up);
      ep->OnKeyEvent( daliKeyEvent );
    }
  }
}

static void _evas_object_focus_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep != NULL && ep->GetEvasImageObject() == obj)
  {
    ep->OnEvasObjectFocusedIn();
  }
}

static void _evas_object_focus_out_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep != NULL && ep->GetEvasImageObject() == obj)
  {
    ep->OnEvasObjectFocusedOut();
  }
}

static void _elm_focus_object_focus_in_cb(void *data, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep != NULL && ep->GetElmFocusObject() == obj)
  {
    Evas_Object* win = elm_object_top_widget_get(obj);
    if(strcmp("elm_win", elm_object_widget_type_get(win)) == 0)
    {
      if(elm_win_focus_highlight_enabled_get(win) == EINA_TRUE)
      {
        // To allow that KeyboardFocusManager can handle the keyboard focus
        KeyEvent fakeKeyEvent("", "", 0, 0, 100 /* fake timestamp */, KeyEvent::Down);
        ep->OnKeyEvent( fakeKeyEvent );
      }
    }
    else
    {
      DALI_LOG_ERROR("It is not elm win\n");
    }

    evas_object_focus_set(ep->GetEvasImageObject(), EINA_TRUE);
  }
}

static void _elm_focus_object_focus_out_cb(void *data, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep != NULL && ep->GetElmFocusObject() == obj)
  {
    evas_object_focus_set(ep->GetEvasImageObject(), EINA_FALSE);
  }
}

static void _canvas_focus_in_cb(void *data, Evas *e, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep != NULL && ep->GetEvasImageObject() == evas_focus_get(e))
  {
    ep->OnEvasObjectFocusedIn();
  }
}

static void _canvas_focus_out_cb(void *data, Evas *e, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep != NULL && ep->GetEvasImageObject() == evas_focus_get(e))
  {
    ep->OnEvasObjectFocusedOut();
  }
}

static void _evas_object_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    ep->Move();
  }
}

static void _evas_object_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    ep->Resize();
  }
}

static void _evas_render_post_cb(void *data, Evas *e, void *event_info)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  if(ep)
  {
    // call RenderSync when the window surface(onscreen) was presented to LCD.
    ep->RenderSync();

    //After first render emit signal to notify
    if(!ep->mFirstRenderCompleteNotified)
    {
      ep->OnFirstRenderCompleted();
    }
  }
}

/*
 * When the evas plugin is resumed,
 * need to forcely dirty set the evas object on idle time to show again the result of dali rendering.
 * One time should be enough.
 */
static Eina_Bool _evas_object_dirty_set_idle_cb(void *data)
{
  EvasPlugin* ep = (EvasPlugin*)data;

  if(ep)
  {
    Evas_Object* eo = ep->GetEvasImageObject();
    if(eo)
    {
      /* dirty set to post the result of rendering via evas */
      evas_object_image_pixels_dirty_set(eo, EINA_TRUE);
    }

    ep->ClearIdler(false); // clear idler handle without deleting handle. because handle will be deleted by ecore
  }

  // we need it once.
  return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _elm_access_highlight_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  bool ret = false;

  if(ep && actionInfo)
  {
    // action_by has NEXT or PREV
    if (actionInfo->action_by == ELM_ACCESS_ACTION_HIGHLIGHT_NEXT)
    {
      ret = ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_HIGHLIGHT_NEXT);
      DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] Next returns %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
    }
    else if (actionInfo->action_by == ELM_ACCESS_ACTION_HIGHLIGHT_PREV)
    {
      ret = ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_HIGHLIGHT_PREV);
      DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] Prev returns %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
    }
    else
    {
      /*
        * In case of access over, action_by has ELM_ACCESS_ACTION_HIGHLIGHT
        * real operation will be done in _elm_access_over_cb
        * so just return true in order to remove the entire focus indicator
        */

      /*
        * Even if action_by has intialized value (-1), highlight action is valid
        */
      ret = true;
    }
  }
  else
  {
    DALI_LOG_WARNING("[%s:%d] has no actionInfo\n", __FUNCTION__, __LINE__);
  }

  return ret;
}

static Eina_Bool _elm_access_read_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  bool ret = false;

  if(ep)
  {
    PositionSize geometry = ep->GetEvasObjectGeometry();

    if(actionInfo)
    {
      ret = ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_READ, (actionInfo->x - geometry.x), (actionInfo->y - geometry.y));
      DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] returns %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
    }
    else
    {
      DALI_LOG_WARNING( "[%s:%d] has no actionInfo\n", __FUNCTION__, __LINE__);
    }
  }

  return ret;
}

static Eina_Bool _elm_access_over_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  bool ret = false;

  if(ep)
  {
    PositionSize geometry = ep->GetEvasObjectGeometry();

    if(actionInfo)
    {
      ret = ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_OVER, (actionInfo->x - geometry.x), (actionInfo->y - geometry.y));
      DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] returns %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
    }
    else
    {
      DALI_LOG_WARNING( "[%s:%d] has no actionInfo\n", __FUNCTION__, __LINE__);
    }
  }

  return ret;
}

static Eina_Bool _elm_access_highlight_next_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_HIGHLIGHT_NEXT);
}

static Eina_Bool _elm_access_highlight_prev_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_HIGHLIGHT_PREV);
}

static Eina_Bool _elm_access_activate_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_ACTIVATE);
}

static Eina_Bool _elm_access_unhighlight_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_UNHIGHLIGHT);
}

static Eina_Bool _elm_access_back_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_BACK);
}

static Eina_Bool _elm_access_value_up_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_UP);
}

static Eina_Bool _elm_access_value_down_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  return ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_DOWN);
}

static Eina_Bool _elm_access_scroll_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  bool ret = false;

  if(actionInfo && ep)
  {
    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    Evas_Object* eo = ep->GetEvasImageObject();

    if(eo)
    {
      evas_object_geometry_get(eo, &obj_x,  &obj_y, &obj_w, &obj_h);

      rel_x = actionInfo->x - obj_x;
      rel_y = actionInfo->y - obj_y;

      ret = ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_SCROLL, rel_x, rel_y, actionInfo->mouse_type);
    }
  }
  else
  {
    DALI_LOG_WARNING("[%s:%d] has no actionInfo\n", __FUNCTION__, __LINE__);
  }

  return ret;
}

static Eina_Bool _elm_access_mouse_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *actionInfo)
{
  EvasPlugin* ep = (EvasPlugin*)data;
  bool ret = false;

  if(actionInfo && ep)
  {
    Evas_Coord rel_x, rel_y;
    Evas_Coord obj_x,  obj_y, obj_w, obj_h;
    Evas_Object* eo = ep->GetEvasImageObject();

    if(eo)
    {
      evas_object_geometry_get(eo, &obj_x,  &obj_y, &obj_w, &obj_h);

      rel_x = actionInfo->x - obj_x;
      rel_y = actionInfo->y - obj_y;

      ret = ep->OnAccessibilityActionEvent(ELM_ACCESS_ACTION_MOUSE, rel_x, rel_y, actionInfo->mouse_type);
    }
  }
  else
  {
    DALI_LOG_WARNING("[%s:%d] has no actionInfo\n", __FUNCTION__, __LINE__);
  }

  return ret;
}

static Eina_Bool _ecore_x_event_selection_clear(void *data, int type, void *event)
{
  EvasPlugin* ep = (EvasPlugin*)data;

  if( ep )
  {
    ep->OnEcoreEventSelectionCleared(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool _ecore_x_event_selection_notify(void *data, int type, void *event)
{
  EvasPlugin* ep = (EvasPlugin*)data;

  if( ep )
  {
    ep->OnEcoreEventSelectionNotified(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}
static Eina_Bool _ecore_x_event_client_message(void* data, int type, void* event)
{
  EvasPlugin* ep = (EvasPlugin*)data;

  if( ep )
  {
    ep->OnEcoreEventClientMessaged(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

// Copied from x server
static unsigned int GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec tp;
  static clockid_t clockid;

  if (!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
      (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if (clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

EvasPlugin::EvasPlugin(Dali::EvasPlugin& evasPlugin, Evas_Object* parent, bool isTransparent, unsigned int initialWidth, unsigned int initialHeight)
: mEvasImageObject(NULL),
  mElmAccessObject(NULL),
  mElmFocusObject(NULL),
  mSurface(NULL),
  mFirstRenderCompleteNotified(false),
  mEvasPlugin(evasPlugin),
  mAdaptor(NULL),
  mEvas(NULL),
  mEvasImageObjectGeometry(0, 0, initialWidth, initialHeight),
  mInitialized(false),
  mIsTransparent(isTransparent),
  mHasFocus(false),
  mRenderNotification(NULL),
  mEvasDirtyIdler(NULL)
{
  DALI_ASSERT_ALWAYS( parent && "No parent object for plugin" );
  mEvas = AnyCast<Evas*>(evas_object_evas_get(parent));

  /* create evas object image */
  CreateEvasImageObject(mEvas, initialWidth, initialHeight, isTransparent);

  /* create elm access object */
  CreateElmAccessObject(parent);

  /* create elm focus object */
  CreateElmFocusObject(parent);

  /* create adaptor */
  CreateAdaptor(initialWidth, initialHeight);

  /* render post callback */
  evas_event_callback_add(mEvas, EVAS_CALLBACK_RENDER_POST, _evas_render_post_cb, this);

  mRenderNotification = new TriggerEvent( boost::bind(&EvasPlugin::Render, this ) );

  mSurface->SetRenderNotification( mRenderNotification );

  mState = Ready;
}

EvasPlugin::~EvasPlugin()
{
  mConnectionTracker.DisconnectAll();

  if (mAdaptor)
  {
    Stop();

    // delete idler
    ClearIdler();

    // no more notifications
    delete mRenderNotification;

    // delete evas canvas callback for render sync
    evas_event_callback_del(mEvas, EVAS_CALLBACK_RENDER_POST, _evas_render_post_cb);

    delete mAdaptor;
    mAdaptor = NULL;

    // delete elm focus object
    DeleteElmFocusObject();

    // delete elm access object
    DeleteElmAccessObject();

    // delete evas object image
    DeleteEvasImageObject();

    if (mSurface)
    {
      delete mSurface;
      mSurface = NULL;
    }
  }
}

void EvasPlugin::CreateEvasImageObject(Evas* evas, unsigned int initialWidth, unsigned int initialHeight, bool isTransparent)
{
  /* create evas object */

  mEvasImageObject = evas_object_image_filled_add(mEvas);
  evas_object_name_set(mEvasImageObject, "dali-evasplugin");
  evas_object_image_content_hint_set(mEvasImageObject, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
  evas_object_size_hint_align_set(mEvasImageObject, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_size_hint_weight_set(mEvasImageObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

  if(isTransparent)
  {
    evas_object_image_alpha_set(mEvasImageObject, EINA_TRUE);
  }

  evas_object_move(mEvasImageObject, 0, 0);
  evas_object_image_size_set(mEvasImageObject, initialWidth, initialHeight);
  evas_object_resize(mEvasImageObject, initialWidth, initialHeight);

  /* event callback */
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MOUSE_DOWN, _evas_object_mouse_down_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MOUSE_UP, _evas_object_mouse_up_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MOUSE_MOVE, _evas_object_mouse_move_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MOUSE_WHEEL, _evas_object_mouse_wheel_cb, this);

  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MULTI_DOWN, _evas_object_multi_down_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MULTI_UP, _evas_object_multi_up_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MULTI_MOVE, _evas_object_multi_move_cb, this);

  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_KEY_DOWN, _evas_object_key_down_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_KEY_UP, _evas_object_key_up_cb, this);

  /* move callback */
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_MOVE, _evas_object_move_cb, this);

  /* resize callback */
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_RESIZE, _evas_object_resize_cb, this);

  /* focus callback */
  evas_event_callback_add(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_IN, _canvas_focus_in_cb, this);
  evas_event_callback_add(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _canvas_focus_out_cb, this);

  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_FOCUS_IN, _evas_object_focus_in_cb, this);
  evas_object_event_callback_add(mEvasImageObject, EVAS_CALLBACK_FOCUS_OUT, _evas_object_focus_out_cb, this);

  evas_object_show(mEvasImageObject);
}

void EvasPlugin::DeleteEvasImageObject()
{
  if(mEvasImageObject)
  {
    /* event callback */
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MOUSE_DOWN, _evas_object_mouse_down_cb);
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MOUSE_UP, _evas_object_mouse_up_cb);
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MOUSE_MOVE, _evas_object_mouse_move_cb);

    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MULTI_DOWN, _evas_object_multi_down_cb);
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MULTI_UP, _evas_object_multi_up_cb);
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MULTI_MOVE, _evas_object_multi_move_cb);

    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_KEY_DOWN, _evas_object_key_down_cb);
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_KEY_UP, _evas_object_key_up_cb);

    /* move callback */
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_MOVE, _evas_object_move_cb);

    /* resize callback */
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_RESIZE, _evas_object_resize_cb);

    /* focus callback */
    evas_event_callback_del(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_IN, _canvas_focus_in_cb);
    evas_event_callback_del(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _canvas_focus_out_cb);

    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_FOCUS_IN, _evas_object_focus_in_cb);
    evas_object_event_callback_del(mEvasImageObject, EVAS_CALLBACK_FOCUS_OUT, _evas_object_focus_out_cb);

    // evas object callbacks are deleted with the object
    evas_object_del(mEvasImageObject);
    mEvasImageObject = NULL;
  }
}

void EvasPlugin::CreateElmAccessObject(Evas_Object* parent)
{
  // elm access register with image object
  mElmAccessObject = elm_access_object_register(mEvasImageObject, parent);

  // elm access action callbacks
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_HIGHLIGHT, _elm_access_highlight_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_UNHIGHLIGHT, _elm_access_unhighlight_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_HIGHLIGHT_NEXT, _elm_access_highlight_next_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_HIGHLIGHT_PREV, _elm_access_highlight_prev_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_ACTIVATE, _elm_access_activate_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_UP, _elm_access_value_up_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_DOWN, _elm_access_value_down_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_SCROLL, _elm_access_scroll_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_MOUSE, _elm_access_mouse_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_BACK, _elm_access_back_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_READ, _elm_access_read_cb, this);
  elm_access_action_cb_set(mElmAccessObject, ELM_ACCESS_ACTION_OVER, _elm_access_over_cb, this);

  /**
   * Dali doesn't set the order of elm focus chain.
   * Application should append mElmAccessObject to layout's custom focus chain
   *
   * e.g) elm_object_focus_custom_chain_append(parent, mElmAccessObject, NULL);
   */
}

void EvasPlugin::DeleteElmAccessObject()
{
  if(mElmAccessObject)
  {
    // elm access action callbacks and elm_access_object will be deleted in unregister
    elm_access_object_unregister(mEvasImageObject);
    mElmAccessObject = NULL;
  }
}

void EvasPlugin::CreateElmFocusObject(Evas_Object* parent)
{
  // create a button and set style as "focus", if does not want to show the focus, then "transparent"
  mElmFocusObject = elm_button_add(parent);
  // don't need to show the focus boundary here
  elm_object_style_set(mElmFocusObject, "transparent");

  // set the evas image object to focus object, but event should not be propagated
  elm_object_part_content_set(mElmFocusObject, "elm.swallow.content", mEvasImageObject);
  evas_object_propagate_events_set(mEvasImageObject, EINA_FALSE);

  // set the evas object you want to make focusable as the content of the swallow part
  evas_object_size_hint_weight_set(mElmFocusObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(mElmFocusObject, EVAS_HINT_FILL, EVAS_HINT_FILL);

  evas_object_smart_callback_add(mElmFocusObject, "focused", _elm_focus_object_focus_in_cb, this);
  evas_object_smart_callback_add(mElmFocusObject, "unfocused", _elm_focus_object_focus_out_cb, this);

  evas_object_show(mElmFocusObject);
}

void EvasPlugin::DeleteElmFocusObject()
{
  if(mElmFocusObject)
  {
    evas_object_smart_callback_del(mElmFocusObject, "focused", _elm_focus_object_focus_in_cb);
    evas_object_smart_callback_del(mElmFocusObject, "unfocused", _elm_focus_object_focus_out_cb);

    evas_object_del(mElmFocusObject);
    mElmFocusObject = NULL;
  }
}

void EvasPlugin::CreateAdaptor(unsigned int initialWidth, unsigned int initialHeight)
{
  mSurface = CreateSurface(initialWidth, initialHeight);

  mAdaptor = Internal::Adaptor::Adaptor::New( mSurface, DeviceLayout::DEFAULT_BASE_LAYOUT );

  Any surface = mSurface->GetSurface();

  Ecore_X_Pixmap pixmap = AnyCast<Ecore_X_Pixmap>(surface);

  /* set native pixmap surface */
  Evas_Native_Surface ns;
  ns.type = EVAS_NATIVE_SURFACE_X11;
  ns.version = EVAS_NATIVE_SURFACE_VERSION;
  ns.data.x11.pixmap = pixmap;
  ns.data.x11.visual = NULL;

  evas_object_image_native_surface_set(mEvasImageObject, &ns);
}

ECoreX::RenderSurface* EvasPlugin::CreateSurface( int width, int height )
{
  PositionSize pixmapSize( 0, 0, width, height );
  Any surface;
  Any display;
  // if we already have surface, reuse its display
  if( mSurface )
  {
    display = mSurface->GetMainDisplay();
  }

  // create a X11 pixmap
  ECoreX::RenderSurface* daliSurface = ECoreX::CreatePixmapSurface( pixmapSize, surface, display, "no name", mIsTransparent );

  daliSurface->SetRenderNotification( mRenderNotification );

  return daliSurface;
}

void EvasPlugin::ResizeSurface()
{
  // remember old surface
  Dali::RenderSurface* oldSurface = mSurface;
  mSurface = CreateSurface( mEvasImageObjectGeometry.width, mEvasImageObjectGeometry.height );

  // ask the replace the surface inside dali
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).ReplaceSurface( *mSurface ); // this method is synchronous => guarantee until rendering next frame

  // update the pixmap for evas
  {
    Any surface = mSurface->GetSurface();
    Ecore_X_Pixmap pixmap = AnyCast<Ecore_X_Pixmap>( surface );

    Evas_Native_Surface ns;
    ns.type = EVAS_NATIVE_SURFACE_X11;
    ns.version = EVAS_NATIVE_SURFACE_VERSION;
    ns.data.x11.pixmap = pixmap;
    ns.data.x11.visual = NULL;

    evas_object_image_native_surface_set(mEvasImageObject, &ns);

    // its now safe to delete the old surface
    delete oldSurface;
  }

  OnResize();
}

void EvasPlugin::ConnectEcoreEvent()
{
  // Get Ecore_Evas using Evas.
  Ecore_Evas* ecoreEvas = ecore_evas_ecore_evas_get( mEvas );

  if( ecoreEvas )  // Check invalid or valid.
  {
    // Get window from Ecore_Evas.
    Ecore_X_Window window = ecore_evas_gl_x11_window_get( ecoreEvas );

    // Set the application window at ime context.
    Dali::ImfManager imfManager = Dali::ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );
    ecore_imf_context_client_window_set( imfContext, reinterpret_cast<void*>( window ) );

    if( window )  // Check invalid or valid.
    {
      // Connect clipboard events.
      mEcoreEventHandler.push_back( ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, _ecore_x_event_selection_clear, this) );
      mEcoreEventHandler.push_back( ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _ecore_x_event_selection_notify, this) );

      // Register Client message events - accessibility etc.
      mEcoreEventHandler.push_back( ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, _ecore_x_event_client_message, this) );
    }
  }
}

void EvasPlugin::DisconnectEcoreEvent()
{
  for( std::vector<Ecore_Event_Handler*>::iterator iter = mEcoreEventHandler.begin(), endIter = mEcoreEventHandler.end(); iter != endIter; ++iter )
  {
    ecore_event_handler_del( *iter );
  }

  mEcoreEventHandler.clear();
}

void EvasPlugin::Run()
{
  if(mState == Ready)
  {
    // Run the adaptor
    mAdaptor->Start();
    mState = Running;

    OnInit();
  }
}

void EvasPlugin::Pause()
{
  if(mState == Running)
  {
    mAdaptor->Pause();
    mState = Suspended;

    mPauseSignalV2.Emit( mEvasPlugin );
  }
}

void EvasPlugin::Resume()
{
  if(mState == Suspended)
  {
    mAdaptor->Resume();
    mState = Running;

    mResumeSignalV2.Emit( mEvasPlugin );
  }

  // forcely dirty_set the evas_object on idle time
  ClearIdler();
  mEvasDirtyIdler = ecore_idler_add(_evas_object_dirty_set_idle_cb, this);
}

void EvasPlugin::ClearIdler(bool deleteHandle)
{
  if(mEvasDirtyIdler)
  {
    if(deleteHandle)
    {
      ecore_idler_del(mEvasDirtyIdler);
    }
    mEvasDirtyIdler = NULL;
  }
}

void EvasPlugin::Stop()
{
  if(mState != Stopped)
  {
    // Stop the adaptor
    mAdaptor->Stop();
    mState = Stopped;

    mTerminateSignalV2.Emit( mEvasPlugin );
  }
}
Evas_Object* EvasPlugin::GetEvasImageObject()
{
  return mEvasImageObject;
}

Evas_Object* EvasPlugin::GetElmAccessObject()
{
  return mElmAccessObject;
}

Evas_Object* EvasPlugin::GetElmFocusObject()
{
  return mElmFocusObject;
}

void EvasPlugin::OnInit()
{
  mInitialized = true;

  mInitSignalV2.Emit( mEvasPlugin );
}

void EvasPlugin::OnFirstRenderCompleted()
{
  mFirstRenderCompletedSignalV2.Emit( mEvasPlugin );

  mFirstRenderCompleteNotified = true;
}

void EvasPlugin::Move()
{
  Evas_Coord x, y, w, h;
  evas_object_geometry_get(mEvasImageObject, &x, &y, &w, &h);

  // update geometry
  mEvasImageObjectGeometry.x = x;
  mEvasImageObjectGeometry.y = y;
  mEvasImageObjectGeometry.width = w;
  mEvasImageObjectGeometry.height = h;

  DALI_LOG_INFO( gEvasPluginLogFilter, Debug::General, "EvasPlugin::Move : %d, %d, %d x %d\n", x, y, w, h );
}

void EvasPlugin::Resize()
{
  Evas_Coord x, y, w, h;
  evas_object_geometry_get(mEvasImageObject, &x, &y, &w, &h);

  // skip meaningless resize signal
  if(w <= 1 || h <= 1)
  {
    return;
  }

  if(mEvasImageObjectGeometry.width == w && mEvasImageObjectGeometry.height == h)
  {
    return;
  }

  DALI_LOG_INFO( gEvasPluginLogFilter, Debug::General, "old size (%d x %d), new size (%d x %d)\n", mEvasImageObjectGeometry.width, mEvasImageObjectGeometry.height, w, h );

  // update geometry
  mEvasImageObjectGeometry.x = x;
  mEvasImageObjectGeometry.y = y;
  mEvasImageObjectGeometry.width = w;
  mEvasImageObjectGeometry.height = h;

  ResizeSurface();
}

void EvasPlugin::OnResize()
{
  if(mInitialized)
  {
    // emit resized signal to application
    mResizeSignalV2.Emit( mEvasPlugin );
  }
}

void EvasPlugin::Render()
{
  // dirty set while adaptor is running
  if( EvasPlugin::Running == mState )
  {
    /* dirty set to post the result of rendering via evas */
    evas_object_image_pixels_dirty_set( mEvasImageObject, EINA_TRUE );
  }
}

void EvasPlugin::OnTouchEvent(TouchPoint& point, int timeStamp)
{
  if ( mAdaptor )
  {
    if(timeStamp < 1)
    {
      timeStamp = GetCurrentMilliSeconds();
    }

    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).FeedTouchPoint( point, timeStamp );
  }
}

void EvasPlugin::OnKeyEvent(KeyEvent& keyEvent)
{
  // Create KeyEvent and send to Core.
  if ( mAdaptor )
  {
    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).FeedKeyEvent( keyEvent );
  }
}

void EvasPlugin::OnMouseWheelEvent(MouseWheelEvent& wheelEvent)
{
  if ( mAdaptor )
  {
    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).FeedWheelEvent( wheelEvent );
  }
}

void EvasPlugin::OnImfActivated(Dali::ImfManager& imfManager)
{
  // When imf is activated, set focus to own evas-object to get key events
  evas_object_focus_set(mEvasImageObject, EINA_TRUE);
}

void EvasPlugin::RenderSync()
{
  if( NULL != mAdaptor )
  {
    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).RenderSync();
  }
}

bool EvasPlugin::OnAccessibilityActionEvent(Elm_Access_Action_Type actionType, int x, int y, int type)
{
  bool ret = false;

  if( NULL == mAdaptor )
  {
    return ret;
  }

  Dali::AccessibilityManager accessibilityManager = Dali::AccessibilityManager::Get();
  if( accessibilityManager )
  {
    switch(actionType)
    {
      case ELM_ACCESS_ACTION_HIGHLIGHT:
      case ELM_ACCESS_ACTION_READ:
      {
        ret = accessibilityManager.HandleActionReadEvent((unsigned int)x, (unsigned int)y, true);
      }
      break;

      case ELM_ACCESS_ACTION_OVER:
      {
        ret = accessibilityManager.HandleActionReadEvent((unsigned int)x, (unsigned int)y, false);
      }
      break;

      case ELM_ACCESS_ACTION_HIGHLIGHT_PREV:
      {
        ret = accessibilityManager.HandleActionPreviousEvent();
        if(!ret)
        {
          // when focus moving was failed, clear the focus
          accessibilityManager.HandleActionClearFocusEvent();
        }
      }
      break;

      case ELM_ACCESS_ACTION_HIGHLIGHT_NEXT:
      {
        ret = accessibilityManager.HandleActionNextEvent();
        if(!ret)
        {
          // when focus moving was failed, clear the focus
          accessibilityManager.HandleActionClearFocusEvent();
        }
      }
      break;

      case ELM_ACCESS_ACTION_ACTIVATE:
      {
        ret = accessibilityManager.HandleActionActivateEvent();
      }
      break;

      case ELM_ACCESS_ACTION_UNHIGHLIGHT:
      {
        ret = accessibilityManager.HandleActionClearFocusEvent();
      }
      break;

      case ELM_ACCESS_ACTION_SCROLL:
      {
        TouchPoint::State state(TouchPoint::Down);

        if (type == 0)
        {
          state = TouchPoint::Down; // mouse down
        }
        else if (type == 1)
        {
          state = TouchPoint::Motion; // mouse move
        }
        else if (type == 2)
        {
          state = TouchPoint::Up; // mouse up
        }
        else
        {
          state = TouchPoint::Interrupted; // error
        }

        // Send touch event to core.
        TouchPoint point( 0, state, (float)x, (float)y );
        ret = accessibilityManager.HandleActionScrollEvent(point, GetCurrentMilliSeconds());
      }
      break;

      case ELM_ACCESS_ACTION_UP:
      {
        ret = accessibilityManager.HandleActionUpEvent();
      }
      break;

      case ELM_ACCESS_ACTION_DOWN:
      {
        ret = accessibilityManager.HandleActionDownEvent();
      }
      break;

      case ELM_ACCESS_ACTION_MOUSE:
      {
        // generate normal mouse event
        TouchPoint::State state(TouchPoint::Down);

        if (type == 0)
        {
          state = TouchPoint::Down; // mouse down
        }
        else if (type == 1)
        {
          state = TouchPoint::Motion; // mouse move
        }
        else if (type == 2)
        {
          state = TouchPoint::Up; // mouse up
        }
        else
        {
          state = TouchPoint::Interrupted; // error
        }

        // Send touch event to core.
        TouchPoint point( 0, state, (float)x, (float)y );
        OnTouchEvent(point, 0);

        DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] (%d, %d)\n", __FUNCTION__, __LINE__, x, y);
      }
      break;

      case ELM_ACCESS_ACTION_BACK:
      default:
      {
        DALI_LOG_WARNING("[%s:%d]\n", __FUNCTION__, __LINE__);
      }

      break;
    }
  }
  else
  {
    DALI_LOG_WARNING("[%s:%d]\n", __FUNCTION__, __LINE__);
  }

  DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] [action : %d] focus manager returns %s\n", __FUNCTION__, __LINE__, (int)(actionType), ret?"TRUE":"FALSE");

  return ret;

}

void EvasPlugin::OnEvasObjectFocusedIn()
{
  if(mHasFocus)
  {
    return;
  }
  mHasFocus = true;

  // If the evas object gains focus and we hide the keyboard then show it again.
  if( Dali::Adaptor::IsAvailable() )
  {
    ConnectEcoreEvent();

    Dali::ImfManager imfManager( Dali::ImfManager::Get() );
    if( imfManager && imfManager.RestoreAfterFocusLost() )
    {
      imfManager.Activate();
    }

    // No need to connect callbacks as KeyboardStatusChanged will be called.

    // emit focused signal to application
    mFocusedSignalV2.Emit( mEvasPlugin );
  }
}

void EvasPlugin::OnEvasObjectFocusedOut()
{
  if(!mHasFocus)
  {
    return;
  }
  mHasFocus = false;

  // If the evas object loses focus then hide the keyboard.
  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager( Dali::ImfManager::Get() );
    if( imfManager && imfManager.RestoreAfterFocusLost() )
    {
      imfManager.Deactivate();
    }

    // Clipboard don't support that whether clipboard is shown or not. Hide clipboard.
    Dali::Clipboard clipboard = Dali::Clipboard::Get();
    clipboard.HideClipboard();

    DisconnectEcoreEvent();

    // emit unfocused signal to application
    mUnFocusedSignalV2.Emit( mEvasPlugin );
  }
}

void EvasPlugin::OnEcoreEventSelectionCleared( void* data, int type, void* event )
{
  Ecore_X_Event_Selection_Clear* selectionClearEvent( (Ecore_X_Event_Selection_Clear*) event );

  if( selectionClearEvent->selection == ECORE_X_SELECTION_SECONDARY )
  {
    // Request to get the content from Ecore.
    ecore_x_selection_secondary_request(selectionClearEvent->win, ECORE_X_SELECTION_TARGET_TEXT);
  }
}

void EvasPlugin::OnEcoreEventSelectionNotified(void* data, int type, void* event)
{
  Ecore_X_Event_Selection_Notify* selectionNotifyEvent( (Ecore_X_Event_Selection_Notify*) event );

  if( selectionNotifyEvent->selection == ECORE_X_SELECTION_SECONDARY )
  {
    // We have got the selected content, inform the clipboard event listener (if we have one).
    Dali::ClipboardEventNotifier clipboardEventNotifier = Dali::ClipboardEventNotifier::Get();
    Ecore_X_Selection_Data* selectionData( (Ecore_X_Selection_Data*) selectionNotifyEvent->data );

    if ( clipboardEventNotifier )
    {
      std::string content( (char*) selectionData->data, selectionData->length );

      if( !content.empty() )
      {
        clipboardEventNotifier.SetContent( content );
        clipboardEventNotifier.EmitContentSelectedSignal();
      }
    }

    // Claim the ownership of the SECONDARY selection.
    ecore_x_selection_secondary_set(selectionNotifyEvent->win, "", 1);
  }
}

void EvasPlugin::OnEcoreEventClientMessaged(void* data, int type, void* event)
{
  Ecore_X_Event_Client_Message* clientMessageEvent( (Ecore_X_Event_Client_Message*)event );

  if(clientMessageEvent->message_type == ecore_x_atom_get(CLIPBOARD_ATOM))
  {
    std::string message(clientMessageEvent->data.b);
    if( message == CLIPBOARD_SET_OWNER_MESSAGE)
    {
      // Claim the ownership of the SECONDARY selection.
      ecore_x_selection_secondary_set(clientMessageEvent->win, "", 1);

      // Show the clipboard window
      Dali::Clipboard clipboard = Dali::Clipboard::Get();
      clipboard.ShowClipboard();
    }
  }
}

void EvasPlugin::SignalConnected( SlotObserver* slotObserver, CallbackBase* callback )
{
  mConnectionTracker.SignalConnected( slotObserver, callback );
}

void EvasPlugin::SignalDisconnected( SlotObserver* signal, CallbackBase* callback )
{
  mConnectionTracker.SignalDisconnected( signal, callback );
}

std::size_t EvasPlugin::GetConnectionCount() const
{
  return mConnectionTracker.GetConnectionCount();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
