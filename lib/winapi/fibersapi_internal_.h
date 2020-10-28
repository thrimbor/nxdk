#ifndef __FIBERSAPI_INTERNAL__H__
#define __FIBERSAPI_INTERNAL__H__

#include <windef.h>

// Each thread gets a node to keep track of its FLS data.
// Threads must register themselves, so their FLS nodes can be put into a list
// for proper handling of destructors.
typedef struct fls_node_t_
{
    LIST_ENTRY listEntry;
    PVOID slots[FLS_MAXIMUM_AVAILABLE];
} fls_node_t;

// Helper struct to store additional per-thread data by allocating additional
// space after the thread's ETHREAD struct to avoid having to resort to
// implicit TLS.
typedef struct fls_aux_thread_data_t_
{
    DWORD lastError;
    fls_node_t fls_node;
} fls_aux_thread_data_t;

fls_aux_thread_data_t *fls_get_aux_thread_data (VOID);

VOID fls_register_thread (VOID);
VOID fls_unregister_thread (VOID);

#endif
