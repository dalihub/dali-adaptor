#pragma once

#define PR_SET_NAME 0

#ifdef __cplusplus
extern "C" {
#endif

int prctl(int type, const char *str);

#ifdef __cplusplus
}
#endif
