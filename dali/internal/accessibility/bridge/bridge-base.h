#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-version.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/public-api/actors/layer.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/accessibility/bridge/accessibility-common.h>

/**
 * @brief The AppAccessible class is to define Accessibility Application.
 */
class AppAccessible : public virtual Dali::Accessibility::Accessible, public virtual Dali::Accessibility::Collection, public virtual Dali::Accessibility::Application
{
public:
  Dali::Accessibility::EmptyAccessibleWithAddress mParent;
  std::vector<Dali::Accessibility::Accessible*>   mChildren;
  std::string                                     mName;

  std::string GetName() override
  {
    return mName;
  }

  std::string GetDescription() override
  {
    return "";
  }

  Dali::Accessibility::Accessible* GetParent() override
  {
    return &mParent;
  }

  size_t GetChildCount() override
  {
    return mChildren.size();
  }

  Dali::Accessibility::Accessible* GetChildAtIndex(size_t index) override
  {
    auto size = mChildren.size();
    if(index >= size)
    {
      throw std::domain_error{"invalid index " + std::to_string(index) + " for object with " + std::to_string(size) + " children"};
    }
    return mChildren[index];
  }

  size_t GetIndexInParent() override
  {
    throw std::domain_error{"can't call GetIndexInParent on application object"};
  }

  Dali::Accessibility::Role GetRole() override
  {
    return Dali::Accessibility::Role::APPLICATION;
  }

  Dali::Accessibility::States GetStates() override
  {
    return {};
  }

  Dali::Accessibility::Attributes GetAttributes() override
  {
    return {};
  }

  /**
   * @brief Gets the Accessible object from the window.
   *
   * @param[in] window The window to find
   * @return Null if mChildren is empty, otherwise the Accessible object
   * @note Currently, the default window would be returned when mChildren is not empty.
   */
  Dali::Accessibility::Accessible* GetWindowAccessible(Dali::Window window)
  {
    if(mChildren.empty())
    {
      return nullptr;
    }

    Dali::Layer rootLayer = window.GetRootLayer();

    // Find a child which is related to the window.
    for(auto i = 0u; i < mChildren.size(); ++i)
    {
      if(rootLayer == mChildren[i]->GetInternalActor())
      {
        return mChildren[i];
      }
    }

    // If can't find its children, return the default window.
    return mChildren[0];
  }

  bool DoGesture(const Dali::Accessibility::GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Dali::Accessibility::Relation> GetRelationSet() override
  {
    return {};
  }

  Dali::Actor GetInternalActor() override
  {
    return Dali::Actor{};
  }

  Dali::Accessibility::Address GetAddress() override
  {
    return {"", "root"};
  }

  std::string GetToolkitName() override
  {
    return {"dali"};
  }

  std::string GetVersion() override
  {
    return std::to_string(Dali::ADAPTOR_MAJOR_VERSION) + "." + std::to_string(Dali::ADAPTOR_MINOR_VERSION);
  }
};

/**
 * @brief Enumeration for FilteredEvents.
 */
enum class FilteredEvents
{
  BOUNDS_CHANGED ///< Bounds changed
};

// Custom specialization of std::hash
namespace std
{
template<>
struct hash<std::pair<FilteredEvents, Dali::Accessibility::Accessible*>>
{
  size_t operator()(std::pair<FilteredEvents, Dali::Accessibility::Accessible*> value) const
  {
    return (static_cast<size_t>(value.first) * 131) ^ reinterpret_cast<size_t>(value.second);
  }
};
} // namespace std

/**
 * @brief The BridgeBase class is basic class for Bridge functions.
 */
class BridgeBase : public Dali::Accessibility::Bridge, public Dali::ConnectionTracker
{
  std::unordered_map<std::pair<FilteredEvents, Dali::Accessibility::Accessible*>, std::pair<unsigned int, std::function<void()>>> mFilteredEvents;

  /**
   * @brief Removes all FilteredEvents using Tick signal.
   *
   * @return False if mFilteredEvents is empty, otherwise true.
   */
  bool TickFilteredEvents();

public:
  /**
   * @brief Adds FilteredEvents, Accessible, and delay time to mFilteredEvents.
   *
   * @param[in] kind FilteredEvents enum value
   * @param[in] obj Accessible object
   * @param[in] delay The delay time
   * @param[in] functor The function to be called // NEED TO UPDATE!
   */
  void AddFilteredEvent(FilteredEvents kind, Dali::Accessibility::Accessible* obj, float delay, std::function<void()> functor);

