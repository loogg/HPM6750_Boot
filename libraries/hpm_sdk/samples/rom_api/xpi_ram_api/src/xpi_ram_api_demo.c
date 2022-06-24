/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "hpm_debug_console.h"
#include "hpm_romapi.h"
#include "hpm_l1c_drv.h"
#include "hpm_clock_drv.h"
#include "hpm_dma_drv.h"
#include "hpm_pmp_drv.h"

#define XPI_RAM_TEST_ALIGNED_START (0x90000000UL)
#define XPI_RAM_TEST_UNALIGNED_START (0x90000001UL)

static xpi_ram_config_t s_ram_config;
static xpi_ram_config_option_t option;

bool configure_aps12808l(void);

static bool byte_write_read_test(void);

static bool halfword_write_read_test(void);

static bool word_write_read_test(void);

static bool dword_write_read_test(void);

static void sdp_write_read_write_test(void);

void xdma_write_read_write_test(void);

int main(void)
{
    board_init();

    printf("XPI RAM API demo\nThis demo will test the XPI RAM initialization and memory test\n");

    bool result = configure_aps12808l();

    printf("XPI1 frequency=%dMHz\n", clock_get_frequency(clock_xpi1) / 1000000);
    if (!result) {
        return status_fail;
    }


    pmp_entry_t pmp_entry[1];
    pmp_entry[0].pmp_addr = PMP_NAPOT_ADDR(XPI_RAM_TEST_ALIGNED_START, 16 * 1024 * 1024);
    pmp_entry[0].pmp_cfg.val = PMP_CFG(READ_EN, WRITE_EN, EXECUTE_EN, ADDR_MATCH_NAPOT, REG_UNLOCK);
    pmp_entry[0].pma_addr = PMA_NAPOT_ADDR(XPI_RAM_TEST_ALIGNED_START, 16 * 1024 * 1024);
    pmp_entry[0].pma_cfg.val = PMA_CFG(ADDR_MATCH_NAPOT, MEM_TYPE_MEM_WB_READ_WRITE_ALLOC, AMO_EN);

    pmp_config(&pmp_entry[0], ARRAY_SIZE(pmp_entry));

    printf("CPU write-read-verify test with D-Cache disabled\n");
    l1c_dc_disable();
    l1c_dc_invalidate(0x90000000, 0x1000000);
    byte_write_read_test();
    halfword_write_read_test();
    word_write_read_test();
    dword_write_read_test();

    printf("CPU write-read-verify test with D-Cache enabled\n\n\\n");
    l1c_dc_enable();
    l1c_dc_invalidate(0x90000000, 0x1000000);
    byte_write_read_test();
    l1c_dc_invalidate(0x90000000, 0x1000000);
    halfword_write_read_test();
    l1c_dc_invalidate(0x90000000, 0x1000000);
    word_write_read_test();
    l1c_dc_invalidate(0x90000000, 0x1000000);
    dword_write_read_test();

    printf("CPU write-read-verify test with DMA engines\n\n\n");
    l1c_dc_disable();
    sdp_write_read_write_test();
    xdma_write_read_write_test();
    while (1) {
    }

    return 0;
}

bool configure_aps12808l(void)
{
    printf("%s() started\n", __func__);
    option.header.tag = XPI_RAM_CFG_OPTION_TAG;
    option.header.words = 2;
    option.option0.freq_opt = 8;
    option.option0.probe_type = xpi_ram_type_apmemory_x8;
    option.option0.misc = 1; /* 1.8V */
    option.option1.channel = xpi_channel_a2;
    option.option1.drive_strength = 1;

    hpm_stat_t status = rom_xpi_ram_get_config(HPM_XPI1, &s_ram_config, &option);
    if (status != status_success) {
        printf("XPI RAM configuration failed, status=%d\n", status);
        return false;
    }
    status = rom_xpi_ram_init(HPM_XPI1, &s_ram_config);
    if (status != status_success) {
        printf("XPI RAM initialization failed, status = %d\n", status);
        return false;
    }


    return true;
}

