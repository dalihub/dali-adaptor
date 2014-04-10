#ifndef __TCT_DALI_ADAPTOR_INTERNAL_CORE_H__
#define __TCT_DALI_ADAPTOR_INTERNAL_CORE_H__

#include "testcase.h"

extern void gif_loader_startup(void);
extern void gif_loader_cleanup(void);
extern void tilt_sensor_startup(void);
extern void tilt_sensor_cleanup(void);
extern void command_line_options_startup(void);
extern void command_line_options_cleanup(void);

extern int UtcDaliGifLoaderInterlaced(void);
extern int UtcDaliGifLoaderErrorBits(void);
extern int UtcDaliGifLoaderPattern(void);
extern int UtcDaliGifLoaderTransparency(void);
extern int UtcDaliTiltSensorEnable(void);
extern int UtcDaliTiltSensorDisable(void);
extern int UtcDaliTiltSensorIsEnabled(void);
extern int UtcDaliTiltSensorGetRoll(void);
extern int UtcDaliTiltSensorGetPitch(void);
extern int UtcDaliTiltSensorGetRotation(void);
extern int UtcDaliTiltSensorSignalTilted(void);
extern int UtcDaliTiltSensorSetUpdateFrequency(void);
extern int UtcDaliTiltSensorSetRotationThreshold01(void);
extern int UtcDaliCommandLineOptionsNoArgs(void);
extern int UtcDaliCommandLineOptionsDaliShortArgs(void);
extern int UtcDaliCommandLineOptionsDaliLongArgsEqualsSign(void);
extern int UtcDaliCommandLineOptionsDaliLongArgsSpaces(void);
extern int UtcDaliCommandLineOptionsNonDaliArgs(void);
extern int UtcDaliCommandLineOptionsMixture(void);
extern int UtcDaliCommandLineOptionsMixtureDaliOpsAtStart(void);
extern int UtcDaliCommandLineOptionsMixtureDaliOpsAtEnd(void);

testcase tc_array[] = {
    {"UtcDaliGifLoaderInterlaced", UtcDaliGifLoaderInterlaced, gif_loader_startup, gif_loader_cleanup},
    {"UtcDaliGifLoaderErrorBits", UtcDaliGifLoaderErrorBits, gif_loader_startup, gif_loader_cleanup},
    {"UtcDaliGifLoaderPattern", UtcDaliGifLoaderPattern, gif_loader_startup, gif_loader_cleanup},
    {"UtcDaliGifLoaderTransparency", UtcDaliGifLoaderTransparency, gif_loader_startup, gif_loader_cleanup},
    {"UtcDaliTiltSensorEnable", UtcDaliTiltSensorEnable, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorDisable", UtcDaliTiltSensorDisable, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorIsEnabled", UtcDaliTiltSensorIsEnabled, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorGetRoll", UtcDaliTiltSensorGetRoll, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorGetPitch", UtcDaliTiltSensorGetPitch, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorGetRotation", UtcDaliTiltSensorGetRotation, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorSignalTilted", UtcDaliTiltSensorSignalTilted, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorSetUpdateFrequency", UtcDaliTiltSensorSetUpdateFrequency, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliTiltSensorSetRotationThreshold01", UtcDaliTiltSensorSetRotationThreshold01, tilt_sensor_startup, tilt_sensor_cleanup},
    {"UtcDaliCommandLineOptionsNoArgs", UtcDaliCommandLineOptionsNoArgs, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsDaliShortArgs", UtcDaliCommandLineOptionsDaliShortArgs, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsDaliLongArgsEqualsSign", UtcDaliCommandLineOptionsDaliLongArgsEqualsSign, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsDaliLongArgsSpaces", UtcDaliCommandLineOptionsDaliLongArgsSpaces, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsNonDaliArgs", UtcDaliCommandLineOptionsNonDaliArgs, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsMixture", UtcDaliCommandLineOptionsMixture, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsMixtureDaliOpsAtStart", UtcDaliCommandLineOptionsMixtureDaliOpsAtStart, command_line_options_startup, command_line_options_cleanup},
    {"UtcDaliCommandLineOptionsMixtureDaliOpsAtEnd", UtcDaliCommandLineOptionsMixtureDaliOpsAtEnd, command_line_options_startup, command_line_options_cleanup},
    {NULL, NULL}
};

#endif // __TCT_DALI_ADAPTOR_INTERNAL_CORE_H__
