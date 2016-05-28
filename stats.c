
/*
 *The MIT License (MIT)
 *Copyright (c) 2016 Vipin Varghese
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
 *files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
 *modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
 *Software is furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
 *WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
 *COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "stats.h"

/* GLOBAL */
pkt_stats_t prtPktStats [MAX_INTFCOUNT];

static struct rte_timer fetchStats;
static struct rte_timer displayStats;

uint8_t doStatsDisplay = 1;

/* EXTERN */
extern numa_Info_t numaNodeInfo[MAX_NUMANODE];
extern uint8_t fifoWrk;


void sigExtraStats(__attribute__((unused)) int signo)
{
    int32_t i = 0, ports = rte_eth_dev_count();

    doStatsDisplay = 0;

    /* clear screen */
    STATS_CLR_SCREEN;

    printf(YELLOW "\033[2;1H INTF " RESET);
    printf("\033[3;1H");
    printf(BLUE "*************************************************" RESET);
    printf("\033[10;5H");
    printf(YELLOW " --- TX PKT BUFF DETAILS ---" RESET);
    printf("\033[11;1H");
    printf(" +  Type:");
    printf("\033[12;1H");
    printf(" +   Ver:");
    printf("\033[13;1H");
    printf(" + Index:");
    printf("\033[18;1H");
    printf(YELLOW " NUMA " RESET );
    printf("\033[19;1H");
    printf(BLUE "*************************************************" RESET);
    printf("\033[20;1H");
    printf(" + LCORE in Use: ");
    printf("\033[21;1H");
    printf(" + INTF in Use: ");
    printf("\033[30;1H");
    printf(BLUE "*************************************************" RESET);

    for (; i < ports; i ++)
    {
        printf("\033[2;%dH", (15 + 10 * i));
        printf(" %8u ", i);
    }

    for (i=0; i < MAX_NUMANODE; i++)
    {
        printf("\033[18;%dH", (25 + 10 * i));
        printf(" %8u ", i);
        printf("\033[20;%dH", (25 + 10 * i));
        printf(" %8u ", numaNodeInfo[i].lcoreUsed);
        printf("\033[21;%dH", (25 + 10 * i));
        printf(" %8u ", numaNodeInfo[i].intfUsed);
    }

    fflush(stdout);
    rte_delay_ms(10000);

    show_static_display();

    doStatsDisplay = 1;
    return;
}

