/*
 * Copyright 2017  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DBUS_HPP
#define DBUS_HPP

#include <Eldbus.h>
#include <memory>

#include "Optional.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#define DBUS_DEBUG( ... )                                \
  do                                                     \
  {                                                      \
    DBus::debugPrint( __FILE__, __LINE__, __VA_ARGS__ ); \
  } while( 0 )

/**
 * @brief Template based, single file, wrapper library around eldbus for DBUS based communication.
 *
 * Main motivation was missing asynchronous calls in AT-SPI library and difficulties,
 * when using eldbus from C++.
 *
 * The library:
 * - takes care of marshalling arguments to and from DBUS calls.
 * - allows synchronous and asynchronous calls.
 * - allows synchronous and asynchronous listeners on signals.
 * - manages all involved objects' lifetimes.
 * - errors are passed as optional-alike objects, no exceptions are used.
 * - allows setting additional debug-print function for more details about
 *   what's going on
 *
 * DBUS's method signatures (and expected return values) are specified as template argument,
 * using functor syntax. For example:
 * \code{.cpp}
 * auto dbus = DBusClient{ ... };
 * auto v = dbus.method<std::tuple<int, float>(float, float, std::string)>("foo").call(1.0f, 2.0f, "qwe");
 * \endcode
 * means (synchronous) call on dbus object, which takes three arguments (thus making call signature \b dds)
 * of types float, float and string (float will be automatically converted to double).
 * Expected return value is std::tuple<int, float>, which gives signature <B>(id)</B> - std::tuple acts
 * as struct container. Returned value v will be of type ValueOrError<std::tuple<int, float>>.\n
 * Slightly different (asynchronous) example:
 * \code{.cpp}
 * auto dbus = DBusClient{ ... };
 * std::function<void(ValueOrError<int, float>)> callback;
 * dbus.method<ValueOrError<int, float>(float, float, std::string)>("foo").asyncCall(callback, 1.0f, 2.0f, "qwe");
 * \endcode
 * Now the call takes the same arguments and has the same signature. But expected values are different -
 * now the signature is simply \b id. ValueOrError acts in this case as placeholder for list of values,
 * which DBUS allows as return data. The call itself is asynchronous - instead of expecting reply
 * you need to pass a callback, which will be called either with received data and error message.
 *
 * Library is not thread-safe, the same object shouldn't be called from different threads without
 * synchronization. There's no guarantee, that callbacks will be executed on the same thread.
 */
namespace DBus
{
/// \cond
class DBusServer;
class DBusClient;
class DBusInterfaceDescription;

/**
   * @brief DBus action enumeration
   *
   * @param METHOD_CALL DBus is about to call a method on some external target
   * @param SETTER_CALL DBus is about to call a setter method on some external target
   * @param GETTER_CALL DBus is about to call a getter method on some external target
   * @param SIGNAL_RECEIVED DBus just received a signal
   * @param METHOD_RESPONSE DBus server received a method call
   * @param SETTER_RESPONSE DBus server received a setter call
   * @param GETTER_RESPONSE DBus server received a getter call
   * @param SIGNAL_EMIT DBus server is about to emit a signal
   */
enum class DBusActionType
{
  METHOD_CALL,
  SETTER_CALL,
  GETTER_CALL,
  SIGNAL_RECEIVED,
  METHOD_RESPONSE,
  SETTER_RESPONSE,
  GETTER_RESPONSE,
  SIGNAL_EMIT,
};

/**
   * @brief Structure containing information about DBus activity, when calling notification callback
   *
   * @param type type of the action
   * @param path path of the object, that's involved in action
   * @param interface interface, on which member was acted upon. Note, that in case of getters and setters
   *        this will be real interface, not org.freedesktop.DBus.Properties
   * @param member member name, that was involved (either method / setter / getter / signal)
   */
struct DBusAction
{
  const DBusActionType type;
  const char* const bus = nullptr;
  const char* const path = nullptr;
  const char* const interface = nullptr;
  const char* const member = nullptr;

  DBusAction( const DBusActionType type,
              const char* const bus = nullptr,
              const char* const path = nullptr,
              const char* const interface = nullptr,
              const char* const member = nullptr ) : type( type ), bus( bus ), path( path ), interface( interface ), member( member ) {}
};

/**
   * @brief Formats debug message and calls debug printer (if any) with it
   */
void debugPrint( const char* file, size_t line, const char* format, ... );

/**
   * @brief Sets debug printer callback, which will be called with debug messages
   *
   * Callback will be called in various moments of DBus activity. First value passed to callback
   * is pointer to text, second it's length. Text is ended with 0 (not counted towards it's size),
   * user can safely printf it.
   */
void setDebugPrinter( std::function< void( const char*, size_t ) > );

/**
   * @brief Sets notification callback about processing of DBus call
   *
   * Notification callback can be set independently either on client or server.
   * On client's side callback will be called, when user calls method / getter / setter
   * or when client has received a signal.
   * On server's side callback will be called, when sever has received a request to
   * handle method / getter / setter or when server is going to emit a signal.
   * Callback should returns as fast as possible.
   * User can't call setDBusActionNotifier from inside the callback call -
   * it will cause a deadlock
   */
void setDBusActionNotifier( std::function< void( DBusAction ) > callback );

namespace detail
{
void emitNotification( const char* bus, const char* path, const char* interface, const char* member, DBusActionType type );
}

struct Error
{
  std::string message;

  Error() = default;
  Error( std::string msg ) : message( std::move( msg ) )
  {
    assert( !message.empty() );
  }
};
struct Success
{
};
/// \endcond

/**
   * @brief Value representing data, that came from DBUS or error message
   *
   * Object of this class either helds series of values (of types ARGS...)
   * or error message. This object will be true in boolean context, if has data
   * and false, if an error occured.
   * It's valid to create ValueOrError object with empty argument list or void:
   * \code{.cpp}
   * ValueOrError<> v1;
   * ValueOrError<void> v2;
   * \endcode
   * Both mean the same - ValueOrError containing no real data and being a marker,
   * wherever operation successed or failed and containing possible error message.
   */
template < typename... ARGS >
class ValueOrError
{
public:
  /**
     * @brief Empty constructor. Valid only, if all ARGS types are default constructible.
     */
  ValueOrError() = default;

  /**
     * @brief Value constructor.
     *
     * This will be initialized as success with passed in values.
     */
  ValueOrError( ARGS... t ) : value( std::move( t )... ) {}

  /**
     * @brief Alternative Value constructor.
     *
     * This will be initialized as success with passed in values.
     */
  ValueOrError( std::tuple< ARGS... > t ) : value( std::move( t ) ) {}

  /**
     * @brief Error constructor. This will be initialized as failure with given error message.
     */
  ValueOrError( Error e ) : error( std::move( e ) )
  {
    assert( !error.message.empty() );
  }

  /**
     * @brief bool operator.
     *
     * Returns true, if operation was successful (getValues member is callable), or false
     * when operation failed (getError is callable).
     */
  explicit operator bool() const
  {
    return error.message.empty();
  }

  /**
     * @brief Returns error message object.
     *
     * Returns object containing error message associated with the failed operation.
     * Only callable, if operation actually failed, otherwise will assert.
     */
  const Error& getError() const
  {
    return error;
  }

  /**
     * @brief Returns modifiable tuple of held data.
     *
     * Returns reference to the internal tuple containing held data.
     * User can modify (or move) data safely.
     * Only callable, if operation actually successed, otherwise will assert.
     */
  std::tuple< ARGS... >& getValues()
  {
    assert( *this );
    return value;
  }

  /**
     * @brief Returns const tuple of held data.
     *
     * Returns const reference to the internal tuple containing held data.
     * Only callable, if operation actually successed, otherwise will assert.
     */
  const std::tuple< ARGS... >& getValues() const
  {
    assert( *this );
    return value;
  }

protected:
  /// \cond
  std::tuple< ARGS... > value;
  Error error;
  /// \endcond
};

/// \cond
template <>
class ValueOrError<>
{
public:
  ValueOrError() = default;
  ValueOrError( std::tuple<> t ) {}
  ValueOrError( Error e ) : error( std::move( e ) )
  {
    assert( !error.message.empty() );
  }

  explicit operator bool() const
  {
    return error.message.empty();
  }
  const Error& getError() const
  {
    return error;
  }
  std::tuple<>& getValues()
  {
    assert( *this );
    static std::tuple<> t;
    return t;
  }
  std::tuple<> getValues() const
  {
    assert( *this );
    return {};
  }

protected:
  Error error;
};

template <>
class ValueOrError< void >
{
public:
  ValueOrError() = default;
  ValueOrError( Success ) {}
  ValueOrError( Error e ) : error( std::move( e ) )
  {
    assert( !error.message.empty() );
  }

  explicit operator bool() const
  {
    return error.message.empty();
  }
  const Error& getError() const
  {
    return error;
  }
  std::tuple<>& getValues()
  {
    assert( *this );
    static std::tuple<> t;
    return t;
  }
  std::tuple<> getValues() const
  {
    assert( *this );
    return {};
  }

protected:
  Error error;
};
struct ObjectPath
{
  std::string value;
};
namespace detail
{
class CallOnDestructionList
{
public:
  CallOnDestructionList() = default;
  CallOnDestructionList( const CallOnDestructionList& ) = delete;
  CallOnDestructionList( CallOnDestructionList&& ) = default;

  CallOnDestructionList& operator=( const CallOnDestructionList& ) = delete;
  CallOnDestructionList& operator=( CallOnDestructionList&& );

