#ifndef DALI_ADAPTOR_ACCESSIBILITY_BRIDGE_H
#define DALI_ADAPTOR_ACCESSIBILITY_BRIDGE_H

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
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
namespace Accessibility
{
class Accessible;
class ProxyAccessible;

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
    ALREADY_UP,
    FAILED
  };

  /**
   * @brief Destructor
   */
  virtual ~Bridge() = default;

  /**
   * @brief Gets bus name which bridge is initialized on.
   *
   * @return The bus name
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
   * @brief Adds object on the top of the stack of "default label" sourcing objects.
   *
   * @see GetDefaultLabel
   *
   * @param[in] object The accessible object
   */
  virtual void RegisterDefaultLabel(Accessible* object) = 0;

  /**
   * @brief Removes object from the stack of "default label" sourcing objects.
   *
   * @see GetDefaultLabel
   *
   * @param[in] object The accessible object
   */
  virtual void UnregisterDefaultLabel(Accessible* object) = 0;

  /**
   * @brief Gets the top-most object from the stack of "default label" sourcing objects.
   *
   * The "default label" is a reading material (text) derived from an accesibility object
   * that could be read by screen-reader immediately after the navigation context has changed
   * (window activates, popup shows up, tab changes) and before first UI element is highlighted.
   *
   * @param[in] root The root of the navigation context for which to retrieve the default label.
   *
   * @return The handler to accessibility object
   * @note This is a Tizen only feature not present in upstream ATSPI.
   * Feature can be enabled/disabled for particular context root object
   * by setting value of its accessibility attribute "default_label".
   * Following strings are valid values for "default_label" attribute: "enabled", "disabled".
   * Any other value will be interpreted as "enabled".
   */
  virtual Accessible* GetDefaultLabel(Accessible* root) const = 0;

  /**
   * @brief Sets name of current application which will be visible on accessibility bus.
   *
   * @param[in] name The application name
   */
  virtual void SetApplicationName(std::string name) = 0;

  /**
   * @brief Sets the name of the GUI toolkit that AT-SPI clients can query.
   *
   * The default name is "dali".
   *
   * @param toolkitName The toolkit name
   */
  virtual void SetToolkitName(std::string_view toolkitName) = 0;

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
   * @brief Notifies accessibility dbus that window has just been shown.
   *
   * @param[in] window The window to be shown
   */
  virtual void WindowShown(Window window) = 0;

  /**
   * @brief Notifies accessibility dbus that window has just been hidden.
   *
   * @param[in] window The window to be hidden
   */
  virtual void WindowHidden(Window window) = 0;

  /**
   * @brief Notifies accessibility dbus that window has just been focused.
   *
   * @param[in] window The window to be focused
   */
  virtual void WindowFocused(Window window) = 0;

  /**
   * @brief Notifies accessibility dbus that window has just been out of focus.
   *
   * @param[in] window The window to be out of focus
   */
  virtual void WindowUnfocused(Window window) = 0;

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
    mData          = std::make_shared<Data>();
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
   * @brief Emits MoveOuted event on at-spi bus.
   *
   * @param[in] obj Accessible object
   * @param[in] type Direction type when an Accessible object moves out of screen
   **/
  virtual void EmitMovedOutOfScreen(Accessible* obj, ScreenRelativeMoveType type) = 0;

  /**
   * @brief Emits "org.a11y.atspi.Socket.Available" event on AT-SPI bus.
   *
   * @param obj Accessible object
   */
  virtual void EmitSocketAvailable(Accessible* obj) = 0;

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
   * @brief Calls socket.Embed(plug) via D-Bus.
   *
   * @param[in] plug The plug
   * @param[in] socket The socket
   *
   * @return Address returned by the D-Bus call.
   *
   * @note Remote object pointed to by 'socket' must implement 'org.a11y.atspi.Socket'.
   * @see UnembedSocket()
   */
  virtual Address EmbedSocket(const Address& plug, const Address& socket) = 0;

  /**
   * @brief Calls socket.Embedded(plug) via D-Bus.
   *
   * The "Embedded" D-Bus method is an ATK extension.
   * See 'impl_Embedded' in AT_SPI2_ATK/atk-adaptor/adaptors/socket-adaptor.c for more information.
   *
   * @param[in] plug The plug
   * @param[in] socket The socket
   */
  virtual void EmbedAtkSocket(const Address& plug, const Address& socket) = 0;

  /**
   * @brief Calls socket.Unmbed(plug) via D-Bus.
   *
   * @param[in] plug The plug
   * @param[in] socket The socket
   *
   * @note Remote object pointed to by 'socket' must implement 'org.a11y.atspi.Socket'.
   * @see EmbedSocket()
   */
  virtual void UnembedSocket(const Address& plug, const Address& socket) = 0;

  /**
   * @brief Calls socket.SetOffset(x, y) via D-Bus.
   *
   * The "SetOffset" D-Bus method is a DALi extension. It can be used to inform a DALi widget about
   * its position on the screen.
   *
   * @param[in] socket The socket
   * @param[in] x Horizontal offset
   * @param[in] y Vertical offset
   *
   * @note Remote object pointed to by 'socket' must implement 'org.a11y.atspi.Socket'.
   * @see EmbedSocket()
   * @see SetExtentsOffset()
   */
  virtual void SetSocketOffset(ProxyAccessible* socket, std::int32_t x, std::int32_t y) = 0;

  /**
   * @brief Sets the global extents offset.
   *
   * This offset will be added during serialization of GetExtents() return value to D-Bus.
   * Local calls to GetExtents() are unaffected.
   *
   * @param[in] x Horizontal offset
   * @param[in] y Vertical offset
   *
   * @see SetSocketOffset()
   * @see Dali::Accessibility::Component::GetExtents()
   */
  virtual void SetExtentsOffset(std::int32_t x, std::int32_t y) = 0;

  /**
   * @brief Sets the preferred bus name.
   *
   * If the Bridge is enabled, it will immediately release the previous name and request the new one.
   *
   * Otherwise, the Bridge will request this name on AT-SPI activation (and release it on deactivation).
   * It is up to the caller to determine whether a given name will be available in the system.
   *
   * @param preferredBusName The preferred bus name
   */
  virtual void SetPreferredBusName(std::string_view preferredBusName) = 0;

  /**
   * @brief Returns instance of bridge singleton object.
   *
   * @return The current bridge object
   **/
  static std::shared_ptr<Bridge> GetCurrentBridge();

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

  /**
   * @brief Encodes a widget ID as a usable bus name.
   *
   * @param widgetInstanceId The instance ID of a widget
   * @return std::string Encoded bus name
   *
   * @see SetPreferredBusName
   */
  static std::string MakeBusNameForWidget(std::string_view widgetInstanceId);

  static Signal<void()>& EnabledSignal()
  {
    return mEnabledSignal;
  }

  static Signal<void()>& DisabledSignal()
  {
    return mDisabledSignal;
  }

  static Signal<void()>& ScreenReaderEnabledSignal()
  {
    return mScreenReaderEnabledSignal;
  }

  static Signal<void()>& ScreenReaderDisabledSignal()
  {
    return mScreenReaderDisabledSignal;
  }

