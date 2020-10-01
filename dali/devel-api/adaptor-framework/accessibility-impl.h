#ifndef DALI_INTERNAL_ATSPI_ACCESSIBILITY_IMPL_H
#define DALI_INTERNAL_ATSPI_ACCESSIBILITY_IMPL_H

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
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/object-registry.h>
#include <atomic>
#include <bitset>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>

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
 * @brief Base class for different accessibility bridges
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
   * @brief Get bus name which bridge is initialized on
   */
  virtual const std::string& GetBusName() const = 0;

  /**
   * @brief Registers top level window
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method allows to connect chosen
   * object as direct ancestor of application and therefore make it visible for
   * accessibility clients.
   */
  virtual void AddTopLevelWindow( Accessible* ) = 0;

  /**
   * @brief Removes top level window
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method removes previously added
   * window from visible accessibility objects.
   */
  virtual void RemoveTopLevelWindow( Accessible* ) = 0;

  /**
   * @brief Adds popup window
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method adds new popup to the tree.
   */
  virtual void AddPopup( Accessible* ) = 0;

  /**
   * @brief Removes popup window
   *
   * Hierarchy of objects visible for accessibility clients is based on tree-like
   * structure created from Actors objects. This method removes previously added
   * popup window.
   */
  virtual void RemovePopup( Accessible* ) = 0;

  /**
   * @brief Set name of current application which will be visible on accessibility bus
   */
  virtual void SetApplicationName( std::string ) = 0;

  /**
   * @brief Get object being root of accessibility tree
   *
   * @return handler to accessibility object
   */
  virtual Accessible* GetApplication() const = 0;

  /**
   * @brief Find an object in accessibility tree
   *
   * @param[in] s path to object
   *
   * @return handler to accessibility object
   */
  virtual Accessible* FindByPath( const std::string& s) const = 0;

  /**
   * @brief Show application on accessibility bus
   */
  virtual void ApplicationShown() = 0;

  /**
   * @brief Hide application on accessibility bus
   */
  virtual void ApplicationHidden() = 0;

  /**
   * @brief Initialize accessibility bus
   */
  virtual void Initialize() = 0;

  /**
   * @brief Terminate accessibility bus
   */
  virtual void Terminate() = 0;

  /**
   * @brief This method is called, when bridge is being activated.
   */
  virtual ForceUpResult ForceUp()
  {
    if( data )
    {
      return ForceUpResult::ALREADY_UP;
    }
    data = std::make_shared< Data >();
    data->bridge = this;
    return ForceUpResult::JUST_STARTED;
  }

  /**
   * @brief This method is called, when bridge is being deactivated.
   */
  virtual void ForceDown() = 0;

  /**
   * @brief Check if bridge is activated or not.
   * @return True if brige is activated.
   */
  bool IsUp() const
  {
    return bool(data);
  }

  /**
   * @brief Emits caret-moved event on at-spi bus.
   **/
  virtual void EmitCaretMoved( Accessible* obj, unsigned int cursorPosition ) = 0;

  /**
   * @brief Emits active-descendant-changed event on at-spi bus.
   **/
  virtual void EmitActiveDescendantChanged( Accessible* obj, Accessible *child ) = 0;

  /**
   * @brief Emits text-changed event on at-spi bus.
   **/
  virtual void EmitTextChanged( Accessible* obj, TextChangedState state, unsigned int position, unsigned int length, const std::string &content ) = 0;

  /**
   * @brief Emits state-changed event on at-spi bus.
   **/
  virtual void EmitStateChanged( Accessible* obj, State state, int val1, int val2 = 0 ) = 0;

  /**
   * @brief Emits window event on at-spi bus.
   **/
  virtual void Emit( Accessible* obj, WindowEvent we, unsigned int detail1 = 0 ) = 0;

  /**
   * @brief Emits property-changed event on at-spi bus.
   **/
  virtual void Emit( Accessible* obj, ObjectPropertyChangeEvent event ) = 0;

  /**
   * @brief Emits bounds-changed event on at-spi bus.
   **/
  virtual void EmitBoundsChanged( Accessible* obj, Rect<> rect ) = 0;

  /**
   * @brief Emits key event on at-spi bus.
   *
   * Screen-reader might receive this event and reply, that given keycode is consumed. In that case
   * further processing of the keycode should be ignored.
   **/
  virtual Consumed Emit( KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText ) = 0;

  /**
   * @brief Reads given text by screen reader
   *
   * @param text The text to read
   * @param discardable If TRUE, reading can be discarded by subsequent reading requests,
   * if FALSE the reading must finish before next reading request can be started
   * @param callback the callback function that is called on reading signals emitted
   * during processing of this reading request.
   * Callback can be one of the following signals:
   * ReadingCancelled, ReadingStopped, ReadingSkipped
   */
  virtual void Say( const std::string& text, bool discardable, std::function<void(std::string)> callback ) = 0;

  /**
   * @brief Force accessibility client to pause.
   */
  virtual void Pause() = 0;

  /**
   * @brief Force accessibility client to resume.
   */
  virtual void Resume() = 0;

  /**
   * @brief Get screen reader status.
   */
  virtual bool GetScreenReaderEnabled() = 0;

  /**
   * @brief Get ATSPI status.
   */
  virtual bool GetIsEnabled() = 0;

  /**
   * @brief Returns instance of bridge singleton object.
   **/
  static Bridge* GetCurrentBridge();

