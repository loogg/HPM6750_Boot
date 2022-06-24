/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include "hpm_gpio_drv.h"
#include "hpm_gpiom_drv.h"

#ifndef BOARD_LED_OFF_LEVEL 
#define BOARD_LED_OFF_LEVEL 0
#endif

#define GPIO_TOGGLE_COUNT 5

void test_gpio_toggle_output(GPIO_Type *ptr)
{
    printf("toggling led %u times in total\n", GPIO_TOGGLE_COUNT);
    gpio_enable_pin_output(ptr, BOARD_LED_GPIO_INDEX,
                           BOARD_LED_GPIO_PIN);
    gpio_write_pin(ptr, BOARD_LED_GPIO_INDEX,
                        BOARD_LED_GPIO_PIN, BOARD_LED_OFF_LEVEL);

    for (uint32_t i = 0; i < GPIO_TOGGLE_COUNT; i++) {
        gpio_toggle_pin(ptr, BOARD_LED_GPIO_INDEX,
                            BOARD_LED_GPIO_PIN);
        board_delay_ms(500);
        gpio_toggle_pin(ptr, BOARD_LED_GPIO_INDEX,
                            BOARD_LED_GPIO_PIN);
        board_delay_ms(500);
        printf("toggling led %u/%u times\n", i + 1, GPIO_TOGGLE_COUNT);
    }
}

void gpiom_configure_pin_control_setting(gpiom_gpio_t gpio_module)
{
    printf("using gpiom configures pin control module\n");
    gpiom_set_pin_controler(BOARD_APP_GPIOM_BASE, BOARD_LED_GPIO_INDEX, BOARD_LED_GPIO_PIN, gpio_module);
    gpiom_enable_pin_visibility(BOARD_APP_GPIOM_BASE, BOARD_LED_GPIO_INDEX, BOARD_LED_GPIO_PIN, gpio_module);
    gpiom_lock_pin(BOARD_APP_GPIOM_BASE, BOARD_LED_GPIO_INDEX, BOARD_LED_GPIO_PIN);
}

int main(void)
{
    board_init();
    board_init_gpio_pins();
    printf("gpiom example\n");

    gpiom_configure_pin_control_setting(BOARD_APP_GPIOM_USING_CTRL_NAME);
    test_gpio_toggle_output(BOARD_APP_GPIOM_USING_CTRL);

    while(1);
    return 0;
}
