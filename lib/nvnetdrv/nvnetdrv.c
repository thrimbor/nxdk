// TODO:
// - tell number of rx buffers on init
// - init reserves and queues buffers
// - IRQ handler hands of buffer index to callback
// - callback (or other thread, make threadsafe) can then free index to requeue
//    ^ doesn't need to be IRQ-safe afaik

#include "nvnetdrv.h"
#include "nvnetdrv_regs.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <xboxkrnl/xboxkrnl.h>

struct __attribute__((packed)) descriptor_t
{
    uint32_t paddr;
    uint16_t length;
    uint16_t flags;
};

struct tx_misc_t
{
    void *bufAddr;
    size_t length;
    tx_callback_t callback;
    void *userdata;
};

#ifdef NVNETDRV_ENABLE_STATS
struct nvnetdrv_stats_t
{
    uint32_t rx_interrupts;
    uint32_t rx_extraByteErrors;
    uint32_t tx_interrupts;
    uint32_t phy_interrupts;
};
static struct nvnetdrv_stats_t nvnetdrv_stats;
#define INC_STAT(statname, val) do {nvnetdrv_stats.statname += (val);} while(0);
#else
#define INC_STAT(statname, val)
#endif

// FIXME
#define BASE ((void *)0xFEF00000)

#define reg32(offset) (*((volatile uint32_t *)((uintptr_t)BASE + (offset))))

// FIXME: A lot of these aren't (re-)initialized yet!
static atomic_bool g_running = false;
static uint8_t g_ethAddr[6];
static uint32_t g_linkSpeed;
static ULONG g_irq;
static KIRQL g_irql;
static KDPC g_dpc_obj;
static KINTERRUPT g_interrupt;
static HANDLE g_irq_thread;
static KEVENT g_irq_event;
static volatile struct descriptor_t *g_rxDescriptors;
static volatile struct descriptor_t *g_txDescriptors;
static uint8_t *g_rx_buffers;
static size_t g_rx_buffer_count;
static size_t g_rxBeginIndex;
static size_t g_rxEndIndex;
static uint32_t g_rxBufferVirtMinusPhys;
static size_t g_txBeginIndex;
static atomic_size_t g_txEndIndex;
static atomic_size_t g_txDescriptorsInUseCount;
static KSEMAPHORE g_freeTxDescriptors;
struct tx_misc_t tx_misc[TX_RING_SIZE];

static nvnetdrv_rx_callback_t g_rx_callback;

const uint8_t *nvnetdrv_get_ethernet_addr()
{
    return g_ethAddr;
}

static void nvnetdrv_irq_disable (void)
{
    reg32(NvRegIrqMask) = 0;
}

static void nvnetdrv_irq_enable (void)
{
    reg32(NvRegIrqMask) = NVREG_IRQMASK_THROUGHPUT;
}

static BOOLEAN NTAPI nvnetdrv_isr (PKINTERRUPT Interrupt, PVOID ServiceContext)
{
    nvnetdrv_irq_disable();
    KeInsertQueueDpc(&g_dpc_obj, NULL, NULL);

    return TRUE;
}