  void add( const std::function< void() >& c );

private:
  std::vector< std::function< void() > > functions;
};

struct caller_eldbus_connection_unref
{
  void operator()( Eldbus_Connection* p ) const
  {
    eldbus_connection_unref( p );
  }
};

struct caller_eldbus_message_unref
{
  void operator()( Eldbus_Message* p ) const
  {
    eldbus_message_unref( p );
  }
};

struct caller_eldbus_proxy_unref
{
  void operator()( Eldbus_Proxy* p ) const
  {
    eldbus_proxy_unref( p );
  }
};
}
/// \endcond

/**
   * @brief Class used to marshall DBUS's variant type
   *
   * Minimalistic class, that allows user to specify DBUS variant type
   * as argument or return value. You need to pass real type hidden under variant as
   * template type \b A. At this point library doesn't allow to expected one of few classes
   * as return data in variant. So for example user can't specify method call, which on return
   * expects DBUS variant holding either string or int.
   */
template < typename A >
struct EldbusVariant
{
  A value;
};

/// \cond
class EldbusConnection
{
  Eldbus_Connection* ptr = nullptr;

public:
  EldbusConnection( Eldbus_Connection* c ) : ptr( c )
  {
    eldbus_init();
  }
  EldbusConnection() = delete;
  EldbusConnection( const EldbusConnection& ) = delete;
  EldbusConnection( EldbusConnection&& ) = delete;
  ~EldbusConnection()
  {
    eldbus_connection_unref( ptr );
    eldbus_shutdown();
  }

  Eldbus_Connection* get() const
  {
    return ptr;
  }
};
//using EldbusConnectionCallbackHandle = std::shared_ptr<Eldbus_Connection>;
using EldbusMessageCallbackHandle = std::unique_ptr< Eldbus_Message, detail::caller_eldbus_message_unref >;
using EldbusObjectCallbackHandle = std::shared_ptr< Eldbus_Object >;
using EldbusProxyHandle = std::shared_ptr< Eldbus_Proxy >;
/// \endcond

/**
   * @brief Namespace for private, internal functions and classes
   *
   */
namespace detail
{
/// \cond
template < typename T, typename = void >
struct signature;
template < typename... ARGS >
struct signature< std::tuple< ARGS... > >;
template < typename A, typename B >
struct signature< std::pair< A, B > >;
template < typename A >
struct signature< std::vector< A > >;
template < typename A, size_t N >
struct signature< std::array< A, N > >;
template < typename A, typename B >
struct signature< std::unordered_map< A, B > >;
template < typename A, typename B >
struct signature< std::map< A, B > >;

template < typename T >
struct signature< T, typename std::enable_if< std::is_enum< T >::value, void >::type >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "enum";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    // TODO: add check for failure in marshalling arguments
    return "i";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, T v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), static_cast< int >( v ) );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, T& v )
  {
    int q;
    auto z = eldbus_message_iter_get_and_next( iter, sig()[0], &q );
    v = static_cast< T >( q );
    return z;
  }
};
/// \endcond

/**
     * @brief Signature class for marshalling uint8 type.
     */
template <>
struct signature< uint8_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "uint8_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "y";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, uint8_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, uint8_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling uint16 type.
     */
template <>
struct signature< uint16_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "uint16_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "q";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, uint16_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, uint16_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling uint32 type.
     */
template <>
struct signature< uint32_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "uint32_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "u";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, uint32_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, uint32_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling uint64 type.
     */
template <>
struct signature< uint64_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "uint64_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "t";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, uint64_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, uint64_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling int16 type.
     */
template <>
struct signature< int16_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "int16_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "n";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, int16_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, int16_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling int32 type.
     */
template <>
struct signature< int32_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "int32_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "i";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, int32_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, int32_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling int64 type.
     */
template <>
struct signature< int64_t >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "int64_t";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "x";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, int64_t v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, int64_t& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
};

/**
     * @brief Signature class for marshalling double type.
     */
template <>
struct signature< double >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "double";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "d";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, double v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, double& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, float& v2 )
  {
    double v = 0;
    auto r = eldbus_message_iter_get_and_next( iter, sig()[0], &v );
    v2 = static_cast< float >( v );
    return r;
  }
};

/**
     * @brief Signature class for marshalling float type.
     */
template <>
struct signature< float >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "float";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "d";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, float v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, double& v )
  {
    return eldbus_message_iter_get_and_next( iter, sig()[0], &v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, float& v2 )
  {
    double v = 0;
    auto r = eldbus_message_iter_get_and_next( iter, sig()[0], &v );
    v2 = static_cast< float >( v );
    return r;
  }
};

/**
     * @brief Signature class for marshalling boolean type.
     */
template <>
struct signature< bool >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "bool";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "b";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, bool v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v ? 1 : 0 );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, bool& v )
  {
    unsigned char q;
    auto z = eldbus_message_iter_get_and_next( iter, sig()[0], &q );
    v = q != 0;
    return z;
  }
};

/**
     * @brief Signature class for marshalling string type.
     *
     * Both (const) char * and std::string types are accepted as value to send.
     * Only std::string is accepted as value to receive.
     */
template <>
struct signature< std::string >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "string";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "s";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::string& v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v.c_str() );
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const char* v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::string& v )
  {
    const char* q;
    if( !eldbus_message_iter_get_and_next( iter, 's', &q ) )
    {
      if( !eldbus_message_iter_get_and_next( iter, 'o', &q ) )
        return false;
    }
    v = q;
    return true;
  }
};
template <>
struct signature< ObjectPath >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "path";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "o";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::string& v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v.c_str() );
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const ObjectPath& v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v.value.c_str() );
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const char* v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, ObjectPath& v )
  {
    const char* q;
    if( !eldbus_message_iter_get_and_next( iter, 'o', &q ) )
      return false;
    v.value = q;
    return true;
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::string& v )
  {
    const char* q;
    if( !eldbus_message_iter_get_and_next( iter, 'o', &q ) )
      return false;
    v = q;
    return true;
  }
};

/**
     * @brief Signature class for marshalling (const) char * type.
     *
     * Both (const) char * and std::string types are accepted as value to send.
     * You can't use (const) char * variable type to receive value.
     */
template <>
struct signature< char* >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "string";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "s";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::string& v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v.c_str() );
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const char* v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
};

/**
     * @brief Signature class for marshalling (const) char[N] type.
     *
     * Both (const) char[N] and std::string types are accepted as value to send.
     * You can't use (const) char[N] variable type to receive value.
     */
template < size_t N >
struct signature< char[N] >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "string";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "s";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::string& v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v.c_str() );
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const char* v )
  {
    eldbus_message_iter_arguments_append( iter, sig().c_str(), v );
  }
};
/// \cond
template < size_t INDEX, typename A, typename... ARGS >
struct signature_tuple_element_type_helper
{
  using type = typename signature_tuple_element_type_helper< INDEX - 1, ARGS... >::type;
};
template < typename A, typename... ARGS >
struct signature_tuple_element_type_helper< 0, A, ARGS... >
{
  using type = A;
};
/// \endcond

/**
     * @brief Helper class to marshall tuples
     *
     * This class marshals all elements of the tuple value starting at the index INDEX
     * and incrementing. This class recursively calls itself with increasing INDEX value
     * until INDEX is equal to SIZE, where recursive calling ends.
     */
template < size_t INDEX, size_t SIZE, typename... ARGS >
struct signature_tuple_helper
{
  using current_type = typename signature_tuple_element_type_helper< INDEX, ARGS... >::type;

  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    if( INDEX + 1 >= SIZE )
      return signature< current_type >::name();
    return signature< current_type >::name() + ", " + signature_tuple_helper< INDEX + 1, SIZE, ARGS... >::name();
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return signature< current_type >::sig() + signature_tuple_helper< INDEX + 1, SIZE, ARGS... >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::tuple< ARGS... >& args )
  {
    signature< current_type >::set( iter, std::get< INDEX >( args ) );
    signature_tuple_helper< INDEX + 1, SIZE, ARGS... >::set( iter, args );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::tuple< ARGS... >& args )
  {
    return signature< current_type >::get( iter, std::get< INDEX >( args ) ) &&
           signature_tuple_helper< INDEX + 1, SIZE, ARGS... >::get( iter, args );
  }
};

/**
     * @brief Helper class to marshall tuples
     *
     * This class marks end of the tuple marshalling. Members of this class are called
     * when INDEX value is equal to SIZE.
     */
template < size_t SIZE, typename... ARGS >
struct signature_tuple_helper< SIZE, SIZE, ARGS... >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::tuple< ARGS... >& args )
  {
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::tuple< ARGS... >& args )
  {
    return true;
  }
};

/**
     * @brief Signature class for marshalling tuple of values
     *
     * This class marshalls tuple of values. This represents
     * DBUS struct typle, encoded with character 'r'
     */
template < typename... ARGS >
struct signature< std::tuple< ARGS... > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "tuple<" + signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "(" + signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::sig() + ")";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::tuple< ARGS... >& args )
  {
    auto entry = eldbus_message_iter_container_new( iter, 'r', NULL );
    signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::set( entry, args );
    eldbus_message_iter_container_close( iter, entry );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::tuple< ARGS... >& args )
  {
    Eldbus_Message_Iter* entry;
    if( !eldbus_message_iter_get_and_next( iter, 'r', &entry ) )
      return false;
    auto z = signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::get( entry, args );
    return z;
  }
};
/**
     * @brief Signature class for marshalling ValueOrError template type
     *
     * ValueOrError template type is used to marshall list of values passed to
     * DBUS (or received from) at the "top" level. For example ss(s) is represented as
     * \code{.cpp} ValueOrError<std::string, std::string, std::tuple<std::string>> \endcode
     * While (ss(s)) is represented as
     * \code{.cpp} std::tuple<std::string, std::string, std::tuple<std::string>> \endcode
     * or
     * \code{.cpp} ValueOrError<std::tuple<std::string, std::string, std::tuple<std::string>>> \endcode
     */