static bool byte_write_read_test(void)
{
    printf("Started byte write test...\n");
    uint32_t total_size = s_ram_config.device_info.size_in_kbytes * 1024;
    register uint8_t *start = (uint8_t *) XPI_RAM_TEST_ALIGNED_START;

    for (uint32_t i = 0; i < total_size; i++) {
        *start++ = i & 0xFF;
    }

    start = (uint8_t *) XPI_RAM_TEST_ALIGNED_START;
    for (uint32_t i = 0; i < total_size; i++) {
        if (*start != (i & 0xFF)) {
            printf("%s, ram write-read-verify failed at 0x%08x, value=%02x\n", __func__, (uint32_t) start, *start);
            return false;
        }
        ++start;
    }

    uint64_t psram_write_start = HPM_MCHTMR->MTIME;
    for (register uint32_t i = 0; i < total_size; i += 16) {
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
        *start++ = 0xaa;
    }
    uint64_t psram_write_end = HPM_MCHTMR->MTIME;
    printf("byte write completed!\n");

    uint64_t psram_read_start = HPM_MCHTMR->MTIME;
    start = (uint8_t *) XPI_RAM_TEST_ALIGNED_START;
    for (register uint32_t i = 0; i < total_size; i++) {
        __asm volatile ("" : : "r" (*((uint8_t *) (XPI_RAM_TEST_ALIGNED_START + i))));
    }

    uint64_t psram_read_end = HPM_MCHTMR->MTIME;
    int32_t mtimer_freq = clock_get_frequency(clock_mchtmr0);
    uint32_t write_time_us = (psram_write_end - psram_write_start) / (mtimer_freq / 1000000U);
    uint32_t read_time_us = (psram_read_end - psram_read_start) / (mtimer_freq / 1000000U);

    printf("%s, ram write-read-verify PASSED, write_time:%dus, read_time:%dus\n", __func__, write_time_us,
           read_time_us);
    printf("Write: %.1fMB/S, Read: %.1fMB/S\n", 1000000.0 / write_time_us * 16, 1000000.0 / read_time_us * 16);
    return true;
}

static bool halfword_write_read_test(void)
{
    printf("Started halfword write test...\n");
    uint32_t total_size = s_ram_config.device_info.size_in_kbytes * 1024;
    register uint16_t *start = (uint16_t *) XPI_RAM_TEST_ALIGNED_START;
    for (uint32_t i = 0; i < total_size; i += 2) {
        uint32_t temp = (i & 0xFF);
        *start++ = temp | (temp << 8);
    }
    start = (uint16_t *) XPI_RAM_TEST_ALIGNED_START;
    for (uint32_t i = 0; i < total_size; i += 2) {
        uint32_t temp = (i & 0xFF);
        uint16_t data_for_verify = temp | (temp << 8);
        if (*start != data_for_verify) {
            printf("%s, ram write-read-verify failed at 0x%08x, value=%02x\n", __func__, (uint32_t) start, *start);
            return false;
        }
        ++start;
    }

    uint64_t psram_write_start = HPM_MCHTMR->MTIME;
    for (register uint32_t i = 0; i < total_size; i += 32) {
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
        *start++ = 0x55aa;
    }
    uint64_t psram_write_end = HPM_MCHTMR->MTIME;
    printf("halfword write completed!\n");
    start = (uint16_t *) XPI_RAM_TEST_ALIGNED_START;
    uint64_t psram_read_start = HPM_MCHTMR->MTIME;
    for (register uint32_t i = 0; i < total_size; i += 2) {
        __asm volatile ("" : : "r" (*((uint16_t *) (XPI_RAM_TEST_ALIGNED_START + i))));
    }
    uint64_t psram_read_end = HPM_MCHTMR->MTIME;
    int32_t mtimer_freq = clock_get_frequency(clock_mchtmr0);
    uint32_t write_time_us = (psram_write_end - psram_write_start) / (mtimer_freq / 1000000U);
    uint32_t read_time_us = (psram_read_end - psram_read_start) / (mtimer_freq / 1000000U);

    printf("%s, ram write-read-verify PASSED, write_time:%dus, read_time:%dus\n", __func__, write_time_us,
           read_time_us);
    printf("Write: %.1fMB/S, Read: %.1fMB/S\n", 1000000.0 / write_time_us * 16, 1000000.0 / read_time_us * 16);
    return true;
}