  /**
   * @brief Callback when the visibility of the window is changed.
   *
   * @param[in] window The window to be changed
   * @param[in] visible The visibility of the window
   */
  void OnWindowVisibilityChanged(Dali::Window window, bool visible);

  /**
   * @brief Callback when the window focus is changed.
   *
   * @param[in] window The window whose focus is changed
   * @param[in] focusIn Whether the focus is in/out
   */
  void OnWindowFocusChanged(Dali::Window window, bool focusIn);

  /**
   * @copydoc Dali::Accessibility::Bridge::GetBusName()
   */
  const std::string& GetBusName() const override;

  /**
   * @copydoc Dali::Accessibility::Bridge::AddTopLevelWindow()
   */
  void AddTopLevelWindow(Dali::Accessibility::Accessible* windowAccessible) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::RemoveTopLevelWindow()
   */
  void RemoveTopLevelWindow(Dali::Accessibility::Accessible* windowAccessible) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::AddPopup()
   */
  void AddPopup(Dali::Accessibility::Accessible* object) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::RemovePopup()
   */
  void RemovePopup(Dali::Accessibility::Accessible* object) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::GetApplication()
   */
  Dali::Accessibility::Accessible* GetApplication() const override
  {
    return &mApplication;
  }

  /**
   * @brief Adds function to dbus interface.
   */
  template<typename SELF, typename... RET, typename... ARGS>
  void AddFunctionToInterface(
    DBus::DBusInterfaceDescription& desc, const std::string& funcName, DBus::ValueOrError<RET...> (SELF::*funcPtr)(ARGS...))
  {
    if(auto self = dynamic_cast<SELF*>(this))
      desc.addMethod<DBus::ValueOrError<RET...>(ARGS...)>(
        funcName,
        [=](ARGS... args) -> DBus::ValueOrError<RET...> {
          try
          {
            return (self->*funcPtr)(std::move(args)...);
          }
          catch(std::domain_error& e)
          {
            return DBus::Error{e.what()};
          }
        });
  }

  /**
   * @brief Adds 'Get' property to dbus interface.
   */
  template<typename T, typename SELF>
  void AddGetPropertyToInterface(DBus::DBusInterfaceDescription& desc,
                                 const std::string&              funcName,
                                 T (SELF::*funcPtr)())
  {
    if(auto self = dynamic_cast<SELF*>(this))
      desc.addProperty<T>(funcName,
                          [=]() -> DBus::ValueOrError<T> {
                            try
                            {
                              return (self->*funcPtr)();
                            }
                            catch(std::domain_error& e)
                            {
                              return DBus::Error{e.what()};
                            }
                          },
                          {});
  }

  /**
   * @brief Adds 'Set' property to dbus interface.
   */
  template<typename T, typename SELF>
  void AddSetPropertyToInterface(DBus::DBusInterfaceDescription& desc,
                                 const std::string&              funcName,
                                 void (SELF::*funcPtr)(T))
  {
    if(auto self = dynamic_cast<SELF*>(this))
      desc.addProperty<T>(funcName, {}, [=](T t) -> DBus::ValueOrError<void> {
        try
        {
          (self->*funcPtr)(std::move(t));
          return {};
        }
        catch(std::domain_error& e)
        {
          return DBus::Error{e.what()};
        }
      });
  }

  /**
   * @brief Adds 'Set' and 'Get' properties to dbus interface.
   */
  template<typename T, typename T1, typename SELF>
  void AddGetSetPropertyToInterface(DBus::DBusInterfaceDescription& desc,
                                    const std::string&              funcName,
                                    T1 (SELF::*funcPtrGet)(),
                                    DBus::ValueOrError<void> (SELF::*funcPtrSet)(T))
  {
    if(auto self = dynamic_cast<SELF*>(this))
      desc.addProperty<T>(
        funcName,
        [=]() -> DBus::ValueOrError<T> {
          try
          {
            return (self->*funcPtrGet)();
          }
          catch(std::domain_error& e)
          {
            return DBus::Error{e.what()};
          }
        },
        [=](T t) -> DBus::ValueOrError<void> {
          try
          {
            (self->*funcPtrSet)(std::move(t));
            return {};
          }
          catch(std::domain_error& e)
          {
            return DBus::Error{e.what()};
          }
        });
  }

