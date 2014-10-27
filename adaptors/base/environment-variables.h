#ifndef __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_VARIABLES_H__
#define __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_VARIABLES_H__

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

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#define DALI_ENV_LOG_PERFORMANCE "DALI_LOG_PERFORMANCE"

// environment variable for enabling/disabling fps tracking
#define DALI_ENV_FPS_TRACKING "DALI_FPS_TRACKING"

// environment variable for enabling/disabling fps tracking
#define DALI_ENV_UPDATE_STATUS_INTERVAL "DALI_UPDATE_STATUS_INTERVAL"

#define DALI_ENV_LOG_PAN_GESTURE "DALI_LOG_PAN_GESTURE"

#define DALI_ENV_PAN_PREDICTION_MODE "DALI_PAN_PREDICTION_MODE"

#define DALI_ENV_PAN_PREDICTION_AMOUNT "DALI_PAN_PREDICTION_AMOUNT"

#define DALI_ENV_PAN_SMOOTHING_MODE "DALI_PAN_SMOOTHING_MODE"

#define DALI_ENV_PAN_SMOOTHING_AMOUNT "DALI_PAN_SMOOTHING_AMOUNT"

#define DALI_ENV_PAN_MINIMUM_DISTANCE "DALI_PAN_MINIMUM_DISTANCE"

#define DALI_ENV_PAN_MINIMUM_EVENTS "DALI_PAN_MINIMUM_EVENTS"

#define DALI_GLES_CALL_TIME "DALI_GLES_CALL_TIME"

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_VARIABLES_H__
