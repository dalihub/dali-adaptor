#ifndef DALI_APPLICATION_CONFIGURATION_H
#define DALI_APPLICATION_CONFIGURATION_H

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
 */

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Enumeration for Application configuration.
 * @SINCE_1_0.0
 */
namespace Configuration
{

/**
 * @brief Enumeration for application context loss policy.
 * @SINCE_1_0.0
 */
enum ContextLoss
{
  APPLICATION_HANDLES_CONTEXT_LOSS,  ///< Application will tear down and recreate UI on context loss and context regained signals. Dali doesn't need to retain data. @SINCE_1_0.0
  APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS, ///< Application expects Dali to retain data ( increased memory footprint ) @SINCE_1_0.0
};

} // Configuration

/**
 * @}
 */

} // namespace Dali

#endif // DALI_APPLICATION_CONFIGURATION_H
