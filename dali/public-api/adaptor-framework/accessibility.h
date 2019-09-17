#ifndef DALI_ACCESSIBILITY_H
#define DALI_ACCESSIBILITY_H

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

#include <atomic>
#include <bitset>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <cassert>
#include <sstream>
#include <string.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */
class Actor;

namespace Accessibility
{

/**
 * @brief Enumeration describing a relation between accessible objects
 *
 * 1 to 0..N relation model is supported. By default relation is not symmetrical.
 * Symmetry must be explicitly maintained.
 */
enum class RelationType : uint32_t
{
  NULL_,
  LABEL_FOR,
  LABELLED_BY,
  CONTROLLER_FOR,
  CONTROLLED_BY,
  MEMBER_OF,
  TOOLTIP_FOR,
  NODE_CHILD_OF,
  NODE_PARENT_OF,
  EXTENDED,
  FLOWS_TO,
  FLOWS_FROM,
  SUBWINDOW_OF,
  EMBEDS,
  EMBEDDED_BY,
  POPUP_FOR,
  PARENT_WINDOW_OF,
  DESCRIPTION_FOR,
  DESCRIBED_BY,
  DETAILS,
  DETAILS_FOR,
  ERROR_MESSAGE,
  ERROR_FOR,
  LAST_DEFINED
};

/**
 * @brief Enumeration describing if coordinates are relative to screen or window
 *
 * @see Accessibility::Component::GetExtents
 * @see Accessibility::Component::Contains
 */
enum class CoordType
{
  SCREEN,
  WINDOW
};

/**
 * @brief Enumeration indicating relative stacking order
 *
 * ComponentLayer allows to describe visibility of all parts of UI
 * basing on the concrete stacking order
 *
 * @see Accessibility::Component::GetLayer
 *
 * @note currently only ComponentLayer::Window is supported
 */
enum class ComponentLayer
{
  INVALID,
  BACKGROUND,
  CANVAS,
  WIDGET,
  MDI,
  POPUP,
  OVERLAY,
  WINDOW,
  LAST_DEFINED,
};

/**
 * @brief Enumeration describing role of the Accessibility object
 *
 * Object can be described by only one role.
 *
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
  _COUNT,
};

/**
 * @brief Enumeration describing states of the Accessibility object
 *
 * Object can be in many states at the same time.
 *
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
  _COUNT
};

/**
 * @brief Enumeration describing change of text object
 */
enum class TextChangedState : uint32_t
{
  INSERT,
  DELETE,
  _COUNT
};

enum class ObjectPropertyChangeEvent {
   NAME,
   DESCRIPTION,
   VALUE,
   ROLE,
   PARENT,
};

/**
 * @brief Enumeration describing change of window object
 *
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
 *
 * @see Accessibility::Text::GetTextAtOffset
 *
 * @note Currently only TextBoundary::Character is supported
 */
enum class TextBoundary : uint32_t
{
  /**
   * Only one character is acquired.
   */
  CHARACTER,

  /**
   * Not supported.
   */
  WORD,

  /**
   * Not supported.
   */
  SENTENCE,

  /**
   * Not supported.
   */
  LINE,

  /**
   * Not supported.
   */
  PARAGRAPH,
  _COUNT
};

/**
 * @brief Enumeration describing type of gesture
 *
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
  _COUNT
};

/**
 * @brief Enumeration indicating current state of gesture
 *
 * @see Dali::Accessibility::GestureInfo
 */
enum class GestureState : int32_t
{
  BEGIN,
  ONGOING,
  ENDED,
  ABORTED
};

enum class ReadingInfoType
{
  NAME,
  ROLE,
  DESCRIPTION,
  STATE
};

/**
 * @brief Helper class for storing values treated as bit sets
 *
 * This class provides all bitset-like methods for bitset size larger, than long long int
 *
 * @see Dali::Accessibility::Accessible::GetStates
 * @see Dali::Accessibility::Accessible::GetRoles
 */
template < size_t I, typename S >
class BitSets
{
  std::array< uint32_t, I > data;

