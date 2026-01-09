#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H

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

// EXTERNAL INCLUDES
#include <array>
#include <tuple>
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/collection.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeCollection class is to correspond with Dali::Accessibility::Collection.
 *
 * Collection interface is designed to allow AT-Clients to query the tree of accessibility objects
 * exposed by an application with a single dbus call.
 * The query takes as an input a match rule and returns zero or more matching accessibility objects as a result.
 *
 * A match rule can be a combination of at least one of four criteria :
 *  Interface, Attribute, Role, State
 *
 * If more than one criteria is specified, the matching rule combines them using "AND" semantics.
 */
class BridgeCollection : public virtual BridgeBase
{
protected:
  BridgeCollection() = default;

  /**
   * @brief Registers Collection functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Collection object of the currently executed DBus method call.
   *
   * @return The Collection object
   */
  std::shared_ptr<Dali::Accessibility::Collection> FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Collection::GetMatches()
   */
  DBus::ValueOrError<std::vector<Dali::Accessibility::Accessible*> > GetMatches(Dali::Accessibility::Collection::MatchRule rule, uint32_t sortBy, int32_t count, bool traverse);

  /**
   * @copydoc Dali::Accessibility::Collection::GetMatchesInMatches()
   */
  DBus::ValueOrError<std::vector<Dali::Accessibility::Accessible*> > GetMatchesInMatches(Dali::Accessibility::Collection::MatchRule firstRule, Dali::Accessibility::Collection::MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount, bool traverse);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H
