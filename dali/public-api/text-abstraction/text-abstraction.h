#ifndef DALI_TEXT_ABSTRACTION_H
#define DALI_TEXT_ABSTRACTION_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-string.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace TextAbstraction
{
/**
 * @brief Adds a custom font directory to the font search path.
 *
 * This registers @p path as an application font directory used by the
 * text abstraction font system. Fonts in the directory may be used for
 * subsequent font matching and fallback font selection.
 *
 * @SINCE_2_5.28
 *
 * @param[in] path The directory path that contains custom font files.
 * @return @e true if the directory was successfully registered, otherwise @e false.
 *
 * @note This function should be called before text is rendered or font fallback
 *       information is cached. It does not guarantee that already cached font
 *       matching or glyph results are refreshed.
 * @note @p path must be a directory path, not a single font file path.
 */
DALI_ADAPTOR_API bool AddCustomFontDirectory(const Dali::String& path);

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TEXT_ABSTRACTION_H
