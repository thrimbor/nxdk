#ifndef __PKTDRV_H__
#define __PKTDRV_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void *PktdrvMmioBase;
extern unsigned int PktdrvInterrupt;

typedef void (*tx_callback_t) (void *userdata);

typedef struct _Pktdrv_Descriptor_t
{
    void *addr;
    size_t length;
    tx_callback_t callback;
    void *userdata;
} Pktdrv_Descriptor_t;

int Pktdrv_Init(void);
void Pktdrv_Quit(void);
int Pktdrv_ReceivePackets(void);
void Pktdrv_GetEthernetAddr(unsigned char *address);
int Pktdrv_GetQueuedTxPkts(void);

int Pktdrv_AcquireTxDescriptors(unsigned int count);
void Pktdrv_SubmitTxDescriptors(Pktdrv_Descriptor_t *buffers, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif
