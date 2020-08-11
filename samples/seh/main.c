#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>

// The nullpointer deref is in its own function, because LLVM can't catch
// asynchronous exceptions in the same frame that they're thrown in. This is a
// documented MSVC-incompatibility in LLVM, see:
// https://clang.llvm.org/docs/MSVCCompatibility.html#abi-features
static void nullptr_deref (void)
{
  int *p = NULL;
  *p = 5;
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    __try {
        __try {
            __try {
                nullptr_deref();
            } __except (EXCEPTION_CONTINUE_SEARCH) {
                // This filter rejects all exceptions, this block should therefore never run
                debugPrint("This should never run!\n");
            }
        } __finally {
            // This will be called during the unwind pass, after the filter funclet ran, but before the except block runs
            debugPrint("__finally called\n");
        }
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // This filter accepts EXCEPTION_ACCESS_VIOLATION, therefore it should accept the nullpointer deref
        debugPrint("Exception catched!\n");
    }


    debugPrint("done\n");

    while(1) {
        Sleep(2000);
    }

    return 0;
}
