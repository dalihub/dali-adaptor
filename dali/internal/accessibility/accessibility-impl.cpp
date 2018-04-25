#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/object-registry.h>
#include <dali/public-api/object/type-info.h>
#include <dali/public-api/object/type-registry-helper.h>

using namespace Dali::Accessibility;

std::string EmptyAccessibleWithAddress::GetRoleName()
{
  return "";
}

std::string Accessible::GetLocalizedRoleName()
{
  return GetRoleName();
}

std::string Accessible::GetRoleName()
{
  switch( GetRole() )
  {
    case Role::Invalid:
    {
      return "invalid";
    }
    case Role::AcceleratorLabel:
    {
      return "accelerator label";
    }
    case Role::Alert:
    {
      return "alert";
    }
    case Role::Animation:
    {
      return "animation";
    }
    case Role::Arrow:
    {
      return "arrow";
    }
    case Role::Calendar:
    {
      return "calendar";
    }
    case Role::Canvas:
    {
      return "canvas";
    }
    case Role::CheckBox:
    {
      return "check box";
    }
    case Role::CheckMenuItem:
    {
      return "check menu item";
    }
    case Role::ColorChooser:
    {
      return "color chooser";
    }
    case Role::ColumnHeader:
    {
      return "column header";
    }
    case Role::ComboBox:
    {
      return "combo box";
    }
    case Role::DateEditor:
    {
      return "date editor";
    }
    case Role::DesktopIcon:
    {
      return "desktop icon";
    }
    case Role::DesktopFrame:
    {
      return "desktop frame";
    }
    case Role::Dial:
    {
      return "dial";
    }
    case Role::Dialog:
    {
      return "dialog";
    }
    case Role::DirectoryPane:
    {
      return "directory pane";
    }
    case Role::DrawingArea:
    {
      return "drawing area";
    }
    case Role::FileChooser:
    {
      return "file chooser";
    }
    case Role::Filler:
    {
      return "filler";
    }
    case Role::FocusTraversable:
    {
      return "focus traversable";
    }
    case Role::FontChooser:
    {
      return "font chooser";
    }
    case Role::Frame:
    {
      return "frame";
    }
    case Role::GlassPane:
    {
      return "glass pane";
    }
    case Role::HtmlContainer:
    {
      return "html container";
    }
    case Role::Icon:
    {
      return "icon";
    }
    case Role::Image:
    {
      return "image";
    }
    case Role::InternalFrame:
    {
      return "internal frame";
    }
    case Role::Label:
    {
      return "label";
    }
    case Role::LayeredPane:
    {
      return "layered pane";
    }
    case Role::List:
    {
      return "list";
    }
    case Role::ListItem:
    {
      return "list item";
    }
    case Role::Menu:
    {
      return "menu";
    }
    case Role::MenuBar:
    {
      return "menu bar";
    }
    case Role::MenuItem:
    {
      return "menu item";
    }
    case Role::OptionPane:
    {
      return "option pane";
    }
    case Role::PageTab:
    {
      return "page tab";
    }
    case Role::PageTabList:
    {
      return "page tab list";
    }
    case Role::Panel:
    {
      return "panel";
    }
    case Role::PasswordText:
    {
      return "password text";
    }
    case Role::PopupMenu:
    {
      return "popup menu";
    }
    case Role::ProgressBar:
    {
      return "progress bar";
    }
    case Role::PushButton:
    {
      return "push button";
    }
    case Role::RadioButton:
    {
      return "radio button";
    }
    case Role::RadioMenuItem:
    {
      return "radio menu item";
    }
    case Role::RootPane:
    {
      return "root pane";
    }
    case Role::RowHeader:
    {
      return "row header";
    }
    case Role::ScrollBar:
    {
      return "scroll bar";
    }
    case Role::ScrollPane:
    {
      return "scroll pane";
    }
    case Role::Separator:
    {
      return "separator";
    }
    case Role::Slider:
    {
      return "slider";
    }
    case Role::SpinButton:
    {
      return "spin button";
    }
    case Role::SplitPane:
    {
      return "split pane";
    }
    case Role::StatusBar:
    {
      return "status bar";
    }
    case Role::Table:
    {
      return "table";
    }
    case Role::TableCell:
    {
      return "table cell";
    }
    case Role::TableColumnHeader:
    {
      return "table column header";
    }
    case Role::TableRowHeader:
    {
      return "table row header";
    }
    case Role::TearoffMenuItem:
    {
      return "tearoff menu item";
    }
    case Role::Terminal:
    {
      return "terminal";
    }
    case Role::Text:
    {
      return "text";
    }
    case Role::ToggleButton:
    {
      return "toggle button";
    }
    case Role::ToolBar:
    {
      return "tool bar";
    }
    case Role::ToolTip:
    {
      return "tool tip";
    }
    case Role::Tree:
    {
      return "tree";
    }
    case Role::TreeTable:
    {
      return "tree table";
    }
    case Role::Unknown:
    {
      return "unknown";
    }
    case Role::Viewport:
    {
      return "viewport";
    }
    case Role::Window:
    {
      return "window";
    }
    case Role::Extended:
    {
      return "extended";
    }
    case Role::Header:
    {
      return "header";
    }
    case Role::Footer:
    {
      return "footer";
    }
    case Role::Paragraph:
    {
      return "paragraph";
    }
    case Role::Ruler:
    {
      return "ruler";
    }
    case Role::Application:
    {
      return "application";
    }
    case Role::Autocomplete:
    {
      return "autocomplete";
    }
    case Role::Editbar:
    {
      return "edit bar";
    }
    case Role::Embedded:
    {
      return "embedded";
    }
    case Role::Entry:
    {
      return "entry";
    }
    case Role::Chart:
    {
      return "chart";
    }
    case Role::Caption:
    {
      return "caution";
    }
    case Role::DocumentFrame:
    {
      return "document frame";
    }
    case Role::Heading:
    {
      return "heading";
    }
    case Role::Page:
    {
      return "page";
    }
    case Role::Section:
    {
      return "section";
    }
    case Role::RedundantObject:
    {
      return "redundant object";
    }
    case Role::Form:
    {
      return "form";
    }
    case Role::Link:
    {
      return "link";
    }
    case Role::InputMethodWindow:
    {
      return "input method window";
    }
    case Role::TableRow:
    {
      return "table row";
    }
    case Role::TreeItem:
    {
      return "tree item";
    }
    case Role::DocumentSpreadsheet:
    {
      return "document spreadsheet";
    }
    case Role::DocumentPresentation:
    {
      return "document presentation";
    }
    case Role::DocumentText:
    {
      return "document text";
    }
    case Role::DocumentWeb:
    {
      return "document web";
    }
    case Role::DocumentEmail:
    {
      return "document email";
    }
    case Role::Comment:
    {
      return "comment";
    }
    case Role::ListBox:
    {
      return "list box";
    }
    case Role::Grouping:
    {
      return "grouping";
    }
    case Role::ImageMap:
    {
      return "image map";
    }
    case Role::Notification:
    {
      return "notification";
    }
    case Role::InfoBar:
    {
      return "info bar";
    }
    case Role::LevelBar:
    {
      return "level bar";
    }
    case Role::TitleBar:
    {
      return "title bar";
    }
    case Role::BlockQuote:
    {
      return "block quote";
    }
    case Role::Audio:
    {
      return "audio";
    }
    case Role::Video:
    {
      return "video";
    }
    case Role::Definition:
    {
      return "definition";
    }
    case Role::Article:
    {
      return "article";
    }
    case Role::Landmark:
    {
      return "landmark";
    }
    case Role::Log:
    {
      return "log";
    }
    case Role::Marquee:
    {
      return "marquee";
    }
    case Role::Math:
    {
      return "math";
    }
    case Role::Rating:
    {
      return "rating";
    }
    case Role::Timer:
    {
      return "timer";
    }
    case Role::Static:
    {
      return "static";
    }
    case Role::MathFraction:
    {
      return "math fraction";
    }
    case Role::MathRoot:
    {
      return "math root";
    }
    case Role::Subscript:
    {
      return "subscript";
    }
    case Role::Superscript:
    {
      return "superscript";
    }
    case Role::_Count:
    {
      break;
    }
  }
  return "";
}