static bool word_write_read_test(void)
{
    printf("Started word write test...\n");
    uint32_t total_size = s_ram_config.device_info.size_in_kbytes * 1024;
    register uint32_t *start = (uint32_t *) XPI_RAM_TEST_ALIGNED_START;
    for (uint32_t i = 0; i < total_size; i += 4) {
        uint32_t temp = (i & 0xFF);
        *start++ = temp | (temp << 8) | (temp << 16) | (temp << 24);
    }
    start = (uint32_t *) XPI_RAM_TEST_ALIGNED_START;
    for (uint32_t i = 0; i < total_size; i += 4) {
        uint32_t temp = (i & 0xFF);
        uint32_t data_for_verify = temp | (temp << 8) | (temp << 16) | (temp << 24);
        if (*start != data_for_verify) {
            printf("%s, ram write-read-verify failed at 0x%08x, value=%08x\n", __func__, (uint32_t) start, *start);
            return false;
        }
        ++start;
    }


    uint64_t psram_write_start = HPM_MCHTMR->MTIME;
    for (register uint32_t i = 0; i < total_size; i += 64) {
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
    }
    uint64_t psram_write_end = HPM_MCHTMR->MTIME;
    printf("word write completed!\n");
    uint64_t psram_read_start = HPM_MCHTMR->MTIME;
    start = (uint32_t *) XPI_RAM_TEST_ALIGNED_START;
    register uint32_t dummy;
    for (uint32_t i = 0; i < total_size; i += 4) {
        __asm volatile ("" : : "r" (*((uint32_t *) (XPI_RAM_TEST_ALIGNED_START + i))));
    }
    (void) dummy;
    uint64_t psram_read_end = HPM_MCHTMR->MTIME;
    int32_t mtimer_freq = clock_get_frequency(clock_mchtmr0);
    uint32_t write_time_us = (psram_write_end - psram_write_start) / (mtimer_freq / 1000000U);
    uint32_t read_time_us = (psram_read_end - psram_read_start) / (mtimer_freq / 1000000U);

    printf("%s, ram write-read-verify PASSED, write_time:%dus, read_time:%dus\n", __func__, write_time_us,
           read_time_us);
    printf("Write: %.1fMB/S, Read: %.1fMB/S\n", 1000000.0 / write_time_us * 16, 1000000.0 / read_time_us * 16);
    return true;
}

static bool dword_write_read_test(void)
{
    printf("Started dword write test...\n");
    uint32_t total_size = s_ram_config.device_info.size_in_kbytes * 1024;
    register uint64_t *start = (uint64_t *) XPI_RAM_TEST_ALIGNED_START;
    uint64_t psram_write_start = HPM_MCHTMR->MTIME;
    for (register uint32_t i = 0; i < total_size; i += 64) {
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
//        *start++ = 0x0123456789abcdefULL;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
        *start++ = 0;
    }
    uint64_t psram_write_end = HPM_MCHTMR->MTIME;
    printf("dword write completed!\n");
    uint64_t psram_read_start = HPM_MCHTMR->MTIME;
    start = (uint64_t *) XPI_RAM_TEST_ALIGNED_START;
    volatile uint64_t dummy;
    for (register uint32_t i = 0; i < total_size; i += 64) {
        dummy = *start++;
        dummy = *start++;
        dummy = *start++;
        dummy = *start++;
        dummy = *start++;
        dummy = *start++;
        dummy = *start++;
        dummy = *start++;
    }
    (void) dummy;
    uint64_t psram_read_end = HPM_MCHTMR->MTIME;
    int32_t mtimer_freq = clock_get_frequency(clock_mchtmr0);
    uint32_t write_time_us = (psram_write_end - psram_write_start) / (mtimer_freq / 1000000U);
    uint32_t read_time_us = (psram_read_end - psram_read_start) / (mtimer_freq / 1000000U);

    printf("%s, ram write-read-verify PASSED, write_time:%dus, read_time:%dus\n", __func__, write_time_us,
           read_time_us);
    printf("Write: %.1fMB/S, Read: %.1fMB/S\n", 1000000.0 / write_time_us * 16, 1000000.0 / read_time_us * 16);
    return true;
}

