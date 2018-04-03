#ifndef DALI_SENSOR_COMMON_TILT_SENSOR_FACTORY_H
#define DALI_SENSOR_COMMON_TILT_SENSOR_FACTORY_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/tilt-sensor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class TiltSensor;

namespace TiltSensorFactory
{

/**
 * Creates new instance of tilt sensor implementation
 * @return pointer to tilt sensor implementation instance
 */
Dali::Internal::Adaptor::TiltSensor* Create();

/**
 * Obtains existing or creates new instance of the tilt sensor
 * @return Tilt sensor handle
 */
Dali::TiltSensor Get();

} // TiltSensorFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_SENSOR_COMMON_TILT_SENSOR_FACTORY_H
