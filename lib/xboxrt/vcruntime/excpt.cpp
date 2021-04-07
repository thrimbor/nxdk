#include <excpt.h>
#include <excpt_cxx.hpp>
#include <eh.h>
#include <xboxkrnl/xboxkrnl.h>
#include <hal/debug.h>
#include <string.h>

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

/*
struct HandlerMapEntry
{
    DWORD Adjectives;
    TypeDescriptor *Type; //(0=any?)
    DWORD CatchObjOffset;
    void *Handler;
};

struct TryBlockMapEntry
{
    DWORD TryLow;
    DWORD TryHigh;
    DWORD CatchHigh;
    DWORD NumCatches;
    HandlerMapEntry *HandlerArray;
};
*/
static HandlerType *getCatchBlock (TryBlockMapEntry *tryBlock, _ThrowInfo *throwInfo)
{
    for (DWORD i = 0; i < tryBlock->nCatches; i++) {
        // catch all block
        // FIXME: test this!
        if (!tryBlock->pHandlerArray[i].pType) {
            return &tryBlock->pHandlerArray[i];
        }

        for (int j = 0; j < throwInfo->pCatchableTypeArray->nCatchableTypes; j++) {
            CatchableType *catchableType = throwInfo->pCatchableTypeArray->arrayOfCatchableTypes[j];
            if (catchableType->pType == 0 || tryBlock->pHandlerArray[i].pType == 0) { // FIXME: last cond already handled above?
                if (catchableType->pType == tryBlock->pHandlerArray[i].pType) {
                    // FIXME: What does this mean? Same RTTI addr?
                    return &tryBlock->pHandlerArray[i];
                }
            }

            //if (*catchableType->pType == *tryBlock->pHandlerArray[i].Type) {
            // TODO: verify that this works
            // FIXME: Use proper comparison operator
            DbgPrint("catchableType: %s\n", catchableType->pType->name);
            DbgPrint("tryBlockType: %s\n", tryBlock->pHandlerArray[i].pType->name);
            if (strcmp(catchableType->pType->name, tryBlock->pHandlerArray[i].pType->name) == 0) {
                DbgPrint("kablamm\n");
                return &tryBlock->pHandlerArray[i];
            }

            // FIXME: This doesn't look like it's enough - proper type comparison needed!
        }
    }

    return nullptr;
}

/**
 * Retrieves the try block and catch block, if any, responsible for the exception
 */
static void getTryCatchBlocks (TryBlockMapEntry **tryBlock, HandlerType **catchBlock, EXCEPTION_REGISTRATION_CXX *exceptionRegistration, FuncInfo *functionInfo, _ThrowInfo *throwInfo)
{
    // FIXME: I think this is incorrect, we need to start at EXCEPTION_REGISTRATION_CXX->id
    for (DWORD i = 0; i < functionInfo->nTryBlocks; i++) {
        const DWORD tryLow = functionInfo->pTryBlockMap[i].tryLow;
        const DWORD tryHigh = functionInfo->pTryBlockMap[i].tryHigh;

        if ((tryLow <= exceptionRegistration->id) && (exceptionRegistration->id <= tryHigh)) {
            TryBlockMapEntry *currentTryBlock = &functionInfo->pTryBlockMap[i];
            HandlerType *currentCatchBlock = getCatchBlock(currentTryBlock, throwInfo);
            if (currentCatchBlock) {
                *tryBlock = currentTryBlock;
                *catchBlock = currentCatchBlock;
                return;
            }
        }
    }

    *tryBlock = nullptr;
    *catchBlock = nullptr;
}

extern "C" int CxxFrameHandlerVC8 (PEXCEPTION_RECORD pExcept, EXCEPTION_REGISTRATION_CXX *pRN, CONTEXT *pContext, void *pDC, FuncInfo *functionInfo)
{
    // - Step through the try blocks inside out
        // - For each try block, step through the catch blocks
            // - For each catch block, compare the catch type to the exception object
            // - if the type is equal, execute catch block
            // - if the type doesn't match, continue searching
    // - nothing found? continue searching

    //DbgPrint("frame handler reached!\n");

    // FIXME: What will functionInfo be in C?
    if (functionInfo->magicNumber != MAGIC_VC8) {
        DbgPrint("Not a C++ frame, skipping\n");
        return DISPOSITION_CONTINUE_SEARCH;
    }

    // FUNC_DESCR_SYNCHRONOUS = 1
    if ((functionInfo->EHFlags & 1) && (pExcept->ExceptionCode != CXX_EXCEPTION)) {
        DbgPrint("Not a C++ exception, skipping\n");
        return DISPOSITION_CONTINUE_SEARCH;
    }

    TryBlockMapEntry *tryBlock;
    HandlerType *catchBlock;
    getTryCatchBlocks(&tryBlock, &catchBlock, pRN, functionInfo, (_ThrowInfo*)pExcept->ExceptionInformation[2]);

    DbgPrint("try block: %x\n", tryBlock);
    DbgPrint("catch block: %x\n", catchBlock);

    if (!tryBlock || !catchBlock) {
        DbgPrint("No handler, found, skipping\n");
        return DISPOSITION_CONTINUE_SEARCH;
    }

    DbgPrint("dispCatchObj: %x %d\n", catchBlock->dispCatchObj, catchBlock->dispCatchObj);
    // FIXME: Copy properly
    DWORD excp_obj_pointer = ((DWORD)&pRN->_ebp) + catchBlock->dispCatchObj;

    *(DWORD *)excp_obj_pointer = *(DWORD *)pExcept->ExceptionInformation[1];

    // FIXME: global unwind
    // FIXME: local unwind
    // FIXME: call handler

    // FIXME: Handle exception here

    const DWORD scopeEbp = (DWORD)&pRN->_ebp;
    const DWORD currentTrylevel = 0;
    const DWORD handlerFunclet = (DWORD)catchBlock->addressOfHandler;
    const DWORD newTrylevel = 0;
    asm volatile (
        "movl %0, %%ebp;"

        "pushl %%ecx;"
        "pushl %%ebx;"

        // Unwind all scopes up to the one handling the exception
        "pushl %%eax;"
        "pushl %%edx;"
        //"call __local_unwind2;" // _local_unwind2(pRegistrationFrame, currentTrylevel);
        "popl %%edx;"
        "addl $4, %%esp;"

        "popl %%ebx;"
        "popl %%ecx;"

        // Set the new trylevel in EXCEPTION_REGISTRATION_SEH3
        //"movl %%ecx, 12(%%edx);"
        // Call the handler
        "call *%%ebx;"

        // FIXME: Do we need to do something before jumping? Any housekeeping? Exception object destruction, maybe?
        // Jump to after the catch block, address returned by handler
        "jmp *%%eax;"
        : : "r"(scopeEbp), "a"(currentTrylevel), "b"(handlerFunclet), "c"(newTrylevel), "d"(pRN) : "memory");

    DbgPrint("Returned from handler\n");

    return DISPOSITION_CONTINUE_SEARCH;
}
