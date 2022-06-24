/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*---------------------------------------------------------------------*
 * Includes
 *---------------------------------------------------------------------*/
#include "board.h"
#include "hpm_enet_drv.h"
#include "hpm_enet_soc_drv.h"
#include "hpm_mchtmr_drv.h"
#include "hpm_gpio_drv.h"
#include "hpm_clock_drv.h"
#include "netconf.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include "lwip.h"
#include "ptpd.h"

#define BOARD_TIMER_TICK_PERIOD_MS  (100UL)

#if RGMII
    #include "hpm_dp83867.h"
    #include "hpm_dp83867_regs.h"
#else
    #include "hpm_dp83848.h"
    #include "hpm_dp83848_regs.h"
#endif

ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(ENET_SOC_DESC_ADDR_ALIGNMENT)
__RW enet_rx_desc_t dma_rx_desc_tab[ENET_RX_BUFF_COUNT] ; /* Ethernet Rx DMA Descriptor */

ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(ENET_SOC_DESC_ADDR_ALIGNMENT)
__RW enet_tx_desc_t dma_tx_desc_tab[ENET_TX_BUFF_COUNT] ; /* Ethernet Tx DMA Descriptor */

ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(ENET_SOC_BUFF_ADDR_ALIGNMENT)
__RW uint8_t rx_buff[ENET_RX_BUFF_COUNT][ENET_RX_BUFF_SIZE]; /* Ethernet Receive Buffer */

ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(ENET_SOC_BUFF_ADDR_ALIGNMENT)
__RW uint8_t tx_buff[ENET_TX_BUFF_COUNT][ENET_TX_BUFF_SIZE]; /* Ethernet Transmit Buffer */

enet_desc_t desc;
uint32_t localtime;
/*---------------------------------------------------------------------*
 * Initialization
 *---------------------------------------------------------------------*/
static void enet_ptp_init(void)
{
    enet_ptp_config_t config;
    enet_ptp_time_t timestamp;

    /* initial the ptp function */
    config.ssinc = ENET_ONE_SEC_IN_NANOSEC / clock_get_frequency(ENET_PTP_CLK);
    config.sub_sec_count_res = enet_ptp_count_res_low;
    config.update_method = enet_ptp_time_fine_update;
    config.addend = 0xffffffff;

    enet_init_ptp(ENET, &config);

    /* set the initial timestamp */
    timestamp.sec = 1651074120UL;
    timestamp.nsec = 0;
    enet_set_ptp_timestamp(ENET, &timestamp);
}

static hpm_stat_t enet_init(ENET_Type *ptr)
{
    uint32_t intr = 0;
    enet_mac_config_t enet_config;
    #if RGMII == 1
        dp83867_config_t phy_config;
    #else
        dp83848_config_t phy_config;
    #endif

    /* Initialize td, rd and the corresponding buffers */
    memset((uint8_t *)dma_tx_desc_tab, 0x00, sizeof(dma_tx_desc_tab));
    memset((uint8_t *)dma_rx_desc_tab, 0x00, sizeof(dma_rx_desc_tab));
    memset((uint8_t *)rx_buff, 0x00, sizeof(rx_buff));
    memset((uint8_t *)tx_buff, 0x00, sizeof(tx_buff));

    desc.tx_desc_list_head = (enet_tx_desc_t *)core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)dma_tx_desc_tab);
    desc.rx_desc_list_head = (enet_rx_desc_t *)core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)dma_rx_desc_tab);

    desc.tx_buff_cfg.buffer = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)tx_buff);
    desc.tx_buff_cfg.count = ENET_TX_BUFF_COUNT;
    desc.tx_buff_cfg.size = ENET_TX_BUFF_SIZE;

    desc.rx_buff_cfg.buffer = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)rx_buff);
    desc.rx_buff_cfg.count = ENET_RX_BUFF_COUNT;
    desc.rx_buff_cfg.size = ENET_RX_BUFF_SIZE;

    /* Set mac0 address */
    enet_config.mac_addr_high[0] = MAC_ADDR5 << 8 | MAC_ADDR4;
    enet_config.mac_addr_low[0]  = MAC_ADDR3 << 24 | MAC_ADDR2 << 16 | MAC_ADDR1 << 8 | MAC_ADDR0;
    enet_config.valid_max_count  = 1;

    /* Initialize enet controller */
    enet_controller_init(ptr, ENET_INF_TYPE, &desc, &enet_config, intr);

    #if RGMII
        enet_rgmii_enable_clock(ptr);
    #else
        board_init_enet_rmii_reference_clock(ptr, BOARD_ENET_RMII_INT_REF_CLK);
        enet_rmii_enable_clock(ptr, BOARD_ENET_RMII_INT_REF_CLK);
    #endif

    /* Set RGMII clock delay */
    #if RGMII
        enet_rgmii_set_clock_delay(ptr, BOARD_ENET_RGMII_TX_DLY, BOARD_ENET_RGMII_RX_DLY);
    #endif

    /* Initialize phy */
    #if RGMII
        dp83867_reset(ptr);
        dp83867_basic_mode_default_config(ptr, &phy_config);
        if (dp83867_basic_mode_init(ptr, &phy_config) == true) {
    #else
        dp83848_reset(ptr);
        dp83848_basic_mode_default_config(ptr, &phy_config);
        if (dp83848_basic_mode_init(ptr, &phy_config) == true) {
    #endif
            printf("Enet phy init passes !\n");
            return status_success;
        } else {
            printf("Enet phy init fails !\n");
            return status_fail;
        }
}

static void time_update(void)
{
    localtime += BOARD_TIMER_TICK_PERIOD_MS;
}
/*---------------------------------------------------------------------*
 * Main
 *---------------------------------------------------------------------*/
int main(void)
{
    /* Initialize BSP */
    board_init();

    /* Initialize PTP clock */
    board_init_enet_ptp_clock(ENET);

    /* Initialize GPIOs */
    board_init_enet_pins(ENET);

    /* Initialize a board timer */
    board_timer_create(BOARD_TIMER_TICK_PERIOD_MS, time_update);

    printf("This is an ethernet demo: PTP Master\n");
    printf("LwIP Version: %s\n", LWIP_VERSION_STRING);
    printf("Local IP: %d.%d.%d.%d\n", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    printf("Speed Rate:%s\n", RGMII == 1 ? "1000Mbps" : "100Mbps");

    #if RGMII == 0
    printf("Reference Clock: %s\n", BOARD_ENET_RMII_INT_REF_CLK ? "Internal Clock" : "External Clock");
    #endif

    /* Initialize MAC and DMA */
    if (enet_init(ENET) == 0) {
        /* Initialize PTP */
        enet_ptp_init();

        /* Initialize the Lwip stack */
        lwip_init();
        netif_config();
        user_notification(&gnetif);

        /* Initialize ptpd */
        ptpd_Init();

        while (1) {
            ethernetif_input(&gnetif);
            ptpd_periodic_handle(localtime);
        }
    } else {
        printf("Enet initialization fails !!!\n");
        while (1) {

        }
    }

    return 0;
}
