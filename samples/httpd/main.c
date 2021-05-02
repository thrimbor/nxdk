#include "lwip/debug.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "pktdrv.h"
#include <hal/input.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <xboxkrnl/xboxkrnl.h>
#include <hal/debug.h>

#define USE_DHCP         1
#define PKT_TMR_INTERVAL 5 /* ms */
#define DEBUGGING        0

#include "httpserver.h"
#include "lwip/apps/http_client.h"

struct netif nforce_netif, *g_pnetif;

err_t nforceif_init(struct netif *netif);
static void packet_timer(void *arg);

static void tcpip_init_done(void *arg)
{
	sys_sem_t *init_complete = arg;
	sys_sem_signal(init_complete);
}

static void packet_timer(void *arg)
{
  LWIP_UNUSED_ARG(arg);
  Pktdrv_ReceivePackets();
  sys_timeout(PKT_TMR_INTERVAL, packet_timer, NULL);
}

void result_fn (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    debugPrint("download done, err = %d, rx_content_len = %d, httpc_result=%d\n", err, rx_content_len, httpc_result);
}

err_t headers_done_fn (httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    debugPrint("headers received, content length: %d, hdr_len: %d\n", content_len, hdr_len);
    return ERR_OK;
}

int main(void)
{
	sys_sem_t init_complete;
	const ip4_addr_t *ip;
	static ip4_addr_t ipaddr, netmask, gw;

#if DEBUGGING
	asm volatile ("jmp .");
	debug_flags = LWIP_DBG_ON;
#else
	debug_flags = 0;
#endif

#if USE_DHCP
	IP4_ADDR(&gw, 0,0,0,0);
	IP4_ADDR(&ipaddr, 0,0,0,0);
	IP4_ADDR(&netmask, 0,0,0,0);
#else
	IP4_ADDR(&gw, 10,0,1,1);
	IP4_ADDR(&ipaddr, 10,0,1,7);
	IP4_ADDR(&netmask, 255,255,255,0);
#endif

	/* Initialize the TCP/IP stack. Wait for completion. */
	sys_sem_new(&init_complete, 0);
	tcpip_init(tcpip_init_done, &init_complete);
	sys_sem_wait(&init_complete);
	sys_sem_free(&init_complete);

	XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

	g_pnetif = netif_add(&nforce_netif, &ipaddr, &netmask, &gw,
	                     NULL, nforceif_init, ethernet_input);
	if (!g_pnetif) {
		debugPrint("netif_add failed\n");
		return 1;
	}

	netif_set_default(g_pnetif);
	netif_set_up(g_pnetif);

#if USE_DHCP
	dhcp_start(g_pnetif);
#endif

	packet_timer(NULL);

#if USE_DHCP
	debugPrint("Waiting for DHCP...\n");
	while (dhcp_supplied_address(g_pnetif) == 0)
		NtYieldExecution();
	debugPrint("DHCP bound!\n");
#endif

	debugPrint("\n");
	debugPrint("IP address.. %s\n", ip4addr_ntoa(netif_ip4_addr(g_pnetif)));
	debugPrint("Mask........ %s\n", ip4addr_ntoa(netif_ip4_netmask(g_pnetif)));
	debugPrint("Gateway..... %s\n", ip4addr_ntoa(netif_ip4_gw(g_pnetif)));
	debugPrint("\n");

    nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\");

	//http_server_netconn_init();
    httpc_state_t *connection;
    httpc_connection_t settings;
    RtlZeroMemory(&settings, sizeof(settings));
    settings.result_fn = result_fn;
    settings.headers_done_fn = headers_done_fn;
    ip_addr_t ip_addr;
    IP_ADDR4(&ip_addr, 192, 168, 1, 166);
    err_t err = httpc_get_file_to_disk(&ip_addr, 8000, "/testfile.bin", &settings, NULL, "E:\\testfile", &connection);

    debugPrint("err: %d\n", err);

	while (1) NtYieldExecution();
	Pktdrv_Quit();
	return 0;
}
