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

#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/adaptor-framework/atspi-accessibility.h>
#include <dali/integration-api/debug.h>

void Dali::AtspiAccessibility::Pause()
{
  DALI_LOG_ERROR("[ERROR] This is NOT supported\n");
}

void Dali::AtspiAccessibility::Resume()
{
  DALI_LOG_ERROR("[ERROR] This is NOT supported\n");
}

void Dali::AtspiAccessibility::Say( const std::string &text, bool discardable, std::function<void(std::string)> callback )
{
  DALI_LOG_ERROR("[ERROR] This is NOT supported\n");
}

int Dali::AtspiAccessibility::SetForcefully( bool turnOn )
{
  DALI_LOG_ERROR("[ERROR] This is NOT supported\n");
  return -1;
}

int Dali::AtspiAccessibility::GetStatus()
{
  DALI_LOG_ERROR("[ERROR] This is NOT supported\n");
  return -1;
}
