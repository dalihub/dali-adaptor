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

#include <dali/integration-api/debug.h>
#include <dlog.h>

/**
 * Logs through Tizen dlog when the DALi log function may not be installed for
 * the current thread.
 */
#define DALI_TIZEN_DLOG(priority, format, ...)                 \
  do                                                           \
  {                                                            \
    if(DALI_UNLIKELY(Dali::Integration::Log::IsLogDisabled())) \
    {                                                          \
      break;                                                   \
    }                                                          \
    print_log(priority, "DALI", format, ##__VA_ARGS__);        \
  } while(0)
