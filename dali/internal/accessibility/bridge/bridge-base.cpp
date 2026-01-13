/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-base.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <atomic>
#include <cstdlib>
#include <limits>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/public-api/adaptor-framework/timer.h>

using namespace Dali::Accessibility;

namespace
{
static Dali::Timer gTickTimer;
}

namespace Dali::Accessibility
{

// ApplicationAccessible implementation
std::string ApplicationAccessible::GetName() const
{
  return mName;
}

std::string ApplicationAccessible::GetDescription() const
{
  return {};
}

std::string ApplicationAccessible::GetValue() const
{
  return {};
}

Dali::Accessibility::Accessible* ApplicationAccessible::GetParent()
{
  return &mParent;
}

size_t ApplicationAccessible::GetChildCount() const
{
  return mChildren.size();
}

std::vector<Dali::Accessibility::Accessible*> ApplicationAccessible::GetChildren()
{
  return mChildren;
}

Dali::Accessibility::Accessible* ApplicationAccessible::GetChildAtIndex(size_t index)
{
  auto size = mChildren.size();
  if(index >= size)
  {
    throw std::domain_error{"invalid index " + std::to_string(index) + " for object with " + std::to_string(size) + " children"};
  }
  return mChildren[index];
}

size_t ApplicationAccessible::GetIndexInParent()
{
  if(mIsEmbedded)
  {
    return 0u;
  }
  throw std::domain_error{"can't call GetIndexInParent on application object"};
}

Dali::Accessibility::Role ApplicationAccessible::GetRole() const
{
  return Dali::Accessibility::Role::APPLICATION;
}

Dali::Accessibility::States ApplicationAccessible::GetStates()
{
  Dali::Accessibility::States result;

  for(auto* child : mChildren)
  {
    result = result | child->GetStates();
  }

  // The Application object should never have the SENSITIVE state
  result[Dali::Accessibility::State::SENSITIVE] = false;

  return result;
}

Dali::Accessibility::Attributes ApplicationAccessible::GetAttributes() const
{
  return {};
}

Dali::Accessibility::ActorAccessible* ApplicationAccessible::GetWindowAccessible(Dali::Window window)
{
  if(mChildren.empty())
  {
    return nullptr;
  }

  Dali::Layer rootLayer = window.GetRootLayer();

  // Find a child which is related to the window.
  for(auto i = 0u; i < mChildren.size(); ++i)
  {
    if(rootLayer == mChildren[i]->GetInternalActor())
    {
      return dynamic_cast<Dali::Accessibility::ActorAccessible*>(mChildren[i]);
    }
  }

  // If can't find its children, return the default window.
  return dynamic_cast<Dali::Accessibility::ActorAccessible*>(mChildren[0]);
}

bool ApplicationAccessible::DoGesture(const Dali::Accessibility::GestureInfo& gestureInfo)
{
  return false;
}

std::vector<Dali::Accessibility::Relation> ApplicationAccessible::GetRelationSet()
{
  return {};
}

Dali::Actor ApplicationAccessible::GetInternalActor() const
{
  return Dali::Actor{};
}

Dali::Accessibility::Address ApplicationAccessible::GetAddress() const
{
  return {"", "root"};
}

std::string ApplicationAccessible::GetStringProperty(std::string propertyName) const
{
  return {};
}

// Application interface implementation
std::string ApplicationAccessible::GetToolkitName() const
{
  return mToolkitName;
}

std::string ApplicationAccessible::GetVersion() const
{
  return std::to_string(Dali::ADAPTOR_MAJOR_VERSION) + "." + std::to_string(Dali::ADAPTOR_MINOR_VERSION);
}

bool ApplicationAccessible::GetIncludeHidden() const
{
  return mShouldIncludeHidden;
}

bool ApplicationAccessible::SetIncludeHidden(bool includeHidden)
{
  if(mShouldIncludeHidden != includeHidden)
  {
    mShouldIncludeHidden = includeHidden;
    return true;
  }
  return false;
}

// Socket interface implementation
Dali::Accessibility::Address ApplicationAccessible::Embed(Dali::Accessibility::Address plug)
{
  mIsEmbedded = true;
  mParent.SetAddress(plug);

  return GetAddress();
}

void ApplicationAccessible::Unembed(Dali::Accessibility::Address plug)
{
  if(mParent.GetAddress() == plug)
  {
    mIsEmbedded = false;
    mParent.SetAddress({});
    if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
    {
      bridge->SetExtentsOffset(0, 0);
    }
  }
}

void ApplicationAccessible::SetOffset(std::int32_t x, std::int32_t y)
{
  if(!mIsEmbedded)
  {
    return;
  }

  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->SetExtentsOffset(x, y);
  }
}

