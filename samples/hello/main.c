#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>

__declspec(dllexport) void testfunc (void)
{
    debugPrint("moo\n");
}

__declspec(dllexport) void testfunc2 (void)
{
    debugPrint("meow\n");
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    debugPrint("&testfunc == 0x%x\n", &testfunc);
    debugPrint("GetProcAddress(NULL, \"testfunc\") == 0x%x\n", GetProcAddress(NULL, "testfunc"));
    debugPrint("&testfunc2 == 0x%x\n", &testfunc2);
    debugPrint("GetProcAddress(NULL, \"testfunc2\") == 0x%x\n", GetProcAddress(NULL, "testfunc2"));

    while(1) {
        Sleep(2000);
    }

    return 0;
}