template < typename... ARGS >
struct signature< ValueOrError< ARGS... > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "ValueOrError<" + signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const ValueOrError< ARGS... >& args )
  {
    signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::set( iter, args.getValues() );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, ValueOrError< ARGS... >& args )
  {
    return signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::get( iter, args.getValues() );
  }
};
/**
     * @brief Signature class for marshalling ValueOrError<void> type
     */
template <>
struct signature< ValueOrError< void > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "ValueOrError<void>";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const ValueOrError< void >& args )
  {
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, ValueOrError< void >& args )
  {
    return true;
  }
};
/**
     * @brief Signature class for marshalling ValueOrError<> type
     */
template <>
struct signature< ValueOrError<> >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "ValueOrError<>";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const ValueOrError<>& args )
  {
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, ValueOrError<>& args )
  {
    return true;
  }
};
/**
     * @brief Signature class for marshalling pair of types
     */
template < typename A, typename B >
struct signature< std::pair< A, B > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "pair<" + signature_tuple_helper< 0, 2, A, B >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "(" + signature_tuple_helper< 0, 2, A, B >::sig() + ")";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::pair< A, B >& ab, bool dictionary = false )
  {
    auto entry = eldbus_message_iter_container_new( iter, dictionary ? 'e' : 'r', NULL );
    signature_tuple_helper< 0, 2, A, B >::set( entry, ab );
    eldbus_message_iter_container_close( iter, entry );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::pair< A, B >& ab )
  {
    char sg = 'r';
    char* t = eldbus_message_iter_signature_get( iter );
    if( t && t[0] == '{' )
      sg = '{';
    free( t );

    Eldbus_Message_Iter* entry;
    if( !eldbus_message_iter_get_and_next( iter, sg, &entry ) )
      return false;
    std::tuple< A, B > ab_tmp;
    auto z = signature_tuple_helper< 0, 2, A, B >::get( entry, ab_tmp );
    if( z )
    {
      ab.first = std::move( std::get< 0 >( ab_tmp ) );
      ab.second = std::move( std::get< 1 >( ab_tmp ) );
    }
    return z;
  }
};
/**
     * @brief Signature class for marshalling std::vector template type
     *
     * This marshals container's content as DBUS a ascii character type code.
     */
template < typename A >
struct signature< std::vector< A > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "vector<" + signature< A >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "a" + signature< A >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::vector< A >& v )
  {
    auto lst = eldbus_message_iter_container_new( iter, 'a', signature< A >::sig().c_str() );
    assert( lst );
    for( auto& a : v )
    {
      signature< A >::set( lst, a );
    }
    eldbus_message_iter_container_close( iter, lst );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::vector< A >& v )
  {
    Eldbus_Message_Iter* s;
    v.clear();
    if( !eldbus_message_iter_get_and_next( iter, 'a', &s ) )
      return false;
    A a;
    while( signature< A >::get( s, a ) )
      v.push_back( std::move( a ) );

    return true;
  }
};

/**
     * @brief Signature class for marshalling std::array template type
     *
     * This marshals container's content as DBUS a ascii character type code.
     */
template < typename A, size_t N >
struct signature< std::array< A, N > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "array<" + signature< A >::name() + ", " + std::to_string( N ) + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "a" + signature< A >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::array< A, N >& v )
  {
    auto lst = eldbus_message_iter_container_new( iter, 'a', signature< A >::sig().c_str() );
    assert( lst );
    for( auto& a : v )
    {
      signature< A >::set( lst, a );
    }
    eldbus_message_iter_container_close( iter, lst );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::array< A, N >& v )
  {
    Eldbus_Message_Iter* s;
    if( !eldbus_message_iter_get_and_next( iter, 'a', &s ) )
      return false;
    for( auto& a : v )
    {
      if( !signature< A >::get( s, a ) )
        return false;
    }
    return true;
  }
};

/**
     * @brief Signature class for marshalling EldbusVariant type
     *
     * This marshals variant's content as DBUS v ascii character type code.
     */
template < typename A >
struct signature< EldbusVariant< A > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "variant<" + signature< A >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "v";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const EldbusVariant< A >& v )
  {
    set( iter, v.value );
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const A& v )
  {
    auto var = eldbus_message_iter_container_new( iter, 'v', signature< A >::sig().c_str() );
    signature< A >::set( var, v );
    eldbus_message_iter_container_close( iter, var );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, EldbusVariant< A >& v )
  {
    Eldbus_Message_Iter* s;
    if( !eldbus_message_iter_get_and_next( iter, 'v', &s ) )
      return false;
    return signature< A >::get( s, v.value );
  }
};
/**
     * @brief Signature class for marshalling std::unordered_map template type
     *
     * This marshals container's content as DBUS {} ascii character type code.
     * Note, that library doesnt check, if the key is basic type, as DBUS
     * specification mandates.
     * User can always exchange std::unordered_map for std::map and the reverse.
     * User can receive such values as std::vector of std::pair<key, value> values.
     * Order of such values is unspecified.
     */
template < typename A, typename B >
struct signature< std::unordered_map< A, B > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "unordered_map<" + signature< A >::name() + ", " + signature< B >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "a{" + signature_tuple_helper< 0, 2, A, B >::sig() + "}";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::unordered_map< A, B >& v )
  {
    auto sig = "{" + signature_tuple_helper< 0, 2, A, B >::sig() + "}";
    auto lst = eldbus_message_iter_container_new( iter, 'a', sig.c_str() );
    assert( lst );
    for( auto& a : v )
    {
      signature< std::pair< A, B > >::set( lst, a, true );
    }
    eldbus_message_iter_container_close( iter, lst );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::unordered_map< A, B >& v )
  {
    Eldbus_Message_Iter* s;
    v.clear();
    if( !eldbus_message_iter_get_and_next( iter, 'a', &s ) )
      return false;
    std::pair< A, B > a;
    while( signature< std::pair< A, B > >::get( s, a ) )
      v.insert( std::move( a ) );
    return true;
  }
};
/**
     * @brief Signature class for marshalling std::unordered_map template type
     *
     * This marshals container's content as DBUS {} ascii character type code.
     * Note, that library doesnt check, if the key is basic type, as DBUS
     * specification mandates.
     * User can always exchange std::unordered_map for std::map and the reverse.
     * User can receive such values as std::vector of std::pair<key, value> values.
     * Order of such values is unspecified.
     */
template < typename A, typename B >
struct signature< std::map< A, B > >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "map<" + signature< A >::name() + ", " + signature< B >::name() + ">";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return "a{" + signature_tuple_helper< 0, 2, A, B >::sig() + "}";
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const std::map< A, B >& v )
  {
    auto sig = "{" + signature_tuple_helper< 0, 2, A, B >::sig() + "}";
    auto lst = eldbus_message_iter_container_new( iter, 'a', sig.c_str() );
    assert( lst );
    for( auto& a : v )
    {
      signature< std::pair< A, B > >::set( lst, a, true );
    }
    eldbus_message_iter_container_close( iter, lst );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static bool get( Eldbus_Message_Iter* iter, std::map< A, B >& v )
  {
    Eldbus_Message_Iter* s;
    if( !eldbus_message_iter_get_and_next( iter, 'a', &s ) )
      return false;
    std::pair< A, B > a;
    while( signature< std::pair< A, B > >::get( s, a ) )
      v.insert( std::move( a ) );
    return true;
  }
};
/**
     * @brief Signature helper class for marshalling const reference types
     */
