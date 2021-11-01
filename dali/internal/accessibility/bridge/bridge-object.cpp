/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
  // DBus::DBusInterfaceDescription desc{ AtspiDbusInterfaceEventObject };
  // mStateChanged = addSignal<std::string, int, int, DBus::EldbusVariant<int>, Accessible*>(desc, "StateChanged");
  // mDbusServer.addInterface("/", desc, true);
}

void BridgeObject::EmitActiveDescendantChanged(Accessible* obj, Accessible* child)
{
  if(!IsUp()) return;
  auto index = child->GetIndexInParent();

  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<Address>, Address>(
    GetAccessiblePath(obj),
    AtspiDbusInterfaceEventObject,
    "ActiveDescendantChanged",
    "",
    index,
    0,
    {child->GetAddress()},
    {"", "root"});
}

void BridgeObject::Emit(Accessible* obj, Dali::Accessibility::ObjectPropertyChangeEvent event)
{
  if(!IsUp()) return;
  const char* name = nullptr;

  switch(event)
  {
    case ObjectPropertyChangeEvent::NAME:
    {
      name = "accessible-name";
      break;
    }
    case ObjectPropertyChangeEvent::DESCRIPTION:
    {
      name = "accessible-description";
      break;
    }
    case ObjectPropertyChangeEvent::VALUE:
    {
      name = "accessible-value";
      break;
    }
    case ObjectPropertyChangeEvent::PARENT:
    {
      name = "accessible-parent";
      break;
    }
    case ObjectPropertyChangeEvent::ROLE:
    {
      name = "accessible-role";
      break;
    }
  }

  if(name)
  {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
      GetAccessiblePath(obj),
      AtspiDbusInterfaceEventObject,
      "PropertyChange",
      name,
      0,
      0,
      {0},
      {"", "root"});
  }
}

void BridgeObject::Emit(Accessible* obj, WindowEvent event, unsigned int detail)
{
  if(!IsUp()) return;
  const char* name = nullptr;

  switch(event)
  {
    case WindowEvent::PROPERTY_CHANGE:
    {
      name = "PropertyChange";
      break;
    }
    case WindowEvent::MINIMIZE:
    {
      name = "Minimize";
      break;
    }
    case WindowEvent::MAXIMIZE:
    {
      name = "Maximize";
      break;
    }
    case WindowEvent::RESTORE:
    {
      name = "Restore";
      break;
    }
    case WindowEvent::CLOSE:
    {
      name = "Close";
      break;
    }
    case WindowEvent::CREATE:
    {
      name = "Create";
      break;
    }
    case WindowEvent::REPARENT:
    {
      name = "Reparent";
      break;
    }
    case WindowEvent::DESKTOP_CREATE:
    {
      name = "DesktopCreate";
      break;
    }
    case WindowEvent::DESKTOP_DESTROY:
    {
      name = "DesktopDestroy";
      break;
    }
    case WindowEvent::DESTROY:
    {
      name = "Destroy";
      break;
    }
    case WindowEvent::ACTIVATE:
    {
      name = "Activate";
      break;
    }
    case WindowEvent::DEACTIVATE:
    {
      name = "Deactivate";
      break;
    }
    case WindowEvent::RAISE:
    {
      name = "Raise";
      break;
    }
    case WindowEvent::LOWER:
    {
      name = "Lower";
      break;
    }
    case WindowEvent::MOVE:
    {
      name = "Move";
      break;
    }
    case WindowEvent::RESIZE:
    {
      name = "Resize";
      break;
    }
    case WindowEvent::SHADE:
    {
      name = "Shade";
      break;
    }
    case WindowEvent::UU_SHADE:
    {
      name = "uUshade";
      break;
    }
    case WindowEvent::RESTYLE:
    {
      name = "Restyle";
      break;
    }
  }

  if(name)
  {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
      GetAccessiblePath(obj),
      AtspiDbusInterfaceEventWindow,
      name,
      "",
      detail,
      0,
      {0},
      {"", "root"});
  }
}

