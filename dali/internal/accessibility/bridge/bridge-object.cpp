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
#include <dali/internal/accessibility/bridge/bridge-object.h>

// EXTERNAL INCLUDES
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace Dali::Accessibility;

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
  // DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT)};
  // mStateChanged = addSignal<std::string, int, int, DBus::EldbusVariant<int>, Accessible*>(desc, "StateChanged");
  // mDbusServer.addInterface("/", desc, true);
}

void BridgeObject::EmitActiveDescendantChanged(Accessible* obj, Accessible* child)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::ACTIVE_DESCENDANT_CHANGED] || child->IsHidden())
  {
    return;
  }

  auto index = child->GetIndexInParent();

  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<Address>, Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
    "ActiveDescendantChanged",
    "",
    index,
    0,
    {child->GetAddress()},
    {"", "root"});
}

void BridgeObject::Emit(std::shared_ptr<Accessible> obj, ObjectPropertyChangeEvent event)
{
  static const std::unordered_map<ObjectPropertyChangeEvent, std::string_view> eventMap{
    {ObjectPropertyChangeEvent::NAME, "accessible-name"},
    {ObjectPropertyChangeEvent::DESCRIPTION, "accessible-description"},
    {ObjectPropertyChangeEvent::VALUE, "accessible-value"},
    {ObjectPropertyChangeEvent::PARENT, "accessible-parent"},
    {ObjectPropertyChangeEvent::ROLE, "accessible-role"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::PROPERTY_CHANGED])
  {
    return;
  }

  auto eventName = eventMap.find(event);

  if(eventName != eventMap.end())
  {
    AddCoalescableMessage(static_cast<CoalescableMessages>(static_cast<int>(CoalescableMessages::PROPERTY_CHANGED_BEGIN) + static_cast<int>(event)), obj.get(), 1.0f, [weakObj = std::weak_ptr<Accessible>(obj), eventName, this]() {
      if(auto obj = weakObj.lock())
      {
        mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
          GetAccessiblePath(obj.get()),
          Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
          "PropertyChange",
          std::string{eventName->second},
          0,
          0,
          {0},
          {"", "root"});
      }
    });
  }
}

void BridgeObject::Emit(Accessible* obj, WindowEvent event, unsigned int detail)
{
  static const std::unordered_map<WindowEvent, std::string_view> eventMap{
    {WindowEvent::PROPERTY_CHANGE, "PropertyChange"},
    {WindowEvent::MINIMIZE, "Minimize"},
    {WindowEvent::MAXIMIZE, "Maximize"},
    {WindowEvent::RESTORE, "Restore"},
    {WindowEvent::CLOSE, "Close"},
    {WindowEvent::CREATE, "Create"},
    {WindowEvent::REPARENT, "Reparent"},
    {WindowEvent::DESKTOP_CREATE, "DesktopCreate"},
    {WindowEvent::DESKTOP_DESTROY, "DesktopDestroy"},
    {WindowEvent::DESTROY, "Destroy"},
    {WindowEvent::ACTIVATE, "Activate"},
    {WindowEvent::DEACTIVATE, "Deactivate"},
    {WindowEvent::RAISE, "Raise"},
    {WindowEvent::LOWER, "Lower"},
    {WindowEvent::MOVE, "Move"},
    {WindowEvent::RESIZE, "Resize"},
    {WindowEvent::SHADE, "Shade"},
    {WindowEvent::UU_SHADE, "uUshade"},
    {WindowEvent::RESTYLE, "Restyle"},
    {WindowEvent::POST_RENDER, "PostRender"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::WINDOW_CHANGED])
  {
    return;
  }

  auto eventName = eventMap.find(event);

  if(eventName != eventMap.end())
  {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
      GetAccessiblePath(obj),
      Accessible::GetInterfaceName(AtspiInterface::EVENT_WINDOW),
      std::string{eventName->second},
      "",
      detail,
      0,
      {0},
      {"", "root"});
  }
}