template < typename A >
struct signature< const A& >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "const " + signature< A >::name() + "&";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return signature< A >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const A& v )
  {
    signature< A >::set( iter, v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static void get( Eldbus_Message_Iter* iter, A& v )
  {
    signature< A >::get( iter, v );
  }
};
/**
     * @brief Signature helper class for marshalling reference types
     */
template < typename A >
struct signature< A& >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return signature< A >::name() + "&";
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return signature< A >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const A& v )
  {
    signature< A >::set( iter, v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static void get( Eldbus_Message_Iter* iter, A& v )
  {
    signature< A >::get( iter, v );
  }
};
/**
     * @brief Signature helper class for marshalling const types
     */
template < typename A >
struct signature< const A >
{
  /**
      * @brief Returns name of type marshalled, for informative purposes
      */
  static std::string name()
  {
    return "const " + signature< A >::name();
  }
  /**
      * @brief Returns DBUS' signature of type marshalled
      */
  static std::string sig()
  {
    return signature< A >::sig();
  }
  /**
      * @brief Marshals value v as marshalled type into message
      */
  static void set( Eldbus_Message_Iter* iter, const A& v )
  {
    signature< A >::set( iter, v );
  }
  /**
      * @brief Marshals value from marshalled type into variable v
      */
  static void get( Eldbus_Message_Iter* iter, A& v )
  {
    signature< A >::get( iter, v );
  }
};
// /**
//  * @brief Signature helper class for marshalling AT-SPI Accessible pointer values
//  *
//  * In AT-SPI specification those values are mandated to be marshalled as struct (so)
//  * where o is object (exactly as string, but with different ascii
//  * character code.
//  */
// template <> struct signature<std::shared_ptr<AtspiAccessible>> {
//  using subtype = std::pair<std::string, std::string>;

//  /**
//  * @brief Returns name of type marshalled, for informative purposes
//  */
//  static std::string name()
//  {
//    return "AtspiAccessiblePtr";
//  }
//  /**
//  * @brief Returns DBUS' signature of type marshalled
//  */
//  static std::string sig()
//  {
//    return "(so)";
//  }
//  /**
//  * @brief Marshals value v as marshalled type into message
//  */
//  static void set(Eldbus_Message_Iter *iter, const std::shared_ptr<AtspiAccessible> &v)
//  {
//    const auto prefixPath = "/org/a11y/atspi/accessible/";
//    const auto nullPath = "/org/a11y/atspi/null";

//    if (v) {
//      auto bus = atspi_accessible_get_bus_name(v.get(), NULL);
//      auto path = atspi_accessible_get_path(v.get(), NULL);
//      signature<subtype>::set(iter, { bus, std::string{prefixPath} + path });
//      g_free(path);
//      g_free(bus);
//    } else {
//      signature<subtype>::set(iter, { {}, std::string{nullPath} });
//    }
//  }
//  /**
//  * @brief Marshals value from marshalled type into variable v
//  */
//  static bool get(Eldbus_Message_Iter *iter, std::shared_ptr<AtspiAccessible> &v);
// };
/// \cond
struct CallId
{
  friend class ::DBus::DBusServer;
  friend class ::DBus::DBusClient;
  friend class ::DBus::DBusInterfaceDescription;
  static std::atomic< unsigned int > LastId;
  unsigned int id = ++LastId;
};
template < typename ValueType >
ValueType unpackValues( CallId callId, const Eldbus_Message* msg )
{
  auto iter = eldbus_message_iter_get( msg );
  ValueType r;

  if( iter )
  {
    if( !signature< ValueType >::get( iter, r ) )
    {
      DBUS_DEBUG( "ValueType is %s", signature< ValueType >::name().c_str() );
      r = Error{"call " + std::to_string( callId.id ) + ": failed to unpack values, got signature '" +
                eldbus_message_signature_get( msg ) + "', expected '" + signature< ValueType >::sig() + "'"};
    }
  }
  else
  {
    r = Error{"call " + std::to_string( callId.id ) + ": failed to get iterator"};
  }
  return r;
}
inline void packValues_helper( Eldbus_Message_Iter* iter ) {}
template < typename A, typename... ARGS >
void packValues_helper( Eldbus_Message_Iter* iter, A&& a, ARGS&&... r )
{
  signature< A >::set( iter, std::forward< A >( a ) );
  packValues_helper( iter, std::forward< ARGS >( r )... );
}
template < typename... ARGS >
void packValues( CallId callId, Eldbus_Message* msg, ARGS&&... r )
{
  auto iter = eldbus_message_iter_get( msg );
  packValues_helper( iter, std::forward< ARGS >( r )... );
}

template < typename >
struct ReturnType;
template < typename R, typename... ARGS >
struct ReturnType< R( ARGS... ) >
{
  using type = R;
};
template < typename R, typename... ARGS >
struct ReturnType< std::function< R( ARGS... ) > >
{
  using type = R;
};
template < int... >
struct sequence
{
};
template < int N, int... S >
struct sequence_gen : sequence_gen< N - 1, N - 1, S... >
{
};
template < int... S >
struct sequence_gen< 0, S... >
{
  typedef sequence< S... > type;
};
template < typename C, typename... ARGS >
struct apply_helper
{
  const std::function< C >& c;
  const std::tuple< ARGS... >& args;

  template < int... S >
  auto apply_2( sequence< S... > ) const -> decltype( c( std::get< S >( args )... ) )
  {
    return c( std::get< S >( args )... );
  }
  auto apply_1() const -> decltype( apply_2( typename sequence_gen< sizeof...( ARGS ) >::type() ) )
  {
    return apply_2( typename sequence_gen< sizeof...( ARGS ) >::type() );
  }
};
template < typename C, typename A, typename... ARGS >
struct apply_helper_2
{
  const std::function< C >& c;
  const A& a;
  const std::tuple< ARGS... >& args;

  template < int... S >
  auto apply_2( sequence< S... > ) const -> decltype( c( a, std::get< S >( args )... ) )
  {
    return c( a, std::get< S >( args )... );
  }
  auto apply_1() const -> decltype( apply_2( typename sequence_gen< sizeof...( ARGS ) >::type() ) )
  {
    return apply_2( typename sequence_gen< sizeof...( ARGS ) >::type() );
  }
};
template < typename C, typename... ARGS >
auto apply( const std::function< C >& c, const std::tuple< ARGS... >& args ) -> typename ReturnType< C >::type
{
  apply_helper< C, ARGS... > ah{c, args};
  return ah.apply_1();
}
template < typename C, typename D, typename... ARGS >
auto apply( const std::function< C >& c, const D& d, const std::tuple< ARGS... >& args ) -> typename ReturnType< C >::type
{
  apply_helper_2< C, D, ARGS... > ah{c, d, args};
  return ah.apply_1();
}

struct EldbusProxyBase
{
  EldbusProxyBase()
  {
    eldbus_init();
  }
  ~EldbusProxyBase()
  {
    eldbus_shutdown();
  }
};

constexpr static int ELDBUS_CALL_TIMEOUT = 1000;

struct ConnectionState
{
  std::shared_ptr< DBus::EldbusConnection > connection;
  EldbusObjectCallbackHandle object;
  EldbusProxyHandle proxy;
  EldbusProxyHandle propertiesProxy;
};
using CallAsyncDataType = std::tuple< CallId, std::function< void( const Eldbus_Message* ) > >;

static void callAsyncCb( void* data, const Eldbus_Message* msg, Eldbus_Pending* pending )
{
  auto d = static_cast< CallAsyncDataType* >( data );
  DBUS_DEBUG( "call %d: got reply", std::get< 0 >( *d ).id );
  std::get< 1 > ( *d )( msg );
}
static void pendingFreeCb( void* data, const void* )
{
  auto d = static_cast< CallAsyncDataType* >( data );
  DBUS_DEBUG( "call %d: deleting", std::get< 0 >( *d ).id );
  delete d;
}
template < typename RETTYPE, typename... ARGS >
RETTYPE call( CallId callId, ConnectionState& connectionState, bool property, const std::string& funcName, const ARGS&... args )
{
  auto proxy = property ? connectionState.propertiesProxy : connectionState.proxy;
  if( !proxy )
  {
    DBUS_DEBUG( "call %d: not initialized", callId.id );
    return Error{"not initialized"};
  }

  DBUS_DEBUG( "call %d: calling '%s'", callId.id, funcName.c_str() );
  EldbusMessageCallbackHandle msg{eldbus_proxy_method_call_new( proxy.get(), funcName.c_str() )};
  if( !msg )
  {
    DBUS_DEBUG( "call %d: failed", callId.id );
    return Error{"failed to create message"};
  }

  detail::packValues( callId, msg.get(), args... );
  auto replyRawPtr = eldbus_proxy_send_and_block( proxy.get(), msg.release(), ELDBUS_CALL_TIMEOUT );
  EldbusMessageCallbackHandle reply{replyRawPtr};
  DBUS_DEBUG( "call %d: calling '%s' done", callId.id, funcName.c_str() );
  if( !reply )
  {
    DBUS_DEBUG( "call %d: failed", callId.id );
    return Error{"eldbus returned null as reply"};
  }
  const char *errname, *errmsg;
  if( eldbus_message_error_get( reply.get(), &errname, &errmsg ) )
  {
    DBUS_DEBUG( "call %d: %s: %s", callId.id, errname, errmsg );
    return Error{std::string( errname ) + ": " + errmsg};
  }
  DBUS_DEBUG( "call %d: got reply with signature '%s'", callId.id, eldbus_message_signature_get( reply.get() ) );
  return detail::unpackValues< RETTYPE >( callId, reply.get() );
}

template < typename RETTYPE, typename... ARGS >
void asyncCall( CallId callId, ConnectionState connectionState,
                bool property, const std::string& funcName,
                std::function< void( RETTYPE ) > callback, const ARGS&... args )
{
  auto proxy = property ? connectionState.propertiesProxy : connectionState.proxy;
  if( !proxy )
  {
    DBUS_DEBUG( "call %d: not initialized", callId.id );
    callback( Error{"not initialized"} );
    return;
  }

  EldbusMessageCallbackHandle msg{eldbus_proxy_method_call_new( proxy.get(), funcName.c_str() )};
  if( !msg )
  {
    DBUS_DEBUG( "call %d: failed", callId.id );
    callback( Error{"failed to create message"} );
    return;
  }

  auto cbData = new CallAsyncDataType{callId, [callback, callId, proxy]( const Eldbus_Message* reply ) {
                                        DBUS_DEBUG( "call %d: calling done", callId.id );
                                        if( !reply )
                                        {
                                          DBUS_DEBUG( "call %d: failed", callId.id );
                                          callback( Error{"eldbus returned null as reply"} );
                                        }
                                        else
                                        {
                                          const char *errname, *errmsg;
                                          if( eldbus_message_error_get( reply, &errname, &errmsg ) )
                                          {
                                            DBUS_DEBUG( "call %d: %s: %s", callId.id, errname, errmsg );
                                            callback( Error{std::string( errname ) + ": " + errmsg} );
                                          }
                                          else
                                          {
                                            DBUS_DEBUG( "call %d: got reply with signature '%s'", callId.id, eldbus_message_signature_get( reply ) );
                                            callback( detail::unpackValues< RETTYPE >( callId, reply ) );
                                          }
                                        }
                                      }};
  detail::packValues( callId, msg.get(), args... );
  auto pending = eldbus_proxy_send( proxy.get(), msg.release(), callAsyncCb, cbData, ELDBUS_CALL_TIMEOUT );
  if( pending )
  {
    eldbus_pending_free_cb_add( pending, pendingFreeCb, cbData );
    DBUS_DEBUG( "call %d: call sent", callId.id );
  }
  else
  {
    DBUS_DEBUG( "call %d: failed to send call", callId.id );
    callback( Error{"failed to send call"} );
  }
}
inline void displayDebugCallInfo( CallId callId, const std::string& funcName, const std::string& info, const std::string& interfaceName )
{
  DBUS_DEBUG( "call %d: %s iname = %s fname = %s", callId.id, info.c_str(), interfaceName.c_str(), funcName.c_str() );
}
inline void displayDebugCallInfoSignal( CallId callId, const std::string& funcName, const std::string& info, const std::string& interfaceName )
{
  DBUS_DEBUG( "call %d: %s signal iname = %s fname = %s", callId.id, info.c_str(), interfaceName.c_str(), funcName.c_str() );
}
inline void displayDebugCallInfoProperty( CallId callId, const std::string& funcName, std::string info, const std::string& interfaceName,
                                          const std::string& propertyName )
{
  DBUS_DEBUG( "call %d: %s iname = %s pname = %s", callId.id, info.c_str(), interfaceName.c_str(), propertyName.c_str() );
}

class StringStorage
{
  struct char_ptr_deleter
  {
    void operator()( char* p )
    {
      free( p );
    }
  };
  std::vector< std::unique_ptr< char, char_ptr_deleter > > storage;

public:
  const char* add( const char* txt )
  {
    auto ptr = strdup( txt );
    storage.push_back( std::unique_ptr< char, char_ptr_deleter >( ptr ) );
    return storage.back().get();
  }
  const char* add( const std::string& txt )
  {
    return add( txt.c_str() );
  }
};
template < typename A, typename... ARGS >
struct EldbusArgGenerator_Helper
{
  static void add( std::vector< Eldbus_Arg_Info >& r, StringStorage& strings )
  {
    auto s = r.size();
    auto sig = signature< A >::sig();
    assert( !sig.empty() );
    auto name = "p" + std::to_string( s + 1 );
    r.push_back( Eldbus_Arg_Info{strings.add( sig ), strings.add( name )} );
    EldbusArgGenerator_Helper< ARGS... >::add( r, strings );
  }
};
template <>
struct EldbusArgGenerator_Helper< void >
{
  static void add( std::vector< Eldbus_Arg_Info >&, StringStorage& )
  {
  }
};
template <>
struct EldbusArgGenerator_Helper< ValueOrError< void >, void >
{
  static void add( std::vector< Eldbus_Arg_Info >&, StringStorage& )
  {
  }
};
template <>
struct EldbusArgGenerator_Helper< ValueOrError<>, void >
{
  static void add( std::vector< Eldbus_Arg_Info >&, StringStorage& )
  {
  }
};
template < typename... ARGS >
struct EldbusArgGenerator_Helper< std::tuple< ARGS... > >
{
  static void add( std::vector< Eldbus_Arg_Info >& r, StringStorage& strings )
  {
    EldbusArgGenerator_Helper< ARGS..., void >::add( r, strings );
  }
};
template < typename RetType >
struct dbus_interface_return_type_traits
{
  using type = ValueOrError< RetType >;
};
template < typename... ARGS >
struct dbus_interface_return_type_traits< ValueOrError< ARGS... > >
{
  using type = ValueOrError< ARGS... >;
};
template < typename T >
struct dbus_interface_traits;
template < typename RetType, typename... ARGS >
struct dbus_interface_traits< RetType( ARGS... ) >
{
  using Ret = typename dbus_interface_return_type_traits< RetType >::type;
  using SyncCB = std::function< Ret( ARGS... ) >;
  using AsyncCB = std::function< void( std::function< void( Ret ) >, ARGS... ) >;
  using VEArgs = ValueOrError< ARGS... >;
};
template < typename T >
struct EldbusArgGenerator_Args;
template < typename RetType, typename... ARGS >
struct EldbusArgGenerator_Args< RetType( ARGS... ) >
{
  static std::string name()
  {
    return signature_tuple_helper< 0, sizeof...( ARGS ), ARGS... >::name();
  }
  static std::vector< Eldbus_Arg_Info > get( StringStorage& strings )
  {
    std::vector< Eldbus_Arg_Info > tmp;
    EldbusArgGenerator_Helper< ARGS..., void >::add( tmp, strings );
    tmp.push_back( {nullptr, nullptr} );
    return tmp;
  }
};
template < typename T >
struct EldbusArgGenerator_ReturnType;
template < typename RetType, typename... ARGS >
struct EldbusArgGenerator_ReturnType< RetType( ARGS... ) >
{
  static std::string name()
  {
    return signature< RetType >::name();
  }
  static std::vector< Eldbus_Arg_Info > get( StringStorage& strings )
  {
    std::vector< Eldbus_Arg_Info > tmp;
    EldbusArgGenerator_Helper< RetType, void >::add( tmp, strings );
    tmp.push_back( {nullptr, nullptr} );
    return tmp;
  }
};
template < typename T >
struct EldbusArgGenerator_ReturnType;
template < typename... ARGS >
struct EldbusArgGenerator_ReturnType< void( ARGS... ) >
{
  static std::string name()
  {
    return "";
  }
  static std::vector< Eldbus_Arg_Info > get( StringStorage& strings )
  {
    std::vector< Eldbus_Arg_Info > tmp;
    tmp.push_back( {nullptr, nullptr} );
    return tmp;
  }
};
/// \endcond
}

