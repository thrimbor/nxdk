// pbKit memory-related functions

// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2007 Guillaume Lamonoca
// SPDX-FileCopyrightText: 2025 Stefan Schmidt

#ifndef PBKIT_MEMORY_H
#define PBKIT_MEMORY_H

/**
 * Flushes the CPU's write combine buffer so that the GPU can see the changes
 * in memory.
 */
void pbFlushWCBuffer (void);

#endif // PBKIT_MEMORY_H
