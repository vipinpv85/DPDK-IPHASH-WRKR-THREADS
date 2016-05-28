#ifndef __STATS_H_
#define __STATS_H_

#include <inttypes.h>
#include <signal.h>

#include "config.h"
#include "node.h"

#include <rte_timer.h>
#include <rte_cycles.h>

#define MAX_FIFO 32

#define STATS_CLR_SCREEN   printf("\033[2J")
#define STATS_ROW(x)       "\033[x;1H"
#define STATS_ROW_COL(x,y) "\033[x;yH"
#define STATS_POS_OFFSET   2

#define RESET       "\033[0m"
#define BLACK       "\033[30m"        /* Black */
#define RED         "\033[31m"        /* Red */
#define GREEN       "\033[32m"        /* Green */
#define YELLOW      "\033[33m"        /* Yellow */
#define BLUE        "\033[34m"        /* Blue */
#define MAGENTA     "\033[35m"        /* Magenta */
#define CYAN        "\033[36m"        /* Cyan */
#define WHITE       "\033[37m"        /* White */
#define BOLDBLACK   "\033[1m\033[30m" /* Bold Black */
#define BOLDRED     "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m" /* Bold White */

typedef struct pkt_stats_s {
    uint64_t rx_ipv4;
    uint64_t rx_ipv6;

    uint64_t non_ip;
    uint64_t ipFrag;
    uint64_t ipCsumErr;

    uint64_t dropped;
    uint64_t txQueueErr; 

    uint64_t queue_add[MAX_FIFO];
    uint64_t queue_fet[MAX_FIFO];
    uint64_t queue_drp[MAX_FIFO];

    uint64_t rxPkts;
    uint64_t txPkts;
    uint64_t rxBytes;
    uint64_t txBytes;
    uint64_t rxMissed;
    uint64_t rxErr;
    uint64_t txErr;
    uint64_t rxNoMbuff;
} pkt_stats_t; /* per interface */

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg);
void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg);

void set_stats_timer(void);
void show_static_display(void);
void sigExtraStats(int signo);
void sigDetails(int signo);

#endif /* __STATS_H__ */