protected:
  struct Data
  {
    std::unordered_set< Accessible* > knownObjects;
    std::string busName;
    Bridge* bridge = nullptr;
    Actor highlightActor, currentlyHighlightedActor;
  };
  std::shared_ptr< Data > data;
  friend class Accessible;

  /**
   * @brief Registers accessible object to be known in bridge object
   *
   * Bridge must known about all currently alive accessible objects, as some requst
   * might come and object will be identified by number id (it's memory address).
   * To avoid memory corruption number id is checked against set of known objects.
   **/
  void RegisterOnBridge( Accessible* );

  /**
   * @brief Tells bridge, that given object is considered root (doesn't have any parents).
   *
   * All root objects will have the same parent - application object. Application object
   * is controlled by bridge and private.
   **/
  void SetIsOnRootLevel( Accessible* );
};

/**
 * @brief Check if ATSPI is activated or not.
 * @return True if ATSPI is activated.
 */
inline bool IsUp()
{
  if( Bridge::GetCurrentBridge() == nullptr )
  {
    return false;
  }
  if( Bridge::GetCurrentBridge()->GetIsEnabled() == false )
  {
    return false;
  }
  return Bridge::GetCurrentBridge()->IsUp();
}

/**
 * @brief Basic interface implemented by all accessibility objects
 */
class Accessible
{
public:
  virtual ~Accessible();

  using utf8_t = unsigned char;

  /**
   * @brief Calculaties word boundaries in given utf8 text.
   *
   * s and length represents source text pointer and it's length respectively. langauge represents
   * language to use. Word boundaries are returned as non-zero values in table breaks, which
   * must be of size at least length.
   **/
  void FindWordSeparationsUtf8( const utf8_t *s, size_t length, const char *language, char *breaks );

  /**
   * @brief Calculaties line boundaries in given utf8 text.
   *
   * s and length represents source text pointer and it's length respectively. langauge represents
   * language to use. Line boundaries are returned as non-zero values in table breaks, which
   * must be of size at least length.
   **/
  void FindLineSeparationsUtf8( const utf8_t *s, size_t length, const char *language, char *breaks );

  /**
   * @brief Helper function for emiting active-descendant-changed event
   **/
  void EmitActiveDescendantChanged( Accessible* obj, Accessible *child );

  /**
   * @brief Helper function for emiting state-changed event
   **/
  void EmitStateChanged( State state, int newValue1, int newValue2 = 0 );

  /**
   * @brief Helper function for emiting bounds-changed event
   **/
  void EmitBoundsChanged( Rect<> rect );

