#ifndef COMMON_HPP
#define COMMON_HPP

#include "DBus.hpp"
#include "dbusLocators.hpp"
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/integration-api/debug.h>
#include <iomanip>
#include <sstream>
#include <string>

#define A11Y_DBUS_NAME "org.a11y.Bus"
#define A11Y_DBUS_PATH "/org/a11y/bus"
#define A11Y_DBUS_STATUS_INTERFACE "org.a11y.Status"
#define ATSPI_DBUS_NAME_REGISTRY "org.a11y.atspi.Registry"
#define ATSPI_DBUS_PATH_ROOT "/org/a11y/atspi/accessible/root"
#define ATSPI_DBUS_INTERFACE_SOCKET "org.a11y.atspi.Socket"
#define ATSPI_PATH "/org/a11y/atspi/accessible"
#define ATSPI_DBUS_INTERFACE_ACCESSIBLE "org.a11y.atspi.Accessible"
#define ATSPI_DBUS_INTERFACE_ACTION "org.a11y.atspi.Action"
#define ATSPI_DBUS_INTERFACE_APPLICATION "org.a11y.atspi.Application"
#define ATSPI_DBUS_INTERFACE_COLLECTION "org.a11y.atspi.Collection"
#define ATSPI_DBUS_INTERFACE_COMPONENT "org.a11y.atspi.Component"
#define ATSPI_DBUS_INTERFACE_DOCUMENT "org.a11y.atspi.Document"
#define ATSPI_DBUS_INTERFACE_EDITABLE_TEXT "org.a11y.atspi.EditableText"
#define ATSPI_DBUS_INTERFACE_EVENT_KEYBOARD "org.a11y.atspi.Event.Keyboard"
#define ATSPI_DBUS_INTERFACE_EVENT_MOUSE "org.a11y.atspi.Event.Mouse"
#define ATSPI_DBUS_INTERFACE_EVENT_OBJECT "org.a11y.atspi.Event.Object"
#define ATSPI_DBUS_INTERFACE_HYPERLINK "org.a11y.atspi.Hyperlink"
#define ATSPI_DBUS_INTERFACE_HYPERTEXT "org.a11y.atspi.Hypertext"
#define ATSPI_DBUS_INTERFACE_IMAGE "org.a11y.atspi.Image"
#define ATSPI_DBUS_INTERFACE_SELECTION "org.a11y.atspi.Selection"
#define ATSPI_DBUS_INTERFACE_TABLE "org.a11y.atspi.Table"
#define ATSPI_DBUS_INTERFACE_TABLE_CELL "org.a11y.atspi.TableCell"
#define ATSPI_DBUS_INTERFACE_TEXT "org.a11y.atspi.Text"
#define ATSPI_DBUS_INTERFACE_VALUE "org.a11y.atspi.Value"
#define ATSPI_DBUS_INTERFACE_SOCKET "org.a11y.atspi.Socket"
#define ATSPI_DBUS_INTERFACE_EVENT_WINDOW "org.a11y.atspi.Event.Window"

#define ATSPI_DBUS_PATH_DEC "/org/a11y/atspi/registry/deviceeventcontroller"
#define ATSPI_DBUS_INTERFACE_DEC "org.a11y.atspi.DeviceEventController"
#define ATSPI_DBUS_INTERFACE_DEVICE_EVENT_LISTENER "org.a11y.atspi.DeviceEventListener"

namespace DBus
{
class CurrentBridgePtr
{
  static Dali::Accessibility::Bridge*& get()
  {
    static thread_local Dali::Accessibility::Bridge* b = nullptr;
    return b;
  }
  Dali::Accessibility::Bridge* prev;
  CurrentBridgePtr( const CurrentBridgePtr& ) = delete;
  CurrentBridgePtr( CurrentBridgePtr&& ) = delete;
  CurrentBridgePtr& operator=( const CurrentBridgePtr& ) = delete;
  CurrentBridgePtr& operator=( CurrentBridgePtr&& ) = delete;

public:
  CurrentBridgePtr( Dali::Accessibility::Bridge* b ) : prev( get() ) { get() = b; }
  ~CurrentBridgePtr() { get() = prev; }

