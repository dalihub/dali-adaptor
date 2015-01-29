#ifndef __DALI_PLATFORM_TEXT_ABSTRACTION_SHAPING_H__
#define __DALI_PLATFORM_TEXT_ABSTRACTION_SHAPING_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{

namespace TextAbstraction
{
class Shaping;
} // TextAbstraction
} // Internal

namespace TextAbstraction
{


/**
 *   Shaping API
 *
 */
class DALI_IMPORT_API Shaping : public BaseHandle
{

public:

    /**
     * @brief Create an uninitialized TextAbstraction handle.
     *
     */
    Shaping();

    /**
     * @brief Destructor
     *
     * This is non-virtual since derived Handle types must not contain data or virtual methods.
     */
    ~Shaping();

    /**
     * @brief This constructor is used by Shaping::Get().
     *
     * @param[in] shaping  A pointer to the internal shaping object.
     */
    explicit DALI_INTERNAL Shaping( Dali::Internal::TextAbstraction::Shaping* shaping);

    /**
     * @brief Retrieve a handle to the Shaping instance.
     *
     * @return A handle to the Shaping
     */
    static Shaping Get();

};

} // namespace TextAbstraction

} // namespace Dali

#endif // __DALI_PLATFORM_TEXT_ABSTRACTION_SHAPING_H__
