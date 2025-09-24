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

/**
 * Allocates a block of write-combined physically contiguous memory.
 *
 * The allocated memory is always aligned to a page-boundary (4KiB).
 *
 * @param size The number of bytes to allocate
 * @param alignment Optional alignment, must be a power of two.
 * @return A pointer in virtual address space to where the allocated memory is
 *         mapped, or NULL if the allocation failed.
 */
void *pbAllocWC (size_t size, size_t alignment);

/**
 * Frees a block of write-combined contiguous physical memory (allocated with
 * pbAllocWC).
 *
 * @param address The pointer in virtual address space to where the allocated
 *                memory is mapped.
 */
void pbFreeWC (void *address);

#endif // PBKIT_MEMORY_H