/**
  * @brief Enumeration determining, which DBUS session user wants to connect to.
  */
enum class ConnectionType
{
  SYSTEM,
  SESSION
};

/**
  * @brief Class representing client's end of DBUS connection
  *
  * Allows calling (synchronous and asynchronos) methods on selected interface
  * Allows (synchronous and asynchronos) setting / getting properties.
  * Allows registering signals.
  */
class DBusClient : private detail::EldbusProxyBase
{
  /// \cond
  struct ConnectionInfo
  {
    std::string interfaceName, busName, pathName;
    void emit( const char* member, DBusActionType type )
    {
      detail::emitNotification( busName.c_str(), pathName.c_str(), interfaceName.c_str(), member, type );
    }
  };
  /// \endcond
public:
  /**
    * @brief Default constructor, creates non-connected object.
    */
  DBusClient() = default;
  /**
    * @brief Connects to dbus choosen by tp, using given arguments
    *
    * @param bus_name name of the bus to connect to
    * @param path_name object's path
    * @param interface_name interface name
    */
  DBusClient( std::string busName_, std::string pathName_, std::string interfaceName_,
              ConnectionType tp );
  /**
    * @brief Connects to dbus using connection conn
    *
    * @param bus_name name of the bus to connect to
    * @param path_name object's path
    * @param interface_name interface name
    * @param conn connection object from getDBusConnectionByType call
    */
  DBusClient( std::string busName_, std::string pathName_, std::string interfaceName_,
              const std::shared_ptr< DBus::EldbusConnection >& conn = {} );
  /**
    * @brief Destructor object.
    *
    * All signals added will be disconnected.
    * All asynchronous calls will be cancelled, their callback's will be called
    * with failure message.
    */
  ~DBusClient() = default;
  DBusClient( const DBusClient& ) = delete;
  DBusClient( DBusClient&& ) = default;

  DBusClient& operator=( DBusClient&& ) = default;
  DBusClient& operator=( const DBusClient& ) = delete;

  /**
    * @brief bool operator
    *
    * Returns true, if object is connected to DBUS
    */
  explicit operator bool() const
  {
    return bool( connectionState.proxy );
  }

  /**
    * @brief Helper class for calling a method
    *
    * Template type T defines both arguments sent to the method
    * and expected values. Receiving different values will be reported as
    * error. For example:
    * \code{.cpp} Method<int(float, float)> \endcode
    * defines method, which takes two arguments (two floats) and return
    * single value of type int.
    */
  template < typename T >
  struct Method
  {
    /// \cond
    using RetType = typename detail::dbus_interface_traits< T >::Ret;
    detail::ConnectionState connectionState;
    std::string funcName;
    std::string info;
    std::shared_ptr< ConnectionInfo > connectionInfo;
    /// \endcond

    /**
       * @brief Executes synchronous call on DBUS's method
       *
       * The function returns ValueOrError<...> object, which
       * contains either received values or error message.
       *
       * @param args arguments to pass to the method
       */
    template < typename... ARGS >
    RetType call( const ARGS&... args )
    {
      detail::CallId callId;
      detail::displayDebugCallInfo( callId, funcName, info, connectionInfo->interfaceName );
      return detail::call< RetType >( callId, connectionState, false, funcName, args... );
    }

    /**
       * @brief Executes asynchronous call on DBUS's method
       *
       * The function calls callback with either received values or error message.
       *
       * @param callback callback functor, which will be called with return value(s) or error message
       * @param args arguments to pass to the method
       */
    template < typename... ARGS >
    void asyncCall( std::function< void( RetType ) > callback, const ARGS&... args )
    {
      detail::CallId callId;
      detail::displayDebugCallInfo( callId, funcName, info, connectionInfo->interfaceName );
      auto connectionState = this->connectionState;
      detail::asyncCall< RetType >( callId, connectionState, false, funcName, std::move( callback ), args... );
    }
  };

