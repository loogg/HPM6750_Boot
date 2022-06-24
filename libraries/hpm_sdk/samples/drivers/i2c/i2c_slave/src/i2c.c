/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "hpm_clock_drv.h"
#include "hpm_i2c_drv.h"

#ifndef TEST_I2C_MASTER
#define TEST_I2C_MASTER BOARD_APP_I2C_BASE
#define TEST_I2C_MASTER_CLOCK_NAME BOARD_APP_I2C_CLK_NAME 
#endif

#ifndef TEST_I2C_SLAVE
#define TEST_I2C_SLAVE BOARD_APP_I2C_SLAVE_BASE
#define TEST_I2C_SLAVE_IRQ BOARD_APP_I2C_SLAVE_IRQ
#define TEST_I2C_SLAVE_CLOCK_NAME BOARD_APP_I2C_SLAVE_CLK_NAME
#endif

#define TEST_I2C_SLAVE_ADDRESS (0x16U)
#define TEST_BUFFER_SIZE (16U)

volatile bool slave_read_done = false;
volatile uint8_t slave_buf_index;
uint8_t master_read_buf[TEST_BUFFER_SIZE];
uint8_t slave_buf[TEST_BUFFER_SIZE];

static void i2c_handle_address_hit(I2C_Type *ptr)
{
    printf("address hit %x\n", ptr->CTRL);
    return;
}

static void i2c_handle_write_complete(I2C_Type *ptr)
{
    printf("Slave sent done: %d bytes\n", i2c_get_data_count(ptr));
}

static void i2c_handle_read_complete(I2C_Type *ptr)
{
    slave_read_done = true;
    printf("Slave got: %d bytes\n", i2c_get_data_count(ptr));
    for (uint32_t i = 0; i < slave_buf_index; i++) {
        printf("Slave got: 0x%x\n", slave_buf[i]);
    }
}

static void i2c_handle_read_data(I2C_Type *ptr)
{
    uint8_t count;
    count = i2c_get_data_count(ptr);
    if (status_success != i2c_slave_read(ptr, slave_buf + slave_buf_index, count - slave_buf_index)) {
        printf("Slave read failed\n");
    } else {
        slave_buf_index += count - slave_buf_index;
    }
}

static void i2c_handle_write_data(I2C_Type *ptr)
{
    if (slave_buf_index > (sizeof(slave_buf) - 1)) {
        /* send a dummy data at the end of transmittion */
        i2c_slave_write(ptr, '\0');
        return;
    }
    if (status_success != i2c_slave_write(ptr, slave_buf[slave_buf_index])) {
        printf("Slave sent data failed\n");
        while(1);
    } else {
        slave_buf_index++;
    }
}

void isr_i2c_slave(void)
{
    volatile bool is_writing = false;
    volatile uint32_t status = i2c_get_status(TEST_I2C_SLAVE);
    i2c_clear_status(TEST_I2C_SLAVE, status);
    is_writing = i2c_is_writing(TEST_I2C_SLAVE);

    if (status & I2C_EVENT_ADDRESS_HIT) {
        i2c_handle_address_hit(TEST_I2C_SLAVE);
        slave_buf_index = 0;
        /*
         * determine what I2C slave should do
         */
        if (is_writing) {
            i2c_enable_irq(TEST_I2C_SLAVE, I2C_EVENT_FIFO_EMPTY);
        } else {
            i2c_enable_irq(TEST_I2C_SLAVE, I2C_EVENT_FIFO_FULL);
        }
    }
    if (is_writing) {
        i2c_handle_write_data(TEST_I2C_SLAVE);
    } else {
        i2c_handle_read_data(TEST_I2C_SLAVE);
    }
    if (status & I2C_EVENT_TRANSACTION_COMPLETE) {
        printf("slave transmit done\n");
        if (is_writing) {
            i2c_handle_write_complete(TEST_I2C_SLAVE);
        } else {
            i2c_handle_read_complete(TEST_I2C_SLAVE);
        }
        i2c_clear_fifo(TEST_I2C_SLAVE);
        i2c_disable_irq(TEST_I2C_SLAVE, I2C_EVENT_ALL_MASK);
    }
}
SDK_DECLARE_EXT_ISR_M(TEST_I2C_SLAVE_IRQ, isr_i2c_slave)

/*
 * prepare random data for testing
 */
static uint8_t prepare_data(uint8_t *p, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++) {
        p[i] = rand() & 0xFF;
    }
    return size;
}

int main(void)
{
    uint8_t size;
    uint8_t send_buf[TEST_BUFFER_SIZE];
    hpm_stat_t stat;
    i2c_config_t config;
    uint32_t freq;

    board_init();

    init_i2c_pins(TEST_I2C_MASTER);
    init_i2c_pins(TEST_I2C_SLAVE);

    config.i2c_mode = i2c_mode_normal;
    config.is_10bit_addressing = false;
    freq = clock_get_frequency(TEST_I2C_MASTER_CLOCK_NAME);
    stat = i2c_init_master(TEST_I2C_MASTER, freq, &config);
    if (stat != status_success) {
        return stat;
    }

    freq = clock_get_frequency(TEST_I2C_SLAVE_CLOCK_NAME);
    stat = i2c_init_slave(TEST_I2C_SLAVE, freq, &config, TEST_I2C_SLAVE_ADDRESS);
    if (stat != status_success) {
        return stat;
    }
    intc_m_enable_irq_with_priority(TEST_I2C_SLAVE_IRQ, 1);

    printf("i2c slave communication example\n");

    while (1) {
        i2c_enable_irq(TEST_I2C_SLAVE, I2C_EVENT_ADDRESS_HIT | I2C_EVENT_TRANSACTION_COMPLETE | I2C_EVENT_FIFO_FULL);
        size = prepare_data(send_buf, sizeof(send_buf));
        slave_read_done = false;
        printf("Master sending\n");
        if (status_success != i2c_master_write(TEST_I2C_MASTER, TEST_I2C_SLAVE_ADDRESS, send_buf, size)) {
            printf("Master failed to write data");
            while(1);
        }

        printf("Master sent %d bytes\n", size);
        for (uint32_t i = 0; i < sizeof(send_buf); i++) {
            printf("Master sent: 0x%x\n", send_buf[i]);
        }

        while (!slave_read_done) {}

        printf("Master reading\n");
        i2c_enable_irq(TEST_I2C_SLAVE, I2C_EVENT_ADDRESS_HIT | I2C_EVENT_TRANSACTION_COMPLETE);
        /*
         * Start master reading data
         */
        if (status_success != i2c_master_read(TEST_I2C_MASTER, TEST_I2C_SLAVE_ADDRESS, master_read_buf, sizeof(master_read_buf))) {
            printf("Master read failed\n");
            while(1);
        }

        printf("master got %d bytes\n", sizeof(master_read_buf));
        for (uint32_t i = 0; i < sizeof(send_buf); i++) {
            if (send_buf[i] != master_read_buf[i]) {
                printf("data read is not expected\n");
                printf("Master got: 0x%x, but expected 0x%x\n", master_read_buf[i], send_buf[i]);
                while(1);
            } else {
                printf("Master got: 0x%x\n", master_read_buf[i]);
            }
        }
        printf("delay 1s\n");
        board_delay_ms(1000);
    }

    return 0;
}
