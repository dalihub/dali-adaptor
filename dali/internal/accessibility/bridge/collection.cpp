/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <iostream>
#include <set>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/collection.h>
#include <dali/internal/accessibility/bridge/accessibility-common.h>

using namespace Dali::Accessibility;

namespace
{
using MatchRule = Collection::MatchRule;

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
 * @brief Enumeration used for quering Accessibility objects.
 *
 * Refer to MatchType enumeration.
 */
enum class AtspiCollection
{
  MATCH_INVALID,
  MATCH_ALL,
  MATCH_ANY,
  MATCH_NONE,
  MATCH_EMPTY,
  MATCH_LAST_DEFINED,
};

/**
 * @brief The Comparer structure.
 *
 * Once the data is de-serialized by DBusWrapper, the data of match rule is passed
 * to Comparer type which do the comparison against a single accessible object.
 */
struct Comparer
{
  using Mode = MatchType;

  /**
   * @brief Enumeration to check the object is found first.
   */
  enum class CompareFuncExit
  {
    FIRST_FOUND,
    FIRST_NOT_FOUND
  };

  static Mode ConvertToMatchType(int32_t mode)
  {
    switch(mode)
    {
      case static_cast<int32_t>(AtspiCollection::MATCH_INVALID):
      {
        return Mode::INVALID;
      }
      case static_cast<int32_t>(AtspiCollection::MATCH_ALL):
      {
        return Mode::ALL;
      }
      case static_cast<int32_t>(AtspiCollection::MATCH_ANY):
      {
        return Mode::ANY;
      }
      case static_cast<int32_t>(AtspiCollection::MATCH_NONE):
      {
        return Mode::NONE;
      }
      case static_cast<int32_t>(AtspiCollection::MATCH_EMPTY):
      {
        return Mode::EMPTY;
      }
    }
    return Mode::INVALID;
  }

  /**
   * @brief The ComparerInterfaces structure
   */
  struct ComparerInterfaces
  {
    std::unordered_set<std::string> mObject;
    std::vector<std::string>        mRequested;
    Mode                            mMode = Mode::INVALID;

    ComparerInterfaces(MatchRule* rule)
    : mMode(ConvertToMatchType(std::get<static_cast<std::size_t>(Index::INTERFACES_MATCH_TYPE)>(*rule)))
    {
      mRequested = {std::get<static_cast<std::size_t>(Index::INTERFACES)>(*rule).begin(), std::get<static_cast<std::size_t>(Index::INTERFACES)>(*rule).end()};
    }

    void Update(Accessible* obj)
    {
      mObject.clear();
      for(auto& interface : obj->GetInterfacesAsStrings())
      {
        mObject.insert(std::move(interface));
      }
    }

    bool IsRequestEmpty() const
    {
      return mRequested.empty();
    }

    bool IsObjectEmpty() const
    {
      return mObject.empty();
    }

    bool Compare(CompareFuncExit exit)
    {
      bool foundAny = false;
      for(auto& iname : mRequested)
      {
        bool found = (mObject.find(iname) != mObject.end());
        if(found)
        {
          foundAny = true;
        }

        if(found == (exit == CompareFuncExit::FIRST_FOUND))
        {
          return found;
        }
      }
      return foundAny;
    }
  }; // ComparerInterfaces struct

  /**
   * @brief The ComparerAttributes structure
   */
  struct ComparerAttributes
  {
    std::unordered_map<std::string, std::string> mRequested;
    std::unordered_map<std::string, std::string> mObject;
    Mode                                         mMode = Mode::INVALID;

    ComparerAttributes(MatchRule* rule)
    : mMode(ConvertToMatchType(std::get<static_cast<std::size_t>(Index::ATTRIBUTES_MATCH_TYPE)>(*rule)))
    {
      mRequested = std::get<static_cast<std::size_t>(Index::ATTRIBUTES)>(*rule);
    }

    void Update(Accessible* obj)
    {
      mObject = obj->GetAttributes();
    }

    bool IsRequestEmpty() const
    {
      return mRequested.empty();
    }

    bool IsObjectEmpty() const
    {
      return mObject.empty();
    }

