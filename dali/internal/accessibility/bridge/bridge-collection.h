#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/accessibility/bridge/bridge-base.h>

class BridgeCollection : public virtual BridgeBase
{
private:
  struct Comparer;
  static void VisitNodes( Dali::Accessibility::Accessible* obj, std::vector< Dali::Accessibility::Accessible* >& result, Comparer& cmp, size_t maxCount );

protected:
  BridgeCollection() = default;

  void RegisterInterfaces();

  Dali::Accessibility::Collection* FindSelf() const;

public:
  using MatchRule = std::tuple<
      std::array< int32_t, 2 >, int32_t,
      std::unordered_map< std::string, std::string >, int32_t,
      std::array< int32_t, 4 >, int32_t,
      std::vector< std::string >, int32_t,
      bool >;
  struct Index
  {
    enum
    {
      States,
      StatesMatchType,
      Attributes,
      AttributesMatchType,
      Roles,
      RolesMatchType,
      Interfaces,
      InterfacesMatchType,
    };
  };

  DBus::ValueOrError< std::vector< Dali::Accessibility::Accessible* > > GetMatches( MatchRule rule, uint32_t sortBy, int32_t count, bool traverse );
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COLLECTION_H
