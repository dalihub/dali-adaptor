#ifndef __DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H__
#define __DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H__

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
class FontClient;
} // TextAbstraction
} // Internal

namespace TextAbstraction
{


/**
 *   FontClient API
 *
 */
class DALI_IMPORT_API FontClient : public BaseHandle
{

public:

    /**
     * @brief Create an uninitialized TextAbstraction handle.
     *
     */
    FontClient();

    /**
     * @brief Destructor
     *
     * This is non-virtual since derived Handle types must not contain data or virtual methods.
     */
    ~FontClient();

    /**
     * @brief This constructor is used by FontClient::Get().
     *
     * @param[in] fontClient  A pointer to the internal fontClient object.
     */
    explicit DALI_INTERNAL FontClient( Dali::Internal::TextAbstraction::FontClient* fontClient);

    /**
     * @brief Retrieve a handle to the FontClient instance.
     *
     * @return A handle to the FontClient
     */
    static FontClient Get();

};

} // namespace TextAbstraction

} // namespace Dali

#endif // __DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H__
