// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2020-2024 Stefan Schmidt

#include <assert.h>
#include <stdlib.h>
#include <eh.h>

void __cdecl __ExceptionPtrCreate (void*)
{
    assert(0);
}

void __cdecl __ExceptionPtrDestroy (void*)
{
    assert(0);
}

void __cdecl __ExceptionPtrCopy (void*, const void*)
{
    assert(0);
}

void __cdecl __ExceptionPtrAssign (void*, const void*)
{
    assert(0);
}

bool __cdecl __ExceptionPtrCompare (const void*, const void*)
{
    assert(0);
    return false;
}

bool __cdecl __ExceptionPtrToBool (const void*)
{
    assert(0);
    return false;
}

void __cdecl __ExceptionPtrSwap (void*, void*)
{
    assert(0);
}

void __cdecl __ExceptionPtrCurrentException (void*)
{
    assert(0);
}

[[noreturn]] void __cdecl __ExceptionPtrRethrow (const void*)
{
    assert(0);
}

void __cdecl __ExceptionPtrCopyException (void*, const void*, const void*)
{
    assert(0);
}

extern "C"
{
    static thread_local terminate_handler current_terminate_handler = NULL;

    terminate_handler __cdecl set_terminate (terminate_handler _NewTerminateHandler) throw()
    {
        terminate_handler old_handler = current_terminate_handler;
        current_terminate_handler = _NewTerminateHandler;
        return old_handler;
    }

    terminate_handler __cdecl _get_terminate ()
    {
        return current_terminate_handler;
    }

    void __cdecl terminate ()
    {
        if (current_terminate_handler) {
            current_terminate_handler();
        }

        // Help finding the issue when running a debug build
        assert(0);

        abort();
    }

    void __cdecl __std_terminate ()
    {
        terminate();
    }

    unexpected_handler __cdecl set_unexpected (unexpected_handler _NewUnexpectedHandler) throw()
    {
        assert(0);
        return 0;
    }

    unexpected_handler __cdecl _get_unexpected ()
    {
        assert(0);
        return 0;
    }

    int __cdecl __uncaught_exceptions ()
    {
        assert(0);
        return 0;
    }
}