  /**
    * @brief Helper class for calling a property
    *
    * Template type T defines type of the value hidden under property.
    * Note, that library automatically wraps both sent and received value into
    * DBUS's wrapper type.
    */
  template < typename T >
  struct Property
  {
    /// \cond
    using RetType = typename detail::dbus_interface_return_type_traits< T >::type;
    using VariantRetType = typename detail::dbus_interface_return_type_traits< EldbusVariant< T > >::type;
    detail::ConnectionState connectionState;
    std::string propName;
    std::string info;
    std::shared_ptr< ConnectionInfo > connectionInfo;
    /// \endcond

    /**
      * @brief executes synchronous get on property
      *
      * The function returns ValueOrError<...> object, which
      * contains either received values or error message.
      */
    RetType get()
    {
      connectionInfo->emit( propName.c_str(), DBusActionType::GETTER_CALL );
      detail::CallId callId;
      detail::displayDebugCallInfoProperty( callId, "Get", info, connectionInfo->interfaceName, propName );
      auto z = detail::call< VariantRetType >( callId, connectionState, true, "Get", connectionInfo->interfaceName, propName );
      if( !z )
        return z.getError();
      return {std::get< 0 >( z.getValues() ).value};
    }

    /**
      * @brief executes asynchronous get on property
      *
      * The function calls callback with either received values or error message.
      *
      * @param callback callback functor, which will be called with return value(s) or error message
      */
    void asyncGet( std::function< void( RetType ) > callback )
    {
      connectionInfo->emit( propName.c_str(), DBusActionType::GETTER_CALL );
      detail::CallId callId;
      detail::displayDebugCallInfoProperty( callId, "Get", info, connectionInfo->interfaceName, propName );
      auto connectionState = this->connectionState;
      auto cc = [callback]( VariantRetType reply ) {
        if( reply )
          callback( std::move( std::get< 0 >( reply.getValues() ).value ) );
        else
          callback( reply.getError() );
      };
      detail::asyncCall< VariantRetType >( callId, connectionState, true, "Get", std::move( cc ), connectionInfo->interfaceName, propName );
    }

    /**
      * @brief executes synchronous set on property
      *
      * The function returns ValueOrError<void> object, with
      * possible error message.
      */
    ValueOrError< void > set( const T& r )
    {
      connectionInfo->emit( propName.c_str(), DBusActionType::SETTER_CALL );
      detail::CallId callId;
      detail::displayDebugCallInfoProperty( callId, "Set", info, connectionInfo->interfaceName, propName );
      EldbusVariant< T > variantValue{std::move( r )};
      return detail::call< ValueOrError< void > >( callId, connectionState, true, "Set", connectionInfo->interfaceName, propName, variantValue );
    }

    /**
      * @brief executes asynchronous get on property
      *
      * The function calls callback with either received values or error message.
      *
      * @param callback callback functor, which will be called with return value(s) or error message
      */
    void asyncSet( std::function< void( ValueOrError< void > ) > callback, const T& r )
    {
      connectionInfo->emit( propName.c_str(), DBusActionType::SETTER_CALL );
      detail::CallId callId;
      detail::displayDebugCallInfoProperty( callId, "Set", info, connectionInfo->interfaceName, propName );
      EldbusVariant< T > variantValue{std::move( r )};
      detail::asyncCall< ValueOrError< void > >( callId, connectionState, true, "Set", std::move( callback ), connectionInfo->interfaceName, propName, variantValue );
    }
  };

  /**
    * @brief Constructs Property<...> object for calling properties
    *
    * The function constructs and returns proxy object for calling property.
    *
    * @param propName property name to set and / or query
    */
  template < typename PropertyType >
  Property< PropertyType > property( std::string propName )
  {
    return Property< PropertyType >{connectionState, std::move( propName ), info, connectionInfo};
  }

  /**
    * @brief Constructs Method<...> object for calling methods
    *
    * The function constructs and returns proxy object for calling method.
    *
    * @param funcName function name to call
    */
  template < typename MethodType >
  Method< MethodType > method( std::string funcName )
  {
    return Method< MethodType >{connectionState, std::move( funcName ), info, connectionInfo};
  }

  /**
     * @brief Registers notification callback, when property has changed
     *
     * The callback will be called with new value, when property's value has changed.
     * Note, that template type V must match expected type, otherwise undefined behavior will occur,
     * there's no check for this.
     */
  template < typename V >
  void addPropertyChangedEvent( std::string propertyName, std::function< void( V ) > callback )
  {
    detail::CallId callId;
    detail::displayDebugCallInfoSignal( callId, propertyName, info, connectionInfo->interfaceName );
    DBUS_DEBUG( "call %d: adding property", callId.id );
    auto cS = this->connectionState;
    auto cI = this->connectionInfo;
    auto callbackLambdaPtr = new std::function< void( Eldbus_Proxy_Event_Property_Changed* ) >;
    *callbackLambdaPtr = [callId, cS, callback, propertyName, cI]( Eldbus_Proxy_Event_Property_Changed* ev ) {
      const char* ifc = eldbus_proxy_interface_get( ev->proxy );
      DBUS_DEBUG( "call %d: property changed iname = %s pname = %s (name %s iface %s)",
                  callId.id, cI->interfaceName.c_str(), propertyName.c_str(), ev->name, ifc );
      V val = 0;
      if( ev->name && ev->name == propertyName && ifc && cI->interfaceName == ifc )
      {
        if( !eina_value_get( ev->value, &val ) )
        {
          DBUS_DEBUG( "unable to get property's value" );
          return;
        }
        DBUS_DEBUG( ". %d", val );
        callback( val );
        DBUS_DEBUG( "." );
      }
      DBUS_DEBUG( "." );
    };
    auto p = connectionState.proxy.get();
    eldbus_proxy_event_callback_add( p, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                     listenerEventChangedCallback, callbackLambdaPtr );
    destructors.add( [=]() {
      eldbus_proxy_event_callback_del( p, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                       listenerEventChangedCallback, callbackLambdaPtr );
      delete callbackLambdaPtr;
    } );
  }
  /**
    * @brief Registers callback on the DBUS' signal
    *
    * The function registers callback signalName. When signal comes, callback will be called.
    * Callback object will exists as long as signal is registered. You can unregister signal
    * by destroying DBusClient object.
    *
    * @param signalName name of the signal to register
    * @param callback callback to call
    */
  template < typename SignalType >
  void addSignal( std::string signalName, std::function< SignalType > callback )
  {
    detail::CallId callId;
    detail::displayDebugCallInfoSignal( callId, signalName, info, connectionInfo->interfaceName );
    auto cS = this->connectionState;
    auto cI = this->connectionInfo;
    auto callbackLambda = [callId, cS, callback, signalName, cI]( const Eldbus_Message* msg ) -> void {
      const char *errname, *aux;
      if( eldbus_message_error_get( msg, &errname, &aux ) )
      {
        DBUS_DEBUG( "call %d: Eldbus error: %s %s", callId.id, errname, aux );
        return;
      }
      cI->emit( signalName.c_str(), DBusActionType::SIGNAL_RECEIVED );
      DBUS_DEBUG( "call %d: received signal with signature '%s'", callId.id, eldbus_message_signature_get( msg ) );
      using ParamsType = typename detail::dbus_interface_traits< SignalType >::VEArgs;
      auto params = detail::unpackValues< ParamsType >( callId, msg );
      if( !params )
      {
        DBUS_DEBUG( "call %d: failed: %s", callId.id, params.getError().message.c_str() );
        return;
      }
      try
      {
        detail::apply( callback, params.getValues() );
      }
      catch( ... )
      {
        DBUS_DEBUG( "unhandled exception" );
        assert( 0 );
      }
    };
    auto tmp = new std::function< void( const Eldbus_Message* msg ) >{std::move( callbackLambda )};
    auto handler = eldbus_proxy_signal_handler_add( connectionState.proxy.get(), signalName.c_str(), listenerCallback, tmp );
    destructors.add( [=]() {
      eldbus_signal_handler_del( handler );
      delete tmp;
    } );
  }

private:
  /// \cond
  detail::ConnectionState connectionState;
  detail::CallOnDestructionList destructors;
  std::string info;
  std::shared_ptr< ConnectionInfo > connectionInfo;
  void emitNotification( DBusActionType type );

  static void listenerCallback( void* data, const Eldbus_Message* msg )
  {
    auto p = static_cast< std::function< void( const Eldbus_Message* msg ) >* >( data );
    ( *p )( msg );
  }
  static void listenerEventChangedCallback( void* data, Eldbus_Proxy* proxy EINA_UNUSED, void* event )
  {
    auto p = static_cast< std::function< void( Eldbus_Proxy_Event_Property_Changed* ) >* >( data );
    ( *p )( static_cast< Eldbus_Proxy_Event_Property_Changed* >( event ) );
  }
  /// \endcond
};

/**
   * @brief Helper class describing DBUS's server interface
   *
   */
class DBusInterfaceDescription
{
  friend class DBusServer;

public:
  /// \cond
  struct MethodInfo
  {
    detail::CallId id;
    std::string memberName;
    std::vector< Eldbus_Arg_Info > in, out;
    std::function< Eldbus_Message*( const Eldbus_Message* msg ) > callback;
  };
  struct SignalInfo
  {
    detail::CallId id;
    std::string memberName;
    std::vector< Eldbus_Arg_Info > args;
    unsigned int uniqueId;
  };
  struct PropertyInfo
  {
    detail::CallId setterId, getterId;
    std::string memberName, typeSignature;
    std::function< ValueOrError< void >( const Eldbus_Message*src, Eldbus_Message_Iter*dst ) > getCallback, setCallback;
  };
  class SignalId
  {
    friend class ::DBus::DBusServer;
    friend class ::DBus::DBusClient;
    friend class ::DBus::DBusInterfaceDescription;
    detail::CallId id;

