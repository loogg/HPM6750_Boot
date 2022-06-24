/*
 * Copyright (c) 2021 - 2022 hpmicro
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include "rtt_board.h"
#include "hpm_uart_drv.h"
#include "hpm_gpio_drv.h"
#include "hpm_mchtmr_drv.h"
#include "hpm_pmp_drv.h"
#include "assert.h"
#include "hpm_clock_drv.h"
#include "hpm_sysctl_drv.h"

#include <rthw.h>
#include <rtthread.h>
#include "drv_uart.h"
#include "drv_spi.h"
#include "drv_gpio.h"
#include "drv_pwm.h"
#include "drv_wdt.h"
#include "drv_hwtimer.h"
#include "drv_can.h"

void os_tick_config(void);

void rtt_board_init(void)
{
    board_init_clock();
    board_init_pmp();

#ifdef BSP_USING_TOUCH
    board_init_cap_touch();
#endif
#ifdef BSP_USING_LCD
    board_init_lcd();
#endif
}

void app_init_led_pins(void)
{
    gpio_enable_pin_output(APP_LED0_GPIO_CTRL, APP_LED0_GPIO_INDEX, APP_LED0_GPIO_PIN);
    gpio_enable_pin_output(APP_LED1_GPIO_CTRL, APP_LED1_GPIO_INDEX, APP_LED1_GPIO_PIN);
    gpio_enable_pin_output(APP_LED2_GPIO_CTRL, APP_LED2_GPIO_INDEX, APP_LED2_GPIO_PIN);

    gpio_write_pin(APP_LED0_GPIO_CTRL, APP_LED0_GPIO_INDEX, APP_LED0_GPIO_PIN, APP_LED_OFF);
    gpio_write_pin(APP_LED1_GPIO_CTRL, APP_LED1_GPIO_INDEX, APP_LED1_GPIO_PIN, APP_LED_OFF);
    gpio_write_pin(APP_LED2_GPIO_CTRL, APP_LED2_GPIO_INDEX, APP_LED2_GPIO_PIN, APP_LED_OFF);
}

void app_led_write(uint32_t index, bool state)
{
    switch (index)
    {
    case 0:
        gpio_write_pin(APP_LED0_GPIO_CTRL, APP_LED0_GPIO_INDEX, APP_LED0_GPIO_PIN, state);
        break;
    case 1:
        gpio_write_pin(APP_LED1_GPIO_CTRL, APP_LED1_GPIO_INDEX, APP_LED1_GPIO_PIN, state);
        break;
    case 2:
        gpio_write_pin(APP_LED2_GPIO_CTRL, APP_LED2_GPIO_INDEX, APP_LED2_GPIO_PIN, state);
        break;
    default:
        /* Suppress the toolchain warnings */
        break;
    }
}

void os_tick_config(void)
{
    sysctl_config_clock(HPM_SYSCTL, clock_node_mchtmr0, clock_source_osc0_clk0, 1);
    sysctl_add_resource_to_cpu0(HPM_SYSCTL, sysctl_resource_mchtmr0);

    mchtmr_set_compare_value(HPM_MCHTMR, BOARD_MCHTMR_FREQ_IN_HZ / RT_TICK_PER_SECOND);

    enable_mchtmr_irq();
}

void rt_hw_board_init(void)
{
    rtt_board_init();

    /* initialize memory system */
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);

    /* Configure the OS Tick */
    os_tick_config();

#ifdef BSP_USING_UART
    /* Initialize UART device */
    rt_hw_uart_init();
#endif

#ifdef BSP_USING_SPI
    /* Initialize SPI device */
    rt_hw_spi_init();
#endif

#ifdef BSP_USING_GPIO
    /* Initialize GPIO device */
    rt_hw_pin_init();
#endif

#ifdef BSP_USING_PWM
    /* Initialize SPI device */
    rt_hw_pwm_init();
#endif

#ifdef BSP_USING_WDG
    /* Initialize WDG device */
    rt_hw_wdt_init();
#endif

#ifdef BSP_USING_GPTMR
    /* Initialize GPTMR device */
    rt_hw_hwtimer_init();
#endif

#ifdef BSP_USING_CAN
    /* Initialize CAN device */
    rt_hw_can_init();
#endif

    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
}

void rt_hw_console_output(const char *str)
{
    while (*str != '\0')
    {
        uart_send_byte(BOARD_APP_UART_BASE, *str++);
    }
}

ATTR_PLACE_AT(".isr_vector") void mchtmr_isr(void)
{
    HPM_MCHTMR->MTIMECMP = HPM_MCHTMR->MTIME + BOARD_MCHTMR_FREQ_IN_HZ / RT_TICK_PER_SECOND;

    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}
