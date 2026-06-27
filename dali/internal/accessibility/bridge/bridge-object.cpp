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
#include <dali/internal/accessibility/bridge/bridge-object.h>

// EXTERNAL INCLUDES
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include <dali/devel-api/atspi-interfaces/accessible.h>

using namespace Dali::Accessibility;
using State = Dali::Integration::Accessibility::State;

namespace
{
inline std::string GetAccessiblePath(Accessible* accessible)
{
  auto address = accessible->GetAddress();
  return address ? ATSPI_PREFIX_PATH + address.GetPath() : ATSPI_NULL_PATH;
}
} // namespace

BridgeObject::BridgeObject()
{
}

void BridgeObject::RegisterInterfaces()
{
  // DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT)};
  // mStateChanged = addSignal<std::string, int, int, DBus::DbusVariant<int>, Accessible*>(desc, "StateChanged");
  // mDbusServer.addInterface("/", desc, true);
}

void BridgeObject::EmitActiveDescendantChanged(Accessible* obj, Accessible* child)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::ACTIVE_DESCENDANT_CHANGED] || child->IsHidden())
  {
    return;
  }

  auto index = child->GetIndexInParent();

  mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<Dali::Devel::Accessibility::Address>, Dali::Devel::Accessibility::Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
    "ActiveDescendantChanged",
    "",
    index,
    0,
    {child->GetAddress()},
    {"", "root"});
}

void BridgeObject::Emit(Dali::SharedPtr<Accessible> obj, Dali::Devel::Accessibility::ObjectPropertyChangeEvent event)
{
  static const std::unordered_map<Dali::Devel::Accessibility::ObjectPropertyChangeEvent, std::string_view> eventMap{
    {Dali::Devel::Accessibility::ObjectPropertyChangeEvent::NAME, "accessible-name"},
    {Dali::Devel::Accessibility::ObjectPropertyChangeEvent::DESCRIPTION, "accessible-description"},
    {Dali::Devel::Accessibility::ObjectPropertyChangeEvent::VALUE, "accessible-value"},
    {Dali::Devel::Accessibility::ObjectPropertyChangeEvent::PARENT, "accessible-parent"},
    {Dali::Devel::Accessibility::ObjectPropertyChangeEvent::ROLE, "accessible-role"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::PROPERTY_CHANGED])
  {
    return;
  }

  auto eventName = eventMap.find(event);

  if(eventName != eventMap.end())
  {
    AddCoalescableMessage(static_cast<CoalescableMessages>(static_cast<int>(CoalescableMessages::PROPERTY_CHANGED_BEGIN) + static_cast<int>(event)), obj.Get(), 1.0f, [this, weakObj = Dali::WeakPtr<Accessible>(obj), eventNameString = std::string{eventName->second}]()
    {
      if(auto accessible = weakObj.Lock())
      {
        mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
          GetAccessiblePath(accessible.Get()),
          Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
          "PropertyChange",
          eventNameString,
          0,
          0,
          {0},
          {"", "root"});
      }
    });
  }
}

