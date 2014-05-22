#ifndef __DALI_SLP_PLATFORM_FONT_CONFIGURATION_PARSER_H__
#define __DALI_SLP_PLATFORM_FONT_CONFIGURATION_PARSER_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <string>

namespace Dali
{

namespace SlpPlatform
{

namespace FontConfigurationParser
{

/**
 * Parses the given font configuration file and sets fontFamily accordingly.
 * @param[in]   confFile    The configuration file to parse.
 * @param[out]  fontFamily  This gets set with the default font family specified in the conf file.
 * @param[out]  fontStyle   This gets set with the default font style specified in the conf file.
 */
void Parse(std::string confFile, std::string& fontFamily, std::string& fontStyle);

} // namespace FontConfParser

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_FONT_CONFIGURATION_PARSER_H__
