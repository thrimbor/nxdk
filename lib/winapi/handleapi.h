// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2019 Stefan Schmidt

#ifndef __HANDLEAPI_H__
#define __HANDLEAPI_H__

#include <winapi_include_lib.h>

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL CloseHandle (HANDLE hObject);

#ifdef __cplusplus
}
#endif

#endif
