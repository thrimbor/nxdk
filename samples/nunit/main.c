#include <xboxrt/debug.h>
#include <pbkit/pbkit.h>
#include <hal/xbox.h>
#include <stdarg.h>
#include "stdio.h"
#include "nunit.h"

void nunit_print (const char *str, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, str);
    vsprintf(buffer, str, args);
    va_end(args);
    debugPrint("%s", buffer);
    DbgPrint("%s", buffer);
}

NUNIT_DEFINE_GLOBALS

int main(void)
{
    int i;

    switch(pb_init())
    {
        case 0: break;
        default:
            XSleep(2000);
            XReboot();
            return -1;
    }

    pb_show_debug_screen();

    // Mount C:
    OBJECT_STRING MountPath = RTL_CONSTANT_STRING("\\??\\C:");
    OBJECT_STRING DrivePath = RTL_CONSTANT_STRING("\\Device\\Harddisk0\\Partition2\\");
    NTSTATUS status;
    status = IoCreateSymbolicLink(&MountPath, &DrivePath);
    if (!NT_SUCCESS(status)) {
        debugPrint("Failed to mount C: drive!\n");
        XSleep(5000);
        return -1;
    }

    nunit_run_tests();

    while(1) {
        XSleep(2000);
    }

    pb_kill();
    XReboot();
    return 0;
}
