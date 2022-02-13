#ifndef __NVNETDRV_H__
#define __NVNETDRV_H__

#include <stddef.h>
#include <stdint.h>

#ifndef RX_RING_SIZE
#define RX_RING_SIZE 64
#endif
#ifndef TX_RING_SIZE
#define TX_RING_SIZE 64
#endif

typedef void (*nvnetdrv_rx_callback_t)(size_t rx_index, void *buffer, uint16_t length);
typedef void (*tx_callback_t) (void *userdata);

typedef struct _nvnetdrv_descriptor_t
{
    void *addr;
    size_t length;
    tx_callback_t callback;
    void *userdata;
} nvnetdrv_descriptor_t;

// FIXME: Should this be here?
void nvnetdrv_start_txrx (void);
void nvnetdrv_stop_txrx (void);

int nvnetdrv_init (size_t rx_buffer_count, nvnetdrv_rx_callback_t rx_callback);

void nvnetdrv_stop (void);

const uint8_t *nvnetdrv_get_ethernet_addr();

/**
 * Reserves 1-4 descriptors. If the requested number is not immediately available,
 * this function will block until the request can be satisfied.
 * This function is thread-safe.
 * @param count The number of descriptors to reserve
 * @return Zero if the reservation failed, non-zero if it succeeded.
 */
int nvnetdrv_acquire_tx_descriptors (size_t count);

/**
 * Queues a packet, which consists of 1-4 buffers, for sending. The descriptors for this
 * need to be allocated beforehand.
 * This function is thread-safe.
 * @param buffers Pointer to an array of buffers which will be queued for sending as a packet
 * @param count The number of buffers to queue
 */
void nvnetdrv_submit_tx_descriptors (nvnetdrv_descriptor_t *buffers, size_t count);

/**
 * Releases an RX buffer given out by nvnetdrv. All RX buffers need to be
 * released eventually, or the NIC will run out of buffers to use.
 */
void nvnetdrv_rx_release (size_t buffer_index);

//void nvnetdrv_input (void *buffer, uint16_t length);

#endif
