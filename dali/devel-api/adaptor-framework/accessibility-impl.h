#ifndef DALI_INTERNAL_ATSPI_ACCESSIBILITY_IMPL_H
#define DALI_INTERNAL_ATSPI_ACCESSIBILITY_IMPL_H

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
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/object-registry.h>
#include <atomic>
#include <bitset>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Accessibility
{
class DALI_ADAPTOR_API Accessible;
class DALI_ADAPTOR_API Text;
class DALI_ADAPTOR_API Value;
class DALI_ADAPTOR_API Component;
class DALI_ADAPTOR_API Collection;
class DALI_ADAPTOR_API Action;

/**
 * @brief Base class for different accessibility bridges.
 *
 * Bridge is resposible for initializing and managing connection on accessibility bus.
 * Accessibility clients will not get any information about UI without initialized and upraised bridge.
 * Concrete implementation depends on the accessibility technology available on the platform.
 *
 * @note This class is singleton.
 */
struct DALI_ADAPTOR_API Bridge
{
  enum class ForceUpResult
  {
    JUST_STARTED,
    ALREADY_UP
  };

  /**
   * @brief Destructor
   */
  virtual ~Bridge() = default;

  /**
   * @brief Gets bus name which bridge is initialized on.
   */
  virtual const std::string& GetBusName() const = 0;

  /**
   * @brief Registers top level window.
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method allows to connect chosen
   * object as direct ancestor of application and therefore make it visible for
   * accessibility clients.
   *
   * @param[in] object The accessible object
   */
  virtual void AddTopLevelWindow(Accessible* object) = 0;

  /**
   * @brief Removes top level window.
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method removes previously added
   * window from visible accessibility objects.
   *
   * @param[in] object The accessible object
   */
  virtual void RemoveTopLevelWindow(Accessible* object) = 0;

  /**
   * @brief Adds popup window.
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method adds new popup to the tree.
   *
   * @param[in] object The accessible object
   */
  virtual void AddPopup(Accessible* object) = 0;

  /**
   * @brief Removes popup window.
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method removes previously added
   * popup window.
   *
   * @param[in] object The accessible object
   */
  virtual void RemovePopup(Accessible* object) = 0;

  /**
   * @brief Sets name of current application which will be visible on accessibility bus.
   *
   * @param[in] name The application name
   */
  virtual void SetApplicationName(std::string name) = 0;

  /**
   * @brief Gets object being root of accessibility tree.
   *
   * @return handler to accessibility object
   */
  virtual Accessible* GetApplication() const = 0;

  /**
   * @brief Finds an object in accessibility tree.
   *
   * @param[in] path The path to object
   *
   * @return The handler to accessibility object
   */
  virtual Accessible* FindByPath(const std::string& path) const = 0;

  /**
   * @brief Shows application on accessibility bus.
   */
  virtual void ApplicationShown() = 0;

  /**
   * @brief Hides application on accessibility bus.
   */
  virtual void ApplicationHidden() = 0;

  /**
   * @brief Initializes accessibility bus.
   */
  virtual void Initialize() = 0;

  /**
   * @brief Terminates accessibility bus.
   */
  virtual void Terminate() = 0;

  /**
   * @brief This method is called, when bridge is being activated.
   */
  virtual ForceUpResult ForceUp()
  {
    if(mData)
    {
      return ForceUpResult::ALREADY_UP;
    }
    mData = std::make_shared<Data>();
    mData->mBridge = this;
    return ForceUpResult::JUST_STARTED;
  }

  /**
   * @brief This method is called, when bridge is being deactivated.
   */
  virtual void ForceDown() = 0;

  /**
   * @brief Checks if bridge is activated or not.
   * @return True if brige is activated.
   */
  bool IsUp() const
  {
    return bool(mData);
  }

  /**
   * @brief Emits cursor-moved event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] cursorPosition The new cursor position
   **/
  virtual void EmitCursorMoved(Accessible* obj, unsigned int cursorPosition) = 0;

  /**
   * @brief Emits active-descendant-changed event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] child The child of the object
   **/
  virtual void EmitActiveDescendantChanged(Accessible* obj, Accessible* child) = 0;

  /**
   * @brief Emits text-changed event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] state The changed state for text, such as Inserted or Deleted
   * @param[in] position The cursor position
   * @param[in] length The text length
   * @param[in] content The changed text
   **/
  virtual void EmitTextChanged(Accessible* obj, TextChangedState state, unsigned int position, unsigned int length, const std::string& content) = 0;

  /**
   * @brief Emits state-changed event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] state The accessibility state (SHOWING, HIGHLIGHTED, etc)
   * @param[in] newValue Whether the state value is changed to new value or not.
   * @param[in] reserved Reserved. (Currently, this argument is not implemented in dali)
   **/
  virtual void EmitStateChanged(Accessible* obj, State state, int newValue, int reserved = 0) = 0;

  /**
   * @brief Emits window event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] event The enumerated window event
   * @param[in] detail The additional parameter which interpretation depends on chosen event
   **/
  virtual void Emit(Accessible* obj, WindowEvent event, unsigned int detail = 0) = 0;

  /**
   * @brief Emits property-changed event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] event Property changed event
   **/
  virtual void Emit(Accessible* obj, ObjectPropertyChangeEvent event) = 0;

  /**
   * @brief Emits bounds-changed event on at-spi bus.
   *
   * @param[in] obj The accessible object
   * @param[in] rect The rectangle for changed bounds
   **/
  virtual void EmitBoundsChanged(Accessible* obj, Rect<> rect) = 0;

  /**
   * @brief Emits key event on at-spi bus.
   *
   * Screen-reader might receive this event and reply, that given keycode is consumed. In that case
   * further processing of the keycode should be ignored.
   *
   * @param[in] type Key event type
   * @param[in] keyCode Key code
   * @param[in] keyName Key name
   * @param[in] timeStamp Time stamp
   * @param[in] isText Whether it's text or not
   * @return Whether this event is consumed or not
   **/
  virtual Consumed Emit(KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText) = 0;

  /**
   * @brief Reads given text by screen reader
   *
   * @param[in] text The text to read
   * @param[in] discardable If TRUE, reading can be discarded by subsequent reading requests,
   * if FALSE the reading must finish before next reading request can be started
   * @param[in] callback the callback function that is called on reading signals emitted
   * during processing of this reading request.
   * Callback can be one of the following signals:
   * ReadingCancelled, ReadingStopped, ReadingSkipped
   */
  virtual void Say(const std::string& text, bool discardable, std::function<void(std::string)> callback) = 0;

  /**
   * @brief Force accessibility client to pause.
   */
  virtual void Pause() = 0;

  /**
   * @brief Force accessibility client to resume.
   */
  virtual void Resume() = 0;

  /**
   * @brief Cancels anything screen-reader is reading / has queued to read
   *
   * @param[in] alsoNonDiscardable whether to cancel non-discardable readings as well
   */
  virtual void StopReading(bool alsoNonDiscardable) = 0;

  /**
   * @brief Suppresses reading of screen-reader
   *
   * @param[in] suppress whether to suppress reading of screen-reader
   */
  virtual void SuppressScreenReader(bool suppress) = 0;

  /**
   * @brief Gets screen reader status.
   *
   * @return True if screen reader is enabled
   */
  virtual bool GetScreenReaderEnabled() = 0;

  /**
   * @brief Gets ATSPI status.
   *
   * @return True if ATSPI is enabled
   */
  virtual bool IsEnabled() = 0;

  /**
   * @brief Returns instance of bridge singleton object.
   *
   * @return The current bridge object
   **/
  static Bridge* GetCurrentBridge();

  /**
   * @brief Blocks auto-initialization of AT-SPI bridge
   *
   * Use this only if your application starts before DBus does, and call it early in main()
   * (before GetCurrentBridge() is called by anyone). GetCurrentBridge() will then return an
   * instance of DummyBridge.
   *
   * When DBus is ready, call EnableAutoInit(). Please note that GetCurrentBridge() may still
   * return an instance of DummyBridge if AT-SPI was disabled at compile time or using an
   * environment variable, or if creating the real bridge failed.
   *
   * @see Dali::Accessibility::DummyBridge
   * @see Dali::Accessibility::Bridge::EnableAutoInit
   */
  static void DisableAutoInit();

  /**
   * @brief Re-enables auto-initialization of AT-SPI bridge
   *
   * Normal applications do not have to call this function. GetCurrentBridge() tries to
   * initialize the AT-SPI bridge when it is called for the first time.
   *
   * @see Dali::Accessibility::Bridge::DisableAutoInit
   * @see Dali::Accessibility::Bridge::AddTopLevelWindow
   * @see Dali::Accessibility::Bridge::SetApplicationName
   */
  static void EnableAutoInit();

protected:
  struct Data
  {
    std::unordered_set<Accessible*> mKnownObjects;
    std::string                     mBusName;
    Bridge*                         mBridge = nullptr;
    Actor                           mHighlightActor;
    Actor                           mCurrentlyHighlightedActor;
  };
  std::shared_ptr<Data> mData;
  friend class Accessible;

  enum class AutoInitState
  {
    DISABLED,
    ENABLED
  };

  inline static AutoInitState autoInitState = AutoInitState::ENABLED;

  /**
   * @brief Registers accessible object to be known in bridge object.
   *
   * Bridge must known about all currently alive accessible objects, as some requst
   * might come and object will be identified by number id (it's memory address).
   * To avoid memory corruption number id is checked against set of known objects.
   *
   * @param[in] object The accessible object
   **/
  void RegisterOnBridge(Accessible* object);

  /**
   * @brief Tells bridge, that given object is considered root (doesn't have any parents).
   *
   * All root objects will have the same parent - application object. Application object
   * is controlled by bridge and private.
   *
   * @param[in] owner The accessible object
   **/
  void SetIsOnRootLevel(Accessible* owner);
};

/**
 * @brief Checks if ATSPI is activated or not.
 * @return True if ATSPI is activated.
 */
inline bool IsUp()
{
  if(Bridge::GetCurrentBridge() == nullptr)
  {
    return false;
  }

  if(Bridge::GetCurrentBridge()->IsEnabled() == false)
  {
    return false;
  }

  return Bridge::GetCurrentBridge()->IsUp();
}

/**
 * @brief Basic interface implemented by all accessibility objects.
 */
class Accessible
{
public:
  virtual ~Accessible();

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
  void FindWordSeparationsUtf8(const utf8_t* string, size_t length, const char* language, char* breaks);

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
  void FindLineSeparationsUtf8(const utf8_t* string, size_t length, const char* language, char* breaks);

  /**
   * @brief Helper function for emiting active-descendant-changed event.
   *
   * @param[in] obj The accessible object
   * @param[in] child The child of the object
   */
  void EmitActiveDescendantChanged(Accessible* obj, Accessible* child);

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
  virtual std::string GetName() = 0;

  /**
   * @brief Gets accessibility description.
   *
   * @return The string with description
   */
  virtual std::string GetDescription() = 0;

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
  virtual size_t GetChildCount() = 0;

  /**
   * @brief Gets collection with all children.
   *
   * @return The collection of accessibility objects
   */
  virtual std::vector<Accessible*> GetChildren();

  /**
   * @brief Gets child of the index.
   *
   * @return The child object
   */
  virtual Accessible* GetChildAtIndex(size_t index) = 0;

  /**
   * @brief Gets index that current object has in its parent's children collection.
   *
   * @return The index of the current object
   */
  virtual size_t GetIndexInParent() = 0;

  /**
   * @brief Gets accessibility role.
   *
   * @return Role enumeration
   *
   * @see Dali::Accessibility::Role
   */
  virtual Role GetRole() = 0;

  /**
   * @brief Gets name of accessibility role.
   *
   * @return The string with human readable role converted from enumeration
   *
   * @see Dali::Accessibility::Role
   * @see Accessibility::Accessible::GetRole
   */
  virtual std::string GetRoleName();

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
  virtual std::string GetLocalizedRoleName();

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
  virtual Attributes GetAttributes() = 0;

  /**
   * @brief Checks if this is proxy.
   *
   * @return True if this is proxy
   */
  virtual bool IsProxy();

  /**
   * @brief Gets unique address on accessibility bus.
   *
   * @return The Address class containing address
   *
   * @see Dali::Accessibility::Address
   */
  virtual Address GetAddress();

  /**
   * @brief Gets accessibility object, which is "default label" for this object.
   *
   * @return The Accessible object
   */
  virtual Accessible* GetDefaultLabel();

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
   * @brief Gets all implemented interfaces.
   *
   * @return The collection of strings with implemented interfaces
   */
  std::vector<std::string> GetInterfaces();

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
   * @brief The method registers functor resposible for converting Actor into Accessible.
   * @param functor The returning Accessible handle from Actor object
   */
  static void RegisterControlAccessibilityGetter(std::function<Accessible*(Dali::Actor)> functor);

  /**
   * @brief Acquires Accessible object from Actor object.
   *
   * @param[in] actor Actor object
   * @param[in] isRoot True, if it's top level object (window)
   *
   * @return The handle to Accessible object
   */
  static Accessible* Get(Dali::Actor actor, bool isRoot = false);

protected:
  Accessible();
  Accessible(const Accessible&)         = delete;
  Accessible(Accessible&&)              = delete;
  Accessible&                   operator=(const Accessible&) = delete;
  Accessible&                   operator=(Accessible&&) = delete;
  std::shared_ptr<Bridge::Data> GetBridgeData();

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

private:
  friend class Bridge;

  std::weak_ptr<Bridge::Data> mBridgeData;
  bool                        mIsOnRootLevel = false;
};

/**
 * @brief Interface enabling to perform provided actions.
 */
class Action : public virtual Accessible
{
public:
  /**
   * @brief Gets name of action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with name of action
   */
  virtual std::string GetActionName(size_t index) = 0;

  /**
   * @brief Gets translated name of action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with name of action translated according to current translation domain
   *
   * @note The translation is not supported in this version
   */
  virtual std::string GetLocalizedActionName(size_t index) = 0;

  /**
   * @brief Gets description of action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with description of action
   */
  virtual std::string GetActionDescription(size_t index) = 0;

  /**
   * @brief Gets key code binded to action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with key name
   */
  virtual std::string GetActionKeyBinding(size_t index) = 0;

  /**
   * @brief Gets number of provided actions.
   *
   * @return The number of actions
   */
  virtual size_t GetActionCount() = 0;

  /**
   * @brief Performs an action with given index.
   *
   * @param index The index of action
   *
   * @return true on success, false otherwise
   */
  virtual bool DoAction(size_t index) = 0;

  /**
   * @brief Performs an action with given name.
   *
   * @param name The name of action
   *
   * @return true on success, false otherwise
   */
  virtual bool DoAction(const std::string& name) = 0;
};

/**
 * @brief Interface enabling advanced quering of accessibility objects.
 *
 * @note since all mathods can be implemented inside bridge,
 * none methods have to be overrided
 */
class Collection : public virtual Accessible
{
public:
};

/**
 * @brief Interface representing objects having screen coordinates.
 */
class Component : public virtual Accessible
{
public:
  /**
   * @brief Gets rectangle describing size.
   *
   * @param[in] type The enumeration with type of coordinate systems
   *
   * @return Rect<> object
   *
   * @see Dali::Rect
   */
  virtual Rect<> GetExtents(CoordinateType type) = 0;

  /**
   * @brief Gets layer current object is localized on.
   *
   * @return The enumeration pointing layer
   *
   * @see Dali::Accessibility::ComponentLayer
   */
  virtual ComponentLayer GetLayer() = 0;

  /**
   * @brief Gets value of z-order.
   *
   * @return The value of z-order
   * @remarks MDI means "Multi Document Interface" (https://en.wikipedia.org/wiki/Multiple-document_interface)
   * which in short means that many stacked windows can be displayed within a single application.
   * In such model, the concept of z-order of UI element became important to deal with element overlapping.
   */
  virtual int16_t GetMdiZOrder() = 0;

  /**
   * @brief Sets current object as "focused".
   *
   * @return true on success, false otherwise
   */
  virtual bool GrabFocus() = 0;

  /**
   * @brief Gets value of alpha channel.
   *
   * @return The alpha channel value in range [0.0, 1.0]
   */
  virtual double GetAlpha() = 0;

  /**
   * @brief Sets current object as "highlighted".
   *
   * The method assings "highlighted" state, simultaneously removing it
   * from currently highlighted object.
   *
   * @return true on success, false otherwise
   */
  virtual bool GrabHighlight() = 0;

  /**
   * @brief Sets current object as "unhighlighted".
   *
   * The method removes "highlighted" state from object.
   *
   * @return true on success, false otherwise
   *
   * @see Dali:Accessibility::State
   */
  virtual bool ClearHighlight() = 0;

  /**
   * @brief Checks whether object can be scrolled.
   *
   * @return true if object is scrollable, false otherwise
   *
   * @see Dali:Accessibility::State
   */
  virtual bool IsScrollable();

  /**
   * @brief Gets Accessible object containing given point.
   *
   * @param[in] point The two-dimensional point
   * @param[in] type The enumeration with type of coordinate system
   *
   * @return The handle to last child of current object which contains given point
   *
   * @see Dali::Accessibility::Point
   */
  virtual Accessible* GetAccessibleAtPoint(Point point, CoordinateType type);

  /**
   * @brief Checks if current object contains given point.
   *
   * @param[in] point The two-dimensional point
   * @param[in] type The enumeration with type of coordinate system
   *
   * @return True if accessible contains in point, otherwise false.
   *
   * @remarks This method is `Contains` in DBus method.
   * @see Dali::Accessibility::Point
   */
  virtual bool IsAccessibleContainedAtPoint(Point point, CoordinateType type);
};

/**
 * @brief Interface representing objects which can store numeric value.
 */
class Value : public virtual Accessible
{
public:
  /**
   * @brief Gets the lowest possible value.
   *
   * @return The minimum value
  */
  virtual double GetMinimum() = 0;

  /**
   * @brief Gets the current value.
   *
   * @return The current value
  */
  virtual double GetCurrent() = 0;

  /**
   * @brief Gets the highest possible value.
   *
   * @return The highest value.
  */
  virtual double GetMaximum() = 0;

  /**
   * @brief Sets the current value.
   *
   * @param[in] value The current value to set
   *
   * @return true if value could have been assigned, false otherwise
  */
  virtual bool SetCurrent(double value) = 0;

  /**
   * @brief Gets the lowest increment that can be distinguished.
   *
   * @return The lowest increment
  */
  virtual double GetMinimumIncrement() = 0;
};

/**
 * @brief Interface representing objects which can store immutable texts.
 *
 * @see Dali::Accessibility::EditableText
 */
class DALI_ADAPTOR_API Text : public virtual Accessible
{
public:
  /**
   * @brief Gets stored text in given range.
   *
   * @param[in] startOffset The index of first character
   * @param[in] endOffset The index of first character after the last one expected
   *
   * @return The substring of stored text
   */
  virtual std::string GetText(size_t startOffset, size_t endOffset) = 0;

  /**
   * @brief Gets number of all stored characters.
   *
   * @return The number of characters
   * @remarks This method is `CharacterCount` in DBus method.
   */
  virtual size_t GetCharacterCount() = 0;

  /**
   * @brief Gets the cursor offset.
   *
   * @return Value of cursor offset
   * @remarks This method is `CaretOffset` in DBus method.
   */
  virtual size_t GetCursorOffset() = 0;

  /**
   * @brief Sets the cursor offset.
   *
   * @param[in] offset Cursor offset
   *
   * @return True if successful
   * @remarks This method is `SetCaretOffset` in DBus method.
   */
  virtual bool SetCursorOffset(size_t offset) = 0;

  /**
   * @brief Gets substring of stored text truncated in concrete gradation.
   *
   * @param[in] offset The position in stored text
   * @param[in] boundary The enumeration describing text gradation
   *
   * @return Range structure containing acquired text and offsets in original string
   *
   * @see Dali::Accessibility::Range
   */
  virtual Range GetTextAtOffset(size_t offset, TextBoundary boundary) = 0;

  /**
   * @brief Gets selected text.
   *
   * @param[in] selectionIndex The selection index
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return Range structure containing acquired text and offsets in original string
   *
   * @remarks This method is `GetSelection` in DBus method.
   * @see Dali::Accessibility::Range
   */
  virtual Range GetRangeOfSelection(size_t selectionIndex) = 0;

  /**
   * @brief Removes the whole selection.
   *
   * @param[in] selectionIndex The selection index
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return bool on success, false otherwise
   */
  virtual bool RemoveSelection(size_t selectionIndex) = 0;

  /**
   * @brief Sets selected text.
   *
   * @param[in] selectionIndex The selection index
   * @param[in] startOffset The index of first character
   * @param[in] endOffset The index of first character after the last one expected
   *
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return true on success, false otherwise
   * @remarks This method is `SetSelection` in DBus method.
   */
  virtual bool SetRangeOfSelection(size_t selectionIndex, size_t startOffset, size_t endOffset) = 0;
};

/**
 * @brief Interface representing objects which can store editable texts.
 *
 * @note Paste method is entirely implemented inside bridge
 *
 * @see Dali::Accessibility::EditableText
 */
class DALI_ADAPTOR_API EditableText : public virtual Accessible
{
public:
  /**
   * @brief Copies text in range to system clipboard.
   *
   * @param[in] startPosition The index of first character
   * @param[in] endPosition The index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool CopyText(size_t startPosition, size_t endPosition) = 0;

  /**
   * @brief Cuts text in range to system clipboard.
   *
   * @param[in] startPosition The index of first character
   * @param[in] endPosition The index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool CutText(size_t startPosition, size_t endPosition) = 0;

  /**
   * @brief Deletes text in range.
   *
   * @param[in] startPosition The index of first character
   * @param[in] endPosition The index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool DeleteText(size_t startPosition, size_t endPosition) = 0;

  /**
   * @brief Inserts text at startPosition.
   *
   * @param[in] startPosition The index of first character
   * @param[in] text The text content
   *
   * @return true on success, false otherwise
   */
  virtual bool InsertText(size_t startPosition, std::string text) = 0;

  /**
   * @brief Replaces text with content.
   *
   * @param[in] newContents The text content
   *
   * @return true on success, false otherwise
   */
  virtual bool SetTextContents(std::string newContents) = 0;
};

/**
 * @brief Interface representing objects which can store a set of selected items.
 */
class DALI_ADAPTOR_API Selection : public virtual Accessible
{
public:
  /**
   * @brief Gets the number of selected children.
   *
   * @return The number of selected children (zero if none)
   */
  virtual int GetSelectedChildrenCount() = 0;

  /**
   * @brief Gets a specific selected child.
   *
   * @param selectedChildIndex The index of the selected child
   *
   * @note @p selectedChildIndex refers to the list of selected children,
   * not the list of all children
   *
   * @return The selected child or nullptr if index is invalid
   */
  virtual Accessible* GetSelectedChild(int selectedChildIndex) = 0;

  /**
   * @brief Selects a child.
   *
   * @param childIndex The index of the child
   *
   * @return true on success, false otherwise
   */
  virtual bool SelectChild(int childIndex) = 0;

  /**
   * @brief Deselects a selected child.
   *
   * @param selectedChildIndex The index of the selected child
   *
   * @note @p selectedChildIndex refers to the list of selected children,
   * not the list of all children
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::Selection::DeselectChild
   */
  virtual bool DeselectSelectedChild(int selectedChildIndex) = 0;

  /**
   * @brief Checks whether a child is selected.
   *
   * @param childIndex The index of the child
   *
   * @return true if given child is selected, false otherwise
   */
  virtual bool IsChildSelected(int childIndex) = 0;

  /**
   * @brief Selects all children.
   *
   * @return true on success, false otherwise
   */
  virtual bool SelectAll() = 0;

  /**
   * @brief Deselects all children.
   *
   * @return true on success, false otherwise
   */
  virtual bool ClearSelection() = 0;

  /**
   * @brief Deselects a child.
   *
   * @param childIndex The index of the child.
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::Selection::DeselectSelectedChild
   */
  virtual bool DeselectChild(int childIndex) = 0;
};

/**
 * @brief The minimalistic, always empty Accessible object with settable address.
 *
 * For those situations, where you want to return address in different bridge
 * (embedding for example), but the object itself ain't planned to be used otherwise.
 * This object has null parent, no children, empty name and so on
 */
class DALI_ADAPTOR_API EmptyAccessibleWithAddress : public virtual Accessible
{
public:
  EmptyAccessibleWithAddress() = default;

  EmptyAccessibleWithAddress(Address address)
  : mAddress(std::move(address))
  {
  }

  void SetAddress(Address address)
  {
    this->mAddress = std::move(address);
  }

  std::string GetName() override
  {
    return "";
  }

  std::string GetDescription() override
  {
    return "";
  }

  Accessible* GetParent() override
  {
    return nullptr;
  }

  size_t GetChildCount() override
  {
    return 0;
  }

  std::vector<Accessible*> GetChildren() override
  {
    return {};
  }

  Accessible* GetChildAtIndex(size_t index) override
  {
    throw std::domain_error{"out of bounds index (" + std::to_string(index) + ") - no children"};
  }

  size_t GetIndexInParent() override
  {
    return static_cast<size_t>(-1);
  }

  Role GetRole() override
  {
    return {};
  }

  std::string GetRoleName() override;

  States GetStates() override
  {
    return {};
  }

  Attributes GetAttributes() override
  {
    return {};
  }

  Address GetAddress() override
  {
    return mAddress;
  }

  bool DoGesture(const GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Relation> GetRelationSet() override
  {
    return {};
  }

private:
  Address mAddress;
};

} // namespace Accessibility
} // namespace Dali

#endif // DALI_INTERNAL_ATSPI_ACCESSIBILITY_IMPL_H
