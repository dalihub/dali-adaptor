#ifndef DALI_INTERNAL_ATSPI_ACCESSIBILITY_COMMON_H
#define DALI_INTERNAL_ATSPI_ACCESSIBILITY_COMMON_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <iomanip>
#include <sstream>
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/adaptor-framework/accessibility-impl.h>
#include <dali/internal/accessibility/bridge/dbus.h>
#include <dali/internal/accessibility/bridge/dbus-locators.h>

#define A11yDbusName "org.a11y.Bus"
#define A11yDbusPath "/org/a11y/bus"
#define A11yDbusStatusInterface "org.a11y.Status"
#define AtspiDbusNameRegistry "org.a11y.atspi.Registry"
#define AtspiDbusPathRoot "/org/a11y/atspi/accessible/root"
#define AtspiDbusInterfaceSocket "org.a11y.atspi.Socket"
#define AtspiPath "/org/a11y/atspi/accessible"
#define AtspiDbusInterfaceAccessible "org.a11y.atspi.Accessible"
#define AtspiDbusInterfaceAction "org.a11y.atspi.Action"
#define AtspiDbusInterfaceApplication "org.a11y.atspi.Application"
#define AtspiDbusInterfaceCollection "org.a11y.atspi.Collection"
#define AtspiDbusInterfaceComponent "org.a11y.atspi.Component"
#define AtspiDbusInterfaceDocument "org.a11y.atspi.Document"
#define AtspiDbusInterfaceEditableText "org.a11y.atspi.EditableText"
#define AtspiDbusInterfaceEventKeyboard "org.a11y.atspi.Event.Keyboard"
#define AtspiDbusInterfaceEventMouse "org.a11y.atspi.Event.Mouse"
#define AtspiDbusInterfaceEventObject "org.a11y.atspi.Event.Object"
#define AtspiDbusInterfaceHyperlink "org.a11y.atspi.Hyperlink"
#define AtspiDbusInterfaceHypertext "org.a11y.atspi.Hypertext"
#define AtspiDbusInterfaceImage "org.a11y.atspi.Image"
#define AtspiDbusInterfaceSelection "org.a11y.atspi.Selection"
#define AtspiDbusInterfaceTable "org.a11y.atspi.Table"
#define AtspiDbusInterfaceTableCell "org.a11y.atspi.TableCell"
#define AtspiDbusInterfaceText "org.a11y.atspi.Text"
#define AtspiDbusInterfaceValue "org.a11y.atspi.Value"
#define AtspiDbusInterfaceSocket "org.a11y.atspi.Socket"
#define AtspiDbusInterfaceEventWindow "org.a11y.atspi.Event.Window"

#define AtspiDbusPathDec "/org/a11y/atspi/registry/deviceeventcontroller"
#define AtspiDbusInterfaceDec "org.a11y.atspi.DeviceEventController"
#define AtspiDbusInterfaceDeviceEventListener "org.a11y.atspi.DeviceEventListener"

#define DirectReadingDBusName "org.tizen.ScreenReader"
#define DirectReadingDBusPath "/org/tizen/DirectReading"
#define DirectReadingDBusInterface "org.tizen.DirectReading"

struct ObjectPath;

/**
 * @brief Enumeration used for quering Accessibility objects
 */
enum class MatchType : int32_t
{
  INVALID,
  ALL,
  ANY,
  NONE,
  EMPTY
};

/**
 * @brief Enumeration used for quering Accessibility objects
 * SortOrder::Canonical uses breadth-first search and sort objects in order of indexes in parent
 * SortOrder::ReverseCanonical uses SortOrder::Canonical and reverse collection
 * The rest of orders is not supported.
 */
enum class SortOrder : uint32_t
{
  INVALID,
  CANONICAL,
  FLOW,
  TAB,
  REVERSE_CANONICAL,
  REVERSE_FLOW,
  REVERSE_TAB,
  LAST_DEFINED
};

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
  CurrentBridgePtr( Dali::Accessibility::Bridge* b )
  : prev( get() )
  {
    get() = b;
  }

  ~CurrentBridgePtr()
  {
    get() = prev;
  }

  static Dali::Accessibility::Bridge* current()
  {
    return get();
  }
};

namespace detail
{

template < typename T >
struct signature_accessible_impl : signature_helper<signature_accessible_impl<T>>
{
  using subtype = std::pair< std::string, ObjectPath >;

  static constexpr auto name_v = concat("AtspiAccessiblePtr");
  static constexpr auto sig_v = concat("(so)");