  static Dali::Accessibility::Bridge* current() { return get(); }
};
namespace detail
{
template < typename T >
struct signature_accessible_impl
{
  using subtype = std::pair< std::string, DBus::ObjectPath >;

  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "AtspiAccessiblePtr";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "(so)";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, T* t )
  {
    const auto prefixPath = "/org/a11y/atspi/accessible/";
    const auto nullPath = "/org/a11y/atspi/null";

    if( t )
    {
      auto v = t->GetAddress();
      signature< subtype >::set( iter, {v.GetBus(), DBus::ObjectPath{std::string{prefixPath} + v.GetPath()}} );
    }
    else
    {
      signature< subtype >::set( iter, {"", DBus::ObjectPath{nullPath}} );
    }
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, T*& v )
  {
    const auto prefixPath = "/org/a11y/atspi/accessible/";
    const auto nullPath = "/org/a11y/atspi/null";
    subtype tmp;
    if( !signature< subtype >::get( iter, tmp ) )
      return false;
    if( tmp.second.value == nullPath )
    {
      v = nullptr;
      return true;
    }
    if( tmp.second.value.substr( 0, strlen( prefixPath ) ) != prefixPath )
      return false;
    auto b = CurrentBridgePtr::current();
    if( b->GetBusName() != tmp.first )
      return false;
    v = b->FindByPath( tmp.second.value.substr( strlen( prefixPath ) ) );
    return v != nullptr;
  }
};
template <>
struct signature< Dali::Accessibility::Accessible* > : public signature_accessible_impl< Dali::Accessibility::Accessible >
{
};

template <>
struct signature< Dali::Accessibility::Address >
{
  using subtype = std::pair< std::string, DBus::ObjectPath >;

  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "AtspiAccessiblePtr";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "(so)";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const Dali::Accessibility::Address& v )
  {
    const auto prefixPath = "/org/a11y/atspi/accessible/";
    const auto nullPath = "/org/a11y/atspi/null";

    if( v )
    {
      signature< subtype >::set( iter, {v.GetBus(), DBus::ObjectPath{std::string{prefixPath} + v.GetPath()}} );
    }
    else
    {
      signature< subtype >::set( iter, {v.GetBus(), DBus::ObjectPath{nullPath}} );
    }
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, Dali::Accessibility::Address& v )
  {
    const auto prefixPath = "/org/a11y/atspi/accessible/";
    const auto nullPath = "/org/a11y/atspi/null";
    subtype tmp;
    if( !signature< subtype >::get( iter, tmp ) )
      return false;
    if( tmp.second.value == nullPath )
    {
      v = {};
      return true;
    }
    if( tmp.second.value.substr( 0, strlen( prefixPath ) ) != prefixPath )
      return false;
    v = {std::move( tmp.first ), tmp.second.value.substr( strlen( prefixPath ) )};
    return true;
  }
};
template <>
struct signature< Dali::Accessibility::States >
{
  using subtype = std::array< uint32_t, 2 >;

  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return signature< subtype >::name();
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return signature< subtype >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const Dali::Accessibility::States& v )
  {
    signature< subtype >::set( iter, v.GetRawData() );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, Dali::Accessibility::States& v )
  {
    subtype tmp;
    if( !signature< subtype >::get( iter, tmp ) )
      return false;
    v = Dali::Accessibility::States{tmp};
    return true;
  }
};
}
}

struct _Logger
{
  const char* file;
  int line;
  std::ostringstream tmp;

  _Logger( const char* f, int l ) : file( f ), line( l ) {}
  ~_Logger()
  {
    Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s:%d: %s", file, line, tmp.str().c_str() );
  }
  template < typename T >
  _Logger& operator<<( T&& t )
  {
    tmp << std::forward< T >( t );
    return *this;
  }
};

struct _LoggerScope
{
  const char* file;
  int line;

  _LoggerScope( const char* f, int l ) : file( f ), line( l )
  {
    Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s:%d: +", file, line );
  }
  ~_LoggerScope()
  {
    Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s:%d: -", file, line );
  }
};
#define LOG() _Logger( __FILE__, __LINE__ )
#define SCOPE() _LoggerScope _l##__LINE__( __FILE__, __LINE__ )

#endif
