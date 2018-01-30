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

#include <dali/internal/sensor/common/tilt-sensor-factory.h>
#include <dali/internal/sensor/common/tilt-sensor-impl.h>
#include <dali/internal/system/common/singleton-service-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace TiltSensorFactory
{
__attribute__((weak)) Dali::Internal::Adaptor::TiltSensor* Create()
{
  // default implementation returns 'dummy' sensor
  return new Internal::Adaptor::TiltSensor();
}

Dali::TiltSensor Get()
{
  Dali::TiltSensor sensor;

  Dali::SingletonService service(SingletonService::Get());

  if (service)
  {
    // Check whether the keyboard focus manager is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TiltSensor));
    if (handle)
    {
      // If so, downcast the handle of singleton to keyboard focus manager
      sensor = Dali::TiltSensor(dynamic_cast< TiltSensor * >( handle.GetObjectPtr()));
    }
    else
    {
      // Create a singleton instance
      sensor = Dali::TiltSensor(TiltSensorFactory::Create());
      service.Register(typeid(sensor), sensor);
      handle = sensor;
    }
  }
  return sensor;
}
}

} // Adaptor

} // Internal

} // Dali