// Component interface implementation
Dali::Rect<float> ApplicationAccessible::GetExtents(Dali::Accessibility::CoordinateType type) const
{
  using limits = std::numeric_limits<float>;

  float minX = limits::max();
  float minY = limits::max();
  float maxX = limits::min();
  float maxY = limits::min();

  for(Dali::Accessibility::Accessible* child : mChildren)
  {
    auto extents = child->GetExtents(type);

    minX = std::min(minX, extents.x);
    minY = std::min(minY, extents.y);
    maxX = std::max(maxX, extents.x + extents.width);
    maxY = std::max(maxY, extents.y + extents.height);
  }

  return {minX, minY, maxX - minX, maxY - minY};
}

Dali::Accessibility::ComponentLayer ApplicationAccessible::GetLayer() const
{
  return Dali::Accessibility::ComponentLayer::WINDOW;
}

std::int16_t ApplicationAccessible::GetMdiZOrder() const
{
  return 0;
}

bool ApplicationAccessible::GrabFocus()
{
  return false;
}

double ApplicationAccessible::GetAlpha() const
{
  return 0.0;
}

bool ApplicationAccessible::GrabHighlight()
{
  return false;
}

bool ApplicationAccessible::ClearHighlight()
{
  return false;
}

bool ApplicationAccessible::IsScrollable() const
{
  return false;
}

Dali::Accessibility::AtspiInterfaces ApplicationAccessible::DoGetInterfaces() const
{
  AtspiInterfaces interfaces              = Accessible::DoGetInterfaces();
  interfaces[AtspiInterface::APPLICATION] = true;
  interfaces[AtspiInterface::COLLECTION]  = true;
  interfaces[AtspiInterface::SOCKET]      = true;
  return interfaces;
}
} //namespace Dali::Accessibility

// BridgeBase implementation
BridgeBase::BridgeBase()
{
}

BridgeBase::~BridgeBase()
{
}

void BridgeBase::AddCoalescableMessage(CoalescableMessages kind, Dali::Accessibility::Accessible* obj, float delay, std::function<void()> functor)
{
  if(delay < 0)
  {
    delay = 0;
  }

  auto countdownBase = static_cast<unsigned int>(delay * 10);
  auto it            = mCoalescableMessages.insert({{kind, obj}, {countdownBase, countdownBase, {}}});

  if(it.second)
  {
    functor();
  }
  else
  {
    std::get<1>(it.first->second) = countdownBase;
    std::get<2>(it.first->second) = std::move(functor);
  }

  if(!gTickTimer)
  {
    gTickTimer = Dali::Timer::New(100);
    gTickTimer.TickSignal().Connect(this, &BridgeBase::TickCoalescableMessages);
  }

  if(!gTickTimer.IsRunning())
  {
    gTickTimer.Start();
  }
}

bool BridgeBase::TickCoalescableMessages()
{
  for(auto it = mCoalescableMessages.begin(); it != mCoalescableMessages.end();)
  {
    auto& countdown     = std::get<0>(it->second);
    auto  countdownBase = std::get<1>(it->second);
    auto& functor       = std::get<2>(it->second);

    if(countdown)
    {
      --countdown;
    }
    else
    {
      if(functor)
      {
        functor();
        functor   = {};
        countdown = countdownBase;
      }
      else
      {
        it = mCoalescableMessages.erase(it);
        continue;
      }
    }

    ++it;
  }

  return !mCoalescableMessages.empty();
}

void BridgeBase::UpdateRegisteredEvents()
{
  using ReturnType = std::vector<std::tuple<std::string, std::string>>;

  mRegistry.method<DBus::ValueOrError<ReturnType>()>("GetRegisteredEvents").asyncCall([this](DBus::ValueOrError<ReturnType> msg)
  {
    if(!msg)
    {
      LOG() << "Get registered events failed";
      return;
    }

    IsBoundsChangedEventAllowed = false;
    ReturnType values           = std::get<ReturnType>(msg.getValues());

    for(long unsigned int i = 0; i < values.size(); i++)
    {
      if(!std::get<1>(values[i]).compare("Object:BoundsChanged"))
      {
        IsBoundsChangedEventAllowed = true;
      }
    }
  });
}

