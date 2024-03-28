#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H

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
#include <array>
#include <tuple>
#include <map>
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
private:
  struct Comparer;

  /**
   * @brief Visits all nodes of Accessible object and pushes the object to 'result' container.
   *
   * To query the entire tree, the BridgeCollection::Comparer is used inside this method,
   * which traverse the tree using GetChildAtIndex().
   * @param[in] obj The Accessible object to search
   * @param[out] result The vector container for result
   * @param[in] comparer BridgeCollection::Comparer which do the comparison against a single accessible object
   * @param[in] maxCount The maximum count of containing Accessible object
   */
  static void VisitNodes(Dali::Accessibility::Accessible* obj, std::vector<Dali::Accessibility::Accessible*>& result, Comparer& comparer, size_t maxCount);

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
  Dali::Accessibility::Collection* FindSelf() const;

public:
  /**
   * MatchRule type is a tuple that only carries data of de-serialized parameter from BridgeCollection::GetMatches dbus method.
   */
  using MatchRule = std::tuple<
    std::array<int32_t, 2>,
    int32_t,
    std::map<std::string, std::string>,
    int32_t,
    std::array<int32_t, 4>,
    int32_t,
    std::vector<std::string>,
    int32_t,
    bool>;

  /**
   * @brief Enumeration for Collection Index.
   */
  enum class Index
  {
    STATES,
    STATES_MATCH_TYPE,
    ATTRIBUTES,
    ATTRIBUTES_MATCH_TYPE,
    ROLES,
    ROLES_MATCH_TYPE,
    INTERFACES,
    INTERFACES_MATCH_TYPE,
  };

  /**
   * @brief Gets the matching Accessible objects with MatchRule.
   *
   * @param[in] rule BridgeCollection::MatchRule
   * @param[in] sortBy SortOrder::CANONICAL or SortOrder::REVERSE_CANONICAL
   * @param[in] count The maximum number of objects
   * @param[in] traverse True if it is traverse, otherwise false.
   * @return The matching Accessible objects
   */
  DBus::ValueOrError<std::vector<Dali::Accessibility::Accessible*> > GetMatches(MatchRule rule, uint32_t sortBy, int32_t count, bool traverse);

  /**
   * @brief Gets the matching Accessible objects with two MatchRules.
   *
   * @param[in] firstRule The initial BridgeCollection::MatchRule.
   * @param[in] secondRule An secondary BridgeCollection::MatchRule.
   * @param[in] sortBy SortOrder::CANONICAL or SortOrder::REVERSE_CANONICAL
   * @param[in] firstCount The maximum number of objects to return for the initial match.
   * @param[in] secondCount The maximum number of objects to return for the secondary match.
   * @param[in] traverse True if it is traverse, otherwise false.
   * @return The matching Accessible objects
   */
  DBus::ValueOrError<std::vector<Dali::Accessibility::Accessible*> > GetMatchesInMatches(MatchRule firstRule, MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount,  bool traverse);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H
