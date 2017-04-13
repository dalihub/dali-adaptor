// CLASS HEADER
#include "x-input2.h"

// EXTERNAL INCLUDES
#include <X11/XKBlib.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include "debug/x-input2-debug.h"


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
// For multi-touch we need XI2 version 2.2
int XI2MinorVersionRequired = 2;
int XI2MajorVersionRequired = 2;
}

XInput2::XInput2( XID window, Display* display, WindowEventInterface* eventInterface )
: mEventInterface( eventInterface ),
  mDisplay( display ),
  mWindow( window ),
  mXI2ExtensionId(-1),
  mMultiTouchSupport( false )
{

}
XInput2::~XInput2()
{
}

void XInput2::Initialize()
{
  // Check if X supports the multi-touch protocol
  QueryMultiTouchSupport();

  // Query what input devices are available on the system.
  QueryDevices();

  // Select the input events we want to receive from the input devices available
  SelectInputEvents();

}

int XInput2::GetExtensionId() const
{
  return mXI2ExtensionId;
}

bool XInput2::FilteredDevice( int deviceId ) const
{
  for( VectorBase::SizeType i = 0; i < mInputDeviceInfo.Count(); ++i )
  {
    if( mInputDeviceInfo[i].deviceId == deviceId )
    {
      return true;
    }
  }
  return false;
}

bool XInput2::PreProcessEvent( XIDeviceEvent *deviceEvent ) const
{
  // @todo need to do some testing to see if this check is actually required
  // E.g. if IME window is sending events, this check may fail
  if( deviceEvent->event != mWindow )
  {
    return false;
  }
  // emulated flags means that the event has been emulated from another XI 2.x event for legacy client support
  // We don't call XISelectEvents on these events so hopefully shouldn't get them.
  if( ( deviceEvent->flags & XIPointerEmulated ) || ( deviceEvent->flags & XITouchEmulatingPointer ) )
  {
    return false;
  }

  if( !FilteredDevice( deviceEvent->deviceid ))
  {
    return false;
  }
  return true;
}

void XInput2::CreateKeyEvent( const XIDeviceEvent* deviceEvent, KeyEvent& keyEvent ) const
{
  // get the physical key code ( range 8..255)
  KeyCode keycode = deviceEvent->detail;

  keyEvent.keyCode = keycode;
  keyEvent.state = KeyEvent::Down;
  keyEvent.keyModifier = deviceEvent->mods.effective;

  // extract key symbol. The symbol is typically the name visible on the key
  // e.g. key code 201 might = Brightness increase, or a Korean character depending on the keyboard mapping.
  // @todo For XKbKeycodeToKeysym to work correctly we need the group and level.
  // investigate using XkbGetState to get initial state then start monitoring for XkbStateNotify events
  KeySym sym = XkbKeycodeToKeysym( mDisplay, keycode, 0 /* group */ , keyEvent.IsShiftModifier() );
  char* keyname = XKeysymToString( sym );

  keyEvent.keyPressedName = keyname;
  keyEvent.time = deviceEvent->time;

}