  /**
   * @brief Emit "showing" event.
   * The method inform accessibility clients about "showing" state
   *
   * @param[in] showing flag pointing if object is showing
   */
  void EmitShowing( bool showing );

  /**
   * @brief Emit "visible" event.
   * The method inform accessibility clients about "visible" state
   *
   * @param[in] visible flag pointing if object is visible
   */
  void EmitVisible( bool visible );

  /**
   * @brief Emit "highlighted" event.
   * The method inform accessibility clients about "highlighted" state
   *
   * @param[in] set flag pointing if object is highlighted
   */
  void EmitHighlighted( bool set );

  /**
   * @brief Emit "focused" event.
   * The method inform accessibility clients about "focused" state
   *
   * @param[in] set flag pointing if object is focused
   */
  void EmitFocused( bool set );

  /**
   * @brief Emit "text inserted" event
   *
   * @param[in] position caret position
   * @param[in] length text length
   * @param[in] content inserted text
   */
  void EmitTextInserted( unsigned int position, unsigned int length, const std::string &content );

  /**
   * @brief Emit "text deleted" event
   *
   * @param[in] position caret position
   * @param[in] length text length
   * @param[in] content deleted text
   */
  void EmitTextDeleted( unsigned int position, unsigned int length, const std::string &content );

  /**
   * @brief Emit "caret moved" event
   *
   * @param[in] cursorPosition new caret position
   */
  void EmitTextCaretMoved( unsigned int cursorPosition );

  /**
   * @brief Emit "highlighted" event
   *
   * @param[in] we enumerated window event
   * @param[in] detail1 additional parameter which interpretation depends on chosen event
   */
  void Emit( WindowEvent we, unsigned int detail1 = 0 );

  /**
   * @brief Emits property-changed event
   * @param[in] event Property changed event
   **/
  void Emit( ObjectPropertyChangeEvent event );

  /**
   * @brief Get accessibility name
   *
   * @return string with name
   */
  virtual std::string GetName() = 0;

  /**
   * @brief Get accessibility description
   *
   * @return string with description
   */
  virtual std::string GetDescription() = 0;

  /**
   * @brief Get parent
   *
   * @return handler to accessibility object
   */
  virtual Accessible* GetParent() = 0;

  /**
   * @brief Get count of children
   *
   * @return unsigned integer value
   */
  virtual size_t GetChildCount() = 0;

  /**
   * @brief Get collection with all children
   *
   * @return collection of accessibility objects
   */
  virtual std::vector< Accessible* > GetChildren();

  /**
   * @brief Get nth child
   *
   * @return accessibility object
   */
  virtual Accessible* GetChildAtIndex( size_t index ) = 0;

  /**
   * @brief Get index that current object has in its parent's children collection
   *
   * @return unsigned integer index
   */
  virtual size_t GetIndexInParent() = 0;

  /**
   * @brief Get accessibility role
   *
   * @return Role enumeration
   *
   * @see Dali::Accessibility::Role
   */
  virtual Role GetRole() = 0;

  /**
   * @brief Get name of accessibility role
   *
   * @return string with human readable role converted from enumeration
   *
   * @see Dali::Accessibility::Role
   * @see Accessibility::Accessible::GetRole
   */
  virtual std::string GetRoleName();

  /**
   * @brief Get localized name of accessibility role
   *
   * @return string with human readable role translated according to current
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
   * @brief Get accessibility states
   *
   * @return collection of states
   *
   * @note States class is instatation of ArrayBitset template class
   *
   * @see Dali::Accessibility::State
   * @see Dali::Accessibility::ArrayBitset
   */
  virtual States GetStates() = 0;

  /**
   * @brief Get accessibility attributes
   *
   * @return map of attributes and their values
   */
  virtual Attributes GetAttributes() = 0;

  /**
   * @brief Check if this is proxy
   *
   * @return True if this is proxy
   */
  virtual bool IsProxy();

  /**
   * @brief Get unique address on accessibility bus
   *
   * @return class containing address
   *
   * @see Dali::Accessibility::Address
   */
  virtual Address GetAddress();

