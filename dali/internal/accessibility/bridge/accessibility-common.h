#ifndef DALI_INTERNAL_ATSPI_ACCESSIBILITY_COMMON_H
#define DALI_INTERNAL_ATSPI_ACCESSIBILITY_COMMON_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/internal/accessibility/bridge/dbus/dbus-locators.h>
#include <dali/internal/accessibility/bridge/dbus/dbus.h>
#include <dali/public-api/dali-adaptor-common.h>

// DBus names

#define A11yDbusName "org.a11y.Bus"
#define A11yDbusStatusInterface "org.a11y.Status"
#define AtspiDbusNameRegistry "org.a11y.atspi.Registry"
#define DirectReadingDBusName "org.tizen.ScreenReader"
#define DirectReadingDBusInterface "org.tizen.DirectReading"

// DBus paths

#define A11yDbusPath "/org/a11y/bus"
#define AtspiDbusPathCache "/org/a11y/atspi/cache"
#define AtspiDbusPathDec "/org/a11y/atspi/registry/deviceeventcontroller"
#define AtspiDbusPathRegistry "/org/a11y/atspi/registry"
#define AtspiDbusPathRoot "/org/a11y/atspi/accessible/root"
#define AtspiPath "/org/a11y/atspi/accessible"
#define DirectReadingDBusPath "/org/tizen/DirectReading"

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

/**
 * @brief The CurrentBridgePtr class is to save the current Accessibility Bridge.
 */
class CurrentBridgePtr
{
  static Dali::Accessibility::Bridge*& Get()
  {
    static thread_local Dali::Accessibility::Bridge* bridge = nullptr;
    return bridge;
  }
  Dali::Accessibility::Bridge* mPrev;
  CurrentBridgePtr(const CurrentBridgePtr&) = delete;
  CurrentBridgePtr(CurrentBridgePtr&&)      = delete;
  CurrentBridgePtr& operator=(const CurrentBridgePtr&) = delete;
  CurrentBridgePtr& operator=(CurrentBridgePtr&&) = delete;

public:
  CurrentBridgePtr(Dali::Accessibility::Bridge* bridge)
  : mPrev(Get())
  {
    Get() = bridge;
  }

  ~CurrentBridgePtr()
  {
    Get() = mPrev;
  }

  static Dali::Accessibility::Bridge* GetCurrentBridge()
  {
    return Get();
  }
}; // CurrentBridgePtr


// Templates for setting and getting Accessible values
namespace detail
{
template<>
struct signature<Dali::Accessibility::Address> : signature_helper<signature<Dali::Accessibility::Address>>
{
  using subtype = std::pair<std::string, ObjectPath>;

  static constexpr auto name_v = concat("AtspiAccessiblePtr");
  static constexpr auto sig_v  = concat("(so)");

  /**
   * @brief Marshals value address as marshalled type into message
   */
  static void set(const DBusWrapper::MessageIterPtr& iter, const Dali::Accessibility::Address& address)
  {
    if(address)
    {
      signature<subtype>::set(iter, {address.GetBus(), ObjectPath{std::string{ATSPI_PREFIX_PATH} + address.GetPath()}});
    }
    else
    {
      signature<subtype>::set(iter, {address.GetBus(), ObjectPath{ATSPI_NULL_PATH}});
    }
  }

  /**
   * @brief Marshals value from marshalled type into variable address
   */
  static bool get(const DBusWrapper::MessageIterPtr& iter, Dali::Accessibility::Address& address)
  {
    subtype tmp;
    if(!signature<subtype>::get(iter, tmp))
    {
      return false;
    }

    if(tmp.second.value == ATSPI_NULL_PATH)
    {
      address = {};
      return true;
    }
    if(tmp.second.value.substr(0, strlen(ATSPI_PREFIX_PATH)) != ATSPI_PREFIX_PATH)
    {
      return false;
    }

    address = {std::move(tmp.first), tmp.second.value.substr(strlen(ATSPI_PREFIX_PATH))};
    return true;
  }
};

template<typename T>
struct SignatureAccessibleImpl : signature_helper<SignatureAccessibleImpl<T>>
{
  using subtype = std::pair<std::string, ObjectPath>;

