//#ifndef __TEST_APPCORE_H__
//#define __TEST_APPCORE_H__

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
 * Dali-env provides a stub for appcore used by dali-core on desktop.
 * This file is used for over-riding appcore functions with test functions
 * which allow us to do things like simulate orientation change
 */


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    size_t event_data_size;
    void *event_data;
} sensor_event_data_t;


void *registered_data = NULL;

typedef enum {
  UNKNOWN_SENSOR  = 0x0000,
  ACCELEROMETER_SENSOR    = 0x0001,
  GEOMAGNETIC_SENSOR  = 0x0002,
  LIGHT_SENSOR  = 0x0004,
  PROXIMITY_SENSOR  = 0x0008,
  THERMOMETER_SENSOR  = 0x0010,
  GYROSCOPE_SENSOR  = 0x0020,
  PRESSURE_SENSOR = 0x0040,
  MOTION_SENSOR = 0x0080,
} sensor_type_t;

enum accelerometer_evet_type {
  ACCELEROMETER_EVENT_ROTATION_CHECK    = (ACCELEROMETER_SENSOR<<16) |0x0001,
  ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME = (ACCELEROMETER_SENSOR<<16) |0x0002,
  ACCELEROMETER_EVENT_CALIBRATION_NEEDED    = (ACCELEROMETER_SENSOR<<16) |0x0004,
};


enum accelerometer_rotate_state {
  ROTATION_UNKNOWN    = 0,
  ROTATION_LANDSCAPE_LEFT   = 1,
  ROTATION_PORTRAIT_TOP   = 2,
  ROTATION_PORTRAIT_BTM   = 3,
  ROTATION_LANDSCAPE_RIGHT  = 4,
  ROTATION_EVENT_0    = 2,  /*CCW base*/
  ROTATION_EVENT_90   = 1,  /*CCW base*/
  ROTATION_EVENT_180    = 3,  /*CCW base*/
  ROTATION_EVENT_270    = 4,  /*CCW base*/
};

 /**
 * Rotaion modes
 * @see appcore_set_rotation_cb(), appcore_get_rotation_state()
 */
enum appcore_rm {
  APPCORE_RM_UNKNOWN,
        /**< Unknown mode */
  APPCORE_RM_PORTRAIT_NORMAL,
           /**< Portrait mode */
  APPCORE_RM_PORTRAIT_REVERSE,
            /**< Portrait upside down mode */
  APPCORE_RM_LANDSCAPE_NORMAL,
            /**< Left handed landscape mode */
  APPCORE_RM_LANDSCAPE_REVERSE,
        /**< Right handed landscape mode */
  APPCORE_RM_INVALID = 0x99 // undefined
};

int (*rotate_function) (enum appcore_rm, void *) = NULL;
bool CallbackInstalled = false;

int appcore_set_rotation_cb(int (*cb) (enum appcore_rm, void *), void *data)
{
  rotate_function = cb;
  registered_data = data;

  tet_printf("installed rotation call back\n");
  CallbackInstalled = true;

  return 0;
}
int appcore_unset_rotation_cb(void)
{
  rotate_function = NULL;
  registered_data = NULL;

  tet_printf("removed rotation call back\n");

  CallbackInstalled = false;

  return 0;
}

// new function to simulate a change in orientation
void appcore_force_orientation(appcore_rm rm)
{
  if (CallbackInstalled)
  {
    rotate_function(rm,registered_data);
  }
}


#ifdef __cplusplus
}
#endif



//#endif // __TEST_APPCORE_H__
