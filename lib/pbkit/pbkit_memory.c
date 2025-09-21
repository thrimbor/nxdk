// pbKit memory-related functions

// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2007 Guillaume Lamonoca
// SPDX-FileCopyrightText: 2025 Stefan Schmidt

#include "pbkit_memory.h"
#include "outer.h"

void pbFlushWCBuffer (void)
{
    __asm__ __volatile__("sfence");
    VIDEOREG(NV_PFB_WC_CACHE) |= NV_PFB_WC_CACHE_FLUSH_TRIGGER;
    while (VIDEOREG(NV_PFB_WC_CACHE) & NV_PFB_WC_CACHE_FLUSH_IN_PROGRESS) {
    };
}
