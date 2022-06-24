/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "hpm_gpio_drv.h"
#include "hpm_ft5406.h"

ft5406_context_t ft5406 = {0};
ft5406_sys_info_t ft5406_sys_info = {0};

/* #define USE_CAP_INT */

#ifdef USE_CAP_INT
#define WORK_MODE "interrupt"
volatile bool touch_available = false;
void isr_gpio0_x(void)
{
    BOARD_CAP_INTR_GPIO->IF[BOARD_CAP_INTR_GPIO_INDEX].VALUE = 1 << BOARD_CAP_INTR_GPIO_PIN;
    touch_available = true;
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_X, isr_gpio0_x)
#else
#define WORK_MODE "polling"
#endif

void show_sys_info(ft5406_sys_info_t *sys_info)
{
    uint32_t i = 0;
    printf("system info:\n");
    printf("device mode: 0x%x\n", sys_info->mode);
    printf("bist comm: 0x%x\n", sys_info->bist_comm);
    printf("bist stat: 0x%x\n", sys_info->bist_stat);
    for (i = 0; i < 8; i++) {
        printf("uid_%d: 0x%x\n", i, sys_info->uid[i]);
    }
    printf("bl verh: 0x%x\n", sys_info->bl_verh);
    printf("bl verl: 0x%x\n", sys_info->bl_verl);
    printf("fts ic verh: 0x%x\n", sys_info->fts_ic_verh);
    printf("fts ic verl: 0x%x\n", sys_info->fts_ic_verl);
    printf("app idh: 0x%x\n", sys_info->app_idh);
    printf("app idl: 0x%x\n", sys_info->app_idl);
    printf("app verh: 0x%x\n", sys_info->app_verh);
    printf("app verl: 0x%x\n", sys_info->app_verl);
    for (i = 0; i < 5; i++) {
        printf("cid_%d: 0x%x\n", i, sys_info->cid[i]);
    }
}

void cap_int_setup(void)
{
    gpio_config_pin_interrupt(BOARD_CAP_INTR_GPIO, BOARD_CAP_INTR_GPIO_INDEX,
                           BOARD_CAP_INTR_GPIO_PIN, gpio_interrupt_trigger_edge_rising);
    gpio_enable_pin_interrupt(BOARD_CAP_INTR_GPIO, BOARD_CAP_INTR_GPIO_INDEX,
                           BOARD_CAP_INTR_GPIO_PIN);
    intc_m_enable_irq_with_priority(BOARD_CAP_INTR_GPIO_IRQ, 1);
}

uint8_t buf[0xAE] = {0};

hpm_stat_t get_ft5406_info(ft5406_context_t *context)
{
    uint32_t i;
    hpm_stat_t stat;
    stat = ft5406_read_data(context, 0, buf, sizeof(buf));
    if (stat != status_success) {
        return stat;
    }

    for (i = 0; i < sizeof(buf); i++) {
        printf("0x%02x: 0x%x", i, buf[i]);
        if ((i > 0) && (i % 7 == 0)) {
            printf("\n");
        } else {
            printf(" ");
        }

    }
    return stat;
}

int main(void)
{
    hpm_stat_t stat;
    uint32_t i = 0;
    ft5406_touch_data_t touch_data;

    board_init();
    board_init_console();
    board_init_cap_touch();

    printf("ft5406 touch example (%s mode)\n", WORK_MODE);
    ft5406.ptr = BOARD_APP_I2C_BASE;
    stat = ft5406_init(&ft5406);
    if (stat != status_success) {
        printf("ft5406 init failed %d\n", stat);
        return 1;
    }

#ifdef USE_CAP_INT
    stat = ft5406_write_register(&ft5406, FT5406_ID_G_MODE, 0);
    if (stat != status_success) {
        printf("ft5406 enable irq to host failed %d\n", stat);
        return 1;
    }

    cap_int_setup();
#endif
    while(1) {
#ifdef USE_CAP_INT
        while (!touch_available) {
            __asm("nop");
        }
#endif

        stat = ft5406_read_touch_data(&ft5406, &touch_data);
        if (stat != status_success) {
            printf("ft5406 read data failed\n");
            return 1;
        }
        if ((touch_data.status < FT5406_MAX_TOUCH_POINTS) && (touch_data.status)) {
            printf("gesture: %d, status: %d\n",
                    touch_data.gesture, touch_data.status);
            for (i = 0; touch_data.points[i].x_h > 0 && touch_data.points[i].x_h < 0xFF; i++) {
                printf("[%d]: even flag: %x, touch id: %x, x: %d, y: %d\n", i,
                        (touch_data.points[i].x_h) >> 6,
                        (touch_data.points[i].y_h) >> 4,
                        (touch_data.points[i].x_h & 0xF) << 8 | touch_data.points[i].x_l,
                        (touch_data.points[i].y_h & 0xF) << 8 | touch_data.points[i].y_l);
            }
        }
#ifdef USE_CAP_INT
        touch_available = false;
#endif
    }

    return 0;
}
