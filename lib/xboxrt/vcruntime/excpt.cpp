#include <excpt.h>
#include <excpt_cxx.hpp>
#include <eh.h>
#include <xboxkrnl/xboxkrnl.h>
#include <hal/debug.h>
#include <assert.h>
#include <string.h>

//#define DbgPrint(...) DbgPrint(__VA_ARGS__)
#define DbgPrint(...) ((void)0)

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
        DWORD currentTrylevel = pRegistrationFrame->TryLevel;

        if (currentTrylevel == TRYLEVEL_NONE) {
            break;
        }

        if (stop != TRYLEVEL_NONE && currentTrylevel <= stop) {
            break;
        }

        DWORD oldTrylevel = currentTrylevel;
        pRegistrationFrame->TryLevel = scopeTable[currentTrylevel].EnclosingLevel;

        if (!scopeTable[oldTrylevel].FilterFunction) {
            // If no filter funclet is present, then it's a __finally statement
            // instead of an __except statement
            auto finallyFunclet = reinterpret_cast<void (*)()>(scopeTable[oldTrylevel].HandlerFunction);
            finallyFunclet();
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
    asm volatile (
        "pushl $0;"
        "pushl $0;"
        "pushl 1f;"
        "pushl %0;"
        "call _RtlUnwind@16;"
        "1:;"
        : : "r"(pRegistrationFrame));
}