// prototyping implementation
void XInput2::ProcessKeyEvent( XKeyEvent* xEvent )
{
  KeyEvent keyEvent;
  keyEvent.keyCode = xEvent->keycode;
  keyEvent.state = KeyEvent::Down;

  KeySym sym = XkbKeycodeToKeysym( mDisplay, xEvent->keycode, 0 /* group */ , keyEvent.IsShiftModifier() );
  char* keyname = XKeysymToString( sym );
  keyEvent.keyPressedName = keyname;
  keyEvent.time = xEvent->time;

  Integration::KeyEvent convertedEvent( keyEvent );

  mEventInterface->KeyEvent( convertedEvent );
}
void XInput2::ProcessClientMessage( XEvent* event )
{
  // Format for client message for key event
  // XEvent xev;
  // xev.xclient.type = ClientMessage;
  // xev.xclient.display = keyrouter.disp;
  // xev.xclient.window = window;
  // xev.xclient.format = 32;
  // xev.xclient.message_type = ecore_x_atom_get("VDINPUT_KEYEVENT");
  // xev.xclient.data.l[0] = ev->time; /* time */
  // xev.xclient.data.l[1] = ev->state; /* modifier */
  // xev.xclient.data.l[2] = ev->code; /* keycode */
  // xev.xclient.data.l[3] = ev->value; /* press/release */
  // xev.xclient.data.l[4] = ev->device_id; /* deviceId */

  Atom input_atom = XInternAtom( mDisplay, "VDINPUT_KEYEVENT", false);
  KeyEvent keyEvent;

  keyEvent.state = KeyEvent::Down;

  // if atom is "VDINPUT_KEYEVENT"
  if( input_atom == event->xclient.message_type )
  {

    keyEvent.keyModifier = event->xclient.data.l[1];
    keyEvent.keyCode = event->xclient.data.l[2];

    // only transmit keydown events
    if( event->xclient.data.l[3] != 2)
    {
      return; // 2 = key down, 3 = key release
    }

    KeySym sym = XkbKeycodeToKeysym( mDisplay,  keyEvent.keyCode, 0 /* group */ , keyEvent.IsShiftModifier() );
    char* keyname = XKeysymToString( sym );
    keyEvent.keyPressedName = keyname;
    keyEvent.time = event->xclient.data.l[0];

    Integration::KeyEvent convertedEvent( keyEvent );

    mEventInterface->KeyEvent( convertedEvent );
  }
}

void XInput2::ProcessGenericEvent( XGenericEventCookie* cookie )
{
  XIDeviceEvent* deviceEvent = static_cast< XIDeviceEvent* >(cookie->data);

  X11Debug::LogXI2Event( cookie );

  bool requiresProcessing  = PreProcessEvent( deviceEvent );

  if( ! requiresProcessing )
  {
    return;
  }

  Integration::Point point;
  point.SetDeviceId( deviceEvent->deviceid );
  point.SetScreenPosition( Vector2( deviceEvent->event_x, deviceEvent->event_y ) );
  Time time( deviceEvent->time ); // X is using uint32 for time field ( see XI2proto.h )

  switch( cookie->evtype)
  {
    case XI_TouchUpdate:
    case XI_Motion:
    {
      point.SetState( PointState::MOTION );
      mEventInterface->TouchEvent( point, time );
      break;
    }
    case XI_TouchBegin:
    case XI_ButtonPress:
    {
      point.SetState( PointState::DOWN );
      mEventInterface->TouchEvent( point, time );
      break;
    }
    case XI_TouchEnd:
    case XI_ButtonRelease:
    {
      point.SetState( PointState::UP );
      mEventInterface->TouchEvent( point, time );
      break;
    }
    case XI_FocusIn:
    {
      mEventInterface->WindowFocusIn();
      break;
    }
    case XI_FocusOut:
    {
      mEventInterface->WindowFocusOut();
      break;
    }
    case XI_KeyPress:
    {
      KeyEvent keyEvent;
      CreateKeyEvent( deviceEvent, keyEvent );
      Integration::KeyEvent convertedEvent( keyEvent );
      mEventInterface->KeyEvent( convertedEvent );
      break;
    }
    default:
    {
      break;
    }
  }
}

