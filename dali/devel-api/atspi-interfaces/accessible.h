#ifndef DALI_ADAPTOR_ATSPI_ACCESSIBLE_H
#define DALI_ADAPTOR_ATSPI_ACCESSIBLE_H

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/object-registry.h>
#include <cstdint>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>

namespace Dali::Accessibility
{
/**
 * @brief Basic interface implemented by all accessibility objects.
 */
class DALI_ADAPTOR_API Accessible
{
public:
  virtual ~Accessible() noexcept;

  using utf8_t = unsigned char;

  /**
   * @brief Calculates and finds word boundaries in given utf8 text.
   *
   * @param[in] string The source text to find
   * @param[in] length The length of text to find
   * @param[in] language The language to use
   * @param[out] breaks The word boundaries in given text
   *
   * @note Word boundaries are returned as non-zero values in table breaks, which must be of size at least length.
   */
  static void FindWordSeparationsUtf8(const utf8_t* string, std::size_t length, const char* language, char* breaks);

  /**
   * @brief Calculates and finds line boundaries in given utf8 text.
   *
   * @param[in] string The source text to find
   * @param[in] length The length of text to find
   * @param[in] language The language to use
   * @param[out] breaks The line boundaries in given text
   *
   * @note Line boundaries are returned as non-zero values in table breaks, which must be of size at least length.
   */
  static void FindLineSeparationsUtf8(const utf8_t* string, std::size_t length, const char* language, char* breaks);

  /**
   * @brief Helper function for emiting active-descendant-changed event.
   *
   * @param[in] child The child of the object
   */
  void EmitActiveDescendantChanged(Accessible* child);

  /**
   * @brief Helper function for emiting state-changed event.
   *
   * @param[in] state The accessibility state (SHOWING, HIGHLIGHTED, etc)
   * @param[in] newValue Whether the state value is changed to new value or not.
   * @param[in] reserved Reserved. (TODO : Currently, this argument is not implemented in dali)
   *
   * @note The second argument determines which value is depending on State.
   * For instance, if the state is PRESSED, newValue means isPressed or isSelected.
   * If the state is SHOWING, newValue means isShowing.
   */
  void EmitStateChanged(State state, int newValue, int reserved = 0);

  /**
   * @brief Helper function for emiting bounds-changed event.
   *
   * @param rect The rectangle for changed bounds
   */
  void EmitBoundsChanged(Rect<> rect);

  /**
   * @brief Emits "showing" event.
   * The method informs accessibility clients about "showing" state.
   *
   * @param[in] isShowing The flag pointing if object is showing
   */
  void EmitShowing(bool isShowing);

  /**
   * @brief Emits "visible" event.
   * The method informs accessibility clients about "visible" state.
   *
   * @param[in] isVisible The flag pointing if object is visible
   */
  void EmitVisible(bool isVisible);

  /**
   * @brief Emits "highlighted" event.
   * The method informs accessibility clients about "highlighted" state.
   *
   * @param[in] isHighlighted The flag pointing if object is highlighted
   */
  void EmitHighlighted(bool isHighlighted);

  /**
   * @brief Emits "focused" event.
   * The method informs accessibility clients about "focused" state.
   *
   * @param[in] isFocused The flag pointing if object is focused
   */
  void EmitFocused(bool isFocused);

  /**
   * @brief Emits "text inserted" event.
   *
   * @param[in] position The cursor position
   * @param[in] length The text length
   * @param[in] content The inserted text
   */
  void EmitTextInserted(unsigned int position, unsigned int length, const std::string& content);

  /**
   * @brief Emits "text deleted" event.
   *
   * @param[in] position The cursor position
   * @param[in] length The text length
   * @param[in] content The deleted text
   */
  void EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content);

  /**
   * @brief Emits "cursor moved" event.
   *
   * @param[in] cursorPosition The new cursor position
   */
  void EmitTextCursorMoved(unsigned int cursorPosition);

  /**
   * @brief Emits "MoveOuted" event.
   *
   * @param[in] type moved out of screen type
   */
  void EmitMovedOutOfScreen(ScreenRelativeMoveType type);

  /**
   * @brief Emits "highlighted" event.
   *
   * @param[in] event The enumerated window event
   * @param[in] detail The additional parameter which interpretation depends on chosen event
   */
  void Emit(WindowEvent event, unsigned int detail = 0);

  /**
   * @brief Emits property-changed event.
   *
   * @param[in] event Property changed event
   **/
  void Emit(ObjectPropertyChangeEvent event);

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
   * @brief Re-emits selected states of an Accessibility Object.
   *
   * @param[in] states The chosen states to re-emit
   * @param[in] isRecursive If true, all children of the Accessibility object will also re-emit the states
   */
  void NotifyAccessibilityStateChange(Dali::Accessibility::States states, bool isRecursive);

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
   * @brief Gets internal Actor to be saved before.
   *
   * @return The internal Actor
   */
  virtual Dali::Actor GetInternalActor() = 0;

  /**
   * @brief Gets all implemented interfaces.
   *
   * @return The collection of strings with implemented interfaces
   */
  std::vector<std::string> GetInterfaces() const;

  /**
   * @brief Checks if object is on root level.
   *
   * @return Whether object is on root level or not
   */
  bool IsOnRootLevel() const
  {
    return mIsOnRootLevel;
  }

protected:
  Accessible();
  Accessible(const Accessible&)         = delete;
  Accessible(Accessible&&)              = delete;
  Accessible&                   operator=(const Accessible&) = delete;
  Accessible&                   operator=(Accessible&&) = delete;
  std::shared_ptr<Bridge::Data> GetBridgeData() const;

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
   * @brief Sets ObjectRegistry.
   *
   * @param[in] registry ObjectRegistry instance
   */
  static void SetObjectRegistry(ObjectRegistry registry);

  /**
   * @brief The method registers functor resposible for converting Actor into Accessible.
   * @param functor The returning Accessible handle from Actor object
   */
  static void RegisterExternalAccessibleGetter(std::function<Accessible*(Dali::Actor)> functor);

  /**
   * @brief Acquires Accessible object from Actor object.
   *
   * @param[in] actor Actor object
   * @param[in] isRoot True, if it's top level object (window)
   *
   * @return The handle to Accessible object
   */
  static Accessible* Get(Dali::Actor actor, bool isRoot = false);

private:
  friend class Bridge;

  mutable std::weak_ptr<Bridge::Data> mBridgeData;
  bool                                mIsOnRootLevel = false;

}; // Accessible class

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_ACCESSIBLE_H