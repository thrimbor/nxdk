#include <cstdarg>
#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include "nunit.h"

void nunit_print (const char *str, ...)
{
    char outstr[4096];
    va_list args;
    va_start(args, str);
    vsprintf(outstr, str, args);
    debugPrint("%s", outstr);
    DbgPrint("%s", outstr);
    va_end(args);
}

NUNIT_DEFINE_GLOBALS

// TODO: Non-virtual inheritance with different offsets

int main ()
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    __try {
        nunit_run_tests();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        nunit_print("Unexpected exception: %08x\n", GetExceptionCode());
    }

    while (1) Sleep(1000);

    return 0;
}
