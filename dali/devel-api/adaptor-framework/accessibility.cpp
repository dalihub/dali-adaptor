/*
 * Copyright 2022  Samsung Electronics Co., Ltd
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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/object-registry.h>
#include <dali/public-api/object/type-info.h>
#include <dali/public-api/object/type-registry-helper.h>
#include <string_view>
#include <unordered_map>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/actor-accessible.h>
#include <dali/devel-api/adaptor-framework/proxy-accessible.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/action.h>
#include <dali/devel-api/atspi-interfaces/application.h>
#include <dali/devel-api/atspi-interfaces/collection.h>
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/devel-api/atspi-interfaces/editable-text.h>
#include <dali/devel-api/atspi-interfaces/hyperlink.h>
#include <dali/devel-api/atspi-interfaces/hypertext.h>
#include <dali/devel-api/atspi-interfaces/selection.h>
#include <dali/devel-api/atspi-interfaces/socket.h>
#include <dali/devel-api/atspi-interfaces/table.h>
#include <dali/devel-api/atspi-interfaces/table-cell.h>
#include <dali/devel-api/atspi-interfaces/text.h>
#include <dali/devel-api/atspi-interfaces/value.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/public-api/dali-adaptor-common.h>

using namespace Dali::Accessibility;
using namespace Dali;

const std::string& Dali::Accessibility::Address::GetBus() const
{
  return mBus.empty() && Bridge::GetCurrentBridge() ? Bridge::GetCurrentBridge()->GetBusName() : mBus;
}

std::string Accessible::GetLocalizedRoleName() const
{
  return GetRoleName();
}

std::string Accessible::GetRoleName() const
{
  static const std::unordered_map<Role, std::string_view> roleMap{
    {Role::INVALID, "invalid"},
    {Role::ACCELERATOR_LABEL, "accelerator label"},
    {Role::ALERT, "alert"},
    {Role::ANIMATION, "animation"},
    {Role::ARROW, "arrow"},
    {Role::CALENDAR, "calendar"},
    {Role::CANVAS, "canvas"},
    {Role::CHECK_BOX, "check box"},
    {Role::CHECK_MENU_ITEM, "check menu item"},
    {Role::COLOR_CHOOSER, "color chooser"},
    {Role::COLUMN_HEADER, "column header"},
    {Role::COMBO_BOX, "combo box"},
    {Role::DATE_EDITOR, "date editor"},
    {Role::DESKTOP_ICON, "desktop icon"},
    {Role::DESKTOP_FRAME, "desktop frame"},
    {Role::DIAL, "dial"},
    {Role::DIALOG, "dialog"},
    {Role::DIRECTORY_PANE, "directory pane"},
    {Role::DRAWING_AREA, "drawing area"},
    {Role::FILE_CHOOSER, "file chooser"},
    {Role::FILLER, "filler"},
    {Role::FOCUS_TRAVERSABLE, "focus traversable"},
    {Role::FONT_CHOOSER, "font chooser"},
    {Role::FRAME, "frame"},
    {Role::GLASS_PANE, "glass pane"},
    {Role::HTML_CONTAINER, "html container"},
    {Role::ICON, "icon"},
    {Role::IMAGE, "image"},
    {Role::INTERNAL_FRAME, "internal frame"},
    {Role::LABEL, "label"},
    {Role::LAYERED_PANE, "layered pane"},
    {Role::LIST, "list"},
    {Role::LIST_ITEM, "list item"},
    {Role::MENU, "menu"},
    {Role::MENU_BAR, "menu bar"},
    {Role::MENU_ITEM, "menu item"},
    {Role::OPTION_PANE, "option pane"},
    {Role::PAGE_TAB, "page tab"},
    {Role::PAGE_TAB_LIST, "page tab list"},
    {Role::PANEL, "panel"},
    {Role::PASSWORD_TEXT, "password text"},
    {Role::POPUP_MENU, "popup menu"},
    {Role::PROGRESS_BAR, "progress bar"},
    {Role::PUSH_BUTTON, "push button"},
    {Role::RADIO_BUTTON, "radio button"},
    {Role::RADIO_MENU_ITEM, "radio menu item"},
    {Role::ROOT_PANE, "root pane"},
    {Role::ROW_HEADER, "row header"},
    {Role::SCROLL_BAR, "scroll bar"},
    {Role::SCROLL_PANE, "scroll pane"},
    {Role::SEPARATOR, "separator"},
    {Role::SLIDER, "slider"},
    {Role::SPIN_BUTTON, "spin button"},
    {Role::SPLIT_PANE, "split pane"},
    {Role::STATUS_BAR, "status bar"},
    {Role::TABLE, "table"},
    {Role::TABLE_CELL, "table cell"},
    {Role::TABLE_COLUMN_HEADER, "table column header"},
    {Role::TABLE_ROW_HEADER, "table row header"},
    {Role::TEAROFF_MENU_ITEM, "tearoff menu item"},
    {Role::TERMINAL, "terminal"},
    {Role::TEXT, "text"},
    {Role::TOGGLE_BUTTON, "toggle button"},
    {Role::TOOL_BAR, "tool bar"},
    {Role::TOOL_TIP, "tool tip"},
    {Role::TREE, "tree"},
    {Role::TREE_TABLE, "tree table"},
    {Role::UNKNOWN, "unknown"},
    {Role::VIEWPORT, "viewport"},
    {Role::WINDOW, "window"},
    {Role::EXTENDED, "extended"},
    {Role::HEADER, "header"},
    {Role::FOOTER, "footer"},
    {Role::PARAGRAPH, "paragraph"},
    {Role::RULER, "ruler"},
    {Role::APPLICATION, "application"},
    {Role::AUTOCOMPLETE, "autocomplete"},
    {Role::EDITBAR, "edit bar"},
    {Role::EMBEDDED, "embedded"},
    {Role::ENTRY, "entry"},
    {Role::CHART, "chart"},
    {Role::CAPTION, "caution"},
    {Role::DOCUMENT_FRAME, "document frame"},
    {Role::HEADING, "heading"},
    {Role::PAGE, "page"},
    {Role::SECTION, "section"},
    {Role::REDUNDANT_OBJECT, "redundant object"},
    {Role::FORM, "form"},
    {Role::LINK, "link"},
    {Role::INPUT_METHOD_WINDOW, "input method window"},
    {Role::TABLE_ROW, "table row"},
    {Role::TREE_ITEM, "tree item"},
    {Role::DOCUMENT_SPREADSHEET, "document spreadsheet"},
    {Role::DOCUMENT_PRESENTATION, "document presentation"},
    {Role::DOCUMENT_TEXT, "document text"},
    {Role::DOCUMENT_WEB, "document web"},
    {Role::DOCUMENT_EMAIL, "document email"},
    {Role::COMMENT, "comment"},
    {Role::LIST_BOX, "list box"},
    {Role::GROUPING, "grouping"},
    {Role::IMAGE_MAP, "image map"},
    {Role::NOTIFICATION, "notification"},
    {Role::INFO_BAR, "info bar"},
    {Role::LEVEL_BAR, "level bar"},
    {Role::TITLE_BAR, "title bar"},
    {Role::BLOCK_QUOTE, "block quote"},
    {Role::AUDIO, "audio"},
    {Role::VIDEO, "video"},
    {Role::DEFINITION, "definition"},
    {Role::ARTICLE, "article"},
    {Role::LANDMARK, "landmark"},
    {Role::LOG, "log"},
    {Role::MARQUEE, "marquee"},
    {Role::MATH, "math"},
    {Role::RATING, "rating"},
    {Role::TIMER, "timer"},
    {Role::STATIC, "static"},
    {Role::MATH_FRACTION, "math fraction"},
    {Role::MATH_ROOT, "math root"},
    {Role::SUBSCRIPT, "subscript"},
    {Role::SUPERSCRIPT, "superscript"},
  };

  auto it = roleMap.find(GetRole());

  if(it == roleMap.end())
  {
    return {};
  }

  return std::string{it->second};
}

AtspiInterfaces Accessible::GetInterfaces() const
{
  if(!mInterfaces)
  {
    mInterfaces = DoGetInterfaces();
    DALI_ASSERT_DEBUG(mInterfaces); // There has to be at least AtspiInterface::ACCESSIBLE
  }

  return mInterfaces;
}

std::vector<std::string> Accessible::GetInterfacesAsStrings() const
{
  std::vector<std::string> ret;
  AtspiInterfaces          interfaces = GetInterfaces();

  for(std::size_t i = 0u; i < static_cast<std::size_t>(AtspiInterface::MAX_COUNT); ++i)
  {
    auto interface = static_cast<AtspiInterface>(i);

    if(interfaces[interface])
    {
      auto name = GetInterfaceName(interface);

      DALI_ASSERT_DEBUG(!name.empty());
      ret.emplace_back(std::move(name));
    }
  }

  return ret;
}

AtspiInterfaces Accessible::DoGetInterfaces() const
{
  AtspiInterfaces interfaces;

  interfaces[AtspiInterface::ACCESSIBLE]    = true;
  interfaces[AtspiInterface::ACTION]        = dynamic_cast<const Action*>(this);
  interfaces[AtspiInterface::APPLICATION]   = dynamic_cast<const Application*>(this);
  interfaces[AtspiInterface::COLLECTION]    = dynamic_cast<const Collection*>(this);
  interfaces[AtspiInterface::COMPONENT]     = dynamic_cast<const Component*>(this);
  interfaces[AtspiInterface::EDITABLE_TEXT] = dynamic_cast<const EditableText*>(this);
  interfaces[AtspiInterface::HYPERLINK]     = dynamic_cast<const Hyperlink*>(this);
  interfaces[AtspiInterface::HYPERTEXT]     = dynamic_cast<const Hypertext*>(this);
  interfaces[AtspiInterface::SELECTION]     = dynamic_cast<const Selection*>(this);
  interfaces[AtspiInterface::SOCKET]        = dynamic_cast<const Socket*>(this);
  interfaces[AtspiInterface::TABLE]         = dynamic_cast<const Table*>(this);
  interfaces[AtspiInterface::TABLE_CELL]    = dynamic_cast<const TableCell*>(this);
  interfaces[AtspiInterface::TEXT]          = dynamic_cast<const Text*>(this);
  interfaces[AtspiInterface::VALUE]         = dynamic_cast<const Value*>(this);

  return interfaces;
}

std::string Accessible::GetInterfaceName(AtspiInterface interface)
{
  static const std::unordered_map<AtspiInterface, std::string_view> interfaceMap{
    {AtspiInterface::ACCESSIBLE, "org.a11y.atspi.Accessible"},
    {AtspiInterface::ACTION, "org.a11y.atspi.Action"},
    {AtspiInterface::APPLICATION, "org.a11y.atspi.Application"},
    {AtspiInterface::CACHE, "org.a11y.atspi.Cache"},
    {AtspiInterface::COLLECTION, "org.a11y.atspi.Collection"},
    {AtspiInterface::COMPONENT, "org.a11y.atspi.Component"},
    {AtspiInterface::DEVICE_EVENT_CONTROLLER, "org.a11y.atspi.DeviceEventController"},
    {AtspiInterface::DEVICE_EVENT_LISTENER, "org.a11y.atspi.DeviceEventListener"},
    {AtspiInterface::DOCUMENT, "org.a11y.atspi.Document"},
    {AtspiInterface::EDITABLE_TEXT, "org.a11y.atspi.EditableText"},
    {AtspiInterface::EVENT_DOCUMENT, "org.a11y.atspi.Event.Document"},
    {AtspiInterface::EVENT_FOCUS, "org.a11y.atspi.Event.Focus"},
    {AtspiInterface::EVENT_KEYBOARD, "org.a11y.atspi.Event.Keyboard"},
    {AtspiInterface::EVENT_MOUSE, "org.a11y.atspi.Event.Mouse"},
    {AtspiInterface::EVENT_OBJECT, "org.a11y.atspi.Event.Object"},
    {AtspiInterface::EVENT_TERMINAL, "org.a11y.atspi.Event.Terminal"},
    {AtspiInterface::EVENT_WINDOW, "org.a11y.atspi.Event.Window"},
    {AtspiInterface::HYPERLINK, "org.a11y.atspi.Hyperlink"},
    {AtspiInterface::HYPERTEXT, "org.a11y.atspi.Hypertext"},
    {AtspiInterface::IMAGE, "org.a11y.atspi.Image"},
    {AtspiInterface::REGISTRY, "org.a11y.atspi.Registry"},
    {AtspiInterface::SELECTION, "org.a11y.atspi.Selection"},
    {AtspiInterface::SOCKET, "org.a11y.atspi.Socket"},
    {AtspiInterface::TABLE, "org.a11y.atspi.Table"},
    {AtspiInterface::TABLE_CELL, "org.a11y.atspi.TableCell"},
    {AtspiInterface::TEXT, "org.a11y.atspi.Text"},
    {AtspiInterface::VALUE, "org.a11y.atspi.Value"},
  };

  auto it = interfaceMap.find(interface);

  if(it == interfaceMap.end())
  {
    return {};
  }

  return std::string{it->second};
}

Dali::Actor Accessible::GetCurrentlyHighlightedActor()
{
  return IsUp() ? Bridge::GetCurrentBridge()->mData->mCurrentlyHighlightedActor : Dali::Actor{};
}

void Accessible::SetCurrentlyHighlightedActor(Dali::Actor actor)
{
  if(IsUp())
  {
    Bridge::GetCurrentBridge()->mData->mCurrentlyHighlightedActor = actor;
  }
}

Dali::Actor Accessible::GetHighlightActor()
{
  return IsUp() ? Bridge::GetCurrentBridge()->mData->mHighlightActor : Dali::Actor{};
}

void Accessible::SetHighlightActor(Dali::Actor actor)
{
  if(IsUp())
  {
    Bridge::GetCurrentBridge()->mData->mHighlightActor = actor;
  }
}

void Bridge::ForceDown()
{
  auto highlighted = Accessible::GetCurrentlyHighlightedActor();
  if(highlighted)
  {
    auto component = dynamic_cast<Component*>(Accessible::Get(highlighted));
    if(component)
    {
      component->ClearHighlight();
    }
  }
  mData = {};
}

void Bridge::SetIsOnRootLevel(Accessible* owner)
{
  owner->mIsOnRootLevel = true;
}

namespace
{
class AdaptorAccessible : public ActorAccessible
{
private:
  std::unique_ptr<TriggerEventInterface> mRenderNotification = nullptr;

protected:
  bool mRoot = false;

public:
  AdaptorAccessible(Dali::Actor actor, bool isRoot)
  : ActorAccessible(actor),
    mRoot(isRoot)
  {
  }

  bool GrabFocus() override
  {
    return false;
  }

  bool GrabHighlight() override
  {
    if(!IsUp())
    {
      return false;
    }

    // Only window accessible is able to grab and clear highlight
    if(!mRoot)
    {
      return false;
    }

    auto self                = Self();
    auto oldHighlightedActor = GetCurrentlyHighlightedActor();
    if(self == oldHighlightedActor)
    {
      return true;
    }

    // Clear the old highlight.
    if(oldHighlightedActor)
    {
      auto oldHighlightedObject = Dali::Accessibility::Component::DownCast(Accessible::Get(oldHighlightedActor));
      if(oldHighlightedObject)
      {
        oldHighlightedObject->ClearHighlight();
      }
    }

    SetCurrentlyHighlightedActor(self);

    auto                             window     = Dali::DevelWindow::Get(self);
    Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(window);
    windowImpl.EmitAccessibilityHighlightSignal(true);

    return true;
  }

  bool ClearHighlight() override
  {
    if(!IsUp())
    {
      return false;
    }

    // Only window accessible is able to grab and clear highlight
    if(!mRoot)
    {
      return false;
    }

    auto self = Self();
    if(self != GetCurrentlyHighlightedActor())
    {
      return false;
    }

    SetCurrentlyHighlightedActor({});

    auto                             window     = Dali::DevelWindow::Get(self);
    Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(window);
    windowImpl.EmitAccessibilityHighlightSignal(false);

    return true;
  }

  Role GetRole() const override
  {
    return mRoot ? Role::WINDOW : Role::REDUNDANT_OBJECT;
  }

  States GetStates() override
  {
    States state;
    if(mRoot)
    {
      auto window             = Dali::DevelWindow::Get(Self());
      auto visible            = window.IsVisible();
      state[State::ENABLED]   = true;
      state[State::SENSITIVE] = true;
      state[State::SHOWING]   = visible;
      state[State::VISIBLE]   = true;
      state[State::ACTIVE]    = visible;
    }
    else if (GetParent())
    {
      auto parentState      = GetParent()->GetStates();
      state[State::SHOWING] = parentState[State::SHOWING];
      state[State::VISIBLE] = parentState[State::VISIBLE];
    }
    else
    {
      state[State::SHOWING] = false;
      state[State::VISIBLE] = false;
    }
    return state;
  }

  Attributes GetAttributes() const override
  {
    Attributes attributes;

    if(mRoot)
    {
      Dali::Window                     window     = Dali::DevelWindow::Get(Self());
      Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(window);
      attributes["resID"]                         = windowImpl.GetNativeResourceId();
    }

    Dali::TypeInfo type;
    Self().GetTypeInfo(type);
    attributes["class"] = type.GetName();

    return attributes;
  }

  bool DoGesture(const GestureInfo& gestureInfo) override
  {
    return false;
  }

  std::vector<Relation> GetRelationSet() override
  {
    return {};
  }

  void SetListenPostRender(bool enabled) override
  {
    if (!mRoot)
    {
      return;
    }

    auto window                                 = Dali::DevelWindow::Get(Self());
    Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(window);

    if(!mRenderNotification)
    {
      mRenderNotification = std::unique_ptr<TriggerEventInterface>(
                                           TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &AdaptorAccessible::OnPostRender),
                                           TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
    }

    if (enabled)
    {
      windowImpl.SetRenderNotification(mRenderNotification.get());
    }
    else
    {
      windowImpl.SetRenderNotification(nullptr);
    }
  }

  void OnPostRender()
  {
    Accessibility::Bridge::GetCurrentBridge()->EmitPostRender(this);
  }
}; // AdaptorAccessible

using AdaptorAccessiblesType = std::unordered_map<const Dali::RefObject*, std::unique_ptr<AdaptorAccessible> >;

// Save RefObject from an Actor in Accessible::Get()
AdaptorAccessiblesType& GetAdaptorAccessibles()
{
  static AdaptorAccessiblesType gAdaptorAccessibles;
  return gAdaptorAccessibles;
}

std::function<Accessible*(Dali::Actor)> convertingFunctor = [](Dali::Actor) -> Accessible* {
  return nullptr;
};

ObjectRegistry objectRegistry;
} // namespace

void Accessible::SetObjectRegistry(ObjectRegistry registry)
{
  objectRegistry = registry;
  objectRegistry.ObjectDestroyedSignal().Connect([](const Dali::RefObject* obj) {
    GetAdaptorAccessibles().erase(obj);
  });
}

void Accessible::RegisterExternalAccessibleGetter(std::function<Accessible*(Dali::Actor)> functor)
{
  convertingFunctor = functor;
}

Accessible* Accessible::Get(Dali::Actor actor)
{
  if(!actor)
  {
    return nullptr;
  }

  auto accessible = convertingFunctor(actor);
  if(!accessible)
  {
    auto pair = GetAdaptorAccessibles().emplace(&actor.GetBaseObject(), nullptr);
    if(pair.second)
    {
      bool                     isRoot = false;
      Dali::Integration::Scene scene  = Dali::Integration::Scene::Get(actor);
      if(scene)
      {
        isRoot = (actor == scene.GetRootLayer());
      }
      pair.first->second.reset(new AdaptorAccessible(actor, isRoot));
    }
    accessible = pair.first->second.get();
  }

  return accessible;
}
