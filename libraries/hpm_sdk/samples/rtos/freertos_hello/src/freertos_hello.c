/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/*  HPM example includes. */
#include <stdio.h>
#include "board.h"
#include "hpm_gpio_drv.h"

#define task1_PRIORITY    (configMAX_PRIORITIES - 5U)
#define task2_PRIORITY    (configMAX_PRIORITIES - 4U)

volatile bool led_on;

static void task1(void *pvParameters)
{
    for (;;) {
        printf("LED is %s.\n", led_on ? "on" : "off");
        vTaskDelay(5);
    }
}

static void task2(void *pvParameters)
{
    for (;;) {
        gpio_toggle_pin(BOARD_LED_GPIO_CTRL, BOARD_LED_GPIO_INDEX, BOARD_LED_GPIO_PIN);
        led_on = !led_on;
        printf("Toggle the led pin level.\n");
        vTaskDelay(20);
    }
}

int main(void)
{
    board_init();
    board_init_gpio_pins();
    gpio_enable_pin_output(BOARD_LED_GPIO_CTRL, BOARD_LED_GPIO_INDEX, BOARD_LED_GPIO_PIN);
    gpio_write_pin(BOARD_LED_GPIO_CTRL, BOARD_LED_GPIO_INDEX, BOARD_LED_GPIO_PIN, BOARD_LED_ON_LEVEL);
    led_on = true;

    if (xTaskCreate(task1, "task1", configMINIMAL_STACK_SIZE + 256U, NULL, task1_PRIORITY, NULL) != pdPASS) {
        printf("Task1 creation failed!.\n");
        for (;;) {
            ;
        }
    }
    if (xTaskCreate(task2, "task2", configMINIMAL_STACK_SIZE + 256U, NULL, task2_PRIORITY, NULL) != pdPASS) {
        printf("Task2 creation failed!.\n");
        for (;;) {
            ;
        }
    }
    vTaskStartScheduler();
    for (;;) {
        ;
    }
    return 0;
}
