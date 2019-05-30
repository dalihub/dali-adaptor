#ifndef DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_FACTORY_H
#define DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_FACTORY_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/system/common/environment-options.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Performance interface factory class
 *
 */
class PerformanceInterfaceFactory
{
public:

  /**
   * Create a new concrete implementation of the performance interface.
   * @param adaptorServices adaptor internal services
   * @param environmentOptions environment options
   * @return pointer to a new performance interface
   */
  static PerformanceInterface* CreateInterface( AdaptorInternalServices& adaptorServices,
                                                const EnvironmentOptions& environmentOptions );

};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_FACTORY_H
