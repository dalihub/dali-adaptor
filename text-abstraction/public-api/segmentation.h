#ifndef __DALI_PLATFORM_TEXT_ABSTRACTION_SEGMENTATION_H__
#define __DALI_PLATFORM_TEXT_ABSTRACTION_SEGMENTATION_H__

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
class Segmentation;
} // TextAbstraction
} // Internal

namespace TextAbstraction
{


/**
 *   Segmentation API
 *
 */
class DALI_IMPORT_API Segmentation : public BaseHandle
{

public:

    /**
     * @brief Create an uninitialized TextAbstraction handle.
     *
     */
    Segmentation();

    /**
     * @brief Destructor
     *
     * This is non-virtual since derived Handle types must not contain data or virtual methods.
     */
    ~Segmentation();

    /**
     * @brief This constructor is used by Segmentation::Get().
     *
     * @param[in] segmentation  A pointer to the internal segmentation object.
     */
    explicit DALI_INTERNAL Segmentation( Dali::Internal::TextAbstraction::Segmentation* segmentation);

    /**
     * @brief Retrieve a handle to the Segmentation instance.
     *
     * @return A handle to the Segmentation
     */
    static Segmentation Get();

};

} // namespace TextAbstraction

} // namespace Dali

#endif // __DALI_PLATFORM_TEXT_ABSTRACTION_SEGMENTATION_H__
