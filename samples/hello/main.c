#include <assert.h>
#include <stdint.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

typedef struct {
    uint32_t header;
    uint32_t Unknown1; // always 2?
    uint8_t data[496];
    uint32_t footer;
    uint32_t zero;
} cache_sector;

typedef struct {
    uint32_t titleId;
    uint32_t unknown1;
    uint32_t unknown2;
} cache_entry;

#define XB_CACHE_DESCRIPTOR_SECTOR_HEADER 0x97315286
#define XB_CACHE_DESCRIPTOR_SECTOR_FOOTER 0xAA550000

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    debugPrint("Cache partition count: %d\n", HalDiskCachePartitionCount);

    NTSTATUS status;
    HANDLE handle;
    IO_STATUS_BLOCK iostatus;
    OBJECT_ATTRIBUTES attributes;
    ANSI_STRING str;
    RtlInitAnsiString(&str, "\\Device\\Harddisk0\\Partition0");
    InitializeObjectAttributes(&attributes, &str, OBJ_CASE_INSENSITIVE, NULL, NULL);
    status = NtOpenFile(&handle, SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE, &attributes, &iostatus, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_ALERT);
    if (!NT_SUCCESS(status)) {
        debugPrint("Could not open part0: 0x%08x\n", status);
        goto end;
    }

    assert(sizeof(cache_sector) == 512);
    LARGE_INTEGER offset;
    offset.QuadPart = 4 * 512;
    cache_sector sector;
    status = NtReadFile(handle, 0, NULL, NULL, &iostatus, &sector, sizeof(sector), &offset);
    if (!NT_SUCCESS(status)) {
        debugPrint("Could not read from part0: 0x%08x", status);
        goto close_end;
    }

    assert(sector.header == XB_CACHE_DESCRIPTOR_SECTOR_HEADER);
    assert(sector.footer == XB_CACHE_DESCRIPTOR_SECTOR_FOOTER);
    debugPrint("unknown: %d\n", sector.Unknown1);
    debugPrint("zero: %d\n", sector.zero);

    cache_entry *entries = sector.data;
    for (int i=0; i < HalDiskCachePartitionCount; i++) {
        debugPrint("ID: 0x%08x u1: 0x%08x u2: 0x%08x\n", entries[i].titleId, entries[i].unknown1, entries[i].unknown2);
    }

close_end:
    NtClose(handle);

end:
    while(1) {
        Sleep(2000);
    }

    return 0;
}
