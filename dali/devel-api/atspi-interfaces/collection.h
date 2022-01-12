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
 * @note since all mathods can be implemented inside bridge,
 * none methods have to be overrided
 */
class DALI_ADAPTOR_API Collection : public virtual Accessible
{
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_COLLECTION_H
