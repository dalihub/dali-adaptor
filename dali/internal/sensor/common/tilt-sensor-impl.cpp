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

#include <dali/internal/sensor/common/tilt-sensor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

TiltSensor::TiltSensor() = default;

TiltSensor::~TiltSensor() = default;

bool TiltSensor::Start()
{
  return false;
}

void TiltSensor::Stop()
{

}

bool TiltSensor::IsStarted() const
{
  return false;
}

float TiltSensor::GetRoll() const
{
  return 0.0f;
}

float TiltSensor::GetPitch() const
{
  return 0.0f;
}

Quaternion TiltSensor::GetRotation() const
{
  return Quaternion::IDENTITY;
}


TiltSensor::TiltedSignalType& TiltSensor::TiltedSignal()
{
  static TiltSensor::TiltedSignalType signal;
  return signal;
}


void TiltSensor::SetUpdateFrequency(float frequencyHertz)
{

}

float TiltSensor::GetUpdateFrequency() const
{
  return 0.0f;
}

void TiltSensor::SetRotationThreshold(Radian rotationThreshold)
{

}

/**
 * @copydoc Dali::TiltSensor::GetRotationThreshold()
 */
Radian TiltSensor::GetRotationThreshold() const
{
  return Radian();
}

/**
 * Connects a callback function with the object's signals.
 * @param[in] object The object providing the signal.
 * @param[in] tracker Used to disconnect the signal.
 * @param[in] signalName The signal to connect to.
 * @param[in] functor A newly allocated FunctorDelegate.
 * @return True if the signal was connected.
 * @post If a signal was connected, ownership of functor was passed to CallbackBase. Otherwise the caller is responsible for deleting the unused functor.
 */
bool TiltSensor::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali