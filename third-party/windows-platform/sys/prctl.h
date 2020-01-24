#ifndef _PRCTL_INCLUDE_H_
#define _PRCTL_INCLUDE_H_

#define PR_SET_NAME 0

int prctl(int type, const char *str);

#endif