#ifndef DALI_ADAPTOR_ATSPI_COLLECTION_H
#define DALI_ADAPTOR_ATSPI_COLLECTION_H

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
 */

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface enabling advanced quering of accessibility objects.
 *
 * @note Since all methods can be implemented inside bridge,
 * no methods have to be overriden.
 */
class DALI_ADAPTOR_API Collection : public virtual Accessible
{
public:
  /**
   * MatchRule type is a tuple that only carries data of de-serialized parameter from BridgeCollection::GetMatches dbus method.
   */
  using MatchRule = std::tuple<
    std::array<int32_t, 2>,
    int32_t,
    std::unordered_map<std::string, std::string>,
    int32_t,
    std::array<int32_t, 4>,
    int32_t,
    std::vector<std::string>,
    int32_t,
    bool>;

  /**
   * @brief Downcasts an Accessible to a Collection.
   *
   * @param obj The Accessible
   * @return A Collection or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Collection* DownCast(Accessible* obj);

  /**
   * @brief Gets the matching Accessible objects with MatchRule.
   *
   * @param[in] rule Collection::MatchRule
   * @param[in] sortBy SortOrder::CANONICAL or SortOrder::REVERSE_CANONICAL
   * @param[in] maxCount The maximum number of objects; returns all matches if 0
   * @return The matching Accessible objects
   */
  std::vector<Accessible*> GetMatches(MatchRule rule, uint32_t sortBy, size_t maxCount);

  /**
   * @brief Gets the matching Accessible objects with two MatchRules.
   *
   * @param[in] firstRule The initial Collection::MatchRule.
   * @param[in] secondRule An secondary Collection::MatchRule.
   * @param[in] sortBy SortOrder::CANONICAL or SortOrder::REVERSE_CANONICAL
   * @param[in] firstCount The maximum number of objects to return for the initial match.; returns all matches if 0
   * @param[in] secondCount The maximum number of objects to return for the secondary match.; returns all matches if 0
   * @return The matching Accessible objects
   */
  std::vector<Accessible*> GetMatchesInMatches(MatchRule firstRule, MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::COLLECTION>
{
  using Type = Collection;
};
} // namespace Internal

inline Collection* Collection::DownCast(Accessible* obj)
{
  return Accessible::DownCast<AtspiInterface::COLLECTION>(obj);
}

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_COLLECTION_H