void Bridge::SetIsOnRootLevel( Accessible* o )
{
  o->isOnRootLevel = true;
}

class NonControlAccessible : public virtual Accessible, public virtual Collection
{
public:
  Dali::Actor actor;
  bool root = false;

  NonControlAccessible( Dali::Actor actor ) : actor( actor )
  {
    Dali::Stage stage = Dali::Stage::GetCurrent();
    root = stage.GetRootLayer() == actor;
  }

  std::string GetName() override
  {
    return actor.GetName();
  }
  std::string GetDescription() override
  {
    return "";
  }
  Accessible* GetParent() override
  {
    if( GetIsOnRootLevel() )
    {
      auto b = GetBridgeData();
      return b->bridge->GetApplication();
    }
    return Get( actor.GetParent() );
  }
  size_t GetChildCount() override
  {
    return static_cast< size_t >( actor.GetChildCount() );
  }
  Accessible* GetChildAtIndex( size_t index ) override
  {
    auto s = static_cast< size_t >( actor.GetChildCount() );
    if( index >= s )
      throw AccessibleError{"invalid index " + std::to_string( index ) + " for object with " + std::to_string( s ) + " children"};
    return Get( actor.GetChildAt( static_cast< unsigned int >( index ) ) );
  }
  size_t GetIndexInParent() override
  {
    auto p = actor.GetParent();
    if( !p )
      return 0;
    auto s = static_cast< size_t >( p.GetChildCount() );
    for( auto i = 0u; i < s; ++i )
    {
      if( p.GetChildAt( i ) == actor )
        return i;
    }
    throw AccessibleError{"actor is not a child of it's parent"};
  }
  Role GetRole() override
  {
    return root ? Role::Window : Role::RedundantObject;
  }
  States GetStates() override
  {
    States s;
    if( root )
    {
      s[State::Highlightable] = true;
      s[State::Enabled] = true;
      s[State::Sensitive] = true;
      s[State::Showing] = true;
      s[State::Active] = true;
    }
    else
    {
      auto t = GetParent()->GetStates();
      s[State::Showing] = t[State::Showing];
      s[State::Visible] = t[State::Visible];
    }
    return s;
  }
  Attributes GetAttributes() override
  {
    Dali::TypeInfo type;
    actor.GetTypeInfo( type );
    return {
        {"t", type.GetName()},
    };
  }
};

using NonControlAccessiblesType = std::unordered_map< const Dali::RefObject*, std::unique_ptr< NonControlAccessible > >;
static NonControlAccessiblesType nonControlAccessibles;

static std::function< Accessible*( Dali::Actor ) > convertingFunctor = []( Dali::Actor ) -> Accessible* {
  return nullptr;
};

void Accessible::RegisterControlAccessibilityGetter( std::function< Accessible*( Dali::Actor ) > functor )
{
  convertingFunctor = functor;
}

Accessible* Accessible::Get( Dali::Actor actor )
{
  if( !actor )
  {
    return nullptr;
  }
  auto p = convertingFunctor( actor );
  if( !p )
  {
    if( nonControlAccessibles.empty() )
    {
      auto registry = Dali::Stage::GetCurrent().GetObjectRegistry();
      registry.ObjectDestroyedSignal().Connect( []( const Dali::RefObject* obj ) {
        nonControlAccessibles.erase( obj );
      } );
    }
    auto it = nonControlAccessibles.emplace( &actor.GetBaseObject(), nullptr );
    if( it.second )
    {
      it.first->second.reset( new NonControlAccessible( actor ) );
    }
    p = it.first->second.get();
  }
  return p;
}