void XInput2::QueryMultiTouchSupport()
{
  int minor = XI2MinorVersionRequired;
  int major = XI2MajorVersionRequired;
  int firstEventCode, firstErrorCode;

  // Check if the extension is available and get the extension id
  if( !XQueryExtension(mDisplay, "XInputExtension",  &mXI2ExtensionId, &firstEventCode, &firstErrorCode) )
  {
    DALI_LOG_ERROR(" XInputExtension not available \n");
    return;
  }

  // inform X that DALi ( the client ) supports XI2 version 2.2
  // it will assign the X servers supported version to the parameters
  int ret = XIQueryVersion( mDisplay, &major, &minor);
  if( ret == BadValue )
  {
    DALI_LOG_ERROR(" XIQueryVersion %d,%d failed \n", major, minor );
    return;
  }

  // check the version is supports multi-touch
  if( ( major * 1000 + minor ) >= ( XI2MajorVersionRequired * 1000 + XI2MinorVersionRequired ) )
  {
      mMultiTouchSupport = true;
  }
  else
  {
    DALI_LOG_ERROR( "XInput 2.2 or greater required for multi-touch\n");
  }

}
void XInput2::QueryDevices()
 {
   int numberOfDevices( 0 );

   // QueryDevice returns information about one or more input devices
   XIDeviceInfo* deviceInfoArray = XIQueryDevice( mDisplay, XIAllDevices, &numberOfDevices);
   XIDeviceInfo* device = deviceInfoArray;

   X11Debug::LogInputDeviceInfo( deviceInfoArray, numberOfDevices );

   mInputDeviceInfo.Resize( numberOfDevices );

   for( int i = 0; i < numberOfDevices; ++i, ++device )
   {
     XInput2Device info;

     info.AssignDeviceInfo( device );

     mInputDeviceInfo.PushBack( info );
   }

   XIFreeDeviceInfo( deviceInfoArray );
 }

void XInput2::SelectEvents( int deviceId, const Dali::Vector< unsigned int >& filter )
{
  if( filter.Size() ==  0)
  {
    return;
  }

  // each event like XI_ButtonPress is stored as unique bit, so if there's 32 events we need 4 bytes
  // the XIMaskLen macro provides the length for us at compile time.
  unsigned char mask[ XIMaskLen( XI_LASTEVENT ) ] = {};
  XIEventMask eventMask;

  eventMask.deviceid = deviceId;
  eventMask.mask_len = sizeof( mask);
  eventMask.mask = mask;

  for( VectorBase::SizeType i = 0; i< filter.Count(); ++i )
  {
      XISetMask( mask, filter[i] );
  }

  XISelectEvents( mDisplay, mWindow, &eventMask, 1);

}
void XInput2::SelectInputEvents()
{
  /*
   * From the X documentation:
   * "A master pointer is a virtual pointer device that does not represent a physical device.
   * If a slave device generates an event, the event is also generated by the respective master device.
   * Multiple slave devices can be attached to a single master device."
   * master = cursor / keyboard focus,
   * slave = physical device
   *
   * For motion events, we currently just listen to the slave devices. This allows us the ability to
   * perform a XIGrabDevice on the slave if we need to, which will temporarily detach it from the master.
   * In DALi we currently don't perform a grab as typically we just have a single x-window displayed.
   * Where as other toolkits may have a window for a popup and want do know when the mouse is clicked outside
   * of the popup, to close it.
   */
  Dali::Vector< unsigned int > eventFilter;
  eventFilter.Reserve( 6 );    // typically filter no more than 6 events

  for( VectorBase::SizeType i = 0; i < mInputDeviceInfo.Count(); ++i )
  {
    const XInput2Device& device( mInputDeviceInfo[ i ] );

    eventFilter.Clear();

    // Floating slave devices also can generate key events. For example, TV remote controllers.
    if( ( device.use == XIFloatingSlave ) || ( device.use == XISlavePointer ) || ( device.use == XISlaveKeyboard ) )
    {
      if( device.buttonClass )
      {
        eventFilter.PushBack( XI_ButtonPress );
        eventFilter.PushBack( XI_ButtonRelease );
        eventFilter.PushBack( XI_Motion );
      }
      if( device.touchClass )
      {
        eventFilter.PushBack( XI_TouchUpdate );
        eventFilter.PushBack( XI_TouchBegin );
        eventFilter.PushBack( XI_TouchEnd );
      }
      if( device.keyClass )
      {
        eventFilter.PushBack( XI_KeyPress );
        eventFilter.PushBack( XI_KeyRelease );
      }
    }

    if( eventFilter.Count() > 0 )
    {
      SelectEvents( device.deviceId, eventFilter );
    }
  }
}
} // namespace internal
} // namespace adaptor
} // namespace dali
