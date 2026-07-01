// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2019 Stefan Schmidt

#ifndef __PROFILEAPI_H__
#define __PROFILEAPI_H__

#include <winapi_include_lib.h>

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL QueryPerformanceCounter (LARGE_INTEGER *lpPerformanceCount);
BOOL QueryPerformanceFrequency (LARGE_INTEGER *lpFrequency);

#ifdef __cplusplus
}
#endif

#endif