void BridgeObject::EmitStateChanged(std::shared_ptr<Accessible> obj, State state, int newValue, int reserved)
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

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::STATE_CHANGED]) // separate ?
  {
    return;
  }

  auto stateName = stateMap.find(state);

  if(stateName != stateMap.end())
  {
    AddCoalescableMessage(static_cast<CoalescableMessages>(static_cast<int>(CoalescableMessages::STATE_CHANGED_BEGIN) + static_cast<int>(state)), obj.get(), 1.0f, [weakObj = std::weak_ptr<Accessible>(obj), stateName, newValue, reserved, this]() {
      if(auto obj = weakObj.lock())
      {
        mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
          GetAccessiblePath(obj.get()),
          Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
          "StateChanged",
          std::string{stateName->second},
          newValue,
          reserved,
          {0},
          {"", "root"});
      }
    });
  }
}

void BridgeObject::EmitBoundsChanged(std::shared_ptr<Accessible> obj, Dali::Rect<> rect)
{
  if(!IsUp() || !IsBoundsChangedEventAllowed || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::BOUNDS_CHANGED])
  {
    return;
  }

  AddCoalescableMessage(CoalescableMessages::BOUNDS_CHANGED, obj.get(), 1.0f, [weakObj = std::weak_ptr<Accessible>(obj), rect = std::move(rect), this]() {
    if(auto obj = weakObj.lock())
    {
      DBus::EldbusVariant<std::tuple<int32_t, int32_t, int32_t, int32_t> > tmp{
        std::tuple<int32_t, int32_t, int32_t, int32_t>{rect.x, rect.y, rect.width, rect.height}};

      mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<std::tuple<int32_t, int32_t, int32_t, int32_t> >, Address>(
        GetAccessiblePath(obj.get()),
        Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
        "BoundsChanged",
        "",
        0,
        0,
        tmp,
        {"", "root"});
    }
  });
}

void BridgeObject::EmitPostRender(std::shared_ptr<Accessible> obj)
{
  if(!IsUp() || obj->IsHidden())
  {
    return;
  }

  AddCoalescableMessage(CoalescableMessages::POST_RENDER, obj.get(), 0.5f, [weakObj = std::weak_ptr<Accessible>(obj), this]() {
    if(auto obj = weakObj.lock())
    {
      Emit(obj.get(), WindowEvent::POST_RENDER);
    }
  });
}

void BridgeObject::EmitCursorMoved(Accessible* obj, unsigned int cursorPosition)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::TEXT_CARET_MOVED])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
    "TextCaretMoved",
    "",
    cursorPosition,
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitTextChanged(Accessible* obj, TextChangedState state, unsigned int position, unsigned int length, const std::string& content)
{
  static const std::unordered_map<TextChangedState, std::string_view> stateMap{
    {TextChangedState::INSERTED, "insert"},
    {TextChangedState::DELETED, "delete"},
  };

  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::TEXT_CHANGED])
  {
    return;
  }

  auto stateName = stateMap.find(state);

  if(stateName != stateMap.end())
  {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<std::string>, Address>(
      GetAccessiblePath(obj),
      Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
      "TextChanged",
      std::string{stateName->second},
      position,
      length,
      {content},
      {"", "root"});
  }
}

void BridgeObject::EmitMovedOutOfScreen(Accessible* obj, ScreenRelativeMoveType type)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::MOVED_OUT])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
    "MoveOuted",
    "",
    static_cast<int>(type),
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitSocketAvailable(Accessible* obj)
{
  if(!IsUp() || obj->IsHidden()) //TODO Suppress SocketAvailable event
  {
    return;
  }

  mDbusServer.emit2<Address, Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(AtspiInterface::SOCKET),
    "Available",
    obj->GetAddress(),
    {"", "root"});
}

void BridgeObject::EmitScrollStarted(Accessible* obj)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::SCROLL_STARTED])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
    "ScrollStarted",
    "",
    0,
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitScrollFinished(Accessible* obj)
{
  if(!IsUp() || obj->IsHidden() || obj->GetSuppressedEvents()[AtspiEvent::SCROLL_FINISHED])
  {
    return;
  }

  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
    GetAccessiblePath(obj),
    Accessible::GetInterfaceName(AtspiInterface::EVENT_OBJECT),
    "ScrollFinished",
    "",
    0,
    0,
    {0},
    {"", "root"});
}