BridgeBase::ForceUpResult BridgeBase::ForceUp()
{
  //TODO: checking mBusName is enough? or a new variable to check bridge state?
  if(Bridge::ForceUp() == ForceUpResult::ALREADY_UP && !GetBusName().empty())
  {
    return ForceUpResult::ALREADY_UP;
  }

  auto proxy = DBus::DBusClient{dbusLocators::atspi::BUS, dbusLocators::atspi::OBJ_PATH, dbusLocators::atspi::BUS_INTERFACE, DBus::ConnectionType::SESSION};
  auto addr  = proxy.method<std::string()>(dbusLocators::atspi::GET_ADDRESS).call();

  if(!addr)
  {
    DALI_LOG_ERROR("failed at call '%s': %s\n", dbusLocators::atspi::GET_ADDRESS, addr.getError().message.c_str());
    return ForceUpResult::FAILED;
  }

  mConnectionPtr  = DBusWrapper::Installed()->eldbus_address_connection_get_impl(std::get<0>(addr));
  mData->mBusName = DBus::getConnectionName(mConnectionPtr);
  mDbusServer     = {mConnectionPtr};

  {
    DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::CACHE)};
    AddFunctionToInterface(desc, "GetItems", &BridgeBase::GetItems);
    mDbusServer.addInterface(AtspiDbusPathCache, desc);
  }

  {
    DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::APPLICATION)};
    AddGetSetPropertyToInterface(desc, "Id", &BridgeBase::GetId, &BridgeBase::SetId);
    mDbusServer.addInterface(AtspiPath, desc);
  }

  mRegistry = {AtspiDbusNameRegistry, AtspiDbusPathRegistry, Accessible::GetInterfaceName(AtspiInterface::REGISTRY), mConnectionPtr};
  UpdateRegisteredEvents();

  mRegistry.addSignal<void(void)>("EventListenerRegistered", [this](void)
  {
    UpdateRegisteredEvents();
  });

  mRegistry.addSignal<void(void)>("EventListenerDeregistered", [this](void)
  {
    UpdateRegisteredEvents();
  });

  return ForceUpResult::JUST_STARTED;
}

void BridgeBase::ForceDown()
{
  Bridge::ForceDown();
  gTickTimer.Reset();
  mCoalescableMessages.clear();
  DBusWrapper::Installed()->Strings.clear();
  mRegistry      = {};
  mDbusServer    = {};
  mConnectionPtr = {};
}

const std::string& BridgeBase::GetBusName() const
{
  static std::string empty;
  return mData ? mData->mBusName : empty;
}

Accessible* BridgeBase::FindByPath(const std::string& name) const
{
  try
  {
    return Find(name);
  }
  catch(std::domain_error&)
  {
    return nullptr;
  }
}

void BridgeBase::AddTopLevelWindow(Accessible* windowAccessible)
{
  if(windowAccessible->GetInternalActor() == nullptr)
  {
    return;
  }

  // Prevent adding the default window twice.
  if(!mApplication.mChildren.empty() &&
     mApplication.mChildren[0]->GetInternalActor() == windowAccessible->GetInternalActor())
  {
    return;
  }

  // Adds Window to a list of Windows.
  mApplication.mChildren.push_back(windowAccessible);
  SetIsOnRootLevel(windowAccessible);
}

void BridgeBase::RemoveTopLevelWindow(Accessible* windowAccessible)
{
  for(auto i = 0u; i < mApplication.mChildren.size(); ++i)
  {
    if(mApplication.mChildren[i] == windowAccessible)
    {
      mApplication.mChildren.erase(mApplication.mChildren.begin() + i);
      Emit(windowAccessible, WindowEvent::DESTROY);
      break;
    }
  }
}

void BridgeBase::CompressDefaultLabels()
{
  // Remove entries for objects which no longer exist
  mDefaultLabels.remove_if([](const DefaultLabelType& label)
  {
    // Check 1) window's weak handle; 2) actor's weak handle
    return !label.first.GetBaseHandle() || !label.second.GetBaseHandle();
  });
}

void BridgeBase::RegisterDefaultLabel(Dali::Actor actor)
{
  CompressDefaultLabels();
  Dali::WeakHandle<Dali::Window> window = GetWindow(actor);

  if(!window.GetBaseHandle())
  {
    DALI_LOG_ERROR("Cannot register default label: actor does not belong to any window");
    return;
  }

  auto it = std::find_if(mDefaultLabels.begin(), mDefaultLabels.end(), [&actor](const DefaultLabelType& label)
  {
    auto actorHandle = label.second.GetBaseHandle();
    return actorHandle && actorHandle == actor;
  });

  Dali::WeakHandle<Dali::Actor> weakActor{actor};

  if(it == mDefaultLabels.end())
  {
    mDefaultLabels.push_back({window, weakActor});
  }
  else if(it->first != window)
  {
    // TODO: Tentative implementation. It is yet to be specified what should happen
    // when the same object is re-registered as a default label for another window.
    *it = {window, weakActor};
  }
  else // it->first == window && it->second == object
  {
    // Nothing to do
  }
}