    SignalId( detail::CallId id ) : id( id ) {}

  public:
    SignalId() = default;
  };
  /// \endcond

  /**
     * @brief Creates empty interface description with given name
     *
     * @param interfaceName name of the interface
     */
  DBusInterfaceDescription( std::string interfaceName );

  /**
     * @brief adds new, synchronous method to the interface
     *
     * When method memberName is called on DBUS, callback functor will be called
     * with values received from DBUS. callback won't be called, if method was
     * called with invalid signature. Value returned from functor (or error message)
     * will be marshalled back to the caller.
     *
     * Template type T defines both arguments sent to the method
     * and expected values. Receiving different values will be reported as
     * error. For example:
     * \code{.cpp} Method<int(float, float)> \endcode
     * defines method, which takes two arguments (two floats) and return
     * single value of type int.
     *
     * @param memberName name of the method
     * @param callback functor, which will be called
     */
  template < typename T >
  void addMethod( const std::string& memberName, typename detail::dbus_interface_traits< T >::SyncCB callback )
  {
    detail::CallId callId;
    MethodInfo mi;
    methods.push_back( std::move( mi ) );
    auto& z = methods.back();
    z.in = detail::EldbusArgGenerator_Args< T >::get( strings );
    z.out = detail::EldbusArgGenerator_ReturnType< T >::get( strings );
    z.memberName = memberName;
    DBUS_DEBUG( "call %d: method %s, in %s, out %s", callId.id, memberName.c_str(),
                detail::EldbusArgGenerator_Args< T >::name().c_str(),
                detail::EldbusArgGenerator_ReturnType< T >::name().c_str() );
    z.callback = construct< T >( callId, callback );
    z.id = callId;
  }

  /**
     * @brief adds new, asynchronous method to the interface
     *
     * When method memberName is called on DBUS, callback functor will be called
     * with values received from DBUS. callback won't be called, if method was
     * called with invalid signature. callback will called with reply callback functor.
     * Reply callback functor must be called with reply value, when it's ready.
     * It's safe to ignore calling reply callback, but some resources might be kept
     * as long as either reply callback exists or reply timeout hasn't yet been met.
     *
     * Template type T defines both arguments sent to the method
     * and expected values. Receiving different values will be reported as
     * error. For example:
     * \code{.cpp} Method<int(float, float)> \endcode
     * defines method, which takes two arguments (two floats) and return
     * single value of type int.
     *
     * @param memberName name of the method
     * @param callback functor, which will be called
     */
  template < typename T >
  void addAsyncMethod( const std::string& memberName, typename detail::dbus_interface_traits< T >::AsyncCB callback );

  /**
     * @brief adds new, synchronous property to the interface
     *
     * When property memberName is called on DBUS, respective callback functor will be called
     * with values received from DBUS. callback won't be called, if method was
     * called with invalid signature. Value returned from functor (or error message)
     * will be marshalled back to the caller.
     *
     * Template type T defines type of the value hidden under property.
     * Note, that library automatically wraps both sent and received value into
     * DBUS's wrapper type.
     *
     * @param memberName name of the method
     * @param getter functor, which will be called when property is being read
     * @param setter functor, which will be called when property is being set
     */
  template < typename T >
  void addProperty( const std::string& memberName, std::function< ValueOrError< T >() > getter, std::function< ValueOrError< void >( T ) > setter )
  {
    properties.push_back( {} );
    auto& z = properties.back();
    z.memberName = memberName;
    z.typeSignature = detail::signature< T >::sig();
    if( getter )
    {
      detail::CallId getterId;
      z.getterId = getterId;
      DBUS_DEBUG( "call %d: property %s (get) type %s", getterId.id, memberName.c_str(), detail::signature< T >::name().c_str() );
      z.getCallback = [=]( const Eldbus_Message* src, Eldbus_Message_Iter* dst ) -> ValueOrError< void > {
        detail::emitNotification( eldbus_message_sender_get( src ),
                                  eldbus_message_path_get( src ), interfaceName.c_str(), memberName.c_str(), DBusActionType::GETTER_RESPONSE );
        try
        {
          auto v = detail::apply( getter, std::tuple<>{} );
          if( v )
          {
            detail::signature< T >::set( dst, std::get< 0 >( v.getValues() ) );
            DBUS_DEBUG( "call %d: success", getterId.id );
            return Success{};
          }
          DBUS_DEBUG( "call %d: failed: %s", getterId.id, v.getError().message.c_str() );
          return v.getError();
        }
        catch( std::exception& e )
        {
          return Error{std::string( "unhandled exception (" ) + e.what() + ")"};
        }
        catch( ... )
        {
          return Error{"unhandled exception"};
        }
      };
    }
    if( setter )
    {
      detail::CallId setterId;
      z.setterId = setterId;
      DBUS_DEBUG( "call %d: property %s (set) type %s", setterId.id, memberName.c_str(), detail::signature< T >::name().c_str() );
      z.setCallback = [=]( const Eldbus_Message* src, Eldbus_Message_Iter* src_iter ) -> ValueOrError< void > {
        detail::emitNotification( eldbus_message_sender_get( src ),
                                  eldbus_message_path_get( src ), interfaceName.c_str(), memberName.c_str(), DBusActionType::SETTER_RESPONSE );
        std::tuple< T > value;
        auto src_signature = eldbus_message_iter_signature_get( src_iter );
        if( detail::signature< T >::get( src_iter, std::get< 0 >( value ) ) )
        {
          try
          {
            auto v = detail::apply( setter, std::move( value ) );
            if( v )
            {
              DBUS_DEBUG( "call %d: success", setterId.id );
              return Success{};
            }
            DBUS_DEBUG( "call %d: failed: %s", setterId.id, v.getError().message.c_str() );
            free( src_signature );
            return v.getError();
          }
          catch( std::exception& e )
          {
            return Error{std::string( "unhandled exception (" ) + e.what() + ")"};
          }
          catch( ... )
          {
            return Error{"unhandled exception"};
          }
        }
        DBUS_DEBUG( "call %d: failed to unpack values, got signature '%s', expected '%s'", setterId.id,
                    src_signature, detail::signature< T >::sig().c_str() );
        return Error{"call " + std::to_string( setterId.id ) + ": failed to unpack values, got signature '" +
                     src_signature + "', expected '" + detail::signature< T >::sig() + "'"};
      };
    }
  }

  /**
     * @brief adds new signal to the interface
     *
     * Template types ARGS defines values, which will be emited with the signal
     *
     * @param memberName name of the method
     */
  template < typename... ARGS >
  SignalId addSignal( const std::string& memberName )
  {
    detail::CallId callId;
    signals.push_back( {} );
    auto& z = signals.back();
    z.memberName = memberName;
    z.args = detail::EldbusArgGenerator_Args< void( ARGS... ) >::get( strings );
    z.id = callId;
    DBUS_DEBUG( "call %d: signal %s", callId.id, memberName.c_str() );
    return SignalId{callId};
  }

private:
  /// \cond
  std::vector< MethodInfo > methods;
  std::vector< PropertyInfo > properties;
  std::vector< SignalInfo > signals;
  std::string interfaceName;
  detail::StringStorage strings;

  template < typename T >
  std::function< Eldbus_Message*( const Eldbus_Message* msg ) > construct( detail::CallId callId,
                                                                           typename detail::dbus_interface_traits< T >::SyncCB callback )
  {
    using VEArgs = typename detail::dbus_interface_traits< T >::VEArgs;
    return [=]( const Eldbus_Message* msg ) -> Eldbus_Message* {
      DBUS_DEBUG( "call %d: entering", callId.id );
      detail::emitNotification( eldbus_message_sender_get( msg ),
                                eldbus_message_path_get( msg ), interfaceName.c_str(), eldbus_message_member_get( msg ), DBusActionType::METHOD_RESPONSE );
      Eldbus_Message* ret = nullptr;
      auto args = detail::unpackValues< VEArgs >( callId, msg );
      if( args )
      {
        try
        {
          auto v = detail::apply( callback, std::move( args.getValues() ) );
          if( v )
          {
            DBUS_DEBUG( "call %d: success", callId.id );
            ret = eldbus_message_method_return_new( msg );
            packValues( callId, ret, v );
          }
          else
          {
            DBUS_DEBUG( "call %d: failed: %s", callId.id, v.getError().message.c_str() );
            ret = eldbus_message_error_new( msg, "org.freedesktop.DBus.Error.Failed", v.getError().message.c_str() );
          }
        }
        catch( std::exception& e )
        {
          auto txt = std::string( "unhandled exception (" ) + e.what() + ")";
          DBUS_DEBUG( "call %d: failed: %s", callId.id, txt.c_str() );
          ret = eldbus_message_error_new( msg, "org.freedesktop.DBus.Error.Failed", txt.c_str() );
        }
        catch( ... )
        {
          DBUS_DEBUG( "call %d: failed: %s", callId.id, "unhandled exception" );
          ret = eldbus_message_error_new( msg, "org.freedesktop.DBus.Error.Failed", "unhandled exception" );
        }
      }
      else
      {
        std::ostringstream err;
        err << "expected signature '" << detail::signature< VEArgs >::sig() << "', got '" << eldbus_message_signature_get( msg ) << "'";
        auto str = err.str();
        DBUS_DEBUG( "call %d: failed: %s", callId.id, str.c_str() );
        ret = eldbus_message_error_new( msg, "org.freedesktop.DBus.Error.InvalidArgs", str.c_str() );
      }
      return ret;
    };
  }
  /// \endcond
};

