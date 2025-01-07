#ifndef DALI_ADAPTOR_ATSPI_ACCESSIBLE_H
#define DALI_ADAPTOR_ATSPI_ACCESSIBLE_H

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/object-registry.h>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/accessibility.h>

namespace Dali::Accessibility
{
/**
 * @brief Basic interface implemented by all accessibility objects.
 */
class DALI_ADAPTOR_API Accessible
{
public:
  virtual ~Accessible() noexcept = default;

  /**
   * @brief Gets accessibility name.
   *
   * @return The string with name
   */
  virtual std::string GetName() const = 0;

  /**
   * @brief Gets accessibility description.
   *
   * @return The string with description
   */
  virtual std::string GetDescription() const = 0;

  /**
   * @brief Gets accessibility value.
   *
   * @return The value text
   */
  virtual std::string GetValue() const = 0;

  /**
   * @brief Gets parent.
   *
   * @return The handler to accessibility object
   */
  virtual Accessible* GetParent() = 0;

  /**
   * @brief Gets the number of children.
   *
   * @return The number of children
   */
  virtual std::size_t GetChildCount() const = 0;

  /**
   * @brief Gets collection with all children.
   *
   * @return The collection of accessibility objects
   */
  virtual std::vector<Accessible*> GetChildren() = 0;

  /**
   * @brief Gets child of the index.
   *
   * @return The child object
   */
  virtual Accessible* GetChildAtIndex(std::size_t index) = 0;

  /**
   * @brief Gets index that current object has in its parent's children collection.
   *
   * @return The index of the current object
   */
  virtual std::size_t GetIndexInParent() = 0;

  /**
   * @brief Gets accessibility role.
   *
   * @return Role enumeration
   *
   * @see Dali::Accessibility::Role
   */
  virtual Role GetRole() const = 0;

  /**
   * @brief Gets name of accessibility role.
   *
   * @return The string with human readable role converted from enumeration
   *
   * @see Dali::Accessibility::Role
   * @see Accessibility::Accessible::GetRole
   */
  virtual std::string GetRoleName() const;

  /**
   * @brief Gets localized name of accessibility role.
   *
   * @return The string with human readable role translated according to current
   * translation domain
   *
   * @see Dali::Accessibility::Role
   * @see Accessibility::Accessible::GetRole
   * @see Accessibility::Accessible::GetRoleName
   *
   * @note translation is not supported in this version
   */
  virtual std::string GetLocalizedRoleName() const;

  /**
   * @brief Gets accessibility states.
   *
   * @return The collection of states
   *
   * @note States class is instatation of ArrayBitset template class
   *
   * @see Dali::Accessibility::State
   * @see Dali::Accessibility::ArrayBitset
   */
  virtual States GetStates() = 0;

  /**
   * @brief Gets accessibility attributes.
   *
   * @return The map of attributes and their values
   */
  virtual Attributes GetAttributes() const = 0;

  /**
   * @brief Checks if this is hidden.
   *
   * @return True if this is hidden
   *
   * @note Hidden means not present in the AT-SPI tree.
   */
  virtual bool IsHidden() const;

  /**
   * @brief Checks if this is proxy.
   *
   * @return True if this is proxy
   */
  virtual bool IsProxy() const;

  /**
   * @brief Checks if this is highlighted
   *
   * @return True if highlighted, false otherwise
   */
  bool IsHighlighted() const;

  /**
   * @brief Gets unique address on accessibility bus.
   *
   * @return The Address class containing address
   *
   * @see Dali::Accessibility::Address
   */
  virtual Address GetAddress() const;

  /**
   * @brief Deputes an object to perform provided gesture.
   *
   * @param[in] gestureInfo The structure describing the gesture
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::GestureInfo
   */
  virtual bool DoGesture(const GestureInfo& gestureInfo) = 0;

  /**
   * @brief Gets information about current object and all relations that connects
   * it with other accessibility objects.
   *
   * @return The iterable collection of Relation objects
   *
   * @see Dali::Accessibility::Relation
   */
  virtual std::vector<Relation> GetRelationSet() = 0;

  /**
   * @brief Gets the Actor associated with this Accessible (if there is one).
   *
   * @return The internal Actor
   */
  virtual Dali::Actor GetInternalActor() const = 0;

  /**
   * @brief Sets whether to listen for post render callback.
   *
   * @param[in] enabled If ture, registration post render callback, false otherwise
   */
  virtual void SetListenPostRender(bool enabled);

  /**
   * @brief Gets all implemented interfaces.
   *
   * Override DoGetInterfaces() to customize the return value of this method.
   *
   * @return The collection of implemented interfaces
   *
   * @see DoGetInterfaces()
   */
  AtspiInterfaces GetInterfaces() const;