  void _set() { }
  static constexpr bool _accepts() { return true; }
  template < typename T > static constexpr bool _accepts()
  {
    return std::is_enum< T >::value;
  }
  template < typename T, typename T2, typename ... ARGS > static constexpr bool _accepts()
  {
    return std::is_enum< T >::value && _accepts< T2, ARGS... >();
  }
  template < typename T, typename ... ARGS > void _set( T t, ARGS ... args )
  {
    ( *this )[ t ] = true;
    _set( args... );
  }
public:
  BitSets()
  {
    for( auto& u : data )
      u = 0;
  }
  BitSets( const BitSets & ) = default;
  BitSets( BitSets&& ) = default;
  template < typename T, typename ... ARGS, typename std::enable_if< _accepts< T, ARGS... >() >::type* = nullptr >BitSets(T t, ARGS ... args) {
    for(auto& u : data)
      u = 0;
    _set(t, args...);
  }
  explicit BitSets( std::array< uint32_t, I > d ) {
    for( auto i = 0u; i < I; ++i )
      data[ i ] = d[ i ];
  }
  explicit BitSets( std::array< int32_t, I > d ) {
    for( auto i = 0u; i < I; ++i )
      data[ i ] = static_cast< uint32_t >( d[ i ]);
  }
  BitSets &operator = ( const BitSets& ) = default;
  BitSets &operator = ( BitSets&& ) = default;

  struct reference
  {
    std::array< uint32_t, I >& data;
    size_t pos;
    reference &operator=( reference r )
    {
      ( *this ) = static_cast< bool >( r );
      return *this;
    }
    reference &operator=( bool v )
    {
      if( v )
        data[ pos / 32 ] |= 1 << ( pos & 31 );
      else
        data[ pos / 32 ] &= ~( 1 << ( pos & 31 ) );
      return *this;
    }
    operator bool() const
    {
      auto i = static_cast< size_t >( pos );
      return ( data[i / 32] & ( 1 << ( i & 31 ) ) ) != 0;
    }
  };
  reference operator[]( S index ) { return {data, static_cast< size_t >( index )}; }
  bool operator[]( S index ) const
  {
    auto i = static_cast< size_t >( index );
    return ( data[i / 32] & ( 1 << ( i & 31 ) ) ) != 0;
  }
  std::array< uint32_t, I > GetRawData() const { return data; }

  BitSets operator|( BitSets b ) const
  {
    BitSets r;
    for( auto i = 0u; i < I; ++i )
      r.data[i] = data[i] | b.data[i];
    return r;
  }
  BitSets operator^( BitSets b ) const
  {
    BitSets r;
    for( auto i = 0u; i < I; ++i )
      r.data[i] = data[i] ^ b.data[i];
    return r;
  }
  BitSets operator&( BitSets b ) const
  {
    BitSets r;
    for( auto i = 0u; i < I; ++i )
      r.data[i] = data[i] & b.data[i];
    return r;
  }
  bool operator==( BitSets b ) const
  {
    for( auto i = 0u; i < I; ++i )
      if( data[i] != b.data[i] )
        return false;
    return true;
  }
  bool operator!=( BitSets b ) const
  {
    return !((*this) == b);
  }
  explicit operator bool() const
  {
    for( auto& u : data )
      if( u )
        return true;
    return false;
  }
  size_t size() const { return I; }
};

using ReadingInfoTypes = BitSets<1, ReadingInfoType>;
using States = BitSets< 2, State >;
using Attributes = std::unordered_map< std::string, std::string >;

/**
 * @brief Class representing unique object address on accessibility bus
 *
 * @see Dali::Accessibility::Accessible::GetAddress
 */
class DALI_ADAPTOR_API Address
{
public:
  Address() = default;
  Address( std::string bus, std::string path ) : bus( std::move( bus ) ), path( std::move( path ) ) {}

  explicit operator bool() const { return !path.empty(); }
  std::string ToString() const
  {
    return *this ? bus + ":" + path : "::null";
  }
  const std::string& GetBus() const;
  const std::string& GetPath() const { return path; }

  bool operator == (const Address &a) const {
    return bus == a.bus && path == a.path;
  }
  bool operator != (const Address &a) const {
    return !(*this == a);
  }
private:
  mutable std::string bus, path;
};

/**
 * @brief Enumeration describing type of key event
 *
 * @see Adaptor::AccessibilityObserver::OnAccessibleKeyEvent
 */
enum class KeyEventType
{
  KEY_PRESSED,
  KEY_RELEASED,
};

/**
 * @brief Enumeration with human readable values describing state of event
 *
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
  Point( int x, int y ) : x( x ), y( y ) {}

  bool operator==( Point p ) const
  {
    return x == p.x && y == p.y;
  }
  bool operator!=( Point p ) const
  {
    return !( *this == p );
  }
};

/**
 * @brief Helper class representing size of rectangle object with usage of two integer values
 */
struct DALI_ADAPTOR_API Size
{
  int width = 0;
  int height = 0;