void BridgeObject::Emit(Accessible* obj, Dali::Devel::Accessibility::WindowEvent event, unsigned int detail)
{
  static const std::unordered_map<Dali::Devel::Accessibility::WindowEvent, std::string_view> eventMap{
    {Dali::Devel::Accessibility::WindowEvent::PROPERTY_CHANGE, "PropertyChange"},
    {Dali::Devel::Accessibility::WindowEvent::MINIMIZE, "Minimize"},
    {Dali::Devel::Accessibility::WindowEvent::MAXIMIZE, "Maximize"},
    {Dali::Devel::Accessibility::WindowEvent::RESTORE, "Restore"},
    {Dali::Devel::Accessibility::WindowEvent::CLOSE, "Close"},
    {Dali::Devel::Accessibility::WindowEvent::CREATE, "Create"},
    {Dali::Devel::Accessibility::WindowEvent::REPARENT, "Reparent"},
    {Dali::Devel::Accessibility::WindowEvent::DESKTOP_CREATE, "DesktopCreate"},
    {Dali::Devel::Accessibility::WindowEvent::DESKTOP_DESTROY, "DesktopDestroy"},
    {Dali::Devel::Accessibility::WindowEvent::DESTROY, "Destroy"},
    {Dali::Devel::Accessibility::WindowEvent::ACTIVATE, "Activate"},
    {Dali::Devel::Accessibility::WindowEvent::DEACTIVATE, "Deactivate"},
    {Dali::Devel::Accessibility::WindowEvent::RAISE, "Raise"},
    {Dali::Devel::Accessibility::WindowEvent::LOWER, "Lower"},
    {Dali::Devel::Accessibility::WindowEvent::MOVE, "Move"},
    {Dali::Devel::Accessibility::WindowEvent::RESIZE, "Resize"},
    {Dali::Devel::Accessibility::WindowEvent::SHADE, "Shade"},
    {Dali::Devel::Accessibility::WindowEvent::UU_SHADE, "uUshade"},
    {Dali::Devel::Accessibility::WindowEvent::RESTYLE, "Restyle"},
    {Dali::Devel::Accessibility::WindowEvent::POST_RENDER, "PostRender"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::WINDOW_CHANGED])
  {
    return;
  }

  auto eventName = eventMap.find(event);

  if(eventName != eventMap.end())
  {
    mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
      GetAccessiblePath(obj),
      Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_WINDOW),
      std::string{eventName->second},
      "",
      detail,
      0,
      {0},
      {"", "root"});
  }
}

void BridgeObject::EmitStateChanged(Dali::SharedPtr<Accessible> obj, State state, int newValue, int reserved)
{
  static const std::unordered_map<State, std::string_view> stateMap{
    {State::INVALID, "invalid"},
    {State::ACTIVE, "active"},
    {State::ARMED, "armed"},
    {State::BUSY, "busy"},
    {State::CHECKED, "checked"},
    {State::COLLAPSED, "collapsed"},
    {State::DEFUNCT, "defunct"},
    {State::EDITABLE, "editable"},
    {State::ENABLED, "enabled"},
    {State::EXPANDABLE, "expandable"},
    {State::EXPANDED, "expanded"},
    {State::FOCUSABLE, "focusable"},
    {State::FOCUSED, "focused"},
    {State::HAS_TOOLTIP, "has-tooltip"},
    {State::HORIZONTAL, "horizontal"},
    {State::ICONIFIED, "iconified"},
    {State::MODAL, "modal"},
    {State::MULTI_LINE, "multi-line"},
    {State::MULTI_SELECTABLE, "multiselectable"},
    {State::OPAQUE, "opaque"},
    {State::PRESSED, "pressed"},
    {State::RESIZEABLE, "resizable"},
    {State::SELECTABLE, "selectable"},
    {State::SELECTED, "selected"},
    {State::SENSITIVE, "sensitive"},
    {State::SHOWING, "showing"},
    {State::SINGLE_LINE, "single-line"},
    {State::STALE, "stale"},
    {State::TRANSIENT, "transient"},
    {State::VERTICAL, "vertical"},
    {State::VISIBLE, "visible"},
    {State::MANAGES_DESCENDANTS, "manages-descendants"},
    {State::INDETERMINATE, "indeterminate"},
    {State::REQUIRED, "required"},
    {State::TRUNCATED, "truncated"},
    {State::ANIMATED, "animated"},
    {State::INVALID_ENTRY, "invalid-entry"},
    {State::SUPPORTS_AUTOCOMPLETION, "supports-autocompletion"},
    {State::SELECTABLE_TEXT, "selectable-text"},
    {State::IS_DEFAULT, "is-default"},
    {State::VISITED, "visited"},
    {State::CHECKABLE, "checkable"},
    {State::HAS_POPUP, "has-popup"},
    {State::READ_ONLY, "read-only"},
    {State::HIGHLIGHTED, "highlighted"},
    {State::HIGHLIGHTABLE, "highlightable"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::STATE_CHANGED]) // separate ?
  {
    return;
  }

  auto stateName = stateMap.find(state);

  if(stateName != stateMap.end())
  {
    AddCoalescableMessage(static_cast<CoalescableMessages>(static_cast<int>(CoalescableMessages::STATE_CHANGED_BEGIN) + static_cast<int>(state)), obj.Get(), 1.0f, [this, weakObj = Dali::WeakPtr<Accessible>(obj), stateNameString = std::string{stateName->second}, newValue, reserved]()
    {
      if(auto accessible = weakObj.Lock())
      {
        mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
          GetAccessiblePath(accessible.Get()),
          Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
          "StateChanged",
          stateNameString,
          newValue,
          reserved,
          {0},
          {"", "root"});
      }
    });
  }
}

