#ifndef __DALI_ACCESSIBILITY_H__
#define __DALI_ACCESSIBILITY_H__

#include <dali/public-api/actors/actor.h>

#include <atomic>
#include <bitset>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Dali
{
class Actor;

namespace Accessibility
{
enum class RelationType
{
  FlowsFrom,
  FlowsTo
};

enum class MatchType : int32_t
{
  Invalid,
  All,
  Any,
  None,
  Empty
};

enum class SortOrder : uint32_t
{
  Invalid,
  Canonical,
  Flow,
  Tab,
  ReverseCanonical,
  ReverseFlow,
  ReverseTab,
  LastDefined
};

enum class CoordType
{
  Screen,
  Window
};

enum class NeighborSearchDirection
{
  Forward,
  Backward
};

enum class ComponentLayer
{
  Invalid,
  Background,
  Canvas,
  Widget,
  Mdi,
  Popup,
  Overlay,
  Window,
  LastDefined,
};

enum class Role : uint32_t
{
  Invalid,
  AcceleratorLabel,
  Alert,
  Animation,
  Arrow,
  Calendar,
  Canvas,
  CheckBox,
  CheckMenuItem,
  ColorChooser,
  ColumnHeader,
  ComboBox,
  DateEditor,
  DesktopIcon,
  DesktopFrame,
  Dial,
  Dialog,
  DirectoryPane,
  DrawingArea,
  FileChooser,
  Filler,
  FocusTraversable,
  FontChooser,
  Frame,
  GlassPane,
  HtmlContainer,
  Icon,
  Image,
  InternalFrame,
  Label,
  LayeredPane,
  List,
  ListItem,
  Menu,
  MenuBar,
  MenuItem,
  OptionPane,
  PageTab,
  PageTabList,
  Panel,
  PasswordText,
  PopupMenu,
  ProgressBar,
  PushButton,
  RadioButton,
  RadioMenuItem,
  RootPane,
  RowHeader,
  ScrollBar,
  ScrollPane,
  Separator,
  Slider,
  SpinButton,
  SplitPane,
  StatusBar,
  Table,
  TableCell,
  TableColumnHeader,
  TableRowHeader,
  TearoffMenuItem,
  Terminal,
  Text,
  ToggleButton,
  ToolBar,
  ToolTip,
  Tree,
  TreeTable,
  Unknown,
  Viewport,
  Window,
  Extended,
  Header,
  Footer,
  Paragraph,
  Ruler,
  Application,
  Autocomplete,
  Editbar,
  Embedded,
  Entry,
  Chart,
  Caption,
  DocumentFrame,
  Heading,
  Page,
  Section,
  RedundantObject,
  Form,
  Link,
  InputMethodWindow,
  TableRow,
  TreeItem,
  DocumentSpreadsheet,
  DocumentPresentation,
  DocumentText,
  DocumentWeb,
  DocumentEmail,
  Comment,
  ListBox,
  Grouping,
  ImageMap,
  Notification,
  InfoBar,
  LevelBar,
  TitleBar,
  BlockQuote,
  Audio,
  Video,
  Definition,
  Article,
  Landmark,
  Log,
  Marquee,
  Math,
  Rating,
  Timer,
  Static,
  MathFraction,
  MathRoot,
  Subscript,
  Superscript,
  _Count,
};

enum class State : uint32_t
{
  Invalid,
  Active,
  Armed,
  Busy,
  Checked,
  Collapsed,
  Defunct,
  Editable,
  Enabled,
  Expandable,
  Expanded,
  Focusable,
  Focused,
  HasTooltip,
  Horizontal,
  Iconified,
  Modal,
  MultiLine,
  MultiSelectable,
  Opaque,
  Pressed,
  Resizeable,
  Selectable,
  Selected,
  Sensitive,
  Showing,
  SingleLine,
  Stale,
  Transient,
  Vertical,
  Visible,
  ManagesDescendants,
  Indeterminate,
  Required,
  Truncated,
  Animated,
  InvalidEntry,
  SupportsAutocompletion,
  SelectableText,
  IsDefault,
  Visited,
  Checkable,
  HasPopup,
  ReadOnly,
  Highlighted,
  Highlightable,
  _Count
};

enum class WindowEvent
{
  PropertyChange,
  Minimize,
  Maximize,
  Restore,
  Close,
  Create,
  Reparent,
  DesktopCreate,
  DesktopDestroy,
  Destroy,
  Activate,
  Deactivate,
  Raise,
  Lower,
  Move,
  Resize,
  Shade,
  UuShade,
  Restyle,
};

enum class TextBoundary : uint32_t
{
  Character,
  WordStart,
  WordEnd,
  SentenceStart,
  SentenceEnd,
  LineStart,
  LineEnd,
  _Count
};

template < size_t I, typename S >
class BitStates
{
  std::array< uint32_t, I > data;

public:
  BitStates()
  {
    for( auto& u : data )
      u = 0;
  }
  explicit BitStates( std::array< uint32_t, I > d )
  {
    for( auto i = 0u; i < I; ++i )
      data[i] = d[i];
  }
  explicit BitStates( std::array< int32_t, I > d )
  {
    for( auto i = 0u; i < I; ++i )
      data[i] = static_cast< uint32_t >( d[i] );
  }

  struct reference
  {
    std::array< uint32_t, I >& data;
    size_t pos;
    bool operator=( reference r )
    {
      return ( *this ) = static_cast< bool >( r );
    }
    bool operator=( bool v )
    {
      if( v )
        data[pos / 32] |= 1 << ( pos & 31 );
      else
        data[pos / 32] &= ~( 1 << ( pos & 31 ) );
      return v;
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

  BitStates operator|( BitStates b ) const
  {
    BitStates r;
    for( auto i = 0u; i < I; ++i )
      r.data[i] = data[i] | b.data[i];
    return r;
  }
  BitStates operator&( BitStates b ) const
  {
    BitStates r;
    for( auto i = 0u; i < I; ++i )
      r.data[i] = data[i] & b.data[i];
    return r;
  }
  bool operator==( BitStates b ) const
  {
    for( auto i = 0u; i < I; ++i )
      if( data[i] != b.data[i] )
        return false;
    return true;
  }
  explicit operator bool() const
  {
    for( auto& u : data )
      if( u )
        return true;
    return false;
  }
};

using States = BitStates< 2, State >;
using Attributes = std::unordered_map< std::string, std::string >;

class DALI_IMPORT_API Address
{
public:
  Address() = default;
  Address( std::string bus, std::string path ) : bus( std::move( bus ) ), path( std::move( path ) ) {}

  explicit operator bool() const { return !path.empty(); }
  std::string ToString() const
  {
    return *this ? bus + ":" + path : "::null";
  }
  const std::string& GetBus() const { return bus; }
  const std::string& GetPath() const { return path; }

private:
  std::string bus, path;
};

enum class KeyEventType
{
  KeyPressed,
  KeyReleased,
};
enum class Consumed
{
  No,
  Yes
};
struct DALI_IMPORT_API Point
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

struct DALI_IMPORT_API Size
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

struct DALI_IMPORT_API Rectangle
{
  Point position;
  Size size;

  bool operator==( Rectangle p ) const
  {
    return position == p.position && size == p.size;
  }
  bool operator!=( Rectangle p ) const
  {
    return !( *this == p );
  }
};
struct DALI_IMPORT_API Range
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

class DALI_IMPORT_API AccessibleError : public std::exception
{
public:
  AccessibleError( std::string msg ) : msg( std::move( msg ) ) {}
  const char* what() const noexcept override { return msg.c_str(); }

private:
  std::string msg;
};
}
}

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
template <>
struct hash< Dali::Accessibility::Rectangle >
{
  size_t operator()( Dali::Accessibility::Rectangle p )
  {
    return hash< Dali::Accessibility::Point >()( p.position ) ^ ( hash< Dali::Accessibility::Size >()( p.size ) * 11 );
  }
};
}

#endif
