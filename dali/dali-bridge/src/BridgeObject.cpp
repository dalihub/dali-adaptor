#include "BridgeObject.hpp"
#include <iostream>
#include <string>

using namespace Dali::Accessibility;

BridgeObject::BridgeObject()
{
}

void BridgeObject::RegisterInterfaces()
{
  // DBus::DBusInterfaceDescription desc{ ATSPI_DBUS_INTERFACE_EVENT_OBJECT };
  // stateChanged = addSignal<std::string, int, int, DBus::EldbusVariant<int>, Accessible*>(desc, "StateChanged");
  // dbusServer.addInterface("/", desc, true);
}

void BridgeObject::Emit( Accessible* obj, WindowEvent we, unsigned int detail1 )
{
  const char* name = nullptr;
  switch( we )
  {
    case WindowEvent::PropertyChange:
    {
      name = "PropertyChange";
      break;
    }
    case WindowEvent::Minimize:
    {
      name = "Minimize";
      break;
    }
    case WindowEvent::Maximize:
    {
      name = "Maximize";
      break;
    }
    case WindowEvent::Restore:
    {
      name = "Restore";
      break;
    }
    case WindowEvent::Close:
    {
      name = "Close";
      break;
    }
    case WindowEvent::Create:
    {
      name = "Create";
      break;
    }
    case WindowEvent::Reparent:
    {
      name = "Reparent";
      break;
    }
    case WindowEvent::DesktopCreate:
    {
      name = "DesktopCreate";
      break;
    }
    case WindowEvent::DesktopDestroy:
    {
      name = "DesktopDestroy";
      break;
    }
    case WindowEvent::Destroy:
    {
      name = "Destroy";
      break;
    }
    case WindowEvent::Activate:
    {
      name = "Activate";
      break;
    }
    case WindowEvent::Deactivate:
    {
      name = "Deactivate";
      break;
    }
    case WindowEvent::Raise:
    {
      name = "Raise";
      break;
    }
    case WindowEvent::Lower:
    {
      name = "Lower";
      break;
    }
    case WindowEvent::Move:
    {
      name = "Move";
      break;
    }
    case WindowEvent::Resize:
    {
      name = "Resize";
      break;
    }
    case WindowEvent::Shade:
    {
      name = "Shade";
      break;
    }
    case WindowEvent::UuShade:
    {
      name = "uUshade";
      break;
    }
    case WindowEvent::Restyle:
    {
      name = "Restyle";
      break;
    }
  }
  if( name )
  {
    auto addr = obj->GetAddress();
    const auto prefixPath = "/org/a11y/atspi/accessible/";
    const auto nullPath = "/org/a11y/atspi/null";
    std::string p;
    if( addr )
      p = prefixPath + addr.GetPath();
    else
      p = nullPath;
    dbusServer.emit2< std::string, int, int, DBus::EldbusVariant< int >, Address >(
        p,
        ATSPI_DBUS_INTERFACE_EVENT_WINDOW,
        name,
        "",
        detail1,
        0,
        {0},
        {GetBusName(), "root"} );
  }
}

void BridgeObject::EmitStateChanged( Accessible* obj, State state, int newValue1, int newValue2 )
{
  const char* stateName = nullptr;
  switch( state )
  {
    case State::Invalid:
    {
      stateName = "invalid";
      break;
    }
    case State::Active:
    {
      stateName = "active";
      break;
    }
    case State::Armed:
    {
      stateName = "armed";
      break;
    }
    case State::Busy:
    {
      stateName = "busy";
      break;
    }
    case State::Checked:
    {
      stateName = "checked";
      break;
    }
    case State::Collapsed:
    {
      stateName = "collapsed";
      break;
    }
    case State::Defunct:
    {
      stateName = "defunct";
      break;
    }
    case State::Editable:
    {
      stateName = "editable";
      break;
    }
    case State::Enabled:
    {
      stateName = "enabled";
      break;
    }
    case State::Expandable:
    {
      stateName = "expandable";
      break;
    }
    case State::Expanded:
    {
      stateName = "expanded";
      break;
    }
    case State::Focusable:
    {
      stateName = "focusable";
      break;
    }
    case State::Focused:
    {
      stateName = "focused";
      break;
    }
    case State::HasTooltip:
    {
      stateName = "has-tooltip";
      break;
    }
    case State::Horizontal:
    {
      stateName = "horizontal";
      break;
    }
    case State::Iconified:
    {
      stateName = "iconified";
      break;
    }
    case State::Modal:
    {
      stateName = "modal";
      break;
    }
    case State::MultiLine:
    {
      stateName = "multi-line";
      break;
    }
    case State::MultiSelectable:
    {
      stateName = "multiselectable";
      break;
    }
    case State::Opaque:
    {
      stateName = "opaque";
      break;
    }
    case State::Pressed:
    {
      stateName = "pressed";
      break;
    }
    case State::Resizeable:
    {
      stateName = "resizable";
      break;
    }
    case State::Selectable:
    {
      stateName = "selectable";
      break;
    }
    case State::Selected:
    {
      stateName = "selected";
      break;
    }
    case State::Sensitive:
    {
      stateName = "sensitive";
      break;
    }
    case State::Showing:
    {
      stateName = "showing";
      break;
    }
    case State::SingleLine:
    {
      stateName = "single-line";
      break;
    }
    case State::Stale:
    {
      stateName = "stale";
      break;
    }
    case State::Transient:
    {
      stateName = "transient";
      break;
    }
    case State::Vertical:
    {
      stateName = "vertical";
      break;
    }
    case State::Visible:
    {
      stateName = "visible";
      break;
    }
    case State::ManagesDescendants:
    {
      stateName = "manages-descendants";
      break;
    }
    case State::Indeterminate:
    {
      stateName = "indeterminate";
      break;
    }
    case State::Required:
    {
      stateName = "required";
      break;
    }
    case State::Truncated:
    {
      stateName = "truncated";
      break;
    }
    case State::Animated:
    {
      stateName = "animated";
      break;
    }
    case State::InvalidEntry:
    {
      stateName = "invalid-entry";
      break;
    }
    case State::SupportsAutocompletion:
    {
      stateName = "supports-autocompletion";
      break;
    }
    case State::SelectableText:
    {
      stateName = "selectable-text";
      break;
    }
    case State::IsDefault:
    {
      stateName = "is-default";
      break;
    }
    case State::Visited:
    {
      stateName = "visited";
      break;
    }
    case State::Checkable:
    {
      stateName = "checkable";
      break;
    }
    case State::HasPopup:
    {
      stateName = "has-popup";
      break;
    }
    case State::ReadOnly:
    {
      stateName = "read-only";
      break;
    }
    case State::Highlighted:
    {
      stateName = "highlighted";
      break;
    }
    case State::Highlightable:
    {
      stateName = "highlightable";
      break;
    }
    case State::_Count:
    {
      break;
    }
  }
  if( stateName )
  {
    auto addr = obj->GetAddress();
    const auto prefixPath = "/org/a11y/atspi/accessible/";
    const auto nullPath = "/org/a11y/atspi/null";
    std::string p;
    if( addr )
      p = prefixPath + addr.GetPath();
    else
      p = nullPath;
    dbusServer.emit2< std::string, int, int, DBus::EldbusVariant< int >, Address >(
        p,
        ATSPI_DBUS_INTERFACE_EVENT_OBJECT,
        "StateChanged",
        stateName,
        newValue1,
        newValue2,
        {0},
        {GetBusName(), "root"} );
  }
}