protected:
  struct Data
  {
    std::unordered_set<const Accessible*> mKnownObjects;
    std::string                           mBusName;
    Bridge*                               mBridge = nullptr;
    Actor                                 mHighlightActor;
    Actor                                 mCurrentlyHighlightedActor;
    std::pair<std::int32_t, std::int32_t> mExtentsOffset{0, 0};
  };
  std::shared_ptr<Data> mData;
  friend class Accessible;

  enum class AutoInitState
  {
    DISABLED,
    ENABLED
  };

  inline static AutoInitState mAutoInitState = AutoInitState::ENABLED;

  inline static Signal<void()> mEnabledSignal;
  inline static Signal<void()> mDisabledSignal;
  inline static Signal<void()> mScreenReaderEnabledSignal;
  inline static Signal<void()> mScreenReaderDisabledSignal;

  /**
   * @brief Registers accessible object to be known in bridge object.
   *
   * Bridge must known about all currently alive accessible objects, as some requst
   * might come and object will be identified by number id (it's memory address).
   * To avoid memory corruption number id is checked against set of known objects.
   *
   * @param[in] object The accessible object
   **/
  void RegisterOnBridge(const Accessible* object);

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

} // namespace Accessibility
} // namespace Dali

#endif // DALI_ADAPTOR_ACCESSIBILITY_BRIDGE_H