  /**
   * @brief Adds 'Get' and 'Set' properties to dbus interface.
   */
  template<typename T, typename T1, typename SELF>
  void AddGetSetPropertyToInterface(DBus::DBusInterfaceDescription& desc,
                                    const std::string&              funcName,
                                    T1 (SELF::*funcPtrGet)(),
                                    void (SELF::*funcPtrSet)(T))
  {
    if(auto self = dynamic_cast<SELF*>(this))
      desc.addProperty<T>(
        funcName,
        [=]() -> DBus::ValueOrError<T> {
          try
          {
            return (self->*funcPtrGet)();
          }
          catch(std::domain_error& e)
          {
            return DBus::Error{e.what()};
          }
        },
        [=](T t) -> DBus::ValueOrError<void> {
          try
          {
            (self->*funcPtrSet)(std::move(t));
            return {};
          }
          catch(std::domain_error& e)
          {
            return DBus::Error{e.what()};
          }
        });
  }

  /**
   * @brief Gets the string of the path excluding the specified prefix.
   *
   * @param path The path to get
   * @return The string stripped of the specific prefix
   */
  static std::string StripPrefix(const std::string& path);

  /**
   * @brief Finds the Accessible object according to the path.
   *
   * @param[in] path The path for Accessible object
   * @return The Accessible object corresponding to the path
   */
  Dali::Accessibility::Accessible* Find(const std::string& path) const;

  /**
   * @brief Finds the Accessible object with the given address.
   *
   * @param[in] ptr The unique Address of the object
   * @return The Accessible object corresponding to the path
   */
  Dali::Accessibility::Accessible* Find(const Dali::Accessibility::Address& ptr) const;

  /**
   * @brief Returns the target object of the currently executed DBus method call.
   *
   * And any subclasses redefine `FindSelf` with a different return type as a convenient wrapper around dynamic_cast.
   * @return The Accessible object
   * @note When a DBus method is called on some object, this target object (`currentObject`) is temporarily saved by the bridge,
   * because DBus handles the invocation target separately from the method arguments.
   * We then use the saved object inside the 'glue' method (e.g. BridgeValue::GetMinimum)
   * to call the equivalent method on the respective C++ object (this could be ScrollBar::AccessibleImpl::GetMinimum in the example given).
   */
  Dali::Accessibility::Accessible* FindSelf() const;

  /**
   * @copydoc Dali::Accessibility::Bridge::FindByPath()
   */
  Dali::Accessibility::Accessible* FindByPath(const std::string& name) const override;

  /**
   * @copydoc Dali::Accessibility::Bridge::SetApplicationName()
   */
  void SetApplicationName(std::string name) override
  {
    mApplication.mName = std::move(name);
  }

protected:
  mutable AppAccessible                         mApplication;
  std::vector<Dali::Accessibility::Accessible*> mPopups;
  bool                                          mIsScreenReaderSuppressed = false;

private:

  /**
   * @brief Sets an ID.
   * @param[in] id An ID (integer value)
   */
  void SetId(int id);

  /**
   * @brief Gets the ID.
   * @return The ID to be set
   */
  int GetId();

  /**
   * @brief Update registered events.
   */
  void UpdateRegisteredEvents();

  using CacheElementType = std::tuple<
    Dali::Accessibility::Address,
    Dali::Accessibility::Address,
    Dali::Accessibility::Address,
    std::vector<Dali::Accessibility::Address>,
    std::vector<std::string>,
    std::string,
    Dali::Accessibility::Role,
    std::string,
    std::array<uint32_t, 2>>;

  /**
   * @brief Gets Items  // NEED TO UPDATE!
   *
   * @return
   */
  DBus::ValueOrError<std::vector<CacheElementType>> GetItems();

  /**
   * @brief Creates CacheElement.
   *
   * CreateCacheElement method works for GetItems which is a part of ATSPI protocol.
   * ATSPI client library (libatspi from at-spi2-core) depending on cacheing policy configuration uses GetItems
   * to pre-load entire accessible tree from application to its own cache in single dbus call.
   * Otherwise the particular nodes in a tree are cached lazily when client library tries to access them.
   * @param item Accessible to get information
   * @return The elements to be cached
   */
  CacheElementType CreateCacheElement(Dali::Accessibility::Accessible* item);

protected:
  BridgeBase();
  virtual ~BridgeBase();

  /**
   * @copydoc Dali::Accessibility::Bridge::ForceUp()
   */
  ForceUpResult ForceUp() override;

  /**
   * @copydoc Dali::Accessibility::Bridge::ForceDown()
   */
  void ForceDown() override;

  DBus::DBusServer           mDbusServer;
  DBusWrapper::ConnectionPtr mConnectionPtr;
  int                        mId = 0;
  DBus::DBusClient           mRegistry;
  bool                       IsBoundsChangedEventAllowed{false};
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H
