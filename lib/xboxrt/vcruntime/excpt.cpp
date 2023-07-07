// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2020-2023 Stefan Schmidt
// SPDX-FileCopyrightText: 2021 Lucas Jansson

#include <assert.h>
#include <excpt.h>
#include <excpt_cxx.hpp>
#include <eh.h>
#include <string.h>
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

static int RethrowFilter (EXCEPTION_POINTERS *p)
{
    PEXCEPTION_RECORD er = p->ExceptionRecord;

    //DbgPrint("RethrowFilter:\n");
    //DbgPrint("0: %x\n", er->ExceptionInformation[0]);
    //DbgPrint("1: %x\n", er->ExceptionInformation[1]);
    //DbgPrint("2: %x\n", er->ExceptionInformation[2]);

    if (er->ExceptionCode == CXX_EXCEPTION
        && er->NumberParameters == 3
        && (er->ExceptionInformation[0] == MAGIC_VC
            || er->ExceptionInformation[0] == MAGIC_VC7
            || er->ExceptionInformation[0] == MAGIC_VC8)) {
        if (er->ExceptionInformation[1] == 0 && er->ExceptionInformation[2] == 0) {
            //DbgPrint("It's a simple rethrow\n");
            return EXCEPTION_EXECUTE_HANDLER;
        } else {
            //DbgPrint("Another C++ exception was thrown\n");
            //return EXCEPTION_EXECUTE_HANDLER;
        }
    }

    //DbgPrint("ExceptionCode: %x\n", p->ExceptionRecord->ExceptionCode);
    //DbgPrint("NumberParameters: %x\n", p->ExceptionRecord->NumberParameters);
    //DbgPrint("magicNumber: %x\n", p->ExceptionRecord->ExceptionInformation[0]);
    return EXCEPTION_CONTINUE_SEARCH;
}

static void *call_catch_block(void *func, void *_ebp, EXCEPTION_REGISTRATION_CXX *pRN, FuncInfo *functionInfo)
{
    DWORD save_esp = ((DWORD*)pRN)[-1];
    //DbgPrint("Saved esp: %x\n", save_esp);
    void *continue_addr;

    //DbgPrint("> %d: %08x\n", __LINE__, pRN);
    DWORD _esp;
    asm volatile ("movl %%esp, %%eax" : "=a"(_esp));
    //DbgPrint("e> %d esp: %x\n", __LINE__, _esp);

    __try {
        __try {
            continue_addr = call_ebp_func(func, _ebp);
        } __except (RethrowFilter((EXCEPTION_POINTERS *)_exception_info())) {
            // we need a proper filter here!
            //DbgPrint("RETHROW!\n");
            continue_addr = nullptr;
        }
        // FIXME: Should we intercept and handle other throws here, too?
    } __finally {
        //DbgPrint("> %d: pRN%08x\n", __LINE__, pRN);
        //DbgPrint("__finally restoring esp: %x\n", save_esp);
        //DbgPrint("previously: %x\n", ((DWORD*)pRN)[-1]);
        ((DWORD*)pRN)[-1] = save_esp;
        //DbgPrint("> %d\n", __LINE__);
    }

    ((DWORD*)pRN)[-1] = save_esp;

    return continue_addr;
}

static inline void continue_after_catch (EXCEPTION_REGISTRATION_CXX *frame, void *addr)
{
    //DbgPrint("continuing at %x\n", addr);
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
    //DbgPrint("ptr1: %x\n", ptr);
    ptr = (char *)objptr;

    //DbgPrint("pmd:\nmdisp: %d\npdisp: %d\nvdisp: %d\n", pmd->mdisp, pmd->pdisp, pmd->vdisp);

    if (pmd->pdisp >= 0) {
        int *offset_ptr;
        ptr += pmd->pdisp;
        offset_ptr = (int *)(*(char **)ptr + pmd->vdisp);
        ptr += *offset_ptr;
    }
    ptr += pmd->mdisp;

    //DbgPrint("ptr2: %x\n", ptr);

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
        //DbgPrint("> %d\n", __LINE__);
        *destination = get_this_ptr(exceptionObject, catchType);
    } else if (catchType->properties & CLASS_IS_SIMPLE_TYPE) {
        // Simple type, can be memmove()d
        //DbgPrint("> %d\n", __LINE__);
        memmove(destination, exceptionObject, catchType->sizeOrOffset);
        //DbgPrint("> %d\n", __LINE__);
        if (catchType->sizeOrOffset == sizeof(void *)) {
            //DbgPrint("> %d\n", __LINE__);
            *destination = get_this_ptr(*destination, catchType);
        }
    } else if (catchType->copyFunction) {
        //DbgPrint("> %d\n", __LINE__);
        call_copy_function((void *)catchType->copyFunction, destination, get_this_ptr(exceptionObject, catchType));
    } else {
        //DbgPrint("> %d\n", __LINE__);
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
        //DbgPrint("state transition: %d->%d\n", currentTrylevel, frame->id);
    }
}

