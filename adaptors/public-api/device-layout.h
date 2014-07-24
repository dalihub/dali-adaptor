#ifndef __DALI_DEVICE_LAYOUT_H__
#define __DALI_DEVICE_LAYOUT_H__

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


/**
 * @addtogroup CAPI_DALI_ADAPTOR_MODULE
 * @{
 */

// EXTERNAL INCLUDES
#include <dali/public-api/math/vector2.h>

namespace Dali DALI_IMPORT_API
{

/**
 * @brief The attributes of the screen on the device.
 *
 * An application can specify the base layout that they used by inputting the values in this
 * structure and passing it to the Application or Adaptor class.
 * @see Dali::Application::Application(* argc, char **argv[], DeviceLayout baseLayout)
 * @see Dali::Adaptor::Adaptor(RenderSurface& surface, DeviceLayout baseLayout)
 */
struct DeviceLayout
{
public: // Construction & Destruction

  /**
   * @brief Default Constructor.
   */
  DeviceLayout();

  /**
   * @brief Create a DeviceLayout with specific parameters.
   * @param[in]  resolution       The resolution of the screen the application is based upon.
   * @param[in]  screenSize       The size of the screen the application is based upon.
   * @param[in]  dpi              The DPI of the screen the application is based upon.
   * @param[in]  viewingDistance  The default viewing distance of the screen the application is based upon.
   */
  DeviceLayout(Vector2 resolution, float screenSize, Vector2 dpi, float viewingDistance);

  /**
   * @brief Destructor.
   */
  ~DeviceLayout();

public: // Data

  Vector2 resolution;      ///< Resolution (width and height) of the screen.
  float   screenSize;      ///< Size of the screen in inches (diagonal size).
  Vector2 dpi;             ///< DPI (Dots per Inch) of the screen on the device (x & y).
  float   viewingDistance; ///< Average distance between the user and the device.

public: // Defaults Layouts

  /**
   * @brief This is the default base layout that Dali will assume if no layout is passed in from the
   * application.
   *
   * Resolution:        720.0f x 1280.0f
   * Screen Size:       4.65f
   * DPI:               316.0f x 316.0f
   * Viewing Distance:  30.0f
   */
  static const DeviceLayout DEFAULT_BASE_LAYOUT;
};

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_DEVICE_LAYOUT_H__
