// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2020-2023 Stefan Schmidt
// SPDX-FileCopyrightText: 2021 Lucas Jansson

#include <excpt.h>
#include <excpt_cxx.hpp>
#include <eh.h>
#include <xboxkrnl/xboxkrnl.h>

struct ScopeTableEntry
{
    DWORD EnclosingLevel;
    void *FilterFunction;
    void *HandlerFunction;
};

struct EXCEPTION_REGISTRATION_SEH3 : EXCEPTION_REGISTRATION
{
    ScopeTableEntry *ScopeTable;
    DWORD TryLevel;
    DWORD _ebp;
};

#define DISPOSITION_DISMISS 0
#define DISPOSITION_CONTINUE_SEARCH 1
#define DISPOSITION_NESTED_EXCEPTION 2
#define DISPOSITION_COLLIDED_UNWIND 3
#define TRYLEVEL_NONE -1

extern "C" int _nested_unwind_handler (_EXCEPTION_RECORD *pExceptionRecord, EXCEPTION_REGISTRATION_SEH3 *pRegistrationFrame, _CONTEXT *pContextRecord, EXCEPTION_REGISTRATION **pDispatcherContext)
{
    if (!(pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND))) {
        return DISPOSITION_CONTINUE_SEARCH;
    }

    *pDispatcherContext = pRegistrationFrame;
    return DISPOSITION_COLLIDED_UNWIND;
}

static inline void *call_ebp_func (void *func, void *_ebp)
{
    void *result;

    asm volatile (
        "pushl %%ebp;"
        "movl %%ebx, %%ebp;"
        "call *%%eax;"
        "popl %%ebp;"
        : "=a"(result)
        : "a"(func), "b"(_ebp)
        : "ecx", "edx", "memory"
    );

    return result;
}

extern "C" void _local_unwind2 (EXCEPTION_REGISTRATION_SEH3 *pRegistrationFrame, int stop)
{
    // Manually install exception handler frame
    EXCEPTION_REGISTRATION nestedUnwindFrame;
    nestedUnwindFrame.handler = reinterpret_cast<void *>(_nested_unwind_handler);
    asm volatile (
        "movl %%fs:0, %%eax;"
        "movl %%eax, (%0);"
        "movl %0, %%fs:0;"
        : : "r"(&nestedUnwindFrame) : "eax", "memory");

    const ScopeTableEntry *scopeTable = pRegistrationFrame->ScopeTable;

    while (true) {
        LONG currentTrylevel = pRegistrationFrame->TryLevel;

        if (currentTrylevel == TRYLEVEL_NONE) {
            break;
        }

        if (stop != TRYLEVEL_NONE && currentTrylevel <= stop) {
            break;
        }

        LONG oldTrylevel = currentTrylevel;
        pRegistrationFrame->TryLevel = scopeTable[currentTrylevel].EnclosingLevel;

        if (!scopeTable[oldTrylevel].FilterFunction) {
            // If no filter funclet is present, then it's a __finally statement
            // instead of an __except statement
            call_ebp_func(scopeTable[oldTrylevel].HandlerFunction, &pRegistrationFrame->_ebp);
        }
    }

    // Manually remove exception handler frame
    asm volatile (
        "movl (%0), %%eax;"
        "movl %%eax, %%fs:0;"
        : : "r"(&nestedUnwindFrame) : "eax", "memory");
}

extern "C" void _global_unwind2 (EXCEPTION_REGISTRATION_SEH3 *pRegistrationFrame)
{
    // This looks like it could be a normal C function call, but it can't.
    // Despite looking like an stdcall function, RtlUnwind doesn't follow its
    // calling convention and happily destroys register values.
    // We work around this by using %eax for the parameter and adding
    // everything else to the clobber list.
    asm volatile (
        "pushl $0;"
        "pushl $0;"
        "pushl $0;"
        "pushl %0;"
        "call _RtlUnwind@16;"
        : : "a"(pRegistrationFrame) : "ebx", "ecx", "edx", "esi", "edi", "ebp");
}

