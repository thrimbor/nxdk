/*
 * Copyright (c) 2021 Stefan Schmidt
 *
 * Licensed under the MIT License
 */

#ifndef __NXDK_NET_H__
#define __NXDK_NET_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <stdint.h>

typedef struct
{
    size_t size; // ?
    int32_t flags;
} nxNetStartupParams;

int nxNetStartup (const nxNetStartupParams *startupParams);

#ifdef __cplusplus
}
#endif

#endif
