/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <stdio.h>
#include <assert.h>
#include "board.h"
#include "hpm_sysctl_drv.h"
#include "hpm_can_drv.h"


typedef struct {
    CAN_Type *can_base;
    uint32_t clock_freq;
} can_info_t;

static bool can_buf_compare(const can_transmit_buf_t *tx_buf, const can_receive_buf_t *rx_buf);

static uint8_t can_get_data_bytes_from_dlc(uint32_t dlc);

bool can_loopback_test(CAN_Type *base);

void can_loopback_test_for_all_cans(void);
void board_can_loopback_test_in_interrupt_mode(void);
void board_can_echo_test_initiator(void);
void board_can_echo_test_responder(void);
void board_can_send_multiple_messages(void);

void show_received_can_message(const can_receive_buf_t *rx_msg);

void board_can_isr(void);

void handle_can_test(void);

void show_help(void);

static can_info_t s_can_info[] = {
        { .can_base = HPM_CAN0 },
        { .can_base = HPM_CAN1 },
#if defined(HPM_CAN2)
        { .can_base = HPM_CAN2 },
#endif
#if defined (HPM_CAN3)
        { .can_base = HPM_CAN3 },
#endif
};

volatile static bool has_new_rcv_msg;
volatile static can_receive_buf_t s_can_rx_buf;

SDK_DECLARE_EXT_ISR_M(BOARD_APP_CAN_IRQn, board_can_isr);

void board_can_isr(void)
{
    uint8_t flags = can_get_tx_rx_flags(BOARD_APP_CAN_BASE);
    if ((flags & CAN_EVENT_RECEIVE) != 0) {
        can_read_received_message(BOARD_APP_CAN_BASE, (can_receive_buf_t *)&s_can_rx_buf);
        has_new_rcv_msg = true;
    }
    can_clear_tx_rx_flags(BOARD_APP_CAN_BASE, flags);
}

void show_received_can_message(const can_receive_buf_t *rx_msg)
{
    uint32_t msg_len = can_get_data_bytes_from_dlc(rx_msg->dlc);
    printf("CAN message info:\nID=%08x\nContent=:\n", rx_msg->id);
    uint32_t remaining_size = msg_len;
    uint32_t print_size;

    for (uint32_t i = 0; i < msg_len; i += 16) {
        print_size = MIN(remaining_size, 16);
        for (uint32_t j = 0; j < print_size; j++) {
            printf("%02x ", rx_msg->data[i + j]);
        }
        printf("\n");
        remaining_size -= print_size;
    }
}


int main(void)
{
    board_init();

    /* Initialize CAN */
    for (uint32_t i=0; i < ARRAY_SIZE(s_can_info); i++) {
        can_info_t  *info = &s_can_info[i];
        board_init_can(info->can_base);
        info->clock_freq = board_init_can_clock(info->can_base);
    }

    handle_can_test();

    return 0;
}

static bool can_buf_compare(const can_transmit_buf_t *tx_buf, const can_receive_buf_t *rx_buf)
{
    bool result = false;

    do {
        HPM_BREAK_IF(tx_buf->id != rx_buf->id);
        HPM_BREAK_IF(tx_buf->dlc != rx_buf->dlc);
        HPM_BREAK_IF(tx_buf->extend_id != rx_buf->extend_id);
        HPM_BREAK_IF(tx_buf->remote_frame != rx_buf->remote_frame);
        bool data_matched = true;

        uint32_t data_bytes = can_get_data_bytes_from_dlc(rx_buf->dlc);
        for (uint32_t i = 0; i < data_bytes; i++) {
            if (tx_buf->data[i] != rx_buf->data[i]) {
                data_matched = false;
                break;
            }
        }

        result = data_matched;

    } while (false);

    return result;
}

static uint8_t can_get_data_bytes_from_dlc(uint32_t dlc)
{
    uint32_t data_bytes = 0;

    dlc &= 0xFU;
    if (dlc <= 8U) {
        data_bytes = dlc;
    }
    else {
        switch (dlc) {
        case can_payload_size_12:
            data_bytes = 12U;
            break;
        case can_payload_size_16:
            data_bytes = 16U;
            break;
        case can_payload_size_20:
            data_bytes = 20U;
            break;
        case can_payload_size_24:
            data_bytes = 24U;
            break;
        case can_payload_size_32:
            data_bytes = 32U;
            break;
        case can_payload_size_48:
            data_bytes = 48U;
            break;
        case can_payload_size_64:
            data_bytes = 64U;
            break;
        default:
            // Code should never touch here
            break;
        }
    }

    return data_bytes;
}