extern "C" int _except_handler3 (_EXCEPTION_RECORD *pExceptionRecord, EXCEPTION_REGISTRATION_SEH3 *pRegistrationFrame, _CONTEXT *pContextRecord, EXCEPTION_REGISTRATION **pDispatcherContext)
{
    // Clear the direction flag - the function triggering the exception might
    // have modified it, but it's expected to not be set
    asm volatile ("cld;" ::: "memory");

    if (pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) {
        // We're in an unwinding pass, so unwind all local scopes
        _local_unwind2(pRegistrationFrame, TRYLEVEL_NONE);
        return DISPOSITION_CONTINUE_SEARCH;
    }

    // A pointer to a EXCEPTION_POINTERS structure needs to be put below below
    // the registration pointer. This is required because the intrinsics
    // implementing GetExceptionInformation() and GetExceptionCode() retrieve
    // their information from this structure.
    EXCEPTION_POINTERS excptPtrs;
    excptPtrs.ExceptionRecord = pExceptionRecord;
    excptPtrs.ContextRecord = pContextRecord;
    reinterpret_cast<volatile PEXCEPTION_POINTERS *>(pRegistrationFrame)[-1] = &excptPtrs;

    const ScopeTableEntry *scopeTable = pRegistrationFrame->ScopeTable;
    LONG currentTrylevel = pRegistrationFrame->TryLevel;

    // Search all scopes from the inside out trying to find a filter that accepts the exception
    while (currentTrylevel != TRYLEVEL_NONE) {
        const void *filterFunclet = scopeTable[currentTrylevel].FilterFunction;
        if (filterFunclet) {
            const DWORD _ebp = (DWORD)&pRegistrationFrame->_ebp;
            LONG filterResult;

            asm volatile (
                "pushl %%ebp;"
                "movl %2, %%ebp;"
                "call *%1;"
                "popl %%ebp;"
                : "=a"(filterResult) : "r"(filterFunclet), "r"(_ebp) : "ecx", "edx");

            if (filterResult != EXCEPTION_CONTINUE_SEARCH) {
                if (filterResult == EXCEPTION_CONTINUE_EXECUTION) {
                    return ExceptionContinueExecution;
                }

                // Trigger a second pass through the stack frames, unwinding all scopes
                _global_unwind2(pRegistrationFrame);

                const DWORD scopeEbp = (DWORD)&pRegistrationFrame->_ebp;
                const DWORD newTrylevel = scopeTable[currentTrylevel].EnclosingLevel;
                const DWORD handlerFunclet = (DWORD)scopeTable[currentTrylevel].HandlerFunction;

                _local_unwind2(pRegistrationFrame, currentTrylevel);
                pRegistrationFrame->TryLevel = newTrylevel;

                asm volatile (
                    "movl %0, %%ebp;"
                    "jmp *%%ebx;"
                    : : "r"(scopeEbp), "b"(handlerFunclet));
            }
        }

        currentTrylevel = pRegistrationFrame->ScopeTable[currentTrylevel].EnclosingLevel;
    }

    // No filter in this frame accepted the exception, continue searching
    return DISPOSITION_CONTINUE_SEARCH;
}


#define MAGIC_VC  0x19930520 // up to VC6
#define MAGIC_VC7 0x19930521 // VC7.x(2002-2003)
#define MAGIC_VC8 0x19930522 // VC8 (2005)

#define CXX_EXCEPTION 0xE06D7363

/*__declspec(noreturn)*/ extern "C" void __stdcall _CxxThrowException (void *pExceptionObject, _ThrowInfo *pThrowInfo)
{
    EXCEPTION_RECORD exceptionRecord;
    exceptionRecord.ExceptionCode = CXX_EXCEPTION;
    exceptionRecord.ExceptionRecord = nullptr;
    exceptionRecord.ExceptionAddress = (PVOID)_CxxThrowException;
    exceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    exceptionRecord.NumberParameters = 3;
    exceptionRecord.ExceptionInformation[0] = MAGIC_VC8;
    exceptionRecord.ExceptionInformation[1] = (ULONG_PTR)pExceptionObject;
    exceptionRecord.ExceptionInformation[2] = (ULONG_PTR)pThrowInfo;
    RtlRaiseException(&exceptionRecord);
}