void BridgeBase::UnregisterDefaultLabel(Dali::Actor actor)
{
  CompressDefaultLabels();
  mDefaultLabels.remove_if([&actor](const DefaultLabelType& label)
  {
    auto actorHandle = label.second.GetBaseHandle();
    return actorHandle && actorHandle == actor;
  });
}

Accessible* BridgeBase::GetDefaultLabel(Accessible* root)
{
  if(!root)
  {
    DALI_LOG_ERROR("Cannot get defaultLabel as given root accessible is null.");
    return nullptr;
  }

  CompressDefaultLabels();
  Dali::WeakHandle<Dali::Window> window = GetWindow(root->GetInternalActor());

  if(!window.GetBaseHandle())
  {
    return root;
  }

  auto it = std::find_if(mDefaultLabels.rbegin(), mDefaultLabels.rend(), [&window](const DefaultLabelType& label)
  {
    return window == label.first;
  });

  Accessible* accessible = root;

  if(it != mDefaultLabels.rend())
  {
    if(auto actorHandle = it->second.GetBaseHandle())
    {
      auto actor = Dali::Actor::DownCast(actorHandle);
      if(actor)
      {
        accessible = Accessible::Get(actor);
      }
    }
  }

  return accessible;
}

std::string BridgeBase::StripPrefix(const std::string& path)
{
  auto size = strlen(AtspiPath);
  return path.substr(size + 1);
}

Accessible* BridgeBase::Find(const std::string& path) const
{
  if(path == "root")
  {
    return &mApplication;
  }

  auto accessible = GetAccessible(path);
  if(!accessible || (!mApplication.mShouldIncludeHidden && accessible->IsHidden()))
  {
    throw std::domain_error{"unknown object '" + path + "'"};
  }

  return accessible.get();
}

Accessible* BridgeBase::Find(const Address& ptr) const
{
  assert(ptr.GetBus() == mData->mBusName);
  return Find(ptr.GetPath());
}

Accessible* BridgeBase::FindCurrentObject() const
{
  auto path = DBus::DBusServer::getCurrentObjectPath();
  auto size = strlen(AtspiPath);

  if(path.size() <= size)
  {
    throw std::domain_error{"invalid path '" + path + "'"};
  }

  if(path.substr(0, size) != AtspiPath)
  {
    throw std::domain_error{"invalid path '" + path + "'"};
  }

  if(path[size] != '/')
  {
    throw std::domain_error{"invalid path '" + path + "'"};
  }

  return Find(StripPrefix(path));
}

void BridgeBase::SetId(int id)
{
  this->mId = id;
}

int BridgeBase::GetId()
{
  return this->mId;
}

auto BridgeBase::GetItems() -> DBus::ValueOrError<std::vector<CacheElementType>>
{
  auto                          root = &mApplication;
  std::vector<CacheElementType> res;

  std::function<void(Accessible*)> proc =
    [&](Accessible* item)
  {
    res.emplace_back(std::move(CreateCacheElement(root)));

    for(auto i = 0u; i < item->GetChildCount(); ++i)
    {
      proc(item->GetChildAtIndex(i));
    }
  };

  return res;
}

auto BridgeBase::CreateCacheElement(Accessible* item) -> CacheElementType
{
  if(!item)
  {
    return {};
  }

  auto                 root   = &mApplication;
  auto                 parent = item->GetParent();
  std::vector<Address> children;

  for(auto i = 0u; i < item->GetChildCount(); ++i)
  {
    children.emplace_back(item->GetChildAtIndex(i)->GetAddress());
  }

  return std::make_tuple(
    item->GetAddress(),
    root->GetAddress(),
    parent ? parent->GetAddress() : Address{},
    children,
    item->GetInterfacesAsStrings(),
    item->GetName(),
    item->GetRole(),
    item->GetDescription(),
    item->GetStates().GetRawData());
}

Dali::WeakHandle<Dali::Window> BridgeBase::GetWindow(Dali::Actor actor)
{
  Dali::WeakHandle<Dali::Window> windowHandle;

  if(actor)
  {
    Dali::Window window = Dali::DevelWindow::Get(actor);
    windowHandle        = {window};
  }

  return windowHandle;
}