bool can_loopback_test(CAN_Type *base)
{
    uint32_t error_cnt = 0;
    bool result = false;
    can_transmit_buf_t tx_buf;
    can_receive_buf_t rx_buf;
    memset(&tx_buf, 0, sizeof(tx_buf));
    memset(&rx_buf, 0, sizeof(rx_buf));

    tx_buf.id = 0x7f;
    tx_buf.dlc = 8;

    for (uint32_t i = 0; i < 8; i++) {
        tx_buf.data[i] = (uint8_t) i | (i << 4);
    }

    can_send_high_priority_message_blocking(HPM_CAN0, &tx_buf);
    can_receive_message_blocking(HPM_CAN0, &rx_buf);
    result = can_buf_compare(&tx_buf, &rx_buf);
    if (!result) {
        error_cnt++;
    }
    printf("    CAN loopback test for standard frame %s\n", result ? "passed" : "failed");

    tx_buf.extend_id = 1U;
    tx_buf.id = 0x12345678U;

    can_send_high_priority_message_blocking(HPM_CAN0, &tx_buf);
    can_receive_message_blocking(HPM_CAN0, &rx_buf);

    result = can_buf_compare(&tx_buf, &rx_buf);
    if (!result) {
        error_cnt++;
    }
    printf("    CAN loopback test for extend frame %s\n", result ? "passed" : "failed");

    return (error_cnt < 1);
}

void can_loopback_test_for_all_cans(void)
{
    can_config_t can_config;
    can_get_default_config(&can_config);
    can_config.baudrate = 1000000; /* 1Mbps */
    can_config.loopback_mode = true;
    hpm_stat_t status;
    for (uint32_t i = 0; i < ARRAY_SIZE(s_can_info); i++) {
        status = can_init(s_can_info[i].can_base, &can_config, s_can_info[i].clock_freq);
        assert(status == status_success);
        (void)status; /* Suppress compiling warning in release build */
        bool result = can_loopback_test(s_can_info[i].can_base);
        printf("    CAN%d loopback test %s\n", i, result ? "PASSED" : "FAILED");
    }
}

void board_can_loopback_test_in_interrupt_mode(void)
{
    can_config_t can_config;
    can_get_default_config(&can_config);
    can_config.baudrate = 1000000; /* 1Mbps */
    can_config.loopback_mode = true;
    board_init_can(BOARD_APP_CAN_BASE);
    uint32_t can_src_clk_freq = board_init_can_clock(BOARD_APP_CAN_BASE);
    hpm_stat_t status = can_init(BOARD_APP_CAN_BASE, &can_config, can_src_clk_freq);
    if (status != status_success) {
        printf("CAN initialization failed, error code: %d\n", status);
        return;
    }

    can_enable_tx_rx_irq(BOARD_APP_CAN_BASE, CAN_EVENT_RECEIVE);
    intc_m_enable_irq_with_priority(BOARD_APP_CAN_IRQn, 1);

    can_transmit_buf_t tx_buf;
    memset(&tx_buf, 0, sizeof(tx_buf));
    tx_buf.dlc = 8;

    for (uint32_t i = 0; i < 8; i++) {
        tx_buf.data[i] = (uint8_t) i | (i << 4);
    }

    for (uint32_t i = 0; i < 2048; i++) {
        tx_buf.id = i;
        can_send_message_blocking(BOARD_APP_CAN_BASE, &tx_buf);

        while (!has_new_rcv_msg) {

        }
        has_new_rcv_msg = false;
        printf("New message received, ID=%08x\n", s_can_rx_buf.id);
    }
}

void board_can_echo_test_initiator(void)
{
    can_config_t can_config;
    can_get_default_config(&can_config);
    can_config.baudrate = 500000; /* 500kbps */
    can_config.loopback_mode = false;
    board_init_can(BOARD_APP_CAN_BASE);
    uint32_t can_src_clk_freq = board_init_can_clock(BOARD_APP_CAN_BASE);
    hpm_stat_t status = can_init(BOARD_APP_CAN_BASE, &can_config, can_src_clk_freq);
    if (status != status_success) {
        printf("CAN initialization failed, error code: %d\n", status);
        return;
    }

    can_enable_tx_rx_irq(BOARD_APP_CAN_BASE, CAN_EVENT_RECEIVE);
    intc_m_enable_irq_with_priority(BOARD_APP_CAN_IRQn, 1);

    can_transmit_buf_t tx_buf;
    memset(&tx_buf, 0, sizeof(tx_buf));
    tx_buf.dlc = 8;
    tx_buf.id = 0x123;
    for (uint32_t i = 0; i < 8; i++) {
        tx_buf.data[i] = (uint8_t) i | (i << 4);
    }

    printf("Can Echo test: Initiator is sending message out...\n");
    status = can_send_message_blocking(BOARD_APP_CAN_BASE, &tx_buf);
    if (status != status_success) {
        printf("CAN sent message failed, error_code:%d\n", status);
        return;
    }
    printf("Waiting for echo message...\n");
    while (!has_new_rcv_msg) {
    }
    has_new_rcv_msg = false;
    show_received_can_message((const can_receive_buf_t *)&s_can_rx_buf);
}