  static constexpr auto name_v = concat("AtspiAccessiblePtr");
  static constexpr auto sig_v  = concat("(so)");

  /**
   * @brief Marshals value address as marshalled type into message
   */
  static void set(const DBusWrapper::MessageIterPtr& iter, T* accessible)
  {
    if(accessible)
    {
      auto address = accessible->GetAddress();
      signature<subtype>::set(iter, {address.GetBus(), ObjectPath{std::string{ATSPI_PREFIX_PATH} + address.GetPath()}});
    }
    else
    {
      signature<subtype>::set(iter, {"", ObjectPath{ATSPI_NULL_PATH}});
    }
  }

  /**
   * @brief Marshals value from marshalled type into variable path
   */
  static bool get(const DBusWrapper::MessageIterPtr& iter, T*& path)
  {
    subtype tmp;
    if(!signature<subtype>::get(iter, tmp))
    {
      return false;
    }

    if(tmp.second.value == ATSPI_NULL_PATH)
    {
      path = nullptr;
      return true;
    }

    if(tmp.second.value.substr(0, strlen(ATSPI_PREFIX_PATH)) != ATSPI_PREFIX_PATH)
    {
      return false;
    }

    auto currentBridge = CurrentBridgePtr::GetCurrentBridge();
    if(currentBridge->GetBusName() != tmp.first)
    {
      return false;
    }

    path = currentBridge->FindByPath(tmp.second.value.substr(strlen(ATSPI_PREFIX_PATH)));
    return path != nullptr;
  }
};

template<>
struct signature<Dali::Accessibility::Accessible*> : public SignatureAccessibleImpl<Dali::Accessibility::Accessible>
{
};

template<>
struct signature<Dali::Accessibility::States> : signature_helper<signature<Dali::Accessibility::States>>
{
  using subtype = std::array<uint32_t, 2>;

  static constexpr auto name_v = signature<subtype>::name_v;
  static constexpr auto sig_v  = signature<subtype>::sig_v;

  /**
   * @brief Marshals value state as marshalled type into message
   */
  static void set(const DBusWrapper::MessageIterPtr& iter, const Dali::Accessibility::States& states)
  {
    signature<subtype>::set(iter, states.GetRawData());
  }

  /**
   * @brief Marshals value from marshalled type into variable state
   */
  static bool get(const DBusWrapper::MessageIterPtr& iter, Dali::Accessibility::States& state)
  {
    subtype tmp;
    if(!signature<subtype>::get(iter, tmp))
    {
      return false;
    }
    state = Dali::Accessibility::States{tmp};
    return true;
  }
};
} // namespace detail
} // namespace DBus

struct _Logger
{
  const char*        mFile;
  int                mLine;
  std::ostringstream mTmp;

  _Logger(const char* file, int line)
  : mFile(file),
    mLine(line)
  {
  }

  ~_Logger()
  {
    Dali::Integration::Log::LogMessage(Dali::Integration::Log::DebugInfo, "%s:%d: %s", mFile, mLine, mTmp.str().c_str());
  }

  template<typename T>
  _Logger& operator<<(T&& t)
  {
    mTmp << std::forward<T>(t);
    return *this;
  }
};

struct _LoggerEmpty
{
  template<typename T>
  _LoggerEmpty& operator<<(T&& t)
  {
    return *this;
  }
};

struct _LoggerScope
{
  const char* mFile;
  int         mLine;

  _LoggerScope(const char* file, int line)
  : mFile(file),
    mLine(line)
  {
    Dali::Integration::Log::LogMessage(Dali::Integration::Log::DebugInfo, "%s:%d: +", mFile, mLine);
  }

  ~_LoggerScope()
  {
    Dali::Integration::Log::LogMessage(Dali::Integration::Log::DebugInfo, "%s:%d: -", mFile, mLine);
  }
};

#define LOG() _Logger(__FILE__, __LINE__)
#define SCOPE() _LoggerScope _l##__LINE__(__FILE__, __LINE__)

#endif // DALI_INTERNAL_ATSPI_ACCESSIBILITY_COMMON_H
