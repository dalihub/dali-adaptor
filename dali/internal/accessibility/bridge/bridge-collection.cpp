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
#include <dali/internal/accessibility/bridge/bridge-collection.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <unordered_set>
#include <vector>

using namespace Dali::Accessibility;

namespace
{
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
} // anonymous namespace

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

/**
 * @brief The BridgeCollection::Comparer structure.
 *
 * Once the data is de-serialized by DBusWrapper, the data of match rule is passed
 * to Comparer type which do the comparison against a single accessible object.
 */
struct BridgeCollection::Comparer
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
    if (mState.mMode == Mode::NONE) return true;
    mState.Update(obj);
    if (mState.IsRequestEmpty() || mState.IsObjectEmpty()) return true;
    if (!mState.mRequested[State::SHOWING] ) return true;
    if (mState.mObject[State::SHOWING]) return true;

    return false;
  }

  ComparerInterfaces mInterface;
  ComparerAttributes mAttribute;
  ComparerRoles      mRole;
  ComparerStates     mState;
}; // BridgeCollection::Comparer struct


void BridgeCollection::VisitNodes(Accessible* obj, std::vector<Accessible*>& result, Comparer& comparer, size_t maxCount)
{
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

  if (!comparer.IsShowing(obj))
  {
    return;
  }

  for(auto i = 0u; i < obj->GetChildCount(); ++i)
  {
    VisitNodes(obj->GetChildAtIndex(i), result, comparer, maxCount);
  }
}

DBus::ValueOrError<std::vector<Accessible*> > BridgeCollection::GetMatches(MatchRule rule, uint32_t sortBy, int32_t count, bool traverse)
{
  std::vector<Accessible*> res;
  auto                     self    = BridgeBase::FindCurrentObject();
  auto                     matcher = Comparer{&rule};
  VisitNodes(self, res, matcher, count);

  switch(static_cast<SortOrder>(sortBy))
  {
    case SortOrder::CANONICAL:
    {
      break;
    }

    case SortOrder::REVERSE_CANONICAL:
    {
      std::reverse(res.begin(), res.end());
      break;
    }

    default:
    {
      throw std::domain_error{"unsupported sorting order"};
    }
      //TODO: other cases
  }

  return res;
}

DBus::ValueOrError<std::vector<Accessible*> > BridgeCollection::GetMatchesInMatches(MatchRule firstRule, MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount, bool traverse)
{
  std::vector<Accessible*> res;
  std::vector<Accessible*> firstRes;
  std::vector<Accessible*> secondRes;
  auto                     self          = BridgeBase::FindCurrentObject();
  auto                     firstMatcher  = Comparer{&firstRule};
  auto                     secondMatcher = Comparer{&secondRule};
  VisitNodes(self, firstRes, firstMatcher, firstCount);

  for (auto &obj : firstRes)
  {
    VisitNodes(obj, secondRes, secondMatcher, secondCount);

    res.insert(res.end(), secondRes.begin(), secondRes.end());
    secondRes.clear();
  }

  switch(static_cast<SortOrder>(sortBy))
  {
    case SortOrder::CANONICAL:
    {
      break;
    }

    case SortOrder::REVERSE_CANONICAL:
    {
      std::reverse(res.begin(), res.end());
      break;
    }

    default:
    {
      throw std::domain_error{"unsupported sorting order"};
    }
      //TODO: other cases
  }

  return res;
}
