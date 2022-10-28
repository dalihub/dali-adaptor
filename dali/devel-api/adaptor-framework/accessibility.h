#ifndef DALI_ATSPI_ACCESSIBILITY_H
#define DALI_ATSPI_ACCESSIBILITY_H
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
#include <string>
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bitset.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Accessibility
{
class Accessible;

/**
 * @brief Enumeration describing type of object move relative to the screen. Only outgoing moves are signalled to AT-clients.
 */
enum class ScreenRelativeMoveType
{
  OUTSIDE               = 0, ///< Object moves outside of the screen
  OUTGOING_TOP_LEFT     = 1, ///< Object moves out through the top(or left) side of the screen. Maps to: ATSPI_MOVE_OUTED_TOP_LEFT
  OUTGOING_BOTTOM_RIGHT = 2, ///< Object moves out through the bottom(or right) side of the screen. Maps to: ATSPI_MOVE_OUTED_BOTTOM_RIGHT
  INSIDE                     ///< Object moves into the screen or moves inside the screen
};

/**
 * @brief Enumeration describing a relation between accessible objects
 * 1 to 0..N relation model is supported. By default relation is not symmetrical.
 * Symmetry must be explicitly maintained.
 */
enum class RelationType : uint32_t
{
  NULL_OF,          ///< Null Relation.
  LABEL_FOR,        ///< Label For.
  LABELLED_BY,      ///< Labelled By.
  CONTROLLER_FOR,   ///< Controller For.
  CONTROLLED_BY,    ///< Controlled By.
  MEMBER_OF,        ///< Member Of.
  TOOLTIP_FOR,      ///< ToolTip For.
  NODE_CHILD_OF,    ///< Node Child Of.
  NODE_PARENT_OF,   ///< Node Parent Of.
  EXTENDED,         ///< Extended.
  FLOWS_TO,         ///< Flows To.
  FLOWS_FROM,       ///< Flows From.
  SUBWINDOW_OF,     ///< Sub Window Of.
  EMBEDS,           ///< Embeds.
  EMBEDDED_BY,      ///< Embedded By.
  POPUP_FOR,        ///< Popup For
  PARENT_WINDOW_OF, ///< Parent Window Of.
  DESCRIPTION_FOR,  ///< Description For.
  DESCRIBED_BY,     ///< Described By.
  DETAILS,          ///< Details.
  DETAILS_FOR,      ///< Details For.
  ERROR_MESSAGE,    ///< Error Message.
  ERROR_FOR,        ///< Error For.
  MAX_COUNT
};

/**
 * @brief Enumeration describing if coordinates are relative to screen or window
 * @see Accessibility::Component::GetExtents
 * @see Accessibility::Component::IsAccessibleContainingPoint
 */
enum class CoordinateType
{
  SCREEN, ///< Screen.
  WINDOW  ///< Window.
};

/**
 * @brief Enumeration indicating relative stacking order
 * ComponentLayer allows to describe visibility of all parts of UI
 * basing on the concrete stacking order
 * @see Accessibility::Component::GetLayer
 * @note currently only ComponentLayer::Window is supported
 */
enum class ComponentLayer
{
  INVALID,    ///< Invalid.
  BACKGROUND, ///< Background.
  CANVAS,     ///< Canvas.
  WIDGET,     ///< Widget.
  MDI,        ///< MDI.
  POPUP,      ///< Popup.
  OVERLAY,    ///< Overlay.
  WINDOW,     ///< Window.
  MAX_COUNT
};

/**
 * @brief Enumeration describing role of the Accessibility object
 * Object can be described by only one role.
 * @see Accessibility::Accessible::GetRole
 */
enum class Role : uint32_t
{
  INVALID,
  ACCELERATOR_LABEL,
  ALERT,
  ANIMATION,
  ARROW,
  CALENDAR,
  CANVAS,
  CHECK_BOX,
  CHECK_MENU_ITEM,
  COLOR_CHOOSER,
  COLUMN_HEADER,
  COMBO_BOX,
  DATE_EDITOR,
  DESKTOP_ICON,
  DESKTOP_FRAME,
  DIAL,
  DIALOG,
  DIRECTORY_PANE,
  DRAWING_AREA,
  FILE_CHOOSER,
  FILLER,
  FOCUS_TRAVERSABLE,
  FONT_CHOOSER,
  FRAME,
  GLASS_PANE,
  HTML_CONTAINER,
  ICON,
  IMAGE,
  INTERNAL_FRAME,
  LABEL,
  LAYERED_PANE,
  LIST,
  LIST_ITEM,
  MENU,
  MENU_BAR,
  MENU_ITEM,
  OPTION_PANE,
  PAGE_TAB,
  PAGE_TAB_LIST,
  PANEL,
  PASSWORD_TEXT,
  POPUP_MENU,
  PROGRESS_BAR,
  PUSH_BUTTON,
  RADIO_BUTTON,
  RADIO_MENU_ITEM,
  ROOT_PANE,
  ROW_HEADER,
  SCROLL_BAR,
  SCROLL_PANE,
  SEPARATOR,
  SLIDER,
  SPIN_BUTTON,
  SPLIT_PANE,
  STATUS_BAR,
  TABLE,
  TABLE_CELL,
  TABLE_COLUMN_HEADER,
  TABLE_ROW_HEADER,
  TEAROFF_MENU_ITEM,
  TERMINAL,
  TEXT,
  TOGGLE_BUTTON,
  TOOL_BAR,
  TOOL_TIP,
  TREE,
  TREE_TABLE,
  UNKNOWN,
  VIEWPORT,
  WINDOW,
  EXTENDED,
  HEADER,
  FOOTER,
  PARAGRAPH,
  RULER,
  APPLICATION,
  AUTOCOMPLETE,
  EDITBAR,
  EMBEDDED,
  ENTRY,
  CHART,
  CAPTION,
  DOCUMENT_FRAME,
  HEADING,
  PAGE,
  SECTION,
  REDUNDANT_OBJECT,
  FORM,
  LINK,
  INPUT_METHOD_WINDOW,
  TABLE_ROW,
  TREE_ITEM,
  DOCUMENT_SPREADSHEET,
  DOCUMENT_PRESENTATION,
  DOCUMENT_TEXT,
  DOCUMENT_WEB,
  DOCUMENT_EMAIL,
  COMMENT,
  LIST_BOX,
  GROUPING,
  IMAGE_MAP,
  NOTIFICATION,
  INFO_BAR,
  LEVEL_BAR,
  TITLE_BAR,
  BLOCK_QUOTE,
  AUDIO,
  VIDEO,
  DEFINITION,
  ARTICLE,
  LANDMARK,
  LOG,
  MARQUEE,
  MATH,
  RATING,
  TIMER,
  STATIC,
  MATH_FRACTION,
  MATH_ROOT,
  SUBSCRIPT,
  SUPERSCRIPT,
  MAX_COUNT
};

/**
 * @brief Enumeration describing states of the Accessibility object
 * Object can be in many states at the same time.
 * @see Accessibility::Accessible::GetStates
 */
enum class State : uint32_t
{
  INVALID,
  ACTIVE,
  ARMED,
  BUSY,
  CHECKED,
  COLLAPSED,
  DEFUNCT,
  EDITABLE,
  ENABLED,
  EXPANDABLE,
  EXPANDED,
  FOCUSABLE,
  FOCUSED,
  HAS_TOOLTIP,
  HORIZONTAL,
  ICONIFIED,
  MODAL,
  MULTI_LINE,
  MULTI_SELECTABLE,
  OPAQUE,
  PRESSED,
  RESIZEABLE,
  SELECTABLE,
  SELECTED,
  SENSITIVE,
  SHOWING,
  SINGLE_LINE,
  STALE,
  TRANSIENT,
  VERTICAL,
  VISIBLE,
  MANAGES_DESCENDANTS,
  INDETERMINATE,
  REQUIRED,
  TRUNCATED,
  ANIMATED,
  INVALID_ENTRY,
  SUPPORTS_AUTOCOMPLETION,
  SELECTABLE_TEXT,
  IS_DEFAULT,
  VISITED,
  CHECKABLE,
  HAS_POPUP,
  READ_ONLY,
  HIGHLIGHTED,
  HIGHLIGHTABLE,
  MAX_COUNT
};

/**
 * @brief Enumeration describing change of text object
 */
enum class TextChangedState : uint32_t
{
  INSERTED,
  DELETED,
  MAX_COUNT
};

/**
 * @brief Enumeration describing change of object property
 */
enum class ObjectPropertyChangeEvent
{
  NAME,
  DESCRIPTION,
  VALUE,
  ROLE,
  PARENT,
};

/**
 * @brief Enumeration describing change of window object
 * @see Accessibility::Accessible::Emit
 */
enum class WindowEvent
{
  PROPERTY_CHANGE,
  MINIMIZE,
  MAXIMIZE,
  RESTORE,
  CLOSE,
  CREATE,
  REPARENT,
  DESKTOP_CREATE,
  DESKTOP_DESTROY,
  DESTROY,
  ACTIVATE,
  DEACTIVATE,
  RAISE,
  LOWER,
  MOVE,
  RESIZE,
  SHADE,
  UU_SHADE,
  RESTYLE,
};

/**
 * @brief Enumeration used to acquire bounded text from accessible object having textual content.
 * @see Accessibility::Text::GetTextAtOffset
 * @note Currently only TextBoundary::Character is supported
 */
enum class TextBoundary : uint32_t
{
  CHARACTER, ///> Only one character is acquired.
  WORD,      ///> Not supported.
  SENTENCE,  ///> Not supported.
  LINE,      ///> Not supported.
  PARAGRAPH, ///> Not supported.
  MAX_COUNT
};

/**
 * @brief Enumeration describing type of gesture
 * @see Accessibility::Accessible::DoGesture
 */
enum class Gesture : int32_t
{
  ONE_FINGER_HOVER,
  TWO_FINGER_HOVER,
  THREE_FINGER_HOVER,
  ONE_FINGER_FLICK_LEFT,
  ONE_FINGER_FLICK_RIGHT,
  ONE_FINGER_FLICK_UP,
  ONE_FINGER_FLICK_DOWN,
  TWO_FINGERS_FLICK_LEFT,
  TWO_FINGERS_FLICK_RIGHT,
  TWO_FINGERS_FLICK_UP,
  TWO_FINGERS_FLICK_DOWN,
  THREE_FINGERS_FLICK_LEFT,
  THREE_FINGERS_FLICK_RIGHT,
  THREE_FINGERS_FLICK_UP,
  THREE_FINGERS_FLICK_DOWN,
  ONE_FINGER_SINGLE_TAP,
  ONE_FINGER_DOUBLE_TAP,
  ONE_FINGER_TRIPLE_TAP,
  TWO_FINGERS_SINGLE_TAP,
  TWO_FINGERS_DOUBLE_TAP,
  TWO_FINGERS_TRIPLE_TAP,
  THREE_FINGERS_SINGLE_TAP,
  THREE_FINGERS_DOUBLE_TAP,
  THREE_FINGERS_TRIPLE_TAP,
  ONE_FINGER_FLICK_LEFT_RETURN,
  ONE_FINGER_FLICK_RIGHT_RETURN,
  ONE_FINGER_FLICK_UP_RETURN,
  ONE_FINGER_FLICK_DOWN_RETURN,
  TWO_FINGERS_FLICK_LEFT_RETURN,
  TWO_FINGERS_FLICK_RIGHT_RETURN,
  TWO_FINGERS_FLICK_UP_RETURN,
  TWO_FINGERS_FLICK_DOWN_RETURN,
  THREE_FINGERS_FLICK_LEFT_RETURN,
  THREE_FINGERS_FLICK_RIGHT_RETURN,
  THREE_FINGERS_FLICK_UP_RETURN,
  THREE_FINGERS_FLICK_DOWN_RETURN,
  ONE_FINGER_DOUBLE_TAP_N_HOLD,
  TWO_FINGERS_DOUBLE_TAP_N_HOLD,
  THREE_FINGERS_DOUBLE_TAP_N_HOLD,
  MAX_COUNT
};

/**
 * @brief Enumeration indicating current state of gesture
 * @see Dali::Accessibility::GestureInfo
 */
enum class GestureState : int32_t
{
  BEGIN,
  ONGOING,
  ENDED,
  ABORTED
};

/**
 * @brief Enumeration of ReadingInfoType
 */
enum class ReadingInfoType
{
  NAME,
  ROLE,
  DESCRIPTION,
  STATE,
  MAX_COUNT
};

/**
 * @brief Enumeration of all AT-SPI interfaces.
 *
 * @see Dali::Accessibility::Accessible::GetInterfaceName()
 * @see Dali::Accessibility::AtspiInterfaceType
 */
enum class AtspiInterface
{
  ACCESSIBLE,
  ACTION,
  APPLICATION,
  CACHE,
  COLLECTION,
  COMPONENT,
  DEVICE_EVENT_CONTROLLER,
  DEVICE_EVENT_LISTENER,
  DOCUMENT,
  EDITABLE_TEXT,
  EVENT_DOCUMENT,
  EVENT_FOCUS,
  EVENT_KEYBOARD,
  EVENT_MOUSE,
  EVENT_OBJECT,
  EVENT_TERMINAL,
  EVENT_WINDOW,
  HYPERLINK,
  HYPERTEXT,
  IMAGE,
  REGISTRY,
  SELECTION,
  SOCKET,
  TABLE,
  TABLE_CELL,
  TEXT,
  VALUE,
  MAX_COUNT
};

/**
 * @brief Enumeration of all AT-SPI events.
 */
enum class AtspiEvent
{
  PROPERTY_CHANGED,
  BOUNDS_CHANGED,
  LINK_SELECTED,
  STATE_CHANGED,
  CHILDREN_CHANGED,
  VISIBLE_DATA_CHANGED,
  SELECTION_CHANGED,
  MODEL_CHANGED,
  ACTIVE_DESCENDANT_CHANGED,
  ROW_INSERTED,
  ROW_REORDERED,
  ROW_DELETED,
  COLUMN_INSERTED,
  COLUMN_REORDERED,
  COLUMN_DELETED,
  TEXT_BOUNDS_CHANGED,
  TEXT_SELECTION_CHANGED,
  TEXT_CHANGED,
  TEXT_ATTRIBUTES_CHANGED,
  TEXT_CARET_MOVED,
  ATTRIBUTES_CHANGED,
  MOVED_OUT,
  WINDOW_CHANGED,
  SCROLL_STARTED,
  SCROLL_FINISHED,
  MAX_COUNT
};

using AtspiInterfaces   = EnumBitSet<AtspiInterface, AtspiInterface::MAX_COUNT>;
using AtspiEvents       = EnumBitSet<AtspiEvent, AtspiEvent::MAX_COUNT>;
using ReadingInfoTypes  = EnumBitSet<ReadingInfoType, ReadingInfoType::MAX_COUNT>;
using States            = EnumBitSet<State, State::MAX_COUNT>;
using Attributes        = std::unordered_map<std::string, std::string>;

namespace Internal
{
/*
 * AT-SPI interfaces exposed as native C++ types should specialize this like so:
 *
 * template<>
 * struct AtspiInterfaceTypeHelper<AtspiInterface::ACCESSIBLE>
 * {
 *   using Type = Dali::Accessibility::Accessible;
 * };
 */
template<AtspiInterface I>
struct AtspiInterfaceTypeHelper; // no default definition

} // namespace Internal

/**
 * @brief Resolves to the native C++ type that represents the given AT-SPI interface.
 *
 * For example, @code AtspiInterfaceType<AtspiInterface::ACCESSIBLE> @endcode is the same as
 * @code Dali::Accessibility::Accessible @endcode. Not all AT-SPI interfaces have native C++
 * representations (in which case, such an expression will not compile).
 *
 * @tparam I Enumeration value indicating the requested AT-SPI interface.
 */
template<AtspiInterface I>
using AtspiInterfaceType = typename Internal::AtspiInterfaceTypeHelper<I>::Type;

/**
 * @brief Class representing unique object address on accessibility bus
 * @see Dali::Accessibility::Accessible::GetAddress
 */
class DALI_ADAPTOR_API Address
{
public:
  Address() = default;

  Address(std::string bus, std::string path)
  : mBus(std::move(bus)),
    mPath(std::move(path))
  {
  }

  explicit operator bool() const
  {
    return !mPath.empty();
  }

  std::string ToString() const
  {
    return *this ? mBus + ":" + mPath : "::null";
  }

  const std::string& GetBus() const;

  const std::string& GetPath() const
  {
    return mPath;
  }

  bool operator==(const Address& a) const
  {
    return mBus == a.mBus && mPath == a.mPath;
  }

  bool operator!=(const Address& a) const
  {
    return !(*this == a);
  }

private:
  mutable std::string mBus, mPath;
};

/**
 * @brief Enumeration describing type of key event
 * @see Adaptor::AccessibilityObserver::OnAccessibleKeyEvent
 */
enum class KeyEventType
{
  KEY_PRESSED,
  KEY_RELEASED,
};

/**
 * @brief Enumeration with human readable values describing state of event
 * @see Dali::Accessibility::Bridge::Emit
 */
enum class Consumed
{
  NO,
  YES
};

/**
 * @brief Helper class representing two dimensional point with integer coordinates
 */
struct DALI_ADAPTOR_API Point
{
  int x = 0;
  int y = 0;

  Point() = default;

  Point(int x, int y)
  : x(x),
    y(y)
  {
  }

  bool operator==(Point p) const
  {
    return x == p.x && y == p.y;
  }
  bool operator!=(Point p) const
  {
    return !(*this == p);
  }
};

/**
* @brief Helper class representing size of rectangle object with usage of two integer values
*/
struct DALI_ADAPTOR_API Size
{
  int width  = 0;
  int height = 0;

  Size() = default;

  Size(int w, int h)
  : width(w),
    height(h)
  {
  }

  bool operator==(Size p) const
  {
    return width == p.width && height == p.height;
  }

  bool operator!=(Size p) const
  {
    return !(*this == p);
  }
};

/**
 * @brief Helper class used to store data related with Accessibility::Text interface
 * @see Dali::Accessibility::Text::GetTextAtOffset
 * @see Dali::Accessibility::Text::GetRangeOfSelection
 */
struct DALI_ADAPTOR_API Range
{
  int32_t     startOffset = 0;
  int32_t     endOffset   = 0;
  std::string content;

  Range() = default;

  Range(size_t start, size_t end)
  : startOffset(start),
    endOffset(end)
  {
  }

  Range(size_t start, size_t end, std::string content)
  : startOffset(start),
    endOffset(end),
    content(content)
  {
  }
};

/**
 * @brief Structure containing all values needed to invoke Accessible::DoGesture
 * type : numerated gesture type
 * startPositionX, startPositionY : point where gesture begins
 * endPositionX, endPositionY : point where gesture ends
 * state : enumerated state of gesture
 * eventTime : time when event occured
 * @see Dali::Accessibility::Accessible::DoGesture
 */
struct DALI_ADAPTOR_API GestureInfo
{
  GestureInfo() = default;
  GestureInfo(Gesture type, int32_t startPositionX, int32_t endPositionX, int32_t startPositionY, int32_t endPositionY, GestureState state, uint32_t eventTime)
  : type(type),
    startPointX(startPositionX),
    endPointX(endPositionX),
    startPointY(startPositionY),
    endPointY(endPositionY),
    state(state),
    eventTime(eventTime)
  {
  }

  Gesture      type{};
  int32_t      startPointX{};
  int32_t      endPointX{};
  int32_t      startPointY{};
  int32_t      endPointY{};
  GestureState state{};
  uint32_t     eventTime{};
};

/**
 * @brief Class representing accessibility relations
 *
 * Class connecting one source object with multiple target objects with usage
 * of specific relation type.
 *
 * A remote target object (i.e. one belonging to a different process) can be
 * represented in terms of a ProxyAccessible.
 *
 * @see Dali::Accessibility::Accessible::Accessible
 * @see Dali::Accessibility::Accessible::RelationType
 */
struct DALI_ADAPTOR_API Relation
{
  Relation(RelationType relationType, const std::vector<Accessible*>& targets)
  : mRelationType(relationType),
    mTargets(targets)
  {
  }

  RelationType             mRelationType;
  std::vector<Accessible*> mTargets;
};

} // namespace Accessibility
} // namespace Dali

#endif // DALI_ATSPI_ACCESSIBILITY_H
