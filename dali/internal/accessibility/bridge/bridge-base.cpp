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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-base.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <atomic>
#include <cstdlib>
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/timer.h>

using namespace Dali::Accessibility;

static Dali::Timer tickTimer;

BridgeBase::BridgeBase()
{
}

BridgeBase::~BridgeBase()
{
  mApplication.mChildren.clear();
}

void BridgeBase::AddFilteredEvent(FilteredEvents kind, Dali::Accessibility::Accessible* obj, float delay, std::function<void()> functor)
{
  if(delay < 0)
  {
    delay = 0;
  }

  auto it = mFilteredEvents.insert({{kind, obj}, {static_cast<unsigned int>(delay * 10), {}}});
  if(it.second)
  {
    functor();
  }
  else
  {
    it.first->second.second = std::move(functor);
  }

  if(!tickTimer)
  {
    tickTimer = Dali::Timer::New(100);
    tickTimer.TickSignal().Connect(this, &BridgeBase::TickFilteredEvents);
  }

  if(!tickTimer.IsRunning())
  {
    tickTimer.Start();
  }
}

bool BridgeBase::TickFilteredEvents()
{
  for(auto it = mFilteredEvents.begin(); it != mFilteredEvents.end();)
  {
    if(it->second.first)
    {
      --it->second.first;
    }
    else
    {
      if(it->second.second)
      {
        it->second.second();
        it->second.second = {};
      }
      else
      {
        it = mFilteredEvents.erase(it);
        continue;
      }
    }
    ++it;
  }
  return !mFilteredEvents.empty();
}

void BridgeBase::UpdateRegisteredEvents()
{
  using ReturnType = std::vector<std::tuple<std::string, std::string>>;
  mRegistry.method<DBus::ValueOrError<ReturnType>()>("GetRegisteredEvents").asyncCall([this](DBus::ValueOrError<ReturnType> msg) {
    if(!msg)
    {
      LOG() << "Get registered events failed";
      return;
    }

    IsBoundsChangedEventAllowed = false;

    ReturnType values = std::get<ReturnType>(msg.getValues());
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

  mRegistry.addSignal<void(void)>("EventListenerRegistered", [this](void) {
    UpdateRegisteredEvents();
  });

  mRegistry.addSignal<void(void)>("EventListenerDeregistered", [this](void) {
    UpdateRegisteredEvents();
  });

  return ForceUpResult::JUST_STARTED;
}

void BridgeBase::ForceDown()
{
  Bridge::ForceDown();
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

  RegisterDefaultLabel(windowAccessible);
}

void BridgeBase::RemoveTopLevelWindow(Accessible* windowAccessible)
{
  UnregisterDefaultLabel(windowAccessible);

  for(auto i = 0u; i < mApplication.mChildren.size(); ++i)
  {
    if(mApplication.mChildren[i] == windowAccessible)
    {
      mApplication.mChildren.erase(mApplication.mChildren.begin() + i);
      break;
    }
  }
}

void BridgeBase::RegisterDefaultLabel(Accessible* object)
{
  if(std::find(mDefaultLabels.begin(), mDefaultLabels.end(), object) == mDefaultLabels.end())
  {
    mDefaultLabels.push_back(object);
  }
}

void BridgeBase::UnregisterDefaultLabel(Accessible* object)
{
  auto it = std::find(mDefaultLabels.begin(), mDefaultLabels.end(), object);
  if(it != mDefaultLabels.end())
  {
    mDefaultLabels.erase(it);
  }
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

  void*              accessible;
  std::istringstream tmp{path};
  if(!(tmp >> accessible))
  {
    throw std::domain_error{"invalid path '" + path + "'"};
  }

  auto it = mData->mKnownObjects.find(static_cast<Accessible*>(accessible));
  if(it == mData->mKnownObjects.end() || (*it)->IsHidden())
  {
    throw std::domain_error{"unknown object '" + path + "'"};
  }

  return static_cast<Accessible*>(accessible);
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
  auto root = &mApplication;

  std::vector<CacheElementType> res;

  std::function<void(Accessible*)> proc =
    [&](Accessible* item) {
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

  auto root   = &mApplication;
  auto parent = item->GetParent();

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