void sigDetails(__attribute__((unused)) int signo)
{
    struct rte_eth_stats stats;
    struct rte_eth_dev_info devInfo;
    struct rte_eth_link link;

    int32_t i = 0, ports = rte_eth_dev_count();
    doStatsDisplay = 0;

    /* clear screen */
    STATS_CLR_SCREEN;

    for (i = 0; i < ports; i++) {
    /* port Info */
    rte_eth_dev_info_get(i, &devInfo);

    printf("\033[1;10H Intf: %d port: %d driver %s", i, devInfo.if_index, devInfo.driver_name);
    printf("\033[2;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
    if (devInfo.pci_dev) {
        printf("\033[3;4H PCI INFO");
        printf("\033[4;6H ADDR: domain:bus:devid.function %04x:%02x:%02x.%x",
              devInfo.pci_dev->addr.domain,
              devInfo.pci_dev->addr.bus,
              devInfo.pci_dev->addr.devid,
              devInfo.pci_dev->addr.function);
        printf("\033[5;6H PCI ID: vendor:device:sub-vendor:sub-device %04x:%04x:%04x:%04x",
              devInfo.pci_dev->id.vendor_id,
              devInfo.pci_dev->id.device_id,
              devInfo.pci_dev->id.subsystem_vendor_id,
              devInfo.pci_dev->id.subsystem_device_id);
        printf("\033[6;6H numa node: %d", devInfo.pci_dev->numa_node);
        printf("\033[7;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
    }

    
    rte_eth_link_get(i, &link);

    printf("\033[10;1H  Speed: %d",link.link_speed);
    printf("\033[11;1H Duplex: %s",(link.link_duplex == ETH_LINK_FULL_DUPLEX)?"Full":"Half");
    printf("\033[12;1H Status: %s",(link.link_status == 1)?"up":"down");
    printf("\033[13;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");


    if (likely(rte_eth_stats_get(i, &stats) == 0)) {
        printf("\033[15;1H Packets ");
        printf("\033[16;1H  -  Input: %" PRIu64, stats.ipackets);
        printf("\033[17;1H  - Output: %" PRIu64, stats.opackets);
        printf("\033[18;1H Bytes ");
        printf("\033[19;1H  -  Input: %" PRIu64 "MB", stats.ibytes/(1024 * 1024));
        printf("\033[20;1H  - Output: %" PRIu64 "MB", stats.obytes/(1024 * 1024));
        printf("\033[21;1H Errors: ");
        printf("\033[22;1H  -  Input: %" PRIu64, stats.ierrors);
        printf("\033[23;1H  - Output: %" PRIu64, stats.oerrors);
        printf("\033[24;1H Input Missed: %" PRIu64, stats.imissed);
        printf("\033[25;1H RX no Mbuffs: %" PRIu64, stats.rx_nombuf);
        printf("\033[26;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
    }
    }

    fflush(stdout);
    rte_delay_ms(10000);

    show_static_display();

    doStatsDisplay = 1;
    return;
}

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg)
{
    int32_t i, ports =  rte_eth_dev_count();
    static uint64_t rx_currStat[MAX_INTFCOUNT] = {0};
    static uint64_t tx_currStat[MAX_INTFCOUNT] = {0};
    static uint64_t rx_prevStat[MAX_INTFCOUNT] = {0};
    static uint64_t tx_prevStat[MAX_INTFCOUNT] = {0};

    /* get link status for DPDK ports */
    struct rte_eth_stats stats;

    for (i = 0; i < ports; i++)
    {
        /* ToDo: use numa info to identify the ports */
        if (likely(rte_eth_stats_get(i, &stats) == 0)) {
            rx_currStat[i] =  stats.ipackets;
            tx_currStat[i] =  stats.opackets;

            prtPktStats[i].rxPkts = (rx_currStat[i] - rx_prevStat[i]);
            prtPktStats[i].txPkts = (tx_currStat[i] - tx_prevStat[i]);

            rx_prevStat[i] =  stats.ipackets;
            tx_prevStat[i] =  stats.opackets;

            prtPktStats[i].rxBytes   = stats.ibytes/(1024 * 1024); 
            prtPktStats[i].txBytes   = stats.obytes/(1024 * 1024); 
            prtPktStats[i].rxMissed  = stats.imissed; 
            prtPktStats[i].rxErr     = stats.ierrors; 
            prtPktStats[i].txErr     = stats.oerrors; 
            prtPktStats[i].rxNoMbuff = stats.rx_nombuf; 
        }
    }

    return;
}

void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg)
{
    int32_t i, j, ports =  rte_eth_dev_count();

    if (likely(doStatsDisplay)) {
      for (i = 0; i < ports; i++)
      {
        /*NUMA_SOCKET*/
        printf("\033[4;%dH", (15 + 10 * i));
        //printf("%-8d |", );

        /*PKTS_PER_SEC_RX*/
        printf("\033[5;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxPkts);

        /*PKTS_PER_SEC_TX*/
        printf("\033[6;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txPkts);

        /*MB_RX*/
        printf("\033[7;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxBytes);

        /*MB_TX*/
        printf("\033[8;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txBytes);

        /* INTF STATS */

        /* RX miss */
        printf("\033[10;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxMissed);

        /* RX err */
        printf("\033[11;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxErr);

        /* TX err */
        printf("\033[12;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txErr);

        /* RX no mbuf */
        printf("\033[13;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxNoMbuff);

        /*RX_IPV4*/
        printf("\033[15;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_ipv4);

        /*RX_IPV6*/
        printf("\033[16;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_ipv6);

        /*ERR NON IP*/
        printf("\033[18;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].non_ip);

        /*ERR IP FRAG*/
        printf("\033[19;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].ipFrag);

        /*ERR IP CSUM*/
        printf("\033[20;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].ipCsumErr);

        /*TX Queue ERR*/
        printf("\033[21;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txQueueErr);

        /*ERR Dropped*/
        printf("\033[22;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].dropped);

      for (j = 0; j < fifoWrk; j++) {
        /*QUEUE Add-Drp*/
        printf("\033[%d;%dH", (24 + j * 3), (15 + 10 * i));
        printf(" %-12lu ", prtPktStats[i].queue_add[j]);
        printf("\033[%d;%dH", (25 + j * 3), (15 + 10 * i));
        printf(" %-12lu ", prtPktStats[i].queue_fet[j]);
        printf("\033[%d;%dH", (26 + j * 3), (15 + 10 * i));
        printf(" %-12lu ", prtPktStats[i].queue_drp[j]);
      }
      }
    }

    fflush(stdout);
    return;
}

void show_static_display(void)
{
    struct rte_eth_link link;
    int32_t i, ports =  rte_eth_dev_count();

    /* clear screen */
    STATS_CLR_SCREEN;

    /* stats header */
    printf("\033[2;1H");
    printf(" %-10s | ", "Cat|Intf");
    printf("\033[3;1H");
    printf(BLUE "======================================================" RESET);

    /*NUMA_SOCKET*/
    /*LINK_SPEED_STATE*/
    printf("\033[4;1H");
    printf(BLUE " %-10s | ", "Speed-Dup");

    /*PKTS_PER_SEC_RX*/
    printf("\033[5;1H");
    printf(BLUE " %-10s | ", "RX pkts/s");

    /*PKTS_PER_SEC_TX*/
    printf("\033[6;1H");
    printf(BLUE " %-10s | ", "TX pkts/s");

    /*MB_RX*/
    printf("\033[7;1H");
    printf(BLUE " %-10s | ", "RX MB");

    /*MB_TX*/
    printf("\033[8;1H");
    printf(BLUE " %-10s | " RESET, "TX MB");

    /*PKT_INFO*/
    printf("\033[9;1H");
    printf(CYAN " %-25s " RESET, "--- INTF STATS ---");

    /* RX miss*/
    printf("\033[10;1H");
    printf(RED " %-10s | ", "RX Miss");

    /* RX Err */
    printf("\033[11;1H");
    printf(" %-10s | ", "RX Err");

    /* RX no Mbuf */
    printf("\033[12;1H");
    printf(" %-10s | ", "RX no MBUF");

    /* TX Err */
    printf("\033[13;1H");
    printf(" %-10s | " RESET, "TX ERR");

    printf("\033[14;1H");
    printf(CYAN " %-25s " RESET, "--- PKT STATS ---");

    /*RX_IPV4*/
    printf("\033[15;1H");
    printf(YELLOW " %-10s | ", "RX IPv4");

    /*RX_IPV6*/
    printf("\033[16;1H");
    printf(" %-10s | " RESET, "RX IPv6");

    printf("\033[17;1H");
    printf(CYAN " %-25s " RESET, "--- PKT ERR STATS ---");

    /*NON IPv4*/
    printf("\033[18;1H");
    printf(BOLDRED " %-10s | ", "NON IPv4");

    /*IP FRAG*/
    printf("\033[19;1H");
    printf(" %-10s | ", "IP FRAG");

    /*IP CHECKSUM*/
    printf("\033[20;1H");
    printf(" %-10s | ", "IP CSUM");

    /* TX Queue Err */
    printf("\033[21;1H");
    printf(" %-10s | ", "TX Queue");

    /*DROPPED*/
    printf("\033[22;1H");
    printf(" %-10s | " RESET, "DROPPED");

    printf("\033[23;1H");
    printf(CYAN " %-25s " RESET, "--- QUEUE STATS ---");

    /* Queue Add */
    for (i = 0; i < fifoWrk; i++) 
    {
      printf("\033[%d;1H", 24 + i * 3);
      printf(GREEN " %d-Add | ", i);
      printf("\033[%d;1H", 25 + i * 3);
      printf(GREEN " %d-Fet | ", i);
      printf("\033[%d;1H", 26 + i * 3);
      printf(GREEN " %d-Drp | ", i);
    }

    /* fetch port info and display */
    for (i =0 ; i < ports; i++)
    {
       rte_eth_link_get_nowait(i, &link);

        /* DPDK port id - up|down */
        printf("\033[2;%dH", (15 + 10 * i));
        if (link.link_status)
            printf("  %d-" GREEN "up" RESET, i);
        else
            printf("  %d-" RED "down" RESET, i);

        /*LINK_SPEED_STATE*/
        printf("\033[4;%dH", (15 + 10 * i));
        if (link.link_duplex == ETH_LINK_HALF_DUPLEX) {
            printf(" %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10M_HD)?10:
                (link.link_speed  == ETH_LINK_SPEED_100M_HD)?100:
                (link.link_speed  == ETH_LINK_SPEED_1G)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10G)?10000:
                link.link_speed),
                "HD");
        } else {
            printf(" %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10M)?10:
                (link.link_speed  == ETH_LINK_SPEED_100M)?100:
                (link.link_speed  == ETH_LINK_SPEED_1G)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10G)?10000:
                link.link_speed),
                "FD");
        }
    }

    fflush(stdout);
    return;
}

void set_stats_timer (void)
{
    int32_t lcoreId = rte_get_master_lcore();

    /* initialize the stats fetch and display timers */
    rte_timer_init(&fetchStats);
    rte_timer_init(&displayStats);

    /* periodic reload for every 1 sec for stats fetch */
    rte_timer_reset(&fetchStats, rte_get_timer_hz(), PERIODICAL, lcoreId, 
                    get_link_stats, NULL);
    /* periodic reload for every 2 sec for stats display */
    rte_timer_reset(&displayStats, rte_get_timer_hz(), PERIODICAL, lcoreId,
                    get_process_stats, NULL);

    return;
}

