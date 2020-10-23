/*
 * Copyright 2020  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// CLASS HEADER

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/object-registry.h>
#include <dali/public-api/object/type-info.h>
#include <dali/public-api/object/type-registry-helper.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/adaptor-framework/accessibility-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

using namespace Dali::Accessibility;
using namespace Dali;

const std::string& Dali::Accessibility::Address::GetBus() const
{
  return mBus.empty() && Bridge::GetCurrentBridge() ? Bridge::GetCurrentBridge()->GetBusName() : mBus;
}

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
    case Role::INVALID:
    {
      return "invalid";
    }
    case Role::ACCELERATOR_LABEL:
    {
      return "accelerator label";
    }
    case Role::ALERT:
    {
      return "alert";
    }
    case Role::ANIMATION:
    {
      return "animation";
    }
    case Role::ARROW:
    {
      return "arrow";
    }
    case Role::CALENDAR:
    {
      return "calendar";
    }
    case Role::CANVAS:
    {
      return "canvas";
    }
    case Role::CHECK_BOX:
    {
      return "check box";
    }
    case Role::CHECK_MENU_ITEM:
    {
      return "check menu item";
    }
    case Role::COLOR_CHOOSER:
    {
      return "color chooser";
    }
    case Role::COLUMN_HEADER:
    {
      return "column header";
    }
    case Role::COMBO_BOX:
    {
      return "combo box";
    }
    case Role::DATE_EDITOR:
    {
      return "date editor";
    }
    case Role::DESKTOP_ICON:
    {
      return "desktop icon";
    }
    case Role::DESKTOP_FRAME:
    {
      return "desktop frame";
    }
    case Role::DIAL:
    {
      return "dial";
    }
    case Role::DIALOG:
    {
      return "dialog";
    }
    case Role::DIRECTORY_PANE:
    {
      return "directory pane";
    }
    case Role::DRAWING_AREA:
    {
      return "drawing area";
    }
    case Role::FILE_CHOOSER:
    {
      return "file chooser";
    }
    case Role::FILLER:
    {
      return "filler";
    }
    case Role::FOCUS_TRAVERSABLE:
    {
      return "focus traversable";
    }
    case Role::FONT_CHOOSER:
    {
      return "font chooser";
    }
    case Role::FRAME:
    {
      return "frame";
    }
    case Role::GLASS_PANE:
    {
      return "glass pane";
    }
    case Role::HTML_CONTAINER:
    {
      return "html container";
    }
    case Role::ICON:
    {
      return "icon";
    }
    case Role::IMAGE:
    {
      return "image";
    }
    case Role::INTERNAL_FRAME:
    {
      return "internal frame";
    }
    case Role::LABEL:
    {
      return "label";
    }
    case Role::LAYERED_PANE:
    {
      return "layered pane";
    }
    case Role::LIST:
    {
      return "list";
    }
    case Role::LIST_ITEM:
    {
      return "list item";
    }
    case Role::MENU:
    {
      return "menu";
    }
    case Role::MENU_BAR:
    {
      return "menu bar";
    }
    case Role::MENU_ITEM:
    {
      return "menu item";
    }
    case Role::OPTION_PANE:
    {
      return "option pane";
    }
    case Role::PAGE_TAB:
    {
      return "page tab";
    }
    case Role::PAGE_TAB_LIST:
    {
      return "page tab list";
    }
    case Role::PANEL:
    {
      return "panel";
    }
    case Role::PASSWORD_TEXT:
    {
      return "password text";
    }
    case Role::POPUP_MENU:
    {
      return "popup menu";
    }
    case Role::PROGRESS_BAR:
    {
      return "progress bar";
    }
    case Role::PUSH_BUTTON:
    {
      return "push button";
    }
    case Role::RADIO_BUTTON:
    {
      return "radio button";
    }
    case Role::RADIO_MENU_ITEM:
    {
      return "radio menu item";
    }
    case Role::ROOT_PANE:
    {
      return "root pane";
    }
    case Role::ROW_HEADER:
    {
      return "row header";
    }
    case Role::SCROLL_BAR:
    {
      return "scroll bar";
    }
    case Role::SCROLL_PANE:
    {
      return "scroll pane";
    }
    case Role::SEPARATOR:
    {
      return "separator";
    }
    case Role::SLIDER:
    {
      return "slider";
    }
    case Role::SPIN_BUTTON:
    {
      return "spin button";
    }
    case Role::SPLIT_PANE:
    {
      return "split pane";
    }
    case Role::STATUS_BAR:
    {
      return "status bar";
    }
    case Role::TABLE:
    {
      return "table";
    }
    case Role::TABLE_CELL:
    {
      return "table cell";
    }
    case Role::TABLE_COLUMN_HEADER:
    {
      return "table column header";
    }
    case Role::TABLE_ROW_HEADER:
    {
      return "table row header";
    }
    case Role::TEAROFF_MENU_ITEM:
    {
      return "tearoff menu item";
    }
    case Role::TERMINAL:
    {
      return "terminal";
    }
    case Role::TEXT:
    {
      return "text";
    }
    case Role::TOGGLE_BUTTON:
    {
      return "toggle button";
    }
    case Role::TOOL_BAR:
    {
      return "tool bar";
    }
    case Role::TOOL_TIP:
    {
      return "tool tip";
    }
    case Role::TREE:
    {
      return "tree";
    }
    case Role::TREE_TABLE:
    {
      return "tree table";
    }
    case Role::UNKNOWN:
    {
      return "unknown";
    }
    case Role::VIEWPORT:
    {
      return "viewport";
    }
    case Role::WINDOW:
    {
      return "window";
    }
    case Role::EXTENDED:
    {
      return "extended";
    }
    case Role::HEADER:
    {
      return "header";
    }
    case Role::FOOTER:
    {
      return "footer";
    }
    case Role::PARAGRAPH:
    {
      return "paragraph";
    }
    case Role::RULER:
    {
      return "ruler";
    }
    case Role::APPLICATION:
    {
      return "application";
    }
    case Role::AUTOCOMPLETE:
    {
      return "autocomplete";
    }
    case Role::EDITBAR:
    {
      return "edit bar";
    }
    case Role::EMBEDDED:
    {
      return "embedded";
    }
    case Role::ENTRY:
    {
      return "entry";
    }
    case Role::CHART:
    {
      return "chart";
    }
    case Role::CAPTION:
    {
      return "caution";
    }
    case Role::DOCUMENT_FRAME:
    {
      return "document frame";
    }
    case Role::HEADING:
    {
      return "heading";
    }
    case Role::PAGE:
    {
      return "page";
    }
    case Role::SECTION:
    {
      return "section";
    }
    case Role::REDUNDANT_OBJECT:
    {
      return "redundant object";
    }
    case Role::FORM:
    {
      return "form";
    }
    case Role::LINK:
    {
      return "link";
    }
    case Role::INPUT_METHOD_WINDOW:
    {
      return "input method window";
    }
    case Role::TABLE_ROW:
    {
      return "table row";
    }
    case Role::TREE_ITEM:
    {
      return "tree item";
    }
    case Role::DOCUMENT_SPREADSHEET:
    {
      return "document spreadsheet";
    }
    case Role::DOCUMENT_PRESENTATION:
    {
      return "document presentation";
    }
    case Role::DOCUMENT_TEXT:
    {
      return "document text";
    }
    case Role::DOCUMENT_WEB:
    {
      return "document web";
    }
    case Role::DOCUMENT_EMAIL:
    {
      return "document email";
    }
    case Role::COMMENT:
    {
      return "comment";
    }
    case Role::LIST_BOX:
    {
      return "list box";
    }
    case Role::GROUPING:
    {
      return "grouping";
    }
    case Role::IMAGE_MAP:
    {
      return "image map";
    }
    case Role::NOTIFICATION:
    {
      return "notification";
    }
    case Role::INFO_BAR:
    {
      return "info bar";
    }
    case Role::LEVEL_BAR:
    {
      return "level bar";
    }
    case Role::TITLE_BAR:
    {
      return "title bar";
    }
    case Role::BLOCK_QUOTE:
    {
      return "block quote";
    }
    case Role::AUDIO:
    {
      return "audio";
    }
    case Role::VIDEO:
    {
      return "video";
    }
    case Role::DEFINITION:
    {
      return "definition";
    }
    case Role::ARTICLE:
    {
      return "article";
    }
    case Role::LANDMARK:
    {
      return "landmark";
    }
    case Role::LOG:
    {
      return "log";
    }
    case Role::MARQUEE:
    {
      return "marquee";
    }
    case Role::MATH:
    {
      return "math";
    }
    case Role::RATING:
    {
      return "rating";
    }
    case Role::TIMER:
    {
      return "timer";
    }
    case Role::STATIC:
    {
      return "static";
    }
    case Role::MATH_FRACTION:
    {
      return "math fraction";
    }
    case Role::MATH_ROOT:
    {
      return "math root";
    }
    case Role::SUBSCRIPT:
    {
      return "subscript";
    }
    case Role::SUPERSCRIPT:
    {
      return "superscript";
    }
    case Role::MAX_COUNT:
    {
      break;
    }
  }
  return "";
}

Dali::Actor Accessible::GetCurrentlyHighlightedActor()
{
  return IsUp() ? Bridge::GetCurrentBridge()->data->currentlyHighlightedActor : Dali::Actor{};
}

void Accessible::SetCurrentlyHighlightedActor(Dali::Actor actor)
{
  if (IsUp())
  {
    Bridge::GetCurrentBridge()->data->currentlyHighlightedActor = actor;
  }
}

Dali::Actor Accessible::GetHighlightActor()
{
  return IsUp() ? Bridge::GetCurrentBridge()->data->highlightActor : Dali::Actor{};
}

void Accessible::SetHighlightActor(Dali::Actor actor)
{
  if (IsUp())
  {
    Bridge::GetCurrentBridge()->data->highlightActor = actor;
  }
}

void Bridge::ForceDown()
{
  auto highlighted = Accessible::GetCurrentlyHighlightedActor();
  if( highlighted )
  {
    auto p = dynamic_cast< Component* >( Accessible::Get( highlighted ) );
    if( p )
    {
      p->ClearHighlight();
    }
  }
  data = {};
}

void Bridge::SetIsOnRootLevel( Accessible* o )
{
  o->isOnRootLevel = true;
}

namespace
{
class NonControlAccessible : public virtual Accessible, public virtual Collection, public virtual Component
{
public:
  Dali::Actor actor;
  bool root = false;

  NonControlAccessible( Dali::Actor actor, bool root ) : actor( actor ), root( root )
  {
  }

  Dali::Rect<> GetExtents( Dali::Accessibility::CoordType ctype ) override
  {
    Vector2 screenPosition = actor.GetProperty( Actor::Property::SCREEN_POSITION ).Get< Vector2 >();
    Vector3 size = actor.GetCurrentProperty< Vector3 >( Actor::Property::SIZE ) * actor.GetCurrentProperty< Vector3 >( Actor::Property::WORLD_SCALE );
    bool positionUsesAnchorPoint = actor.GetProperty( Actor::Property::POSITION_USES_ANCHOR_POINT ).Get< bool >();
    Vector3 anchorPointOffSet = size * ( positionUsesAnchorPoint ? actor.GetCurrentProperty< Vector3 >( Actor::Property::ANCHOR_POINT ) : AnchorPoint::TOP_LEFT );
    Vector2 position = Vector2( screenPosition.x - anchorPointOffSet.x, screenPosition.y - anchorPointOffSet.y );

    return { position.x, position.y, size.x, size.y };
  }
  Dali::Accessibility::ComponentLayer GetLayer() override
  {
    return Dali::Accessibility::ComponentLayer::WINDOW;
  }
  int16_t GetMdiZOrder()
  {
    return 0;
  }
  double GetAlpha()
  {
    return 0;
  }
  bool GrabFocus() override
  {
    return false;
  }
  bool GrabHighlight() override
  {
    return false;
  }
  bool ClearHighlight() override
  {
    return false;
  }
  bool IsScrollable() override
  {
    return false;
  }
  std::string GetName() override
  {
    return actor.GetProperty< std::string >( Dali::Actor::Property::NAME );
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
    {
      throw std::domain_error{"invalid index " + std::to_string( index ) + " for object with " + std::to_string( s ) + " children"};
    }
    return Get( actor.GetChildAt( static_cast< unsigned int >( index ) ) );
  }
  size_t GetIndexInParent() override
  {
    auto parent = actor.GetParent();
    if( !parent )
    {
      return 0;
    }
    auto size = static_cast< size_t >( parent.GetChildCount() );
    for( auto i = 0u; i < size; ++i )
    {
      if( parent.GetChildAt( i ) == actor )
      {
        return i;
      }
    }
    throw std::domain_error{"actor is not a child of it's parent"};
  }
  Role GetRole() override
  {
    return root ? Role::WINDOW : Role::REDUNDANT_OBJECT;
  }
  States GetStates() override
  {
    States s;
    if( root )
    {
      s[State::ENABLED] = true;
      s[State::SENSITIVE] = true;
      s[State::SHOWING] = true;
      s[State::VISIBLE] = true;
      s[State::ACTIVE] = true;
    }
    else
    {
      auto t = GetParent()->GetStates();
      s[State::SHOWING] = t[State::SHOWING];
      s[State::VISIBLE] = t[State::VISIBLE];
    }
    return s;
  }
  Attributes GetAttributes() override
  {
    Dali::TypeInfo type;
    actor.GetTypeInfo( type );
    return { {"t", type.GetName()}, };
  }

  bool DoGesture(const GestureInfo &gestureInfo) override
  {
    return false;
  }

  std::vector<Relation> GetRelationSet() override
  {
    return {};
  }
};

using NonControlAccessiblesType = std::unordered_map< const Dali::RefObject*, std::unique_ptr< NonControlAccessible > >;

NonControlAccessiblesType nonControlAccessibles;

std::function< Accessible*( Dali::Actor ) > convertingFunctor = []( Dali::Actor ) -> Accessible*
{
  return nullptr;
};

ObjectRegistry objectRegistry;
} // namespace

void Accessible::SetObjectRegistry(ObjectRegistry registry)
{
  objectRegistry = registry;
}

void Accessible::RegisterControlAccessibilityGetter( std::function< Accessible*( Dali::Actor ) > functor )
{
  convertingFunctor = functor;
}

Accessible* Accessible::Get( Dali::Actor actor, bool root )
{
  if( !actor )
  {
    return nullptr;
  }
  auto accessible = convertingFunctor( actor );
  if( !accessible )
  {
    if( nonControlAccessibles.empty() && objectRegistry )
    {
      objectRegistry.ObjectDestroyedSignal().Connect( []( const Dali::RefObject* obj )
      {
        nonControlAccessibles.erase( obj );
      }
      );
    }
    auto it = nonControlAccessibles.emplace( &actor.GetBaseObject(), nullptr );
    if( it.second )
    {
      it.first->second.reset( new NonControlAccessible( actor, root ) );
    }
    accessible = it.first->second.get();
  }
  return accessible;
}
