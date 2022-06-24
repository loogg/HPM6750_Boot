/*
 * Copyright (c) 2022 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "hpm_i2c_drv.h"
#include "hpm_dma_drv.h"
#include "hpm_dmamux_drv.h"

/* define I2C MASTER */
#define TEST_I2C_MASTER BOARD_APP_I2C_BASE
#define TEST_I2C_MASTER_IRQ BOARD_APP_I2C_IRQ
#define TEST_I2C_MASTER_CLOCK_NAME BOARD_APP_I2C_CLK_NAME

/* define I2C SLAVE */
#define TEST_I2C_SLAVE BOARD_APP_I2C_SLAVE_BASE
#define TEST_I2C_SLAVE_IRQ BOARD_APP_I2C_SLAVE_IRQ
#define TEST_I2C_SLAVE_CLOCK_NAME BOARD_APP_I2C_SLAVE_CLK_NAME

/* define I2C DMA and DMAMUX */
#define TEST_I2C_DMA_CONTROLLER BOARD_APP_HDMA
#define TEST_I2C_DMAMUX_CONTROLLER BOARD_APP_DMAMUX


#define TEST_I2C_SLAVE_ADDRESS (0x16U)
#define TEST_BUFFER_SIZE (255U)

volatile bool slave_read_done;
volatile uint8_t slave_buf_index;
uint8_t slave_buf[TEST_BUFFER_SIZE];

static void i2c_handle_address_hit(I2C_Type *ptr)
{
    printf("Slave address hit %x\n", ptr->CTRL);
    return;
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

    if (count == TEST_BUFFER_SIZE) {
        slave_read_done = true;
        printf("Slave read data complete!\n");
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
        while (1) {
		}
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


hpm_stat_t i2c_trigger_dma(DMA_Type *dma_ptr, uint8_t ch_num, I2C_Type *i2c_ptr, uint32_t src, uint32_t size)
{
    dma_handshake_config_t config;
    config.ch_index = ch_num;
    config.dst = (uint32_t)&i2c_ptr->DATA;
    config.dst_fixed = true;
    config.src = src;
    config.src_fixed = false;
    config.size_in_byte = size;

    return dma_setup_handshake(dma_ptr, &config);
}

int main(void)
{
    ATTR_PLACE_AT_NONCACHEABLE static uint8_t send_buf[TEST_BUFFER_SIZE];
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
    i2c_dma_enable(TEST_I2C_MASTER);

    freq = clock_get_frequency(TEST_I2C_SLAVE_CLOCK_NAME);
    stat = i2c_init_slave(TEST_I2C_SLAVE, freq, &config, TEST_I2C_SLAVE_ADDRESS);
    if (stat != status_success) {
        return stat;
    }

    intc_m_enable_irq_with_priority(TEST_I2C_SLAVE_IRQ, 1);
    i2c_enable_irq(TEST_I2C_SLAVE, I2C_EVENT_ADDRESS_HIT | I2C_EVENT_TRANSACTION_COMPLETE | I2C_EVENT_FIFO_FULL);

    prepare_data(send_buf, sizeof(send_buf));
    slave_read_done = false;
    printf("I2c Master sending\n");

    i2c_trigger_dma(TEST_I2C_DMA_CONTROLLER, DMAMUX_MUXCFG_HDMA_MUX0, TEST_I2C_MASTER, (uint32_t)core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)send_buf), sizeof(send_buf));
    dmamux_config(TEST_I2C_DMAMUX_CONTROLLER, DMAMUX_MUXCFG_HDMA_MUX0, HPM_DMA_SRC_I2C0, 1);

    i2c_master_start_dma_write(TEST_I2C_MASTER, TEST_I2C_SLAVE_ADDRESS, sizeof(send_buf));

    while (!slave_read_done) {
	}

    for (uint32_t i = 0; i < sizeof(send_buf); i++) {
        if (send_buf[i] != slave_buf[i]) {
            printf("Master send data ERROR!\n");
            printf("Master got: 0x%x, but expected 0x%x\n", slave_buf[i], send_buf[i]);
            while (1) {
			}
        } else {
            printf("Master send data OK!  send_buf[%d]=0x%x\n", i, slave_buf[i]);
        }
    }
}