  /**
   * @brief Get accessibility object, which is "default label" for this object
   */
  virtual Accessible* GetDefaultLabel();

  /**
   * @brief Depute an object to perform provided gesture
   *
   * @param[in] gestureInfo structure describing the gesture
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::GestureInfo
   */
  virtual bool DoGesture(const GestureInfo &gestureInfo) = 0;

  /**
   * @brief Re-emits selected states of an Accessibility Object
   *
   * @param[in] states chosen states to re-emit
   * @param[in] doRecursive if true all children of the Accessibility Object will also re-emit the states
   */
  void NotifyAccessibilityStateChange( Dali::Accessibility::States states, bool doRecursive );

  /**
   * @brief Get information about current object and all relations that connects
   * it with other accessibility objects
   *
   * @return iterable collection of Relation objects
   *
   * @see Dali::Accessibility::Relation
   */
  virtual std::vector<Relation> GetRelationSet() = 0;

  /**
   * @brief Get all implemented interfaces
   *
   * @return collection of strings with implemented interfaces
   */
  std::vector< std::string > GetInterfaces();

  /**
   * @brief Check if object is on root level
   */
  bool GetIsOnRootLevel() const { return isOnRootLevel; }

  /**
   * @brief The method registers functor resposible for converting Actor into Accessible
   * @param functor returning Accessible handle from Actor object
   */
  static void RegisterControlAccessibilityGetter( std::function< Accessible*( Dali::Actor ) > functor);

  /**
   * @brief Acquire Accessible object from Actor object
   *
   * @param[in] actor Actor object
   * @param[in] root true, if it's top level object (window)
   *
   * @return handle to Accessible object
   */
  static Accessible* Get( Dali::Actor actor, bool root = false );

protected:
  Accessible();
  Accessible( const Accessible& ) = delete;
  Accessible( Accessible&& ) = delete;
  Accessible& operator=( const Accessible& ) = delete;
  Accessible& operator=( Accessible&& ) = delete;
  std::shared_ptr< Bridge::Data > GetBridgeData();

public:
  static Dali::Actor GetHighlightActor();
  static void SetHighlightActor(Dali::Actor actor);
  static Dali::Actor GetCurrentlyHighlightedActor();
  static void SetCurrentlyHighlightedActor(Dali::Actor);
  static void SetObjectRegistry(ObjectRegistry registry);

private:
  friend class Bridge;

  std::weak_ptr< Bridge::Data > bridgeData;
  bool isOnRootLevel = false;
};

/**
 * @brief Interface enabling to perform provided actions
 */
class Action : public virtual Accessible
{
public:
  /**
   * @brief Get name of action with given index
   *
   * @param[in] index index of action
   *
   * @return string with name of action
   */
  virtual std::string GetActionName( size_t index ) = 0;

  /**
   * @brief Get translated name of action with given index
   *
   * @param[in] index index of action
   *
   * @return string with name of action translated according to current translation domain
   *
   * @note translation is not supported in this version
   */
  virtual std::string GetLocalizedActionName( size_t index ) = 0;

  /**
   * @brief Get description of action with given index
   *
   * @param[in] index index of action
   *
   * @return string with description of action
   */
  virtual std::string GetActionDescription( size_t index ) = 0;

  /**
   * @brief Get key code binded to action with given index
   *
   * @param[in] index index of action
   *
   * @return string with key name
   */
  virtual std::string GetActionKeyBinding( size_t index ) = 0;

  /**
   * @brief Get number of provided actions
   *
   * @return unsigned integer with number of actions
   */
  virtual size_t GetActionCount() = 0;

  /**
   * @brief Perform an action with given index
   *
   * @param index index of action
   *
   * @return true on success, false otherwise
   */
  virtual bool DoAction( size_t index ) = 0;

  /**
   * @brief Perform an action with given name
   *
   * @param name name of action
   *
   * @return true on success, false otherwise
   */
  virtual bool DoAction( const std::string& name ) = 0;

};

