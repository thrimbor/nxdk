#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <net/pktdrv/pktdrv.h>

unsigned int received = 0;

int Pktdrv_Callback(unsigned char *packetaddr, unsigned int size)
{
    received += size;
    return 1;
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    Pktdrv_Init();

    while(1) {
        debugPrint("Received: %d\n", received);
        received = 0;
        Sleep(1000);
    }

    return 0;
}