void board_can_echo_test_responder(void)
{
    can_config_t can_config;
    can_get_default_config(&can_config);
    can_config.baudrate = 500000; /* 500kbps */
    can_config.loopback_mode = false;
    board_init_can(BOARD_APP_CAN_BASE);
    uint32_t can_src_clk_freq = board_init_can_clock(BOARD_APP_CAN_BASE);
    hpm_stat_t status = can_init(BOARD_APP_CAN_BASE, &can_config, can_src_clk_freq);
    if (status != status_success) {
        printf("CAN initialization failed, error code: %d\n", status);
        return;
    }

    can_enable_tx_rx_irq(BOARD_APP_CAN_BASE, CAN_EVENT_RECEIVE);
    intc_m_enable_irq_with_priority(BOARD_APP_CAN_IRQn, 1);
    printf("CAN echo test: Responder is waiting for echo message...\n");
    while (!has_new_rcv_msg) {
    }
    has_new_rcv_msg = false;
    show_received_can_message((const can_receive_buf_t *)&s_can_rx_buf);

    can_transmit_buf_t tx_buf;
    memset(&tx_buf, 0, sizeof(tx_buf));
    tx_buf.dlc = s_can_rx_buf.dlc;
    tx_buf.id = 0x321;
    uint32_t msg_len = can_get_data_bytes_from_dlc(s_can_rx_buf.dlc);
    memcpy(&tx_buf.data, (uint8_t *)&s_can_rx_buf.data, msg_len);
    status = can_send_message_blocking(BOARD_APP_CAN_BASE, &tx_buf);
    if (status != status_success) {
        printf("CAN sent message failed, error_code:%d\n", status);
        return;
    }
    printf("Sent echo message back\n");
}


void board_can_send_multiple_messages(void)
{
    can_config_t can_config;
    can_get_default_config(&can_config);
    can_config.baudrate = 500000; /* 500kbps */
    can_config.loopback_mode = false;
    board_init_can(BOARD_APP_CAN_BASE);
    uint32_t can_src_clk_freq = board_init_can_clock(BOARD_APP_CAN_BASE);
    hpm_stat_t status = can_init(BOARD_APP_CAN_BASE, &can_config, can_src_clk_freq);
    if (status != status_success) {
        printf("CAN initialization failed, error code: %d\n", status);
        return;
    }

    can_transmit_buf_t tx_buf;
    memset(&tx_buf, 0, sizeof(tx_buf));
    tx_buf.dlc = 8;
    uint32_t msg_len = can_get_data_bytes_from_dlc(tx_buf.dlc);
    for (uint32_t i = 0; i < msg_len; i++) {
        tx_buf.data[i] = i | (i << 4);
    }
    for (uint32_t i = 0; i < 2048; i++) {
        tx_buf.id = i;
        status = can_send_message_blocking(BOARD_APP_CAN_BASE, &tx_buf);
        if (status != status_success) {
            printf("CAN sent message failed, error_code:%d\n", status);
            return;
        }
    }
    printf("Sent messages with ID from 0 to 2047 out\n");
}


void handle_can_test(void)
{
    show_help();

    while (true) {
        char option = getchar();
        putchar(option);
        putchar('\n');

        switch (option) {
        default:
            show_help();
            break;
        case '0':
            can_loopback_test_for_all_cans();
            break;
        case '1':
            board_can_loopback_test_in_interrupt_mode();
            break;
        case '2':
            board_can_echo_test_initiator();
            break;
        case '3':
            board_can_echo_test_responder();
            break;
        case '4':
            board_can_send_multiple_messages();
            break;
        }
    }
}

void show_help(void)
{
    static const char help_info[] = ""
                                    "*********************************************************************************\n"
                                    "*                                                                               *\n"
                                    "*                         CAN Example Menu                                      *\n"
                                    "*                                                                               *\n"
                                    "* 0 - Run loopback test for all supported CAN controllers (CAN and CANFD)       *\n"
                                    "* 1 - Run loopback test for board supported CAN controller (interrupt mode)     *\n"
                                    "* 2 - Echo test between two board:initiator                                     *\n"
                                    "* 3 - Echo test between two board:responder                                     *\n"
                                    "* 4 - Send mulitple messages for transmission check                             *\n"
                                    "*                                                                               *\n"
                                    "*********************************************************************************\n";
    printf("%s\n", help_info);
}