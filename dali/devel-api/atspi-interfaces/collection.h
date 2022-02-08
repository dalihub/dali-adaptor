#ifndef DALI_ADAPTOR_ATSPI_COLLECTION_H
#define DALI_ADAPTOR_ATSPI_COLLECTION_H

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
  /**
   * @brief Downcasts an Accessible to a Collection.
   *
   * @param obj The Accessible
   * @return A Collection or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Collection* DownCast(Accessible* obj);
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
