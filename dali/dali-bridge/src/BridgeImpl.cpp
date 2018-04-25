#include "BridgeImpl.hpp"
#include "BridgeAccessible.hpp"
#include "BridgeAction.hpp"
#include "BridgeCollection.hpp"
#include "BridgeComponent.hpp"
#include "BridgeObject.hpp"
#include "BridgeValue.hpp"
#include "DBus.hpp"
#include <iostream>

#include <dali/integration-api/debug.h>

using namespace Dali::Accessibility;

class BridgeImpl : public virtual BridgeBase, public BridgeAccessible, public BridgeObject, public BridgeComponent, public BridgeCollection, public BridgeAction, public BridgeValue
{
  DBus::DBusClient listenOnAtspiEnabledSignalClient;
  DBus::DBusClient registryClient;
  Accessible* currentWindow = nullptr;
  bool screenReaderEnabled = false, isEnabled = false;

public:
  BridgeImpl()
  {
    DBus::setDebugPrinter( []( const char* buf, size_t len ) {
      std::string s{buf, len};
      Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s", s.c_str() );
    } );
  }

  Consumed Emit( KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText ) override
  {
    unsigned int evType = 0;

    switch( type )
    {
      case KeyEventType::KeyPressed:
        evType = 0;
        {
          break;
        }
      case KeyEventType::KeyReleased:
        evType = 1;
        {
          break;
        }
      default:
      {
        return Consumed::No;
      }
    }
    auto m = registryClient.method< bool( std::tuple< uint32_t, int32_t, int32_t, int32_t, int32_t, std::string, bool > ) >( "NotifyListenersSync" );
    auto result = m.call( std::tuple< uint32_t, int32_t, int32_t, int32_t, int32_t, std::string, bool >{evType, 0, static_cast< int32_t >( keyCode ), 0, static_cast< int32_t >( timeStamp ), keyName, isText ? 1 : 0} );
    if( !result )
    {
      LOG() << result.getError().message;
      return Consumed::No;
    }
    return std::get< 0 >( result ) ? Consumed::Yes : Consumed::No;
  }

  void ForceDown() override
  {
    ApplicationHidden();
    BridgeAccessible::ForceDown();
    registryClient = {};
  }

  ForceUpResult ForceUp() override
  {
    if( BridgeAccessible::ForceUp() == ForceUpResult::alreadyUp )
      return ForceUpResult::alreadyUp;

    BridgeObject::RegisterInterfaces();
    BridgeAccessible::RegisterInterfaces();
    BridgeComponent::RegisterInterfaces();
    BridgeCollection::RegisterInterfaces();
    BridgeAction::RegisterInterfaces();
    BridgeValue::RegisterInterfaces();

    RegisterOnBridge( &application );

    registryClient = {ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_DEC, ATSPI_DBUS_INTERFACE_DEC, con};
    auto proxy = DBus::DBusClient{ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_ROOT, ATSPI_DBUS_INTERFACE_SOCKET, con};
    Address root{
        data->busName, "root"};
    auto res = proxy.method< Address( Address ) >( "Embed" ).call( root );
    assert( res );
    application.parent.SetAddress( std::move( std::get< 0 >( res ) ) );
    ApplicationShown();
    return ForceUpResult::justStarted;
  }

  void ApplicationHidden() override
  {
    if( currentWindow )
    {
      currentWindow->Emit( WindowEvent::Deactivate, 0 );
      currentWindow = nullptr;
    }
  }
  void ApplicationShown() override
  {
    auto win = application.getActiveWindow();
    if( win && win != currentWindow )
    {
      currentWindow = win;
      win->Emit( WindowEvent::Activate, 0 );
    }
  }
  void Initialize() override
  {
    auto req = DBus::DBusClient{A11Y_DBUS_NAME, A11Y_DBUS_PATH, A11Y_DBUS_STATUS_INTERFACE, DBus::ConnectionType::SESSION};
    auto p = req.property< bool >( "ScreenReaderEnabled" ).get();
    if( p )
      screenReaderEnabled = std::get< 0 >( p );
    p = req.property< bool >( "IsEnabled" ).get();
    if( p )
      isEnabled = std::get< 0 >( p );
    if( screenReaderEnabled || isEnabled )
      ForceUp();
  }
  static std::shared_ptr< Bridge > Create()
  {
    auto ptr = std::make_shared< BridgeImpl >();
    ptr->MakePublic( Bridge::Visibility::allThreads );
    ptr->listenOnAtspiEnabledSignalClient = DBus::DBusClient{A11Y_DBUS_NAME, A11Y_DBUS_PATH, A11Y_DBUS_STATUS_INTERFACE,
                                                             DBus::ConnectionType::SESSION};
    {
      auto p = ptr.get();
      ptr->listenOnAtspiEnabledSignalClient.addPropertyChangedEvent< bool >( "ScreenReaderEnabled", [p]( bool res ) {
        p->screenReaderEnabled = res;
        if( p->screenReaderEnabled || p->isEnabled )
          p->ForceUp();
        else
          p->ForceDown();
      } );
      ptr->listenOnAtspiEnabledSignalClient.addPropertyChangedEvent< bool >( "IsEnabled", [p]( bool res ) {
        p->isEnabled = res;
        if( p->screenReaderEnabled || p->isEnabled )
          p->ForceUp();
        else
          p->ForceDown();
      } );
    }
    return ptr;
  }
};

std::shared_ptr< Bridge > Dali::Accessibility::CreateBridge()
{
  try
  {
    return BridgeImpl::Create();
  }
  catch( AccessibleError& e )
  {
    return {};
  }
}
