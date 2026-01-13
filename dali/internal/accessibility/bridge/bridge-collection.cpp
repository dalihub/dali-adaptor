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
#include <dali/internal/accessibility/bridge/bridge-collection.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <unordered_set>
#include <vector>

#include <dali/devel-api/atspi-interfaces/accessible.h>

using namespace Dali::Accessibility;

void BridgeCollection::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::COLLECTION)};
  AddFunctionToInterface(desc, "GetMatches", &BridgeCollection::GetMatches);
  AddFunctionToInterface(desc, "GetMatchesInMatches", &BridgeCollection::GetMatchesInMatches);

  mDbusServer.addInterface("/", desc, true);
}

Collection* BridgeCollection::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::COLLECTION>();
}

DBus::ValueOrError<std::vector<Accessible*> > BridgeCollection::GetMatches(Collection::MatchRule rule, uint32_t sortBy, int32_t count, bool traverse)
{
  if(auto collection = FindSelf())
  {
    return collection->GetMatches(std::move(rule), sortBy, count);
  }

  return {};
}

DBus::ValueOrError<std::vector<Accessible*> > BridgeCollection::GetMatchesInMatches(Collection::MatchRule firstRule, Collection::MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount, bool traverse)
{
  if(auto collection = FindSelf())
  {
    return collection->GetMatchesInMatches(std::move(firstRule), std::move(secondRule), sortBy, firstCount, secondCount);
  }

  return {};
}