    bool Compare(CompareFuncExit exit)
    {
      bool foundAny = false;
      for(auto& iname : mRequested)
      {
        auto it    = mObject.find(iname.first);
        bool found = it != mObject.end() && iname.second == it->second;
        if(found)
        {
          foundAny = true;
        }

        if(found == (exit == CompareFuncExit::FIRST_FOUND))
        {
          return found;
        }
      }
      return foundAny;
    }
  }; // ComparerAttributes struct

  /**
   * @brief The ComparerRoles structure
   */
  struct ComparerRoles
  {
    using Roles = EnumBitSet<Role, Role::MAX_COUNT>;

    Roles mRequested;
    Roles mObject;
    Mode  mMode = Mode::INVALID;

    ComparerRoles(MatchRule* rule)
    : mMode(ConvertToMatchType(std::get<static_cast<std::size_t>(Index::ROLES_MATCH_TYPE)>(*rule)))
    {
      mRequested = Roles{std::get<static_cast<std::size_t>(Index::ROLES)>(*rule)};
    }

    void Update(Accessible* obj)
    {
      mObject                 = {};
      mObject[obj->GetRole()] = true;
      assert(mObject);
    }

    bool IsRequestEmpty() const
    {
      return !mRequested;
    }

    bool IsObjectEmpty() const
    {
      return !mObject;
    }

    bool Compare(CompareFuncExit exit)
    {
      switch(mMode)
      {
        case Mode::INVALID:
        {
          return true;
        }
        case Mode::EMPTY:
        case Mode::ALL:
        {
          return mRequested == (mObject & mRequested);
        }
        case Mode::ANY:
        {
          return bool(mObject & mRequested);
        }
        case Mode::NONE:
        {
          return bool(mObject & mRequested);
        }
      }
      return false;
    }
  }; // ComparerRoles struct

  /**
   * @brief The ComparerStates structure
   */
  struct ComparerStates
  {
    States mRequested;
    States mObject;
    Mode   mMode = Mode::INVALID;

    ComparerStates(MatchRule* rule)
    : mMode(ConvertToMatchType(std::get<static_cast<std::size_t>(Index::STATES_MATCH_TYPE)>(*rule)))
    {
      mRequested = States{std::get<static_cast<std::size_t>(Index::STATES)>(*rule)};
    }

    void Update(Accessible* obj)
    {
      mObject = obj->GetStates();
    }

    bool IsRequestEmpty() const
    {
      return !mRequested;
    }

    bool IsObjectEmpty() const
    {
      return !mObject;
    }

    bool Compare(CompareFuncExit exit)
    {
      switch(mMode)
      {
        case Mode::INVALID:
        {
          return true;
        }
        case Mode::EMPTY:
        case Mode::ALL:
        {
          return mRequested == (mObject & mRequested);
        }
        case Mode::ANY:
        {
          return bool(mObject & mRequested);
        }
        case Mode::NONE:
        {
          return bool(mObject & mRequested);
        }
      }
      return false;
    }
  }; // ComparerStates struct

  template<typename T>
  bool CompareFunc(T& cmp, Accessible* obj)
  {
    if(cmp.mMode == Mode::INVALID)
    {
      return true;
    }

    cmp.Update(obj);
    switch(cmp.mMode)
    {
      case Mode::ANY:
      {
        if(cmp.IsRequestEmpty() || cmp.IsObjectEmpty())
        {
          return false;
        }
        break;
      }
      case Mode::ALL:
      {
        if(cmp.IsRequestEmpty())
        {
          return true;
        }
        if(cmp.IsObjectEmpty())
        {
          return false;
        }
        break;
      }
      case Mode::NONE:
      {
        if(cmp.IsRequestEmpty() || cmp.IsObjectEmpty())
        {
          return true;
        }
        break;
      }
      case Mode::EMPTY:
      {
        if(cmp.IsRequestEmpty() && cmp.IsObjectEmpty())
        {
          return true;
        }
        if(cmp.IsRequestEmpty() || cmp.IsObjectEmpty())
        {
          return false;
        }
        break;
      }
      case Mode::INVALID:
      {
        return true;
      }
    }

    switch(cmp.mMode)
    {
      case Mode::EMPTY:
      case Mode::ALL:
      {
        if(!cmp.Compare(CompareFuncExit::FIRST_NOT_FOUND))
        {
          return false;
        }
        break;
      }
      case Mode::ANY:
      {
        if(cmp.Compare(CompareFuncExit::FIRST_FOUND))
        {
          return true;
        }
        break;
      }
      case Mode::NONE:
      {
        if(cmp.Compare(CompareFuncExit::FIRST_FOUND))
        {
          return false;
        }
        break;
      }
      case Mode::INVALID:
      {
        return true;
      }
    }

    switch(cmp.mMode)
    {
      case Mode::EMPTY:
      case Mode::ALL:
      case Mode::NONE:
      {
        return true;
      }
      case Mode::ANY:
      {
        return false;
      }
      case Mode::INVALID:
      {
        return true;
      }
    }
    return false;
  }

