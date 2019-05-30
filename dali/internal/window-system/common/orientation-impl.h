#ifndef DALI_INTERNAL_ORIENTATION_H
#define DALI_INTERNAL_ORIENTATION_H

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
#include <cmath>
#include <dali/public-api/common/constants.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/orientation.h>
#include <dali/internal/window-system/common/rotation-event.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class Window;
class Orientation;

typedef IntrusivePtr<Orientation> OrientationPtr;

class Orientation : public BaseObject
{
public:

  typedef Dali::Orientation::OrientationSignalType OrientationSignalType;

  static Orientation* New(Window* window);

  /**
   * Constructor
   */
  Orientation(Window* window);

protected:
  /**
   * Destructor
   */
  virtual ~Orientation();

public:

  /**
   * Returns the actual orientation in degrees
   * @return The device's orientation
   */
  int GetDegrees() const;

  /**
   * Returns the actual orientation in radians
   * @return The device's orientation
   */
  float GetRadians() const;

  /**
   * Called by the Window when orientation is changed
   * @param[in] rotation The rotation event
   */
  void OnOrientationChange( const RotationEvent& rotation );

public: // Signals

  /**
   * @copydoc Dali::Orientation::ChangedSignal()
   */
  OrientationSignalType& ChangedSignal();

private:

  // Undefined
  Orientation(const Orientation&);
  Orientation& operator=(Orientation&);

private:

  Window*                                  mWindow;

  OrientationSignalType mChangedSignal;

  int                                      mOrientation;
  int                                      mWindowWidth;
  int                                      mWindowHeight;
};

inline Orientation& GetImplementation (Dali::Orientation& orientation)
{
  DALI_ASSERT_ALWAYS(orientation && "Orientation handle is empty");

  BaseObject& handle = orientation.GetBaseObject();

  return static_cast<Orientation&>(handle);
}

inline const Orientation& GetImplementation(const Dali::Orientation& orientation)
{
  DALI_ASSERT_ALWAYS(orientation && "Orientation handle is empty");

  const BaseObject& handle = orientation.GetBaseObject();

  return static_cast<const Orientation&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ORIENTATION_H
