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
 */

#pragma once

#include <cstdio>
#include <string>

#include <dali/internal/system/common/logging.h>

// Use this macro only if need to log messages before the log function is set.
#define FONT_LOG_MESSAGE(level, format, ...)                                    \
  do                                                                            \
  {                                                                             \
    if(DALI_UNLIKELY(Dali::Integration::Log::IsLogDisabled()))                  \
    {                                                                           \
      break;                                                                    \
    }                                                                           \
    char buffer[256];                                                           \
    int  result = std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
    if(result >= static_cast<int>(sizeof(buffer)))                              \
    {                                                                           \
      std::string log("Font log message is too long to fit in the buffer.\n");  \
      Dali::TizenPlatform::LogMessage(Dali::Integration::Log::ERROR, log);      \
      break;                                                                    \
    }                                                                           \
    std::string log(buffer);                                                    \
    Dali::TizenPlatform::LogMessage(level, log);                                \
  } while(0)
