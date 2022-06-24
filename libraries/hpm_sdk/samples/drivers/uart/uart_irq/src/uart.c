/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include "hpm_clock_drv.h"
#include "hpm_uart_drv.h"

#ifndef CONFIG_UART_FIFO_MODE
#define CONFIG_UART_FIFO_MODE 1
#endif

#ifndef TEST_UART
#define TEST_UART BOARD_APP_UART_BASE
#define TEST_UART_IRQ BOARD_APP_UART_IRQ
#ifdef BOARD_APP_UART_SRC_FREQ
#define TEST_UART_SRC_FREQ BOARD_APP_UART_SRC_FREQ
#elif defined(BOARD_APP_UART_CLK_NAME)
#define TEST_UART_CLK_NAME BOARD_APP_UART_CLK_NAME
#else
#error unknow UART source frequency
#endif
#endif

volatile uint8_t out[16];
uint8_t rx_data_count;
uint8_t tx_data_count;

void uart_isr(void)
{
    uint8_t i;
    uint8_t c;
    if (uart_get_irq_id(TEST_UART) & uart_intr_id_rx_data_avail) {
        for (i = 0; i < rx_data_count; i++) {
            if (status_success != uart_receive_byte(TEST_UART, &c)) {
                while(1) {};
            }
            out[i] = c;
        }
        uart_disable_irq(TEST_UART, uart_intr_rx_data_avail_or_timeout);
        uart_enable_irq(TEST_UART, uart_intr_tx_slot_avail);
    } 
    if (uart_get_irq_id(TEST_UART) & uart_intr_id_tx_slot_avail) {
        for(i = 0; i < tx_data_count; i++) {
            if (status_success != uart_send_byte(TEST_UART, out[i])) {
                while(1) {};
            }
        }
        uart_flush(TEST_UART);
        uart_disable_irq(TEST_UART, uart_intr_tx_slot_avail);
        uart_enable_irq(TEST_UART, uart_intr_rx_data_avail_or_timeout);
    }
}

SDK_DECLARE_EXT_ISR_M(TEST_UART_IRQ, uart_isr)

int main(void)
{
    hpm_stat_t stat;
    uint8_t fifo_size;

    board_init();

    uart_config_t config = {0};
    uart_default_config(TEST_UART, &config);
#if CONFIG_UART_FIFO_MODE
    config.fifo_enable = true;
#else
    config.fifo_enable = false;
#endif
#ifdef TEST_UART_SRC_FREQ
    config.src_freq_in_hz = TEST_UART_SRC_FREQ;
#else
    config.src_freq_in_hz = clock_get_frequency(TEST_UART_CLK_NAME);
#endif

    printf("uart driver example\n");
    if (!config.fifo_enable) {
        printf("non-fifo mode\n");
        tx_data_count = 1;
        rx_data_count = 1;

    } else {
        fifo_size = uart_get_fifo_size(TEST_UART);
        tx_data_count = (fifo_size >> 2) * (4 - config.tx_fifo_level);
        rx_data_count = (fifo_size >> 2) * config.rx_fifo_level;
        if (tx_data_count == 0) {
            tx_data_count = 1;
        }
        if (rx_data_count == 0) {
            rx_data_count = 1;
        }

        printf("fifo mode\n");
        printf("tx fifo level: %d bytes\n", tx_data_count);
        printf("rx fifo level: %d bytes\n", rx_data_count);
    }

    board_init_uart(TEST_UART);
    stat = uart_init(TEST_UART, &config);
    if (stat != status_success) {
        /* uart failed to be initialized */
        printf("failed to initialize uart\n");
        while(1);
    }

    uart_enable_irq(TEST_UART, uart_intr_rx_data_avail_or_timeout);
    intc_m_enable_irq_with_priority(TEST_UART_IRQ, 1);

    while(1)
    {
        __asm("wfi;\n");
    }

    return 0;
}