/**
 * @brief Interface enabling advanced quering of accessibility objects
 *
 * @note since all mathods can be implemented inside bridge,
 * none methods have to be overrided
 */
class Collection : public virtual Accessible
{
public:
};

/**
 * @brief Interface representing objects having screen coordinates
 */
class Component : public virtual Accessible
{
public:
  /**
   * @brief Get rectangle describing size
   *
   * @param[in] ctype enumeration with type of coordinate systems
   *
   * @return Rect<> object
   *
   * @see Dali::Rect
   */
  virtual Rect<> GetExtents( CoordType ctype ) = 0;

  /**
   * @brief Get layer current object is localized on
   *
   * @return enumeration pointing layer
   *
   * @see Dali::Accessibility::ComponentLayer
   */
  virtual ComponentLayer GetLayer() = 0;

  /**
   * @brief Get value of z-order
   *
   * @return value of z-order
   */
  virtual int16_t GetMdiZOrder() = 0;

  /**
   * @brief Set current object as "focused"
   *
   * @return true on success, false otherwise
   */
  virtual bool GrabFocus() = 0;

  /**
   * @brief Get value of alpha channel
   *
   * @return alpha channel value in range [0.0, 1.0]
   */
  virtual double GetAlpha() = 0;

  /**
   * @brief Set current object as "highlighted"
   *
   * The method assings "highlighted" state, simultaneously removing it
   * from currently highlighted object.
   *
   * @return true on success, false otherwise
   */
  virtual bool GrabHighlight() = 0;

  /**
   * @brief Set current object as "unhighlighted"
   *
   * The method removes "highlighted" state from object.
   *
   * @return true on success, false otherwise
   *
   * @see Dali:Accessibility::State
   */
  virtual bool ClearHighlight() = 0;

  /**
   * @brief Check whether object can be scrolled
   *
   * @return true if object is scrollable, false otherwise
   *
   * @see Dali:Accessibility::State
   */
  virtual bool IsScrollable();

  /**
   * @brief Get Accessible object containing given point
   *
   * @param[in] p two-dimensional point
   * @param[in] ctype enumeration with type of coordinate system
   *
   * @return handle to last child of current object which contains given point
   *
   * @see Dali::Accessibility::Point
   */
  virtual Accessible* GetAccessibleAtPoint( Point p, CoordType ctype );

  /**
   * @brief Check if current object contains given point
   *
   * @param[in] p two-dimensional point
   * @param[in] ctype enumeration with type of coordinate system
   *
   * @return handle to Accessible object
   *
   * @see Dali::Accessibility::Point
   */
  virtual bool Contains( Point p, CoordType ctype );
};

/**
 * @brief Interface representing objects which can store numeric value
 */
class Value : public virtual Accessible
{
public:
  /**
   * @brief Get the lowest possible value
   *
   * @return double value
  */
  virtual double GetMinimum() = 0;

  /**
   * @brief Get current value
   *
   * @return double value
  */
  virtual double GetCurrent() = 0;

  /**
   * @brief Get the highest possible value
   *
   * @return double value
  */
  virtual double GetMaximum() = 0;

  /**
   * @brief Set value
   *
   * @param[in] val double value
   *
   * @return true if value could have been assigned, false otherwise
  */
  virtual bool SetCurrent( double val) = 0;

  /**
   * @brief Get the lowest increment that can be distinguished
   *
   * @return double value
  */
  virtual double GetMinimumIncrement() = 0;
};

/**
 * @brief Interface representing objects which can store immutable texts
 *
 * @see Dali::Accessibility::EditableText
 */
class DALI_ADAPTOR_API Text : public virtual Accessible
{
public:
  /**
   * @brief Get stored text in given range
   *
   * @param[in] startOffset index of first character
   * @param[in] endOffset index of first character after the last one expected
   *
   * @return substring of stored text
   */
  virtual std::string GetText( size_t startOffset, size_t endOffset ) = 0;

  /**
   * @brief Get number of all stored characters
   *
   * @return number of characters
   */
  virtual size_t GetCharacterCount() = 0;

