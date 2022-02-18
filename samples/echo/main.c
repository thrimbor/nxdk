#include <assert.h>
#include <stdbool.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>

#include <lwip/dhcp.h>
#include <lwip/dhcp6.h>
#include <lwip/init.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <lwip/timeouts.h>
#include <netif/etharp.h>
#include <pktdrv.h>

void testWithSock () {
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char buf[2048];
    size_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(buf, '0', sizeof(buf));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    while(1)
    {
      int n, bytes_recvd = 0;
      connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
      printf("New connection\n");
      while ((n = read(connfd, buf, sizeof(buf)-1)) > 0) {
        //DbgPrint("a\n");
        bytes_recvd += n;
        bool corrupt = false;
        for (int i=0; i<n; i++) {
          if (buf[i] != 1) {
            DbgPrint("corruption found at %d: %x\n", i, buf[i]);
            corrupt = true;
          }
        }
        if (corrupt) DbgPrint("check done\n");
        //n = send(connfd, buf, n, 0);
        //if (n <= 0) break;
      }
      printf("Recv'd %d MiB\n", bytes_recvd/(1024*1024));
      close(connfd);
      Sleep(1);
      break;
    }
    Pktdrv_Quit();
    debugPrint("quit\n");
}

#define PKT_TMR_INTERVAL 1 /* ms */

  struct netif nforce_netif, *g_pnetif;
  err_t nforceif_init(struct netif *netif);

//extern KAPC apc;
//VOID __stdcall apcRoutine(PKAPC Apc, PKNORMAL_ROUTINE *NormalRoutine, PVOID *NormalContext, PVOID *SystemArgument1, PVOID *SystemArgument2);

static void tcpip_init_done(void *arg)
{
  sys_sem_t *init_complete = (sys_sem_t*)(arg);
  sys_sem_signal(init_complete);
}

static void packet_timer(void *arg)
{
  LWIP_UNUSED_ARG(arg);
  Pktdrv_ReceivePackets();
  sys_timeout(PKT_TMR_INTERVAL, packet_timer, NULL);
}

void init_net () {
  //KeInitializeApc(&apc, KeGetCurrentThread(), apcRoutine, NULL, NULL, UserMode, NULL);
    static ip4_addr_t ipaddr, netmask, gw;
  sys_sem_t init_complete;
  IP4_ADDR(&gw, 0,0,0,0);
    IP4_ADDR(&ipaddr, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);
    /* Initialize the TCP/IP stack. Wait for completion. */
  sys_sem_new(&init_complete, 0);
  tcpip_init(tcpip_init_done, &init_complete);
  sys_sem_wait(&init_complete);
  sys_sem_free(&init_complete);

  g_pnetif = netif_add(&nforce_netif, &ipaddr, &netmask, &gw,
                       NULL, nforceif_init, tcpip_input);
  if (!g_pnetif) {
      debugPrint("netif_add failed\n");
    return;
  }
  netif_set_default(g_pnetif);
  netif_set_up(g_pnetif);

    //dhcp6_enable_stateless(g_pnetif);
    dhcp_start(g_pnetif);

  //packet_timer(NULL);
//debugPrint("Waiting for DHCP...\n");
    while (dhcp_supplied_address(g_pnetif) == 0) {
      NtYieldExecution();
    }
    debugPrint("DHCP bound!\n");
    debugPrint("\n");
    debugPrint("IP address.. %s\n", ip4addr_ntoa(netif_ip4_addr(g_pnetif)));
    debugPrint("Mask........ %s\n", ip4addr_ntoa(netif_ip4_netmask(g_pnetif)));
    debugPrint("Gateway..... %s\n", ip4addr_ntoa(netif_ip4_gw(g_pnetif)));
    debugPrint("\n");
  return;

}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    init_net();
    testWithSock();


    while(1) {
        //debugPrint("Hello NXDK!\n");
        Sleep(2000);
    }

    return 0;
}
