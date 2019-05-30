#ifndef DALI_ADAPTOR_VERSION_H
#define DALI_ADAPTOR_VERSION_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
DALI_ADAPTOR_API extern const unsigned int ADAPTOR_MAJOR_VERSION; ///< The major version number of the Adaptor.
DALI_ADAPTOR_API extern const unsigned int ADAPTOR_MINOR_VERSION; ///< The minor version number of the Adaptor.
DALI_ADAPTOR_API extern const unsigned int ADAPTOR_MICRO_VERSION; ///< The micro version number of the Adaptor.
DALI_ADAPTOR_API extern const char * const ADAPTOR_BUILD_DATE;    ///< The date/time the Adaptor library was built.
} // namespace Dali

#endif // DALI_ADAPTOR_VERSION_H
