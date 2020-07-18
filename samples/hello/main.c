#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>

struct idtr_t
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_descriptor_t
{
	uint16_t offset_low;
	uint16_t selector;
    uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed));

void get_idtr (struct idtr_t *idtr)
{
    asm ("sidt %0" : "=m" (*idtr));
}

uint32_t dbg_output_original_isr_addr;
extern char dbg_output_isr_stub;

void hook_int_2d ()
{
    struct idtr_t idtr;
    struct idt_descriptor_t *entries;
    uint32_t new_isr = (uint32_t)&dbg_output_isr_stub;

    get_idtr(&idtr);
    entries = (struct idt_descriptor_t *)idtr.base;

    uint32_t old_isr = (((uint32_t)entries[0x2d].offset_high) << 16) | entries[0x2d].offset_low;
    dbg_output_original_isr_addr = old_isr;
    entries[0x2d].offset_low = ((uint32_t)new_isr) & 0xffff;
    entries[0x2d].offset_high = (((uint32_t)new_isr) >> 16) & 0xffff;
}

void unhook_int_2d ()
{
    struct idtr_t idtr;
    struct idt_descriptor_t *entries;

    get_idtr(&idtr);
    entries = (struct idt_descriptor_t *)idtr.base;
    entries[0x2d].offset_low = dbg_output_original_isr_addr & 0xffff;
    entries[0x2d].offset_high = (dbg_output_original_isr_addr >> 16) & 0xffff;
}

__cdecl void dbg_output (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp)
{
    if (eax == 1) {
        PANSI_STRING s = (PANSI_STRING)ecx;
        debugPrint("%s\n", s->Buffer);
    }
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    hook_int_2d();

    // This will be visible on screen
    OutputDebugString("test\n");

    unhook_int_2d();

    // This will not be visible on screen
    OutputDebugString("test2\n");

    debugPrint("tests done\n");

    while(1) {
        Sleep(2000);
    }

    return 0;
}
