// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2019 Stefan Schmidt

#ifndef __DEBUGAPI_H__
#define __DEBUGAPI_H__

#include <winapi_include_lib.h>

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsDebuggerPresent (VOID);

#ifdef __cplusplus
}
#endif

#endif