  Comparer(MatchRule* rule)
  : mInterface(rule),
    mAttribute(rule),
    mRole(rule),
    mState(rule)
  {
  }

  bool operator()(Accessible* obj)
  {
    return CompareFunc(mInterface, obj) &&
           CompareFunc(mAttribute, obj) &&
           CompareFunc(mRole, obj) &&
           CompareFunc(mState, obj);
  }

  bool IsShowing(Accessible* obj)
  {
    if(mState.mMode == Mode::NONE) return true;
    mState.Update(obj);
    if(mState.IsRequestEmpty() || mState.IsObjectEmpty()) return true;
    if(!mState.mRequested[State::SHOWING]) return true;
    if(mState.mObject[State::SHOWING]) return true;

    return false;
  }

  ComparerInterfaces mInterface;
  ComparerAttributes mAttribute;
  ComparerRoles      mRole;
  ComparerStates     mState;
}; // BridgeCollection::Comparer struct

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
void VisitNodes(Accessible* obj, std::vector<Accessible*>& result, Comparer& comparer, size_t maxCount, std::set<Accessible*>& visitedNodes)
{
  if(visitedNodes.count(obj) > 0)
  {
    return;
  }

  visitedNodes.insert(obj);

  if(maxCount > 0 && result.size() >= maxCount)
  {
    return;
  }

  if(comparer(obj))
  {
    result.emplace_back(obj);
    // the code below will never return for maxCount equal 0
    if(result.size() == maxCount)
    {
      return;
    }
  }

  if(!comparer.IsShowing(obj))
  {
    return;
  }

  for(auto i = 0u; i < obj->GetChildCount(); ++i)
  {
    VisitNodes(obj->GetChildAtIndex(i), result, comparer, maxCount, visitedNodes);
  }
}

void SortMatchedResult(std::vector<Accessible*>& result, SortOrder sortBy)
{
  switch(sortBy)
  {
    case SortOrder::CANONICAL:
    {
      break;
    }

    case SortOrder::REVERSE_CANONICAL:
    {
      std::reverse(result.begin(), result.end());
      break;
    }

    default:
    {
      throw std::domain_error{"unsupported sorting order"};
    }
      //TODO: other cases
  }
}

} // namespace

std::vector<Accessible*> Collection::GetMatches(MatchRule rule, uint32_t sortBy, size_t maxCount)
{
  std::vector<Accessible*> res;
  auto                     matcher = Comparer{&rule};
  std::set<Accessible*>    visitedNodes;
  VisitNodes(this, res, matcher, maxCount, visitedNodes);
  SortMatchedResult(res, static_cast<SortOrder>(sortBy));
  return res;
}

std::vector<Accessible*> Collection::GetMatchesInMatches(MatchRule firstRule, MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount)
{
  std::vector<Accessible*> res;
  std::vector<Accessible*> firstRes;
  auto                     firstMatcher = Comparer{&firstRule};
  std::set<Accessible*>    visitedNodes;
  VisitNodes(this, firstRes, firstMatcher, firstCount, visitedNodes);

  if(!firstRes.empty())
  {
    visitedNodes.clear();
    auto secondMatcher = Comparer{&secondRule};
    for(auto* obj : firstRes)
    {
      std::vector<Accessible*> secondRes;
      VisitNodes(obj, secondRes, secondMatcher, secondCount, visitedNodes);

      if(!secondRes.empty())
      {
        res.insert(res.end(), secondRes.begin(), secondRes.end());
      }
    }

    SortMatchedResult(res, static_cast<SortOrder>(sortBy));
  }

  return res;
}