static void global_unwind_cxx (PEXCEPTION_RECORD pExcept, EXCEPTION_REGISTRATION_CXX *pRN)
{
    // This looks like it could be a normal C function call, but it can't.
    // Despite looking like an stdcall function, RtlUnwind doesn't follow its
    // calling convention and happily destroys register values.
    // We work around this by using %eax and %ebx for the parameters and adding
    // everything else to the clobber list.
    asm volatile (
        "pushl $0;"
        "pushl %1;"
        "pushl $0;"
        "pushl %0;"
        "call _RtlUnwind@16;"
        : : "a"(pRN), "b"(pExcept) : "ecx", "edx", "esi", "edi", "ebp");
}

extern "C" int CxxFrameHandlerVC8 (PEXCEPTION_RECORD pExcept, EXCEPTION_REGISTRATION_CXX *pRN, CONTEXT *pContext, void *pDC, FuncInfo *functionInfo)
{
    //DbgPrint("CxxFrameHandlerVC8\n");
    // Catch block funclet might clobber esp, so save and restore it before continuing execution
    DWORD save_esp = ((DWORD*)pRN)[-1];
    //DbgPrint("Saving esp: %x\n", save_esp);

    assert(functionInfo->magicNumber == MAGIC_VC8);
//DbgPrint("> %d\n", __LINE__);
    // FUNC_DESCR_SYNCHRONOUS = 1
    if (!(functionInfo->EHFlags & 1) || (pExcept->ExceptionCode != CXX_EXCEPTION)) {
        //DbgPrint("Not a C++ exception, skipping\n");
        return DISPOSITION_CONTINUE_SEARCH;
    }
//DbgPrint("> %d\n", __LINE__);
    if (pExcept->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) {
        //DbgPrint("maxState: %d\n", functionInfo->maxState);
        if (functionInfo->maxState != 0) {
            local_unwind_cxx(pRN, functionInfo, -1);
        }
        return DISPOSITION_CONTINUE_SEARCH;
    }
//DbgPrint("> %d\n", __LINE__);
    _ThrowInfo *throwInfo = (_ThrowInfo*)pExcept->ExceptionInformation[2];
    assert(throwInfo);
//DbgPrint("> %d\n", __LINE__);
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
        //DbgPrint("> %d\n", __LINE__);

        // First global unwinding, then local unwinding
        global_unwind_cxx(pExcept, pRN);
        //DbgPrint("> %d\n", __LINE__);
        local_unwind_cxx(pRN, functionInfo, tryBlock->tryLow);
        //DbgPrint("> %d\n", __LINE__);
        pRN->id = tryBlock->tryHigh + 1;
//DbgPrint("> %d\n", __LINE__);
        void *continue_addr = call_catch_block(catchBlock->addressOfHandler, &pRN->_ebp, pRN, functionInfo);
//DbgPrint("> %d\n", __LINE__);
        // Restore potentially clobbered saved esp before continuing execution
        //DbgPrint("Restoring esp: %x\n", save_esp);
        //((DWORD*)pRN)[-1] = save_esp;
//DbgPrint("> %d\n", __LINE__);
        // TODO: Check correctness
        if (!continue_addr) {
            //DbgPrint("> %d\n", __LINE__);
            continue;
        }
//DbgPrint("> %d\n", __LINE__);
        // Destroy exception object, if necessary
        if (throwInfo->pmfnUnwind) {
            //DbgPrint("> %d\n", __LINE__);
            void *objaddr = (void *)(((DWORD)&pRN->_ebp) + catchBlock->dispCatchObj);
            call_destructor((void *)throwInfo->pmfnUnwind, objaddr);
            //DbgPrint("> %d\n", __LINE__);
        }
//DbgPrint("> %d\n", __LINE__);

        // Resume normal execution after the catch block
        continue_after_catch(pRN, continue_addr);
    }

    // No matching catch block in this frame
    return DISPOSITION_CONTINUE_SEARCH;
}
