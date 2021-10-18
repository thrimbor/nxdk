/*
 * Copyright (c) 2021 Stefan Schmidt
 *
 * Licensed under the MIT License
 */


#include <nxdk/net.h>

#include <lwip/dhcp.h>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <netif/etharp.h>
#include <pktdrv.h>

#define PKT_TMR_INTERVAL 5

struct netif nforce_netif, *g_pnetif;
err_t nforceif_init(struct netif *netif);

static void tcpip_init_done (void *arg)
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

#define USE_DHCP 1

int nxNetStartup (const nxNetStartupParams *startupParams)
{
    // TODO: if no startupParams, or no flags, read network config from disk!

    sys_sem_t init_complete;
    const ip4_addr_t *ip;
    static ip4_addr_t ipaddr, netmask, gw;

    IP4_ADDR(&gw, 0,0,0,0);
    IP4_ADDR(&ipaddr, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);

    sys_sem_new(&init_complete, 0);
    tcpip_init(tcpip_init_done, &init_complete);
    sys_sem_wait(&init_complete);
    sys_sem_free(&init_complete);

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

    return 0;
}