extern "C" int _except_handler3 (_EXCEPTION_RECORD *pExceptionRecord, EXCEPTION_REGISTRATION_SEH3 *pRegistrationFrame, _CONTEXT *pContextRecord, EXCEPTION_REGISTRATION **pDispatcherContext)
{
    // Clear the direction flag - the function triggering the exception might
    // have modified it, but it's expected to not be set
    asm volatile ("cld;");

    if (pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) {
        // We're in an unwinding pass, so unwind all local scopes
        // Unwinding must be done with the ebp of the frame in which we want to unwind
        const DWORD scopeEbp = (DWORD)&pRegistrationFrame->_ebp;
        asm volatile (
            "pushl %%ebp;"
            "movl %0, %%ebp;"
            "pushl %2;"
            "pushl %1;"
            "call __local_unwind2;" // Call _local_unwind2(pRegistrationFrame, TRYLEVEL_NONE);
            "addl $8, %%esp;"
            "popl %%ebp;"
            : : "r"(scopeEbp), "r"(pRegistrationFrame), "i"(TRYLEVEL_NONE) : "eax", "ecx", "edx");

        return DISPOSITION_CONTINUE_SEARCH;
    }

    // A pointer to a EXCEPTION_POINTERS structure needs to be put below below
    // the registration pointer. This is required because the intrinsics
    // implementing GetExceptionInformation() and GetExceptionCode() retrieve
    // their information from this structure.
    EXCEPTION_POINTERS excptPtrs;
    excptPtrs.ExceptionRecord = pExceptionRecord;
    excptPtrs.ContextRecord = pContextRecord;
    reinterpret_cast<PEXCEPTION_POINTERS *>(pRegistrationFrame)[-1] = &excptPtrs;

    const ScopeTableEntry *scopeTable = pRegistrationFrame->ScopeTable;
    DWORD currentTrylevel = pRegistrationFrame->TryLevel;

    // Search all scopes from the inside out trying to find a filter that accepts the exception
    while (currentTrylevel != TRYLEVEL_NONE) {
        const void *filterFunclet = scopeTable[currentTrylevel].FilterFunction;
        if (filterFunclet) {
            const DWORD _ebp = (DWORD)&pRegistrationFrame->_ebp;
            DWORD filterResult;

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
                asm volatile (
                    "movl %0, %%ebp;"

                    "pushl %%ecx;"
                    "pushl %%ebx;"

                    // Unwind all scopes up to the one handling the exception
                    "pushl %%eax;"
                    "pushl %%edx;"
                    "call __local_unwind2;" // _local_unwind2(pRegistrationFrame, currentTrylevel);
                    "popl %%edx;"
                    "addl $4, %%esp;"

                    "popl %%ebx;"
                    "popl %%ecx;"

                    // Set the new trylevel in EXCEPTION_REGISTRATION_SEH3
                    "movl %%ecx, 12(%%edx);"
                    // Jump into the handler
                    "jmp *%%ebx;"
                    : : "r"(scopeEbp), "a"(currentTrylevel), "b"(handlerFunclet), "c"(newTrylevel), "d"(pRegistrationFrame) : "memory");
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

__declspec(noreturn) extern "C" void __stdcall _CxxThrowException (void *pExceptionObject, _ThrowInfo *pThrowInfo)
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

struct EXCEPTION_REGISTRATION_CXX : EXCEPTION_REGISTRATION
{
    DWORD id;
    DWORD _ebp;
};

/**
 * Checks all catch blocks of a given try block to find one that can catch the
 * exception. If a match is found, a pointer to the catch block is returned,
 * along with a pointer to the type the exception was caught as.
 * @param catchableType A pointer to a pointer that will receive the address of the type the exception was caught as. Will be NULL for catch(...) blocks
 * @param tryBlock The try block map entry that will get checked
 * @param throwInfo The exception type info
 * @return A pointer to the catch block that caught the exception, NULL if no block caught it
 */
static HandlerType *getCatchBlock (CatchableType **catchableType, TryBlockMapEntry *tryBlock, _ThrowInfo *throwInfo)
{
    assert(catchableType);
    assert(tryBlock);
    assert(throwInfo);

    for (int i = 0; i < tryBlock->nCatches; i++) {
        HandlerType *const catchBlock = &tryBlock->pHandlerArray[i];

        // catch(...) aka catch-all block, accepts all exception types
        if (!catchBlock->pType) {
            *catchableType = nullptr;
            return catchBlock;
        }

        for (int j = 0; j < throwInfo->pCatchableTypeArray->nCatchableTypes; j++) {
            CatchableType *const type = throwInfo->pCatchableTypeArray->arrayOfCatchableTypes[j];

            assert(type->pType);
            assert(catchBlock->pType);

            // Try direct pointer comparison for speed, if it fails, fall back to mangled name string comparison
            if (type->pType != catchBlock->pType) {
                if (strcmp(type->pType->name, catchBlock->pType->name) != 0) {
                    continue;
                }
            }

            // const or volatile exceptions need a catch block with the same property, reverse would be fine
            if ((throwInfo->attributes & TYPE_FLAG_CONST) && !(catchBlock->adjectives & TYPE_FLAG_CONST)) continue;
            if ((throwInfo->attributes & TYPE_FLAG_VOLATILE) && !(catchBlock->adjectives & TYPE_FLAG_VOLATILE)) continue;

            *catchableType = type;
            return catchBlock;
        }
    }

    return nullptr;
}

static inline void *call_ebp_func(void *func, void *_ebp)
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

static void *call_catch_block(void *func, void *_ebp)
{
    void *continue_addr = nullptr;
    __try {
        continue_addr = call_ebp_func(func, _ebp);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("RETHROW!\n");
    }
    return continue_addr;
}

static inline void continue_after_catch (EXCEPTION_REGISTRATION_CXX *frame, void *addr)
{
    asm volatile (
        "movl -4(%0), %%esp;"
        "leal 12(%0), %%ebp;"
        "jmp *%1;"
        : : "r"(frame), "a"(addr)
    );
}

static inline void *get_this_ptr (void *objptr, CatchableType *type)
{
    if (!objptr) return nullptr;
    assert(type);

    const PMD *pmd = &type->thisDisplacement;
    char *ptr = (char *)objptr;

    ptr += pmd->mdisp;
    if (pmd->pdisp != -1) {
        char *vbtable = ptr + pmd->pdisp;
        ptr += *(int*)(vbtable + pmd->vdisp);
    }
    DbgPrint("ptr1: %x\n", ptr);
    ptr = (char *)objptr;

    DbgPrint("pmd:\nmdisp: %d\npdisp: %d\nvdisp: %d\n", pmd->mdisp, pmd->pdisp, pmd->vdisp);

    if (pmd->pdisp >= 0) {
        int *offset_ptr;
        ptr += pmd->pdisp;
        offset_ptr = (int *)(*(char **)ptr + pmd->vdisp);
        ptr += *offset_ptr;
    }
    ptr += pmd->mdisp;

    DbgPrint("ptr2: %x\n", ptr);

    return ptr;
}

static inline void call_copy_function (void *func, void *thisptr, void *exceptionObject)
{
    // Call copy constructor with thiscall calling convention
    asm volatile (
        "pushl $1;"
        "pushl %1;"
        "call *%0;"
        "popl %1;"
        : : "r"(func), "r"(exceptionObject), "c"(thisptr)
        : "eax", "edx", "memory"
    );
}

static inline void call_destructor (void *func, void *thisptr)
{
    // Call destructor with thiscall calling convention
    asm volatile (
        "call *%0;"
        : : "r"(func), "c"(thisptr)
        : "eax", "edx", "memory"
    );
}

static void copy_exception_object (EXCEPTION_REGISTRATION_CXX *frame, HandlerType *catchBlock, CatchableType *catchType, void *exceptionObject)
{
    // FIXME: What about virtual stuff?
    if (!catchType) return;

    // The destination is an ebp-relative address, and may be different for each catch block
    void **destination = (void **)(((DWORD)&frame->_ebp) + catchBlock->dispCatchObj);

    if (catchBlock->adjectives & TYPE_FLAG_REFERENCE) {
        DbgPrint("> %d\n", __LINE__);
        *destination = get_this_ptr(exceptionObject, catchType);
    } else if (catchType->properties & CLASS_IS_SIMPLE_TYPE) {
        // Simple type, can be memmove()d
        DbgPrint("> %d\n", __LINE__);
        memmove(destination, exceptionObject, catchType->sizeOrOffset);
        DbgPrint("> %d\n", __LINE__);
        if (catchType->sizeOrOffset == sizeof(void *)) {
            DbgPrint("> %d\n", __LINE__);
            *destination = get_this_ptr(*destination, catchType);
        }
    } else if (catchType->copyFunction) {
        DbgPrint("> %d\n", __LINE__);
        call_copy_function((void *)catchType->copyFunction, destination, get_this_ptr(exceptionObject, catchType));
    } else {
        DbgPrint("> %d\n", __LINE__);
        // Simple type, can be memmove()d
        memmove(destination, get_this_ptr(exceptionObject, catchType), catchType->sizeOrOffset);
    }
}

static void local_unwind_cxx (EXCEPTION_REGISTRATION_CXX *frame, FuncInfo *functionInfo, DWORD trylevelEnd)
{
    while (true)
    {
        DWORD currentTrylevel = frame->id;

        if (currentTrylevel == trylevelEnd) {
            break;
        }

        if (functionInfo->pUnwindMap[currentTrylevel].action) {
            call_ebp_func((void *)functionInfo->pUnwindMap[currentTrylevel].action, &frame->_ebp);
        }

        frame->id = functionInfo->pUnwindMap[currentTrylevel].toState;
        DbgPrint("state transition: %d->%d\n", currentTrylevel, frame->id);
    }
}

extern "C" int CxxFrameHandlerVC8 (PEXCEPTION_RECORD pExcept, EXCEPTION_REGISTRATION_CXX *pRN, CONTEXT *pContext, void *pDC, FuncInfo *functionInfo)
{
    // Catch block funclet might clobber esp, so save and restore it before continuing execution
    DWORD save_esp = ((DWORD*)pRN)[-1];

    assert(functionInfo->magicNumber == MAGIC_VC8);

    // FUNC_DESCR_SYNCHRONOUS = 1
    if (!(functionInfo->EHFlags & 1) || (pExcept->ExceptionCode != CXX_EXCEPTION)) {
        DbgPrint("Not a C++ exception, skipping\n");
        return DISPOSITION_CONTINUE_SEARCH;
    }

    if (pExcept->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) {
        DbgPrint("maxState: %d\n", functionInfo->maxState);
        if (functionInfo->maxState != 0) {
            local_unwind_cxx(pRN, functionInfo, -1);
        }
        return DISPOSITION_CONTINUE_SEARCH;
    }

    _ThrowInfo *throwInfo = (_ThrowInfo*)pExcept->ExceptionInformation[2];
    assert(throwInfo);

    for (DWORD i = 0; i < functionInfo->nTryBlocks; i++) {
        const DWORD tryLow = functionInfo->pTryBlockMap[i].tryLow;
        const DWORD tryHigh = functionInfo->pTryBlockMap[i].tryHigh;

        if (tryLow > pRN->id) continue;
        if (pRN->id > tryHigh) continue;

        TryBlockMapEntry *tryBlock = &functionInfo->pTryBlockMap[i];
        CatchableType *catchType;
        HandlerType *catchBlock = getCatchBlock(&catchType, tryBlock, throwInfo);

        if (!catchBlock) continue;

        // Copy the exception object to the memory reserved by the catch block, needs to happen before unwinding
        copy_exception_object(pRN, catchBlock, catchType, (void *)pExcept->ExceptionInformation[1]);

        // First global unwinding, then local unwinding
        RtlUnwind(pRN, 0, pExcept, 0);
        local_unwind_cxx(pRN, functionInfo, tryBlock->tryLow);
        pRN->id = tryBlock->tryHigh + 1;

        void *continue_addr = call_catch_block(catchBlock->addressOfHandler, &pRN->_ebp);

        // Restore potentially clobbered saved esp before continuing execution
        ((DWORD*)pRN)[-1] = save_esp;

        // TODO: Check correctness
        if (!continue_addr) {
            continue;
        }

        // Destroy exception object, if necessary
        if (throwInfo->pmfnUnwind) {
            void *objaddr = (void *)(((DWORD)&pRN->_ebp) + catchBlock->dispCatchObj);
            call_destructor((void *)throwInfo->pmfnUnwind, objaddr);
        }

        // Resume normal execution after the catch block
        continue_after_catch(pRN, continue_addr);
    }

    // No matching catch block in this frame
    return DISPOSITION_CONTINUE_SEARCH;
}