  Size() = default;
  Size( int w, int h ) : width( w ), height( h ) {}

  bool operator==( Size p ) const
  {
    return width == p.width && height == p.height;
  }
  bool operator!=( Size p ) const
  {
    return !( *this == p );
  }
};

/**
 * @brief Helper class used to store data related with Accessibility::Text interface
 *
 * @see Dali::Accessibility::Text::GetTextAtOffset
 * @see Dali::Accessibility::Text::GetSelection
 */
struct DALI_ADAPTOR_API Range
{
  int startOffset = 0;
  int endOffset = 0;
  std::string content;

  Range() = default;
  Range( size_t start, size_t end ) : startOffset( start ), endOffset( end )
  {
  }
  Range( size_t start, size_t end, std::string content ) : startOffset( start ), endOffset( end ), content( content )
  {
  }
};

/**
 * @brief Structure containing all values needed to invoke Accessible::DoGesture
 *
 * type
 *     enumerated gesture type
 * xBeg, yBeg
 *     point where gesture begins
 * xEnd, yEnd
 *     point where gesture ends
 * state
 *     enumerated state of gesture
 * eventTime
 *     time when event occured
 *
 * @see Dali::Accessibility::Accessible::DoGesture
 */
struct DALI_ADAPTOR_API GestureInfo
{
  GestureInfo() = default;
  GestureInfo(Gesture type, int32_t xBeg, int32_t xEnd, int32_t yBeg, int32_t yEnd, GestureState state, uint32_t eventTime)
    : type(type), xBeg(xBeg), xEnd(xEnd), yBeg(yBeg), yEnd(yEnd), state(state), eventTime(eventTime)
  {}

  Gesture type{};
  int32_t xBeg{};
  int32_t xEnd{};
  int32_t yBeg{};
  int32_t yEnd{};
  GestureState state{};
  uint32_t eventTime{};
};

class Accessible;

/**
 * @brief Class representing accessibility relations
 *
 * Class connecting one source object with multiple target objects with usage
 * of specific relation type.
 *
 * @note std::string representing source and targets are string values of Accessibility::Address
 *
 * @see Dali::Accessibility::Accessible::Address
 * @see Dali::Accessibility::Accessible::RelationType
 */
struct DALI_ADAPTOR_API Relation
{
  Relation(RelationType relationType, std::vector<Address> targets)
    : relationType(relationType), targets(targets)
  {}
  RelationType relationType;
  std::vector<Address> targets;
};

/**
 * @brief Reads given text by screen reader
 *
 * @param text The text to read
 * @param discardable If TRUE, reading can be discarded by subsequent reading requests,
 * if FALSE the reading must finish before next reading request can be started
 * @param callback the callback function that is called on reading signals emitted
 * during processing of this reading request.
 * Callback can be one of the following signals:
 * “ReadingCancelled”
 * “ReadingStopped”
 * “ReadingSkipped”
 */
DALI_ADAPTOR_API void Say( const std::string &text, bool discardable, std::function<void(std::string)> callback );

/**
 * @brief Force accessibility client to pause / resume.
 *
 * @param[in] pause true if you want to pause, false if you want to resume
 */
DALI_ADAPTOR_API void PauseResume( bool pause );

/// \endcond
} // namespace Accessibility
/**
 * @}
 */
} // namespace Dali

namespace std
{
template <>
struct hash< Dali::Accessibility::Point >
{
  size_t operator()( Dali::Accessibility::Point p )
  {
    return static_cast< size_t >( p.x ) ^ ( static_cast< size_t >( p.y ) * 11 );
  }
};
template <>
struct hash< Dali::Accessibility::Size >
{
  size_t operator()( Dali::Accessibility::Size p )
  {
    return static_cast< size_t >( p.width ) ^ ( static_cast< size_t >( p.height ) * 11 );
  }
};
}

#endif // DALI_ACCESSIBILITY_H
