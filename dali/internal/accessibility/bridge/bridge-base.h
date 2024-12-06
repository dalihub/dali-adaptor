#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/dali-adaptor-version.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <memory>
#include <tuple>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/actor-accessible.h>
#include <dali/devel-api/adaptor-framework/proxy-accessible.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/application.h>
#include <dali/devel-api/atspi-interfaces/collection.h>
#include <dali/devel-api/atspi-interfaces/socket.h>
#include <dali/internal/accessibility/bridge/accessibility-common.h>

/**
 * @brief The ApplicationAccessible class is to define Accessibility Application.
 */
class ApplicationAccessible : public virtual Dali::Accessibility::Accessible,
                              public virtual Dali::Accessibility::Application,
                              public virtual Dali::Accessibility::Collection,
                              public virtual Dali::Accessibility::Component,
                              public virtual Dali::Accessibility::Socket
{
public:
  Dali::Accessibility::ProxyAccessible          mParent;
  std::vector<Dali::Accessibility::Accessible*> mChildren;
  std::string                                   mName;
  std::string                                   mToolkitName{"dali"};
  bool                                          mIsEmbedded{false};
  bool                                          mShouldIncludeHidden{false};

  std::string GetName() const override
  {
    return mName;
  }

  std::string GetDescription() const override
  {
    return {};
  }

  std::string GetValue() const override
  {
    return {};
  }

  Dali::Accessibility::Accessible* GetParent() override
  {
    return &mParent;
  }

  size_t GetChildCount() const override
  {
    return mChildren.size();
  }

  std::vector<Dali::Accessibility::Accessible*> GetChildren() override
  {
    return mChildren;
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
    if(mIsEmbedded)
    {
      return 0u;
    }

    throw std::domain_error{"can't call GetIndexInParent on application object"};
  }

  Dali::Accessibility::Role GetRole() const override
  {
    return Dali::Accessibility::Role::APPLICATION;
  }

  Dali::Accessibility::States GetStates() override
  {
    Dali::Accessibility::States result;

    for(auto* child : mChildren)
    {
      result = result | child->GetStates();
    }

    // The Application object should never have the SENSITIVE state
    result[Dali::Accessibility::State::SENSITIVE] = false;

    return result;
  }

  Dali::Accessibility::Attributes GetAttributes() const override
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
  Dali::Accessibility::ActorAccessible* GetWindowAccessible(Dali::Window window)
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
        return dynamic_cast<Dali::Accessibility::ActorAccessible*>(mChildren[i]);
      }
    }

    // If can't find its children, return the default window.
    return dynamic_cast<Dali::Accessibility::ActorAccessible*>(mChildren[0]);
  }

  bool DoGesture(const Dali::Accessibility::GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Dali::Accessibility::Relation> GetRelationSet() override
  {
    return {};
  }

  Dali::Actor GetInternalActor() const override
  {
    return Dali::Actor{};
  }

  Dali::Accessibility::Address GetAddress() const override
  {
    return {"", "root"};
  }

  // Application

  std::string GetToolkitName() const override
  {
    return mToolkitName;
  }

  std::string GetVersion() const override
  {
    return std::to_string(Dali::ADAPTOR_MAJOR_VERSION) + "." + std::to_string(Dali::ADAPTOR_MINOR_VERSION);
  }

  bool GetIncludeHidden() const override
  {
    return mShouldIncludeHidden;
  }

  bool SetIncludeHidden(bool includeHidden) override
  {
    if(mShouldIncludeHidden != includeHidden)
    {
      mShouldIncludeHidden = includeHidden;
      return true;
    }
    return false;
  }

  // Socket

  Dali::Accessibility::Address Embed(Dali::Accessibility::Address plug) override
  {
    mIsEmbedded = true;
    mParent.SetAddress(plug);

    return GetAddress();
  }

  void Unembed(Dali::Accessibility::Address plug) override
  {
    if(mParent.GetAddress() == plug)
    {
      mIsEmbedded = false;
      mParent.SetAddress({});
      Dali::Accessibility::Bridge::GetCurrentBridge()->SetExtentsOffset(0, 0);
    }
  }

  void SetOffset(std::int32_t x, std::int32_t y) override
  {
    if(!mIsEmbedded)
    {
      return;
    }

    Dali::Accessibility::Bridge::GetCurrentBridge()->SetExtentsOffset(x, y);
  }

  // Component

  Dali::Rect<> GetExtents(Dali::Accessibility::CoordinateType type) const override
  {
    using limits = std::numeric_limits<float>;

    float minX = limits::max();
    float minY = limits::max();
    float maxX = limits::min();
    float maxY = limits::min();

    for(Dali::Accessibility::Accessible* child : mChildren)
    {
      auto* component = Dali::Accessibility::Component::DownCast(child);
      if(!component)
      {
        continue;
      }

      auto extents = component->GetExtents(type);

      minX = std::min(minX, extents.x);
      minY = std::min(minY, extents.y);
      maxX = std::max(maxX, extents.x + extents.width);
      maxY = std::max(maxY, extents.y + extents.height);
    }

    return {minX, minY, maxX - minX, maxY - minY};
  }

  Dali::Accessibility::ComponentLayer GetLayer() const override
  {
    return Dali::Accessibility::ComponentLayer::WINDOW;
  }

  std::int16_t GetMdiZOrder() const override
  {
    return 0;
  }

  bool GrabFocus() override
  {
    return false;
  }

  double GetAlpha() const override
  {
    return 0.0;
  }

  bool GrabHighlight() override
  {
    return false;
  }

  bool ClearHighlight() override
  {
    return false;
  }

  bool IsScrollable() const override
  {
    return false;
  }
};