  /**
   * @brief Gets all implemented interfaces.
   *
   * Converts all interfaces returned by GetInterfaces() to their DBus names
   * using GetInterfaceName().
   *
   * @return The collection of names of implemented interfaces
   *
   * @see GetInterfaces()
   * @see GetInterfaceName()
   */
  std::vector<std::string> GetInterfacesAsStrings() const;

  /**
   * @brief Checks if object is on root level.
   *
   * @return Whether object is on root level or not
   */
  bool IsOnRootLevel() const
  {
    return mIsOnRootLevel;
  }

  /**
   * @brief Gets all suppressed events.
   *
   * @return All suppressed events
   */
  AtspiEvents GetSuppressedEvents() const
  {
    return mSuppressedEvents;
  }

  /**
   * @brief Gets all suppressed events.
   *
   * @return All suppressed events
   */
  AtspiEvents& GetSuppressedEvents()
  {
    return mSuppressedEvents;
  }

  enum class DumpDetailLevel
  {
    DUMP_SHORT              = 0,
    DUMP_SHORT_SHOWING_ONLY = 1,
    DUMP_FULL               = 2,
    DUMP_FULL_SHOWING_ONLY  = 3,
  };

  /**
   * @brief Dumps tree structure of accessible objects starting from self.
   * @param [in] detailLevel Detail level of dumped json output.
   */
  std::string DumpTree(DumpDetailLevel detailLevel);

protected:
  Accessible()                          = default;
  Accessible(const Accessible&)         = delete;
  Accessible(Accessible&&)              = delete;
  Accessible&                   operator=(const Accessible&) = delete;
  Accessible&                   operator=(Accessible&&) = delete;
  std::shared_ptr<Bridge::Data> GetBridgeData() const;

  /**
   * @brief Returns the collection of AT-SPI interfaces implemented by this Accessible.
   *
   * This method is called only once and its return value is cached. The default implementation
   * uses dynamic_cast to determine which interfaces are implemented. Override this if you
   * conceptually provide fewer interfaces than dynamic_cast can see.
   *
   * @return The collection of implemented interfaces
   *
   * @see GetInterfaces()
   * @see GetInterfaceName()
   */
  virtual AtspiInterfaces DoGetInterfaces() const;

public:
  /**
   * @brief Gets the highlight actor.
   *
   * This method is to get the highlight itself.
   * @return The highlight actor
   */
  static Dali::Actor GetHighlightActor();

  /**
   * @brief Sets the highlight actor.
   *
   * This method is to set the highlight itself.
   * @param[in] actor The highlight actor
   */
  static void SetHighlightActor(Dali::Actor actor);

  /**
   * @brief Gets the currently highlighted actor.
   *
   * @return The current highlighted actor
   */
  static Dali::Actor GetCurrentlyHighlightedActor();

  /**
   * @brief Sets currently highlighted actor.
   *
   * @param[in] actor The highlight actor
   */
  static void SetCurrentlyHighlightedActor(Dali::Actor actor);

  /**
   * @brief The method registers functor resposible for converting Actor into Accessible.
   * @param functor The returning Accessible handle from Actor object
   */
  static void RegisterExternalAccessibleGetter(std::function<std::pair<std::shared_ptr<Accessible>, bool>(Dali::Actor)> functor);

  /**
   * @brief Acquires Accessible object from Actor object.
   *
   * @param[in] actor Actor object
   *
   * @return The raw pointer to Accessible object
   */
  static Accessible* Get(Dali::Actor actor);

  /**
   * @brief Acquires Accessible object from Actor object.
   *
   * @param[in] actor Actor object
   *
   * @return The owning pointer to Accessible object
   */
  static std::shared_ptr<Accessible> GetOwningPtr(Dali::Actor actor);

  /**
   * @brief Obtains the DBus interface name for the specified AT-SPI interface.
   *
   * @param interface AT-SPI interface identifier (e.g. AtspiInterface::ACCESSIBLE)
   * @return AT-SPI interface name (e.g. "org.a11y.atspi.Accessible")
   */
  static std::string GetInterfaceName(AtspiInterface interface);

  /**
   * @brief Downcasts an Accessible pointer to an AT-SPI interface pointer.
   *
   * @tparam I Desired AT-SPI interface
   *
   * @param obj Object to cast.
   *
   * @return Pointer to an AT-SPI interface or null if the interface is not implemented.
   */
  template<AtspiInterface I>
  static AtspiInterfaceType<I>* DownCast(Accessible* obj)
  {
    if(!obj || !obj->GetInterfaces()[I])
    {
      return nullptr;
    }

    return dynamic_cast<AtspiInterfaceType<I>*>(obj);
  }

private:
  friend class Bridge;

  mutable std::weak_ptr<Bridge::Data> mBridgeData;
  mutable AtspiInterfaces             mInterfaces;
  AtspiEvents                         mSuppressedEvents;
  bool                                mIsOnRootLevel = false;

}; // Accessible class

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::ACCESSIBLE>
{
  using Type = Accessible;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_ACCESSIBLE_H