/**
  * @brief Class representing server's end of DBUS connection
  *
  * Allows listening (synchronously and asynchronosly) on methods on selected interface
  * Allows listening (synchronously and asynchronosly) on setting / getting properties.
  * Allows emiting signals.
  */
class DBusServer : private detail::EldbusProxyBase
{
public:
  /**
     * @brief Constructs non-connected dbus server.
     */
  DBusServer() = default;

  /**
     * @brief Constructs dbus server on either system or user dbus connection.
     */

  DBusServer( ConnectionType tp );
  /**
     * @brief Constructs dbus server on connection from getDBusConnectionByType
     */

  DBusServer( const std::shared_ptr< DBus::EldbusConnection >& conn );
  /**
     * @brief Destructor
     *
     * Destructor will properly destroy everything. Destructor will cancel
     * pending replies.
     */
  ~DBusServer() = default;

  DBusServer( const DBusServer& ) = delete;
  DBusServer( DBusServer&& ) = default;

  DBusServer& operator=( DBusServer&& ) = default;
  DBusServer& operator=( const DBusServer& ) = delete;

  /**
     * @brief Registers interface on given path name
     *
     * @param pathName path object to register interface on.
     * @param dscr
     * @param fallback
     */
  void addInterface( const std::string& pathName, DBusInterfaceDescription& dscr, bool fallback = false );

  /**
     * @brief Gets bus name of the current connection (must be connected)
     */
  std::string getBusName() const;

  /**
     * @brief Returns connection object for this dbus server object
     *
     * @return connection object
     */
  std::shared_ptr< DBus::EldbusConnection > getConnection();

  /**
     * @brief Emits signal
     *
     * You must pass identifier of the signal, got as result of calling DBusInterfaceDescription::addSignal.
     * Types of the arguments must match EXACTLY types used to call addSignal.
     *
     * @param signal identifier of the signal
     * @param args values to emit
     */
  template < typename... ARGS >
  void emit( DBusInterfaceDescription::SignalId signal, const ARGS&... args )
  {
    auto it = signalData.find( signal.id.id );
    if( it != signalData.end() )
    {
      auto msg = eldbus_service_signal_new( it->second.first, it->second.second );
      detail::packValues( signal.id, msg, args... );
      eldbus_service_signal_send( it->second.first, msg );
    }
    else
    {
      DBUS_DEBUG( "signal %d not found", signal.id.id );
    }
  }

  /**
     * @brief Emits signal
     *
     * Emits signal based only on data passed to the function
     *
     * @param signal identifier of the signal
     * @param args values to emit
     */
  template < typename... ARGS >
  void emit2( const std::string& path, const std::string& interfaceName,
              const std::string& signalName, const ARGS&... args )
  {
    auto msg = eldbus_message_signal_new( path.c_str(), interfaceName.c_str(), signalName.c_str() );
    detail::CallId id;
    detail::packValues( id, msg, args... );
    eldbus_connection_send( connection->get(), msg, nullptr, nullptr, -1 );
  }
  /**
     * @brief Returns current object path, when handling call to property / method
     *
     * User can call this function from inside callback used to handle property / method calls.
     * It will retrieve object's path used in the call. Note, that in asynchronous handling
     * of those calls user need to retrieve and store the current object / current connection
     * as the value will change at the moment user's callback handling will exit. For example:
     * \code{.cpp}
     * DBusInterfaceDescription interface{"name"};
     * auto handler_later = [](std::function<void(void)> done_cb) {
     *   // process something later on
     *   DBusServer::getCurrentObjectPath(); // this will return empty string
     * };
     * interface.addAsyncMethod<void()>("m", [=](std::function<void(void)> done_cb) {
     *   DBusServer::getCurrentObjectPath(); // this will current object's path
     *
     *   // do some processing later on and call done_cb, when it's done
     *   register_to_call_sometime_later_on(std::move(done_cb));
     * };
     * \endcode
     */
  static std::string getCurrentObjectPath();

  /**
     * @brief Returns current connection object, when handling call to property / method
     *
     * User can call this function from inside callback used to handle property / method calls.
     * It will retrieve object's path used in the call. Note, that in asynchronous handling
     * of those calls user need to retrieve and store the current object / current connection
     * as the value will change at the moment user's callback handling will exit. For example:
     * \code{.cpp}
     * DBusInterfaceDescription interface{"name"};
     * auto handler_later = [](std::function<void(void)> done_cb) {
     *   // process something later on
     *   DBusServer::getCurrentObjectPath(); // this will return empty string
     * };
     * interface.addAsyncMethod<void()>("m", [=](std::function<void(void)> done_cb) {
     *   DBusServer::getCurrentObjectPath(); // this will current object's path
     *
     *   // do some processing later on and call done_cb, when it's done
     *   register_to_call_sometime_later_on(std::move(done_cb));
     * };
     * \endcode
     */
  static std::shared_ptr< DBus::EldbusConnection > getCurrentConnection();

private:
  /// \cond
  std::shared_ptr< DBus::EldbusConnection > connection;
  detail::CallOnDestructionList destructors;
  std::unordered_map< unsigned int, std::pair< const Eldbus_Service_Interface*, unsigned int > > signalData;
  /// \endcond
};

template < typename T >
void DBusInterfaceDescription::addAsyncMethod( const std::string& memberName, typename detail::dbus_interface_traits< T >::AsyncCB callback )
{
  detail::CallId callId;
  MethodInfo mi;
  methods.push_back( std::move( mi ) );
  auto& z = methods.back();
  z.in = detail::EldbusArgGenerator_Args< T >::get( strings );
  z.out = detail::EldbusArgGenerator_ReturnType< T >::get( strings );
  z.memberName = memberName;
  DBUS_DEBUG( "call %d: method %s, in %s, out %s", callId.id, memberName.c_str(),
              detail::EldbusArgGenerator_Args< T >::name().c_str(),
              detail::EldbusArgGenerator_ReturnType< T >::name().c_str() );
  using VEArgs = typename detail::dbus_interface_traits< T >::VEArgs;
  z.callback = [=]( const Eldbus_Message* msg ) -> Eldbus_Message* {
    detail::emitNotification( eldbus_message_sender_get( msg ),
                              eldbus_message_path_get( msg ), interfaceName.c_str(), memberName.c_str(), DBusActionType::METHOD_RESPONSE );
    struct CallState
    {
      bool replyRunning = true;
      Eldbus_Message* reply = nullptr;
      EldbusMessageCallbackHandle message;
    };
    auto callState = std::make_shared< CallState >();
    callState->message.reset( eldbus_message_ref( const_cast< Eldbus_Message* >( msg ) ) );
    auto connection = DBusServer::getCurrentConnection();
    auto retCallback = [=]( typename detail::dbus_interface_traits< T >::Ret v ) {
      if( v )
      {
        callState->reply = eldbus_message_method_return_new( callState->message.get() );
        packValues( callId, callState->reply, v );
      }
      else
      {
        DBUS_DEBUG( "call %d: failed: %s", callId.id, v.getError().message.c_str() );
        callState->reply = eldbus_message_error_new( callState->message.get(), "org.freedesktop.DBus.Error.Failed", v.getError().message.c_str() );
      }
      if( !callState->replyRunning )
      {
        eldbus_connection_send( connection->get(), callState->reply, NULL, NULL, -1 );
      }
    };
    Eldbus_Message* ret = nullptr;
    auto args = detail::unpackValues< VEArgs >( callId, msg );
    if( args )
    {
      auto error = [&]( const std::string& txt ) {
        if( !callState->reply )
        {
          DBUS_DEBUG( "call %d: failed: %s", callId.id, txt.c_str() );
          callState->reply = eldbus_message_error_new( callState->message.get(), "org.freedesktop.DBus.Error.Failed", txt.c_str() );
        }
      };
      try
      {
        detail::apply( callback, std::move( retCallback ), std::move( args.getValues() ) );
      }
      catch( std::exception& e )
      {
        error( std::string( "unhandled exception (" ) + e.what() + ")" );
      }
      catch( ... )
      {
        error( "unhandled exception" );
      }

      callState->replyRunning = false;
      ret = callState->reply;
    }
    else
    {
      std::ostringstream err;
      err << "expected signature '" << detail::signature< VEArgs >::sig() << "', got '" << eldbus_message_signature_get( msg ) << "'";
      auto str = err.str();
      ret = eldbus_message_error_new( msg, "org.freedesktop.DBus.Error.InvalidArgs", str.c_str() );
    }
    return ret;
  };

  z.id = callId;
}

/// \cond
std::shared_ptr< EldbusConnection > getDBusConnectionByType( ConnectionType tp );
std::shared_ptr< EldbusConnection > getDBusConnectionByName( const std::string& name );
std::string getConnectionName( const std::shared_ptr< EldbusConnection >& );
/// \endcond
}

/// \cond
namespace std
{
template < size_t INDEX, typename... ARGS >
inline auto get( DBus::ValueOrError< ARGS... >& v ) -> decltype( std::get< INDEX >( v.getValues() ) ) &
{
  return std::get< INDEX >( v.getValues() );
}
template < size_t INDEX, typename... ARGS >
inline auto get( const DBus::ValueOrError< ARGS... >& v ) -> decltype( std::get< INDEX >( v.getValues() ) )
{
  return std::get< INDEX >( v.getValues() );
}
}
/// \endcond

#endif
