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

#include <dali-test-suite-utils.h>
#include <dali/internal/legacy/common/tizen-platform-abstraction.h>

using namespace Dali;
using namespace Dali::TizenPlatform;

int UtcDaliTizenPlatformAbstractionConstructorP(void)
{
  TizenPlatformAbstraction platformAbstraction;

  // Constructor should succeed
  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliTizenPlatformAbstractionCreateInstanceP(void)
{
  TizenPlatformAbstraction* platform = CreatePlatformAbstraction();

  DALI_TEST_CHECK(platform != nullptr);

  delete platform;

  END_TEST;
}

int UtcDaliTizenPlatformAbstractionCancelNonExistentTimerP(void)
{
  TizenPlatformAbstraction platformAbstraction;

  // Cancelling a non-existent timer should not throw
  try
  {
    platformAbstraction.CancelTimer(99999);
    DALI_TEST_CHECK(true);
  }
  catch(const std::exception& e)
  {
    DALI_TEST_CHECK(false);
  }

  END_TEST;
}
