#include <assert.h>
#include <errno.h>
#include <string.h>
#include <processthreadsapi.h>
#include <process.h>
#include <fibersapi_internal_.h>
#include <winbase.h>
#include <pdclib/_PDCLIB_xbox_tls.h>
#include <xboxkrnl/xboxkrnl.h>

uintptr_t __cdecl _beginthreadex (void *_Security, unsigned _StackSize, _beginthreadex_proc_type _StartAddress, void *_ArgList, unsigned _InitFlag, unsigned *_ThrdAddr)
{
    HANDLE thread;
    thread = CreateThread(_Security, _StackSize, (LPTHREAD_START_ROUTINE)_StartAddress, _ArgList, _InitFlag, (LPDWORD)_ThrdAddr);
    if (thread == NULL) {
        // FIXME: Do proper translation from GetLastError() to errno
        errno = -EINVAL;
    }

    return (uintptr_t)thread;
}

void __cdecl _endthreadex (unsigned _ReturnCode)
{
    ExitThread(_ReturnCode);
}

static VOID NTAPI WinapiThreadStartup (PKSTART_ROUTINE StartRoutine, PVOID StartContext)
{
    // Make sure the TLS pointer is valid
    assert(KeGetCurrentThread()->TlsData);

    // Retrieve TLS area address and point first entry to itself
    DWORD *TlsData;
    TlsData = ((DWORD *)KeGetCurrentThread()->TlsData) + 1;
    TlsData[-1] = (DWORD)TlsData;

    // 16 byte alignment required
    assert(((DWORD)TlsData & 15) == 0);

    // Copy initial values of TLS area
    DWORD TlsDataSize;
    TlsDataSize = _tls_used.EndAddressOfRawData - _tls_used.StartAddressOfRawData;
    memcpy(TlsData, (void *)_tls_used.StartAddressOfRawData, TlsDataSize);

    // Zero-initialize the rest
    RtlZeroMemory((char *)TlsData + TlsDataSize, _tls_used.SizeOfZeroFill);

    // Register the thread for proper FLS destructor handling
    fls_register_thread();

    int res;
    res = (*(LPTHREAD_START_ROUTINE)StartRoutine)(StartContext);
    ExitThread(res);
}

HANDLE CreateThread (LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
    // lpThreadAttributes is unsupported on the Xbox and thus ignored
    NTSTATUS status;
    HANDLE handle;

    if (dwStackSize == 0) {
        // FIXME: We're directly reading the XBE StackCommit field here.
        //        A cleaner way with proper structs would be nice.
        dwStackSize = *((SIZE_T *)0x00010130);
    }

    ULONG tlssize;
    // Sum up the required amount of memory, round it up to a multiple of
    // 16 bytes and add 4 bytes for the self-reference
    tlssize = (_tls_used.EndAddressOfRawData - _tls_used.StartAddressOfRawData) + _tls_used.SizeOfZeroFill;
    tlssize = (tlssize + 15) & ~15;
    tlssize += 4;

    status = PsCreateSystemThreadEx(&handle, sizeof(fls_aux_thread_data_t), dwStackSize, tlssize, (PHANDLE)lpThreadId, (PKSTART_ROUTINE)lpStartAddress, lpParameter, dwCreationFlags & CREATE_SUSPENDED ? TRUE : FALSE, FALSE, WinapiThreadStartup);

    if (!NT_SUCCESS(status)) {
        SetLastError(RtlNtStatusToDosError(status));
        return NULL;
    }

    return handle;
}

VOID ExitThread (DWORD dwExitCode)
{
    fls_unregister_thread();
    PsTerminateSystemThread(dwExitCode);
}

BOOL GetExitCodeThread (HANDLE hThread, LPDWORD lpExitCode)
{
    PETHREAD thread;
    NTSTATUS status;

    status = ObReferenceObjectByHandle(hThread, NULL, (PVOID *)&thread);
    if (!NT_SUCCESS(status)) {
        SetLastError(RtlNtStatusToDosError(status));
        return FALSE;
    }

    if (thread->Tcb.HasTerminated) {
        *lpExitCode = thread->ExitStatus;
    } else {
        *lpExitCode = STILL_ACTIVE;
    }
    ObfDereferenceObject(thread);
    return TRUE;
}

HANDLE GetCurrentThread (VOID)
{
    return (HANDLE)-2;
}

DWORD GetCurrentThreadId (VOID)
{
    return (DWORD)((PETHREAD)KeGetCurrentThread())->UniqueThread;
}

DWORD GetThreadId (HANDLE Thread)
{
    PETHREAD threadObject;
    HANDLE id;
    NTSTATUS status;

    status = ObReferenceObjectByHandle(Thread, NULL, (PVOID)&threadObject);
    if (!NT_SUCCESS(status)) {
        SetLastError(RtlNtStatusToDosError(status));
        return 0;
    }

    id = threadObject->UniqueThread;
    ObfDereferenceObject(threadObject);

    return (DWORD)id;
}

BOOL SwitchToThread (VOID)
{
    if (NtYieldExecution() != STATUS_NO_YIELD_PERFORMED) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL SetThreadPriority (HANDLE hThread, int nPriority)
{
    NTSTATUS status;
    PETHREAD thread;

    status = ObReferenceObjectByHandle(hThread, &PsThreadObjectType, (PVOID)&thread);
    if (!NT_SUCCESS(status)) {
        SetLastError(RtlNtStatusToDosError(status));
        return FALSE;
    }

    LONG priority = (LONG)nPriority;
    if (priority == THREAD_PRIORITY_TIME_CRITICAL) {
        priority = (HIGH_PRIORITY + 1) / 2;
    } else if (priority == THREAD_PRIORITY_IDLE) {
        priority = -((HIGH_PRIORITY + 1) / 2);
    }

    KeSetBasePriorityThread(&thread->Tcb, priority);

    ObfDereferenceObject(thread);
    return TRUE;
}
