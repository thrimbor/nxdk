#ifndef __PKTDRV_H__
#define __PKTDRV_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void *PktdrvMmioBase;
extern unsigned int PktdrvInterrupt;

int Pktdrv_Init(void);
void Pktdrv_Quit(void);
int Pktdrv_ReceivePackets(void);
void Pktdrv_SendPacket(unsigned char *buffer, int length);
void Pktdrv_GetEthernetAddr(unsigned char *address);
int Pktdrv_GetQueuedTxPkts(void);

#ifdef __cplusplus
}
#endif

#endif