void BridgeObject::EmitStateChanged(Accessible* obj, State state, int newValue, int reserved)
{
  if(!IsUp()) return;
  const char* stateName = nullptr;

  switch(state)
  {
    case State::INVALID:
    {
      stateName = "invalid";
      break;
    }
    case State::ACTIVE:
    {
      stateName = "active";
      break;
    }
    case State::ARMED:
    {
      stateName = "armed";
      break;
    }
    case State::BUSY:
    {
      stateName = "busy";
      break;
    }
    case State::CHECKED:
    {
      stateName = "checked";
      break;
    }
    case State::COLLAPSED:
    {
      stateName = "collapsed";
      break;
    }
    case State::DEFUNCT:
    {
      stateName = "defunct";
      break;
    }
    case State::EDITABLE:
    {
      stateName = "editable";
      break;
    }
    case State::ENABLED:
    {
      stateName = "enabled";
      break;
    }
    case State::EXPANDABLE:
    {
      stateName = "expandable";
      break;
    }
    case State::EXPANDED:
    {
      stateName = "expanded";
      break;
    }
    case State::FOCUSABLE:
    {
      stateName = "focusable";
      break;
    }
    case State::FOCUSED:
    {
      stateName = "focused";
      break;
    }
    case State::HAS_TOOLTIP:
    {
      stateName = "has-tooltip";
      break;
    }
    case State::HORIZONTAL:
    {
      stateName = "horizontal";
      break;
    }
    case State::ICONIFIED:
    {
      stateName = "iconified";
      break;
    }
    case State::MODAL:
    {
      stateName = "modal";
      break;
    }
    case State::MULTI_LINE:
    {
      stateName = "multi-line";
      break;
    }
    case State::MULTI_SELECTABLE:
    {
      stateName = "multiselectable";
      break;
    }
    case State::OPAQUE:
    {
      stateName = "opaque";
      break;
    }
    case State::PRESSED:
    {
      stateName = "pressed";
      break;
    }
    case State::RESIZEABLE:
    {
      stateName = "resizable";
      break;
    }
    case State::SELECTABLE:
    {
      stateName = "selectable";
      break;
    }
    case State::SELECTED:
    {
      stateName = "selected";
      break;
    }
    case State::SENSITIVE:
    {
      stateName = "sensitive";
      break;
    }
    case State::SHOWING:
    {
      stateName = "showing";
      break;
    }
    case State::SINGLE_LINE:
    {
      stateName = "single-line";
      break;
    }
    case State::STALE:
    {
      stateName = "stale";
      break;
    }
    case State::TRANSIENT:
    {
      stateName = "transient";
      break;
    }
    case State::VERTICAL:
    {
      stateName = "vertical";
      break;
    }
    case State::VISIBLE:
    {
      stateName = "visible";
      break;
    }
    case State::MANAGES_DESCENDANTS:
    {
      stateName = "manages-descendants";
      break;
    }
    case State::INDETERMINATE:
    {
      stateName = "indeterminate";
      break;
    }
    case State::REQUIRED:
    {
      stateName = "required";
      break;
    }
    case State::TRUNCATED:
    {
      stateName = "truncated";
      break;
    }
    case State::ANIMATED:
    {
      stateName = "animated";
      break;
    }
    case State::INVALID_ENTRY:
    {
      stateName = "invalid-entry";
      break;
    }
    case State::SUPPORTS_AUTOCOMPLETION:
    {
      stateName = "supports-autocompletion";
      break;
    }
    case State::SELECTABLE_TEXT:
    {
      stateName = "selectable-text";
      break;
    }
    case State::IS_DEFAULT:
    {
      stateName = "is-default";
      break;
    }
    case State::VISITED:
    {
      stateName = "visited";
      break;
    }
    case State::CHECKABLE:
    {
      stateName = "checkable";
      break;
    }
    case State::HAS_POPUP:
    {
      stateName = "has-popup";
      break;
    }
    case State::READ_ONLY:
    {
      stateName = "read-only";
      break;
    }
    case State::HIGHLIGHTED:
    {
      stateName = "highlighted";
      break;
    }
    case State::HIGHLIGHTABLE:
    {
      stateName = "highlightable";
      break;
    }
    case State::MAX_COUNT:
    {
      break;
    }
  }

  if(stateName)
  {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
      GetAccessiblePath(obj),
      AtspiDbusInterfaceEventObject,
      "StateChanged",
      stateName,
      newValue,
      reserved,
      {0},
      {"", "root"});
  }
}

void BridgeObject::EmitBoundsChanged(Accessible* obj, Dali::Rect<> rect)
{
  if(!IsBoundsChangedEventAllowed) return;

  DBus::EldbusVariant<std::tuple<int32_t, int32_t, int32_t, int32_t> > tmp{
    std::tuple<int32_t, int32_t, int32_t, int32_t>{rect.x, rect.y, rect.width, rect.height}};

  AddFilteredEvent(FilteredEvents::BOUNDS_CHANGED, obj, 1.0f, [=]() {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<std::tuple<int32_t, int32_t, int32_t, int32_t> >, Address>(
      GetAccessiblePath(obj),
      AtspiDbusInterfaceEventObject,
      "BoundsChanged",
      "",
      0,
      0,
      tmp,
      {"", "root"});
  });
}

void BridgeObject::EmitCursorMoved(Accessible* obj, unsigned int cursorPosition)
{
  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
    GetAccessiblePath(obj),
    AtspiDbusInterfaceEventObject,
    "TextCaretMoved",
    "",
    cursorPosition,
    0,
    {0},
    {"", "root"});
}

void BridgeObject::EmitTextChanged(Accessible* obj, TextChangedState state, unsigned int position, unsigned int length, const std::string& content)
{
  const char* stateName = nullptr;
  switch(state)
  {
    case TextChangedState::INSERTED:
    {
      stateName = "insert";
      break;
    }
    case TextChangedState::DELETED:
    {
      stateName = "delete";
      break;
    }
    case TextChangedState::MAX_COUNT:
    {
      break;
    }
  }

  if(stateName)
  {
    mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<std::string>, Address>(
      GetAccessiblePath(obj),
      AtspiDbusInterfaceEventObject,
      "TextChanged",
      stateName,
      position,
      length,
      {content},
      {"", "root"});
  }
}

void BridgeObject::EmitMovedOutOfScreen(Accessible* obj, ScreenRelativeMoveType type)
{
  mDbusServer.emit2<std::string, int, int, DBus::EldbusVariant<int>, Address>(
    GetAccessiblePath(obj),
    AtspiDbusInterfaceEventObject,
    "MoveOuted",
    "",
    static_cast<int>(type),
    0,
    {0},
    {"", "root"});
}