  /**
   * @brief Get caret offset
   *
   * @return Value of caret offset
   */
  virtual size_t GetCaretOffset() = 0;

  /**
   * @brief Set caret offset
   *
   * @param[in] offset Caret offset
   *
   * @return True if successful
   */
  virtual bool SetCaretOffset(size_t offset) = 0;

  /**
   * @brief Get substring of stored text truncated in concrete gradation
   *
   * @param[in] offset position in stored text
   * @param[in] boundary enumeration describing text gradation
   *
   * @return Range structure containing acquired text and offsets in original string
   *
   * @see Dali::Accessibility::Range
   */
  virtual Range GetTextAtOffset( size_t offset, TextBoundary boundary ) = 0;

  /**
   * @brief Get selected text
   *
   * @param[in] selectionNum selection index
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return Range structure containing acquired text and offsets in original string
   *
   * @see Dali::Accessibility::Range
   */
  virtual Range GetSelection( size_t selectionNum ) = 0;

  /**
   * @brief Remove selection
   *
   * @param[in] selectionNum selection index
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return bool on success, false otherwise
   */
  virtual bool RemoveSelection( size_t selectionNum ) = 0;

  /**
   * @brief Get selected text
   *
   * @param[in] selectionNum selection index
   * @param[in] startOffset index of first character
   * @param[in] endOffset index of first character after the last one expected
   *
   * @note Currently only one selection (i.e. with index = 0) is supported
   *
   * @return true on success, false otherwise
   */
  virtual bool SetSelection( size_t selectionNum, size_t startOffset, size_t endOffset ) = 0;
};

/**
 * @brief Interface representing objects which can store editable texts
 *
 * @note Paste method is entirely implemented inside bridge
 *
 * @see Dali::Accessibility::EditableText
 */
class DALI_ADAPTOR_API EditableText : public virtual Accessible
{
public:
  /**
   * @brief Copy text in range to system clipboard
   *
   * @param[in] startPosition index of first character
   * @param[in] endPosition index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool CopyText( size_t startPosition, size_t endPosition ) = 0;

  /**
   * @brief Cut text in range to system clipboard
   *
   * @param[in] startPosition index of first character
   * @param[in] endPosition index of first character after the last one expected
   *
   * @return true on success, false otherwise
   */
  virtual bool CutText( size_t startPosition, size_t endPosition ) = 0;
};

/**
 * @brief minimalistic, always empty Accessible object with settable address
 *
 * For those situations, where you want to return address in different bridge
 * (embedding for example), but the object itself ain't planned to be used otherwise.
 * This object has null parent, no children, empty name and so on
 */
class DALI_ADAPTOR_API EmptyAccessibleWithAddress : public virtual Accessible
{
public:
  EmptyAccessibleWithAddress() = default;
  EmptyAccessibleWithAddress( Address address ) : address( std::move( address ) ) {}

  void SetAddress( Address address ) { this->address = std::move( address ); }

  std::string GetName() override { return ""; }
  std::string GetDescription() override { return ""; }
  Accessible* GetParent() override { return nullptr; }
  size_t GetChildCount() override { return 0; }
  std::vector< Accessible* > GetChildren() override { return {}; }
  Accessible* GetChildAtIndex( size_t index ) override
  {
    throw std::domain_error{"out of bounds index (" + std::to_string( index ) + ") - no children"};
  }
  size_t GetIndexInParent() override { return static_cast< size_t >( -1 ); }
  Role GetRole() override { return {}; }
  std::string GetRoleName() override;
  States GetStates() override { return {}; }
  Attributes GetAttributes() override { return {}; }
  Address GetAddress() override
  {
    return address;
  }
  bool DoGesture(const GestureInfo &gestureInfo) override
  {
    return false;
  }
  std::vector<Relation> GetRelationSet() override
  {
    return {};
  }

private:
  Address address;
};

}
}

#endif // DALI_INTERNAL_ATSPI_ACCESSIBILITY_IMPL_H