void BridgeObject::EmitBoundsChanged(Dali::SharedPtr<Accessible> obj, Dali::BoundsInteger rect)
{
  if(!IsUp() || !IsBoundsChangedEventAllowed || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::BOUNDS_CHANGED])
  {
    return;
  }

  AddCoalescableMessage(CoalescableMessages::BOUNDS_CHANGED, obj.Get(), 1.0f, [this, weakObj = Dali::WeakPtr<Accessible>(obj), rect = std::move(rect)]()
  {
    if(auto accessible = weakObj.Lock())
    {
      DBus::DbusVariant<std::tuple<int32_t, int32_t, int32_t, int32_t> > tmp{
        std::tuple<int32_t, int32_t, int32_t, int32_t>{rect.x, rect.y, rect.width, rect.height}};

      mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<std::tuple<int32_t, int32_t, int32_t, int32_t> >, Dali::Devel::Accessibility::Address>(
        GetAccessiblePath(accessible.Get()),
        Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
        "BoundsChanged",
        "",
        0,
        0,
        tmp,
        {"", "root"});
    }
  });
}

void BridgeObject::EmitPostRender(Dali::SharedPtr<Accessible> obj)
{
  if(!IsUp() || obj->IsHidden())
  {
    return;
  }

  AddCoalescableMessage(CoalescableMessages::POST_RENDER, obj.Get(), 0.5f, [this, weakObj = Dali::WeakPtr<Accessible>(obj)]()
  {
    if(auto accessible = weakObj.Lock())
    {
      Emit(accessible.Get(), Dali::Devel::Accessibility::WindowEvent::POST_RENDER);
    }
  });
}

void BridgeObject::EmitCursorMoved(Accessible* obj, unsigned int cursorPosition)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::TEXT_CARET_MOVED])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
    "TextCaretMoved",
    "",
    cursorPosition,
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitTextChanged(Accessible* obj, Dali::Devel::Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string& content)
{
  static const std::unordered_map<Dali::Devel::Accessibility::TextChangedState, std::string_view> stateMap{
    {Dali::Devel::Accessibility::TextChangedState::INSERTED, "insert"},
    {Dali::Devel::Accessibility::TextChangedState::DELETED, "delete"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::TEXT_CHANGED])
  {
    return;
  }

  auto stateName = stateMap.find(state);

  if(stateName != stateMap.end())
  {
    mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<std::string>, Dali::Devel::Accessibility::Address>(
      GetAccessiblePath(obj),
      Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
      "TextChanged",
      std::string{stateName->second},
      position,
      length,
      {content},
      {"", "root"});
  }
}

void BridgeObject::EmitMovedOutOfScreen(Accessible* obj, Dali::Devel::Accessibility::ScreenRelativeMoveType type)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::MOVED_OUT])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
    "MoveOuted",
    "",
    static_cast<int>(type),
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitScrollStarted(Accessible* obj)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::SCROLL_STARTED])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
    "ScrollStarted",
    "",
    0,
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitScrollFinished(Accessible* obj)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[Dali::Integration::Accessibility::AccessibilityEvent::SCROLL_FINISHED])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::DbusVariant<int>, Dali::Devel::Accessibility::Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(Dali::Integration::Accessibility::AccessibilityInterface::EVENT_OBJECT),
    "ScrollFinished",
    "",
    0,
    0,
    {0},
    {"", "root"});
}