static void __stdcall nvnetdrv_dpc (PKDPC Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
{
    KeSetEvent(&g_irq_event, IO_NETWORK_INCREMENT, FALSE);
    return;
}

static uint32_t nvnetdrv_rx_ptov (uint32_t phys_address)
{
    return phys_address + g_rxBufferVirtMinusPhys;
}

static uint32_t nvnetdrv_rx_vtop (uint32_t virt_address)
{
    return virt_address - g_rxBufferVirtMinusPhys;
}

void nvnetdrv_rx_queue_buffer (void *buffer_virt)
{
    assert((uint32_t)buffer_virt >= (uint32_t)g_rx_buffers && (uint32_t)buffer_virt < (uint32_t)g_rx_buffers + g_rx_buffer_count*2048);

    uint32_t virt_addr = (uint32_t)buffer_virt;

    g_rxDescriptors[g_rxEndIndex].flags = NV_RX_AVAIL;
    g_rxDescriptors[g_rxEndIndex].length = 2048;

    // Increment g_rxEndIndex *after* setting up the descriptor
    g_rxEndIndex = (g_rxEndIndex + 1) % RX_RING_SIZE;
}

static void nvnetdrv_handle_rx_irq (void)
{
    uint32_t work_left = RX_RING_SIZE;
    while (work_left) {
        uint16_t flags = g_rxDescriptors[g_rxBeginIndex].flags;

        if (flags & NV_RX_AVAIL) {
            // Reached a descriptor that still belongs to the NIC
            break;
        }

        if ((flags & NV_RX_DESCRIPTORVALID) == 0) {
            // Re-queue the buffer
            nvnetdrv_rx_queue_buffer((void *)nvnetdrv_rx_ptov(g_rxDescriptors[g_rxBeginIndex].paddr));
            goto next_packet;
        }

        if (!(flags & NV_RX_ERROR) || (flags & NV_RX_FRAMINGERR)) {
            uint16_t packet_length = g_rxDescriptors[g_rxBeginIndex].length;

            if (flags & NV_RX_SUBTRACT1) {
                INC_STAT(rx_extraByteErrors, 1);
                packet_length--;
            }

            INC_STAT(rx_receivedPackets, 1);

            uint32_t buffer_virt = nvnetdrv_rx_ptov(g_rxDescriptors[g_rxBeginIndex].paddr);
            // Hand off the packet to the network stack
            // FIXME: We need to hand out the index, too! - I don't think we really do
            assert(g_rx_callback);
            g_rx_callback(g_rxBeginIndex, (void *)buffer_virt, packet_length);
            goto next_packet;
        } else {
            if (flags & NV_RX_FRAMINGERR) INC_STAT(rx_framingError, 1);
            if (flags & NV_RX_OVERFLOW) INC_STAT(rx_overflowError, 1);
            if (flags & NV_RX_CRCERR) INC_STAT(rx_crcError, 1);
            if (flags & NV_RX_ERROR4) INC_STAT(rx_error4, 1);
            if (flags & NV_RX_ERROR3) INC_STAT(rx_error3, 1);
            if (flags & NV_RX_ERROR2) INC_STAT(rx_error2, 1);
            if (flags & NV_RX_ERROR1) INC_STAT(rx_error1, 1);
            if (flags & NV_RX_MISSEDFRAME) INC_STAT(rx_missedFrameError, 1);
        }

next_packet:
        g_rxBeginIndex = (g_rxBeginIndex + 1) % RX_RING_SIZE;
        work_left--;
    }

    INC_STAT(rx_interrupts, 1);
}

static void nvnetdrv_handle_tx_irq (void)
{
    LONG freed_descriptors = 0;

    while (g_txDescriptorsInUseCount > 0) {
        uint16_t flags = g_txDescriptors[g_txBeginIndex].flags;

        if (flags & NV_TX_VALID) {
            // We reached a descriptor that wasn't processed by hw yet
            assert(g_txDescriptorsInUseCount == 0);
            break;
        }
        if (tx_misc[g_txBeginIndex].callback)
        {
            tx_misc[g_txBeginIndex].callback(tx_misc[g_txBeginIndex].userdata);
        }
        // Buffers get locked before sending and unlocked after sending
        MmLockUnlockBufferPages(tx_misc[g_txBeginIndex].bufAddr, tx_misc[g_txBeginIndex].length, TRUE);

        freed_descriptors++;
        g_txBeginIndex = (g_txBeginIndex + 1) % TX_RING_SIZE;
        g_txDescriptorsInUseCount--;
    }

    KeReleaseSemaphore(&g_freeTxDescriptors, IO_NETWORK_INCREMENT, freed_descriptors, FALSE);
    INC_STAT(tx_interrupts, 1);
}

static void nvnetdrv_handle_mii_irq (uint32_t miiStatus, bool init)
{
    uint32_t adapterControl = reg32(NvRegAdapterControl);
    uint32_t linkState = PhyGetLinkState(!init);

    if (miiStatus & NVREG_MIISTAT_LINKCHANGE) {
        nvnetdrv_stop_txrx();
    }

    if (linkState & XNET_ETHERNET_LINK_10MBPS) {
        g_linkSpeed = NVREG_LINKSPEED_10MBPS;
    } else {
        g_linkSpeed = NVREG_LINKSPEED_100MBPS;
    }

    if (linkState & XNET_ETHERNET_LINK_FULL_DUPLEX) {
        reg32(NvRegDuplexMode) &= NVREG_DUPLEX_MODE_FDMASK;
    } else {
        reg32(NvRegDuplexMode) |= NVREG_DUPLEX_MODE_HDFLAG;
    }

    if (miiStatus & NVREG_MIISTAT_LINKCHANGE) {
        nvnetdrv_start_txrx();

        INC_STAT(phy_interrupts, 1);
    }
}

static void nvnetdrv_handle_irq (void)
{
    while (true) {
        uint32_t irq = reg32(NvRegIrqStatus);
        uint32_t mii = reg32(NvRegMIIStatus);

        if (!irq && !mii) break;

        reg32(NvRegMIIStatus) = mii;
        reg32(NvRegIrqStatus) = irq;

        if (irq & NVREG_IRQ_RX_ALL) {
            nvnetdrv_handle_rx_irq();
        }
        if (irq & NVREG_IRQ_TX_ALL) {
            nvnetdrv_handle_tx_irq();
        }
        if (irq & NVREG_IRQ_LINK) {
            nvnetdrv_handle_mii_irq(mii, false);
        }
    }
}

static void NTAPI nvnetdrv_irq_thread (PKSTART_ROUTINE StartRoutine, PVOID StartContext)
{
    (void)StartRoutine;
    (void)StartContext;

    while (true) {
        KeWaitForSingleObject(&g_irq_event, Executive, KernelMode, FALSE, NULL);

        if (!g_running) break;

        nvnetdrv_handle_irq();

        nvnetdrv_irq_enable();
    }

    PsTerminateSystemThread(0);
}

// FIXME: we need error codes or something
// FIXME: Just temporary :)
#define NVNET_OK 0
#define NVNET_NO_MEM -1
#define NVNET_NO_MAC -2
#define NVNET_PHY_ERR -3
#define NVNET_SYS_ERR -4

int nvnetdrv_init (size_t rx_buffer_count, nvnetdrv_rx_callback_t rx_callback)
{
    assert(!g_running);

    assert(rx_callback);
    g_rx_callback = rx_callback;

    ULONG type;
    NTSTATUS status = ExQueryNonVolatileSetting(XC_FACTORY_ETHERNET_ADDR, &type, &g_ethAddr, 6, NULL);
    assert(status == STATUS_SUCCESS);
    if (!NT_SUCCESS(status)) {
        return NVNET_NO_MAC;
    }

    //_Static_Assert(RX_RING_SIZE <= 1024);
    //_Static_Assert(TX_RING_SIZE <= 1024);

    void *descriptors = MmAllocateContiguousMemoryEx((RX_RING_SIZE + TX_RING_SIZE) * sizeof(struct descriptor_t), 0, 0xFFFFFFFF, 0, PAGE_READWRITE);
    assert(descriptors);
    if (!descriptors) {
        return NVNET_NO_MEM;
    }

    RtlZeroMemory(descriptors, (RX_RING_SIZE + TX_RING_SIZE) * sizeof(struct descriptor_t));

    g_rxDescriptors = (struct descriptor_t *)descriptors;
    g_txDescriptors = (struct descriptor_t *)descriptors + RX_RING_SIZE;

    assert(rx_buffer_count > 0);
    g_rx_buffer_count = rx_buffer_count;

    g_rx_buffers = MmAllocateContiguousMemoryEx(rx_buffer_count * 2048, 0, 0xFFFFFFFF, 0, PAGE_READWRITE);
    assert(g_rx_buffers);
    if (!g_rx_buffers) {
        MmFreeContiguousMemory(descriptors);
        return NVNET_NO_MEM;
    }

    g_rxBufferVirtMinusPhys = ((uint32_t)g_rx_buffers) - (uint32_t)MmGetPhysicalAddress(g_rx_buffers);

    //Setup the RX ring descriptors
    for (int i = 0; i < RX_RING_SIZE; i++)
    {
        nvnetdrv_rx_queue_buffer(g_rx_buffers + (i * 2048));
    }

    //Mac is NOT reversed
    reg32(NvRegMacAddrA) = (g_ethAddr[0] << 0) | (g_ethAddr[1] << 8) | (g_ethAddr[2] << 16) | (g_ethAddr[3] << 24);
    reg32(NvRegMacAddrB) = (g_ethAddr[4] << 0) | (g_ethAddr[5] << 8);
    reg32(NvRegMulticastAddrA) = NVREG_MCASTMASKA_NONE;
    reg32(NvRegMulticastAddrB) = NVREG_MCASTMASKB_NONE;
    reg32(NvRegMulticastMaskA) = NVREG_MCASTMASKA_NONE;
    reg32(NvRegMulticastMaskB) = NVREG_MCASTMASKB_NONE;

    //FIXME: MSDash sets up these registers too: Needed?
    //reg32(NvRegOffloadConfig) = NVREG_OFFLOAD_NORMAL;
    //reg32(NvRegPacketFilterFlags) = (NVREG_PFF_ALWAYS_MYADDR | NVREG_PFF_MYADDR);
    //reg32(NvRegDuplexMode) = NVREG_DUPLEX_MODE_FORCEH;
    //reg32(NvRegSlotTime) = ((rand() % 0xFF) & NVREG_SLOTTIME_MASK) | NVREG_SLOTTIME_10_100_FULL;

    reg32(NvRegTxDeferral) = NVREG_TX_DEFERRAL_RGMII_10_100;
	reg32(NvRegRxDeferral) = NVREG_RX_DEFERRAL_DEFAULT;

    // Configure descriptor ring buffers. The size-1 thing is a hw quirk.
    reg32(NvRegTxRingPhysAddr) = MmGetPhysicalAddress((void *)g_txDescriptors);
    reg32(NvRegRxRingPhysAddr) = MmGetPhysicalAddress((void *)g_rxDescriptors);
    reg32(NvRegRingSizes) = ((RX_RING_SIZE - 1) << NVREG_RINGSZ_RXSHIFT) | ((TX_RING_SIZE - 1) << NVREG_RINGSZ_TXSHIFT);

    // FIXME ?? (MS Dash does this and sets up both these registers with 0x300010)
    reg32(NvRegUnknownSetupReg7) = NVREG_UNKSETUP7_VAL1;
    reg32(NvRegTxWatermark) = NVREG_UNKSETUP7_VAL1;

    // Phy init
    reg32(NvRegAdapterControl) = (1 << NVREG_ADAPTCTL_PHYSHIFT) | NVREG_ADAPTCTL_PHYVALID;
    reg32(NvRegMIISpeed) = NVREG_MIISPEED_BIT8 | NVREG_MIIDELAY;
    reg32(NvRegMIIMask) = NVREG_MII_LINKCHANGE;
    //

    if (PhyInitialize(FALSE, NULL) != STATUS_SUCCESS) {
        MmFreeContiguousMemory(descriptors);
        MmFreeContiguousMemory(g_rx_buffers);
        assert(0);
        return NVNET_PHY_ERR;
    }

    reg32(NvRegAdapterControl) |= NVREG_ADAPTCTL_RUNNING;

    KeInitializeEvent(&g_irq_event, SynchronizationEvent, FALSE);

    KeInitializeSemaphore(&g_freeTxDescriptors, TX_RING_SIZE, TX_RING_SIZE);

    // Create a minimal stack, no TLS thread to handle NIC events
    PsCreateSystemThreadEx(&g_irq_thread, 0, 4096, 0, NULL, NULL, NULL, FALSE, FALSE, nvnetdrv_irq_thread);

    KeInitializeDpc(&g_dpc_obj, nvnetdrv_dpc, NULL);

    g_irq = HalGetInterruptVector(4, &g_irql);

    KeInitializeInterrupt(&g_interrupt, &nvnetdrv_isr, NULL, g_irq, g_irql, LevelSensitive, TRUE);

    // Get speed settings from Phy
    nvnetdrv_handle_mii_irq(0, true);

     // FIXME: Should we reset here?

    // FIXME: Start recv/send here
    //nvnetdrv_start_txrx();

    // FIXME: Necessary? Maybe do it earlier?
    reg32(NvRegMIIStatus) = reg32(NvRegMIIStatus);
    reg32(NvRegIrqStatus) = reg32(NvRegIrqStatus);

    // NOTE: We haven't queued any buffers yet

    // This needs to happen before we connect the irq - otherwise an early irq could make the irq thread terminate
    bool prev_value = atomic_exchange(&g_running, true);
    assert(!prev_value);

    status = KeConnectInterrupt(&g_interrupt);
    if (!NT_SUCCESS(status)) {
        reg32(NvRegAdapterControl) = 0;

        // Set g_running to false, then wake the irq thread up to make it terminate itself
        atomic_exchange(&g_running, false);
        KeSetEvent(&g_irq_event, IO_NETWORK_INCREMENT, FALSE);
        NtWaitForSingleObject(g_irq_thread, FALSE, NULL);
        NtClose(g_irq_thread);

        MmFreeContiguousMemory(descriptors);
        MmFreeContiguousMemory(g_rx_buffers);
        assert(0);
        return NVNET_SYS_ERR;
    }

    nvnetdrv_irq_enable();
    nvnetdrv_start_txrx();

    return NVNET_OK;
}

void nvnetdrv_stop (void)
{
    assert(g_running);

    KeDisconnectInterrupt(&g_interrupt);

    // Disable DMA and wait for it to idle, re-checking every 50 microseconds
    reg32(NvRegTxRxControl) = NVREG_TXRXCTL_DISABLE;
    LARGE_INTEGER duration = {.QuadPart = -500};
    for (int i = 0; i < 10000; i++) {
        if (reg32(NvRegTxRxControl) & NVREG_TXRXCTL_IDLE) {
            break;
        }
        KeDelayExecutionThread(KernelMode, FALSE, &duration);
    }

    nvnetdrv_stop_txrx();

    bool prev_value = atomic_exchange(&g_running, false);
    assert(prev_value);

    // Stop the event-handling thread
    KeSetEvent(&g_irq_event, IO_NETWORK_INCREMENT, FALSE);
    NtWaitForSingleObject(g_irq_thread, FALSE, NULL);
    NtClose(g_irq_thread);

    // FIXME: There's more to clean up here, like TX buffers we still control etc.

    // Reset TX & RX control
    reg32(NvRegTxRxControl) = NVREG_TXRXCTL_DISABLE | NVREG_TXRXCTL_RESET;
    duration.QuadPart = -40;
    KeDelayExecutionThread(KernelMode, FALSE, &duration);
    reg32(NvRegTxRxControl) = NVREG_TXRXCTL_DISABLE;

    MmFreeContiguousMemory((void *)g_rxDescriptors);
    MmFreeContiguousMemory((void *)g_rx_buffers);
}

void nvnetdrv_start_txrx (void)
{
    reg32(NvRegLinkSpeed) = g_linkSpeed | NVREG_LINKSPEED_FORCE;

    reg32(NvRegTransmitterControl) |= NVREG_XMITCTL_START;
    reg32(NvRegReceiverControl) |= NVREG_RCVCTL_START;
    reg32(NvRegTxRxControl) = NVREG_TXRXCTL_KICK | NVREG_TXRXCTL_GET;
}

void nvnetdrv_stop_txrx (void)
{
    reg32(NvRegReceiverControl) &= ~NVREG_RCVCTL_START;
    reg32(NvRegTransmitterControl) &= ~NVREG_XMITCTL_START;

    LARGE_INTEGER duration = {.QuadPart = -100};
    for (int i = 0; i < 50000; i++) {
        if (!((reg32(NvRegReceiverStatus) & NVREG_RCVSTAT_BUSY) ||
              (reg32(NvRegTransmitterStatus) & NVREG_XMITSTAT_BUSY))) {
            break;
        }
        KeDelayExecutionThread(KernelMode, FALSE, &duration);
    }

    reg32(NvRegLinkSpeed) = 0;
    reg32(NvRegTransmitPoll) = 0;
}

int nvnetdrv_acquire_tx_descriptors (size_t count)
{
    NTSTATUS status;
    LARGE_INTEGER timeout = {.QuadPart = 0};

    // Sanity check
    assert(count > 0);
    // Avoid excessive requests
    assert(count <= 4);

    while (true) {
        // Wait for TX descriptors to become available
        // FIXME: Really no timeout? What about shutdown?
        status = KeWaitForSingleObject(&g_freeTxDescriptors, Executive, KernelMode, FALSE, NULL);
        if (!NT_SUCCESS(status)) {
            return false;
        }

        for (size_t i = 0; i < count - 1; i++) {
            // Try to acquire remaining descriptors without sleeping
            status = KeWaitForSingleObject(&g_freeTxDescriptors, Executive, KernelMode, FALSE, &timeout);
            if (!NT_SUCCESS(status) || status == STATUS_TIMEOUT) {
                // Couldn't acquire all at once, back off
                KeReleaseSemaphore(&g_freeTxDescriptors, IO_NETWORK_INCREMENT, i + 1, NULL);
                if (status == STATUS_TIMEOUT) {
                    // Sleep for 10 microseconds to avoid immediate re-locking
                    LARGE_INTEGER duration;
                    duration.QuadPart = -100;
                    KeDelayExecutionThread(UserMode, FALSE, &duration);
                    // Retry
                    break;
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    return true;
}

void nvnetdrv_submit_tx_descriptors (nvnetdrv_descriptor_t *buffers, size_t count)
{
    // Sanity check
    assert(count > 0);
    // Avoid excessive requests
    assert(count <= 4);

    // We don't check for buffer overrun here, because the Semaphore already protects us
    size_t descriptors_index = g_txEndIndex;
    while (!atomic_compare_exchange_weak(&g_txEndIndex, &descriptors_index, (descriptors_index + count) % TX_RING_SIZE));

    for (size_t i = 0; i < count; i++) {
        size_t current_descriptor_index = (descriptors_index + i) % TX_RING_SIZE;

        tx_misc[current_descriptor_index].bufAddr = buffers[i].addr;
        tx_misc[current_descriptor_index].length = buffers[i].length;
        tx_misc[current_descriptor_index].userdata = buffers[i].userdata;
        tx_misc[current_descriptor_index].callback = buffers[i].callback;

        // Buffers get locked before sending and unlocked after sending
        MmLockUnlockBufferPages(buffers[i].addr, buffers[i].length, FALSE);
        g_txDescriptors[current_descriptor_index].paddr = MmGetPhysicalAddress(buffers[i].addr); //FIXME? Is this contiguous?
        g_txDescriptors[current_descriptor_index].length = buffers[i].length - 1;
        g_txDescriptors[current_descriptor_index].flags = (i != 0 ? NV_TX_VALID : 0);
    }

    // Terminate descriptor chain
    g_txDescriptors[(descriptors_index + count - 1) % TX_RING_SIZE].flags |= NV_TX_LASTPACKET;

    // Enable first descriptor last to keep the NIC from sending incomplete packets
    g_txDescriptors[descriptors_index].flags |= NV_TX_VALID;

    // Keep track of how many descriptors are in use
    g_txDescriptorsInUseCount += count;

    reg32(NvRegTxRxControl) = NVREG_TXRXCTL_KICK;
}

void nvnetdrv_rx_release(size_t buffer_index)
{
    assert(buffer_index < RX_RING_SIZE);
    g_rxDescriptors[buffer_index].flags = NV_RX_AVAIL;
    g_rxDescriptors[buffer_index].length = 2048;
}
