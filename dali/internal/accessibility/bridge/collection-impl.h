#ifndef DALI_INTERNAL_ACCESSIBILITY_COLLECTION_IMPL_H
#define DALI_INTERNAL_ACCESSIBILITY_COLLECTION_IMPL_H

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

#include <dali/devel-api/atspi-interfaces/collection.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
class CollectionImpl : public Collection
{
public:
    /**
     * @brief Constructor.
     *
     * @param[in] accessible The accessible object associated with this collection
     */
    CollectionImpl(std::weak_ptr<Accessible> accessible);

    /**
     * @brief Gets matching accessible objects based on the provided match rule.
     *
     * @param[in] rule The match rule to apply
     * @param[in] sortBy Sort order (SortOrder::CANONICAL or SortOrder::REVERSE_CANONICAL)
     * @param[in] maxCount Maximum number of objects to return (0 for all matches)
     * @return Vector of matching accessible objects
     */
    std::vector<Accessible*> GetMatches(MatchRule rule, uint32_t sortBy, size_t maxCount) override;

    /**
     * @brief Gets matching accessible objects based on two match rules.
     *
     * @param[in] firstRule The initial match rule
     * @param[in] secondRule The secondary match rule
     * @param[in] sortBy Sort order (SortOrder::CANONICAL or SortOrder::REVERSE_CANONICAL)
     * @param[in] firstCount Maximum number of objects for initial match (0 for all)
     * @param[in] secondCount Maximum number of objects for secondary match (0 for all)
     * @return Vector of matching accessible objects
     */
    std::vector<Accessible*> GetMatchesInMatches(MatchRule firstRule, MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount) override;

private:
    std::weak_ptr<Accessible> mAccessible;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_COLLECTION_H
