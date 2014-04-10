#ifndef __TCT_DALI_ADAPTOR_CORE_H__
#define __TCT_DALI_ADAPTOR_CORE_H__

#include "testcase.h"

extern void utc_dali_adaptor_key_startup(void);
extern void utc_dali_adaptor_key_cleanup(void);
extern void utc_dali_timer_startup(void);
extern void utc_dali_timer_cleanup(void);

extern int UtcDaliKeyIsKey(void);
extern int UtcDaliKeyIsKeyNegative(void);
extern int UtcDaliTimerCreation(void);
extern int UtcDaliTimerUnitializedStart(void);
extern int UtcDaliTimerUnitializedStop(void);
extern int UtcDaliTimerUnitializedGetInterval(void);
extern int UtcDaliTimerUnitializedSetInterval(void);
extern int UtcDaliTimerUnitializedIsRunning(void);
extern int UtcDaliTimerUnitializedSignalTick(void);
extern int UtcDaliTimerSetInterval(void);
extern int UtcDaliTimerCopyConstructor(void);
extern int UtcDaliTimerAssignmentOperator(void);
extern int UtcDaliTimerIsRunning(void);
extern int UtcDaliTimerSignalTickContinue(void);
extern int UtcDaliTimerSignalTickStop(void);
extern int UtcDaliTimerReset(void);

testcase tc_array[] = {
    {"UtcDaliKeyIsKey", UtcDaliKeyIsKey, utc_dali_adaptor_key_startup, utc_dali_adaptor_key_cleanup},
    {"UtcDaliKeyIsKeyNegative", UtcDaliKeyIsKeyNegative, utc_dali_adaptor_key_startup, utc_dali_adaptor_key_cleanup},
    {"UtcDaliTimerCreation", UtcDaliTimerCreation, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerUnitializedStart", UtcDaliTimerUnitializedStart, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerUnitializedStop", UtcDaliTimerUnitializedStop, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerUnitializedGetInterval", UtcDaliTimerUnitializedGetInterval, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerUnitializedSetInterval", UtcDaliTimerUnitializedSetInterval, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerUnitializedIsRunning", UtcDaliTimerUnitializedIsRunning, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerUnitializedSignalTick", UtcDaliTimerUnitializedSignalTick, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerSetInterval", UtcDaliTimerSetInterval, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerCopyConstructor", UtcDaliTimerCopyConstructor, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerAssignmentOperator", UtcDaliTimerAssignmentOperator, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerIsRunning", UtcDaliTimerIsRunning, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerSignalTickContinue", UtcDaliTimerSignalTickContinue, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerSignalTickStop", UtcDaliTimerSignalTickStop, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {"UtcDaliTimerReset", UtcDaliTimerReset, utc_dali_timer_startup, utc_dali_timer_cleanup},
    {NULL, NULL}
};

#endif // __TCT_DALI_ADAPTOR_CORE_H__