/**
 * @brief Enumeration for CoalescableMessages.
 */
enum class CoalescableMessages
{
  BOUNDS_CHANGED,                                     ///< Bounds changed
  SET_OFFSET,                                         ///< Set offset
  POST_RENDER,                                        ///< Post render
  STATE_CHANGED_BEGIN = 500,                          ///< State changed (begin of reserved range)
  STATE_CHANGED_END   = STATE_CHANGED_BEGIN + 99,     ///< State changed (end of reserved range)
  PROPERTY_CHANGED_BEGIN,                             ///< Property changed (begin of reserved range)
  PROPERTY_CHANGED_END = PROPERTY_CHANGED_BEGIN + 99, ///< Property changed (end of reserved range)
};

// Custom specialization of std::hash
namespace std
{
template<>
struct hash<std::pair<CoalescableMessages, Dali::Accessibility::Accessible*>>
{
  size_t operator()(std::pair<CoalescableMessages, Dali::Accessibility::Accessible*> value) const
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
  std::unordered_map<std::pair<CoalescableMessages, Dali::Accessibility::Accessible*>, std::tuple<unsigned int, unsigned int, std::function<void()>>> mCoalescableMessages;

  /**
   * @brief Removes all CoalescableMessages using Tick signal.
   *
   * @return False if mCoalescableMessages is empty, otherwise true.
   */
  bool TickCoalescableMessages();

public:
  /**
   * @brief Adds CoalescableMessages, Accessible, and delay time to mCoalescableMessages.
   *
   * @param[in] kind CoalescableMessages enum value
   * @param[in] obj Accessible object
   * @param[in] delay The delay time
   * @param[in] functor The function to be called // NEED TO UPDATE!
   */
  void AddCoalescableMessage(CoalescableMessages kind, Dali::Accessibility::Accessible* obj, float delay, std::function<void()> functor);

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
   * @copydoc Dali::Accessibility::Bridge::RegisterDefaultLabel()
   */
  void RegisterDefaultLabel(Dali::Actor actor) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::UnregisterDefaultLabel()
   */
  void UnregisterDefaultLabel(Dali::Actor actor) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::GetDefaultLabel()
   */
  Dali::Accessibility::Accessible* GetDefaultLabel(Dali::Accessibility::Accessible* root) override;

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
   * @return The Accessible object
   * @note When a DBus method is called on some object, this target object (`currentObject`) is temporarily saved by the bridge,
   * because DBus handles the invocation target separately from the method arguments.
   * We then use the saved object inside the 'glue' method (e.g. BridgeValue::GetMinimum)
   * to call the equivalent method on the respective C++ object (this could be ScrollBar::AccessibleImpl::GetMinimum in the example given).
   */
  Dali::Accessibility::Accessible* FindCurrentObject() const;

  /**
   * @brief Returns the target object of the currently executed DBus method call.
   *
   * This method tries to downcast the return value of FindCurrentObject() to the requested type,
   * issuing an error reply to the DBus caller if the requested type is not implemented. Whether
   * a given type is implemented is decided based on the return value of Accessible::GetInterfaces()
   * for the current object.
   *
   * @tparam I The requested AT-SPI interface
   * @return The Accessible object (cast to a more derived type)
   *
   * @see FindCurrentObject()
   * @see Dali::Accessibility::AtspiInterface
   * @see Dali::Accessibility::AtspiInterfaceType
   * @see Dali::Accessibility::Accessible::GetInterfaces()
   */
  template<Dali::Accessibility::AtspiInterface I>
  auto* FindCurrentObjectWithInterface() const
  {
    using Type = Dali::Accessibility::AtspiInterfaceType<I>;

    Type* result;
    auto* currentObject = FindCurrentObject();
    DALI_ASSERT_DEBUG(currentObject); // FindCurrentObject() throws domain_error

    if(!(result = Dali::Accessibility::Accessible::DownCast<I>(currentObject)))
    {
      std::stringstream s;

      s << "Object " << currentObject->GetAddress().ToString();
      s << " does not implement ";
      s << Dali::Accessibility::Accessible::GetInterfaceName(I);

      throw std::domain_error{s.str()};
    }

    return result;
  }

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

  /**
   * @copydoc Dali::Accessibility::Bridge::SetToolkitName()
   */
  void SetToolkitName(std::string_view toolkitName) override
  {
    mApplication.mToolkitName = std::string{toolkitName};
  }

protected:
  // We use a weak handle in order not to keep a window alive forever if someone forgets to UnregisterDefaultLabel()
  using DefaultLabelType  = std::pair<Dali::WeakHandle<Dali::Window>, Dali::WeakHandle<Dali::Actor>>;
  using DefaultLabelsType = std::list<DefaultLabelType>;

  mutable ApplicationAccessible mApplication;
  DefaultLabelsType             mDefaultLabels;
  bool                          mIsScreenReaderSuppressed = false;

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

  /**
   * @brief Removes expired elements from the default label collection.
   */
  void CompressDefaultLabels();

  /**
   * @brief Gets the window to which this actor belongs (or an empty handle).
   *
   * @param actor The actor
   * @return The window
   */
  static Dali::WeakHandle<Dali::Window> GetWindow(Dali::Actor actor);

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