void sdp_write_read_write_test(void)
{
    printf("Started DMA read-write test for PSRAM\n");

    volatile uint8_t *xram0_start = (volatile uint8_t *) 0x01080000;

    for (uint32_t i = 0; i < 0x100000; i++) {
        *xram0_start++ = i & 0xFF;
    }

    rom_sdp_init();
    sdp_dma_ctx_t dma_ctx;
    sdp_dma_ctx_t *sys_dma_ctx = (sdp_dma_ctx_t *) core_local_mem_to_sys_address(HPM_CORE0, (uint32_t) &dma_ctx);

    uint64_t psram_write_start = HPM_MCHTMR->MTIME;
    rom_sdp_memcpy(sys_dma_ctx, (void *) XPI_RAM_TEST_ALIGNED_START, (void *) 0x01080000, 0x100000);
    uint64_t psram_write_end = HPM_MCHTMR->MTIME;
    uint8_t *psram_start = (uint8_t *) 0x90000000;
    for (uint32_t i = 0; i < 1024 * 1024; i++) {
        uint32_t byte_read = *psram_start++;
        if (byte_read != (i & 0xFF)) {
            printf("DMA write failed at %08x, value=%02x\n", XPI_RAM_TEST_ALIGNED_START + i, byte_read);
            break;
        }
    }

    uint32_t accum_cycles = 0;

    uint64_t psram_read_start = HPM_MCHTMR->MTIME;
    rom_sdp_memcpy(sys_dma_ctx, (void *) 0x01080000, (void *) XPI_RAM_TEST_ALIGNED_START, 0x100000);
    uint64_t psram_read_end = HPM_MCHTMR->MTIME;
    accum_cycles += (psram_read_end - psram_read_start);

    xram0_start = (volatile uint8_t *) 0x01080000;
    for (uint32_t j = 0; j < 0x100000; j++) {
        uint8_t byte_read = *xram0_start++;
        if (byte_read != (j & 0xFF)) {
            printf("DMA read failed at %08x, value=%x\n", XPI_RAM_TEST_ALIGNED_START + j, byte_read);
            break;
        }
    }

    uint32_t mtimer_freq = clock_get_frequency(clock_mchtmr0);
    uint32_t write_time_us = (psram_write_end - psram_write_start) / (mtimer_freq / 1000000U);
    uint32_t read_time_us = accum_cycles / (mtimer_freq / 1000000U);

    printf("%s completed, write:%dus, read:%dus\n", __func__, write_time_us, read_time_us);
    printf("Write: %.1fMB/S, Read: %.1fMB/S\n", 1000000.0 / write_time_us, 1000000.0 / read_time_us);
}


void xdma_write_read_write_test(void)
{
    printf("Started XDMA read-write test for PSRAM\n");
    for (uint32_t burst_len = 4; burst_len <= 8192; burst_len <<= 1) {
        uint64_t psram_read_start = HPM_MCHTMR->MTIME;
        dma_start_memcpy(HPM_XDMA, 0, 0x01080000, XPI_RAM_TEST_ALIGNED_START, 0x100000, burst_len);
        while (0 == (dma_check_transfer_status(HPM_XDMA, 0) & DMA_CHANNEL_STATUS_TC)) {
		}
        uint64_t psram_read_end = HPM_MCHTMR->MTIME;

        uint64_t psram_write_start = HPM_MCHTMR->MTIME;

        dma_start_memcpy(HPM_XDMA, 0, XPI_RAM_TEST_ALIGNED_START, 0x01080000, 0x100000, burst_len);
        while (0 == (dma_check_transfer_status(HPM_XDMA, 0) & DMA_CHANNEL_STATUS_TC)) {
		}

        uint64_t psram_write_end = HPM_MCHTMR->MTIME;

        uint32_t mtimer_freq = clock_get_frequency(clock_mchtmr0);
        uint32_t write_time_us = (psram_write_end - psram_write_start) / (mtimer_freq / 1000000U);
        uint32_t read_time_us = (psram_read_end - psram_read_start) / (mtimer_freq / 1000000U);

        printf("Test completed, write:%dus, read:%dus, burst_len=%d\n", write_time_us, read_time_us, burst_len);
        printf("Write: %.1fMB/S, Read: %.1fMB/S\n", 1000000.0 / write_time_us, 1000000.0 / read_time_us);
    }
}
