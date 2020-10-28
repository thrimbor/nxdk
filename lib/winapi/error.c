#include <winbase.h>
#include <threads.h>
#include <fibersapi_internal_.h>

DWORD GetLastError (void)
{
    return fls_get_aux_thread_data()->lastError;
}

void SetLastError (DWORD error)
{
    fls_get_aux_thread_data()->lastError = error;
}