  /**
   * @brief Marshals value v as marshalled type into message
   */
  static void set( const DBusWrapper::MessageIterPtr& iter, T* t )
  {
    if( t )
    {
      auto v = t->GetAddress();
      signature< subtype >::set( iter, { v.GetBus(), ObjectPath{std::string{ ATSPI_PREFIX_PATH } +v.GetPath()} } );
    }
    else
    {
      signature< subtype >::set( iter, { "", ObjectPath{ ATSPI_NULL_PATH } } );
    }
  }

  /**
   * @brief Marshals value from marshalled type into variable v
   */
  static bool get( const DBusWrapper::MessageIterPtr& iter, T*& v )
  {
    subtype tmp;
    if( !signature< subtype >::get( iter, tmp ) )
    {
      return false;
    }

    if( tmp.second.value == ATSPI_NULL_PATH )
    {
      v = nullptr;
      return true;
    }

    if( tmp.second.value.substr( 0, strlen( ATSPI_PREFIX_PATH ) ) != ATSPI_PREFIX_PATH )
    {
      return false;
    }

    auto b = CurrentBridgePtr::current();
    if( b->GetBusName() != tmp.first )
    {
      return false;
    }

    v = b->FindByPath( tmp.second.value.substr( strlen( ATSPI_PREFIX_PATH ) ) );
    return v != nullptr;
  }
};

template <>
struct signature< Dali::Accessibility::Accessible* > : public signature_accessible_impl< Dali::Accessibility::Accessible >
{
};

template <>
struct signature< Dali::Accessibility::Address > : signature_helper<signature<Dali::Accessibility::Address>>
{
  using subtype = std::pair< std::string, ObjectPath >;

  static constexpr auto name_v = concat("AtspiAccessiblePtr");
  static constexpr auto sig_v = concat("(so)");

  /**
   * @brief Marshals value v as marshalled type into message
   */
  static void set( const DBusWrapper::MessageIterPtr& iter, const Dali::Accessibility::Address& v )
  {
    if( v )
    {
      signature< subtype >::set( iter, { v.GetBus(), ObjectPath{ std::string{ ATSPI_PREFIX_PATH } + v.GetPath() } } );
    }
    else
    {
      signature< subtype >::set( iter, { v.GetBus(), ObjectPath{ ATSPI_NULL_PATH } } );
    }
  }

  /**
   * @brief Marshals value from marshalled type into variable v
   */
  static bool get( const DBusWrapper::MessageIterPtr& iter, Dali::Accessibility::Address& v )
  {
    subtype tmp;
    if( !signature< subtype >::get( iter, tmp ) )
    {
      return false;
    }

    if( tmp.second.value == ATSPI_NULL_PATH )
    {
      v = {};
      return true;
    }
    if( tmp.second.value.substr( 0, strlen( ATSPI_PREFIX_PATH ) ) != ATSPI_PREFIX_PATH )
    {
      return false;
    }

  v = { std::move( tmp.first ), tmp.second.value.substr( strlen( ATSPI_PREFIX_PATH ) ) };
  return true;
  }
};

template <>
struct signature< Dali::Accessibility::States > : signature_helper<signature<Dali::Accessibility::States>>
{
  using subtype = std::array<uint32_t, 2>;

  static constexpr auto name_v = signature<subtype>::name_v;
  static constexpr auto sig_v = signature<subtype>::sig_v;

  /**
   * @brief Marshals value v as marshalled type into message
   */
  static void set( const DBusWrapper::MessageIterPtr& iter, const Dali::Accessibility::States& v )
  {
    signature< subtype >::set( iter, v.GetRawData() );
  }

  /**
   * @brief Marshals value from marshalled type into variable v
   */
  static bool get( const DBusWrapper::MessageIterPtr& iter, Dali::Accessibility::States& v )
  {
    subtype tmp;
    if( !signature< subtype >::get( iter, tmp ) )
    {
      return false;
    }
    v = Dali::Accessibility::States{ tmp };
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

  _Logger( const char* f, int l )
  : file( f ),
    line( l ){}

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

struct _LoggerEmpty
{
  template < typename T >
  _LoggerEmpty& operator<<( T&& t )
  {
    return *this;
  }
};

struct _LoggerScope
{
  const char* file;
  int line;

  _LoggerScope( const char* f, int l )
  : file( f ),
    line( l )
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

#endif // DALI_INTERNAL_ATSPI_ACCESSIBILITY_COMMON_H
