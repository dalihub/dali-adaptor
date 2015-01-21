#ifndef __DALI_PLATFORM_TEXT_ABSTRACTION_REORDERING_H__
#define __DALI_PLATFORM_TEXT_ABSTRACTION_REORDERING_H__

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
class Reordering;
} // TextAbstraction
} // Internal

namespace TextAbstraction
{


/**
 *   Reordering API
 *
 */
class DALI_IMPORT_API Reordering : public BaseHandle
{

public:

    /**
     * @brief Create an uninitialized TextAbstraction handle.
     *
     */
    Reordering();

    /**
     * @brief Destructor
     *
     * This is non-virtual since derived Handle types must not contain data or virtual methods.
     */
    ~Reordering();

    /**
     * @brief This constructor is used by Reordering::Get().
     *
     * @param[in] reordering a pointer to the internal reordering object.
     */
    explicit DALI_INTERNAL Reordering( Dali::Internal::TextAbstraction::Reordering* reordering);

    /**
     * @brief Retrieve a handle to the Reordering instance.
     *
     * @return A handle to the Reordering
     */
    static Reordering Get();


};

} // namespace TextAbstraction

} // namespace Dali

#endif // __DALI_PLATFORM_TEXT_ABSTRACTION_REORDERING_H__
