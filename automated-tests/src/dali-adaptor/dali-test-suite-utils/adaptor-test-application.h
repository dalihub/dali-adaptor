#ifndef DALI_ADAPTOR_TEST_APPLICATION_H
#define DALI_ADAPTOR_TEST_APPLICATION_H

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

// INTERNAL INCLUDES
#include "test-application.h"
#include "adaptor-test-adaptor-impl.h"

namespace Dali
{

/**
 * Adds some functionality on top of TestApplication that is required by the Adaptor.
 */
class AdaptorTestApplication : public TestApplication
{
public:

  AdaptorTestApplication( size_t surfaceWidth  = DEFAULT_SURFACE_WIDTH,
                          size_t surfaceHeight = DEFAULT_SURFACE_HEIGHT,
                          float  horizontalDpi = DEFAULT_HORIZONTAL_DPI,
                          float  verticalDpi   = DEFAULT_VERTICAL_DPI )
  : TestApplication( surfaceWidth, surfaceHeight, horizontalDpi, verticalDpi )
  {
    Internal::Adaptor::Adaptor::SetAvailable();
  }

  ~AdaptorTestApplication()
  {
  }
};

} // namespace Dali

#endif // DALI_ADAPTOR_TEST_APPLICATION_H
