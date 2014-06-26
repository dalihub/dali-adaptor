#ifndef __TCT_DALI_PLATFORM_ABSTRACTION_CORE_H__
#define __TCT_DALI_PLATFORM_ABSTRACTION_CORE_H__

#include "testcase.h"

extern void utc_dali_loading_startup(void);
extern void utc_dali_loading_cleanup(void);

extern int UtcDaliLoadCompletion(void);

testcase tc_array[] = {
    {"UtcDaliLoadCompletion", UtcDaliLoadCompletion, utc_dali_loading_startup, utc_dali_loading_cleanup},
    {NULL, NULL}
};

#endif // __TCT_DALI_PLATFORM_ABSTRACTION_CORE_H__
