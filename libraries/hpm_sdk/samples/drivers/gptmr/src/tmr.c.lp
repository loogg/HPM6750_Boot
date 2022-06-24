/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "hpm_sysctl_drv.h"
#include "hpm_clock_drv.h"
#include "hpm_gpio_drv.h"
#include "hpm_tmr_drv.h"
#include "hpm_debug_console.h"

#ifndef TMR
#define TMR HPM_GTMR0
#define TMR_CH  0
#define TMR_CMP  0
#define TMR_IRQ IRQn_GTMR0
#define TMR_CLK_NAME clock_gtmr0
#endif

#define TEST_LOOP (200U)

#define TMR_PWM_DUTY_CYCLE_FP_MAX ((1U << 24) - 1)

volatile bool time_is_up = false;
volatile bool capture_is_done = false;
volatile uint32_t count;
tmr_counter_type_t target_counter;

void delay(uint32_t duration)
{
    for (uint32_t i = 0; i < duration * 10000; i++) {
       __asm("nop");
    }
}

void isr_tmr(void)
{
    volatile uint32_t s = TMR->SR;
    TMR->SR = s;
    if (s & TMR_CH_CMP_STAT_MASK(TMR_CH, TMR_CMP)) {
        time_is_up = true;
        count = tmr_channel_get_counter(TMR, TMR_CH, tmr_counter_type_normal);
    }
    if (s & TMR_CH_CAP_STAT_MASK(TMR_CH)) {
        capture_is_done = true;
        count = tmr_channel_get_counter(TMR, TMR_CH, target_counter);
    }
}

void isr_tmr_gpio(void)
{
    volatile uint32_t s = TMR->SR;
    TMR->SR = s;
    gpio_toggle_pin_level(BOARD_LED_GPIO_CTRL, BOARD_LED_GPIO_INDEX,
                        BOARD_LED_GPIO_PIN);
}
SDK_DECLARE_EXT_ISR_M(TMR_IRQ, isr_tmr_gpio)

static void enable_peripherals(void)
{
    hpm_stat_t stat = sysctl_enable_group_resource(HPM_SYSCTL, SYSCTL_RESOURCE_GROUP3,
                                 sysctl_resource_gtmr0, true);
    if (stat != status_success) {
        printf("failed to enable gtmr0 for group%x\n", SYSCTL_RESOURCE_GROUP3);
        return;
    }

    sysctl_set_cpu_affiliate(HPM_SYSCTL, SYSCTL_CPU_CPU0, SYSCTL_RESOURCE_GROUP3);
}

static void test_compare(void)
{
    tmr_channel_config_t config;
    tmr_channel_get_default_config(TMR, &config);
    config.cmp[0] = 0x1000;

    tmr_enable_irq(TMR, TMR_CH_CMP_IRQ_MASK(TMR_CH, TMR_CMP), true);
    tmr_channel_config(TMR, TMR_CH, &config, true);
    intc_m_enable_irq_with_priority(TMR_IRQ, 1);

    while(!time_is_up) {
        __asm("nop");
    }
    time_is_up = false;
    intc_m_disable_irq(TMR_IRQ);
    printf("time is up: %x\n", count);
}

static void test_capture(void)
{
    tmr_channel_config_t config;
    tmr_channel_get_default_config(TMR, &config);
    config.mode = tmr_work_mode_capture_at_rising_edge;
    target_counter = tmr_counter_type_rising_edge;

    tmr_enable_irq(TMR, TMR_CH_CAP_IRQ_MASK(TMR_CH), true);
    tmr_channel_config(TMR, TMR_CH, &config, true);
    intc_m_enable_irq_with_priority(TMR_IRQ, 1);

    while(!capture_is_done) {
        __asm("nop");
    }
    capture_is_done = false;
    intc_m_disable_irq(TMR_IRQ);
    printf("captured rising edge: %x\n", count);
}

static void test_measure(void)
{
    tmr_channel_config_t config;
    tmr_channel_get_default_config(TMR, &config);
    config.mode = tmr_work_mode_measure_width;

    tmr_enable_irq(TMR, TMR_CH_CAP_IRQ_MASK(TMR_CH), true);
    tmr_channel_config(TMR, TMR_CH, &config, true);
    intc_m_enable_irq_with_priority(TMR_IRQ, 1);

    while(!capture_is_done) {
        __asm("nop");
    }
    capture_is_done = false;
    intc_m_disable_irq(TMR_IRQ);
    printf("measured period: %x\n", tmr_channel_get_counter(TMR, TMR_CH, tmr_counter_type_measured_period));
    printf("measured duty cycle: %x\n", tmr_channel_get_counter(TMR, TMR_CH, tmr_counter_type_measured_duty_cycle));

}

static void test_generate_pwm_waveform_edge_aligned(void)
{
    uint32_t duty_step, duty, cmp;
    uint32_t reload = 0x10000;
    bool increase_duty_cycle;

    tmr_channel_config_t config;
    tmr_channel_get_default_config(TMR, &config);

    config.reload = reload;
    config.cmp[0] = reload - 1;
    config.cmp_initial_polarity_high = false;
    tmr_channel_config(TMR, TMR_CH, &config, false);
    tmr_start_counter(TMR, TMR_CH);

    duty_step = TMR_PWM_DUTY_CYCLE_FP_MAX / 100;
    duty = TMR_PWM_DUTY_CYCLE_FP_MAX / 100;
    increase_duty_cycle = true;
    for (uint32_t i = 0; ; i++) {
        if (increase_duty_cycle) {
            if ((duty + duty_step) > TMR_PWM_DUTY_CYCLE_FP_MAX) {
                increase_duty_cycle = false;
                continue;
            }
            duty += duty_step;
        } else {
            if (duty < duty_step) {
                increase_duty_cycle = true;
                continue;
            }
            duty -= duty_step;
        }

        /* in case overflow */
        cmp = (uint64_t) (reload) * duty / TMR_PWM_DUTY_CYCLE_FP_MAX;
        tmr_update_cmp(TMR, TMR_CH, 0, cmp);
        delay(100);
    }
}

int main(void)
{
    board_init();
    enable_peripherals();
    init_gpio_pins();

    /* init_tmr_pins(); */
    printf("timer testing\n");
    /* test_generate_pwm_waveform_edge_aligned(); */
    /* test_compare(); */
    /* test_capture(); */
    /* test_measure(); */
    sysctl_set_cpu_lp_mode(HPM_SYSCTL, 0, cpu_lp_mode_trigger_system_lp);
    sysctl_set_cpu_retention(HPM_SYSCTL, 0, 0xFFFF);
    sysctl_resource_set_mode(HPM_SYSCTL, sysctl_resource_gtmr0, 1);
    sysctl_resource_set_mode(HPM_SYSCTL, sysctl_resource_xpi0, 1);
    sysctl_resource_set_mode(HPM_SYSCTL, sysctl_resource_gpio, 1);
    sysctl_resource_set_mode(HPM_SYSCTL, sysctl_resource_lmm0, 1);

    *(volatile uint32_t *)0xF4002880 = 0xFFFFFFFF;
    *(volatile uint32_t *)0xF4002884 = 0xFFFFFFFF;
    *(volatile uint32_t *)0xF4002888 = 0xFFFFFFFF;
    *(volatile uint32_t *)0xF400288C = 0xFFFFFFFF;
    gpio_enable_pin_output(BOARD_LED_GPIO_CTRL, BOARD_LED_GPIO_INDEX,
                           BOARD_LED_GPIO_PIN, true);

    tmr_channel_config_t config;
    tmr_channel_get_default_config(TMR, &config);

    config.reload = clock_get_frequency(TMR_CLK_NAME) / 10;
    /* config.enable_cmp_output = false; */
    tmr_channel_config(TMR, TMR_CH, &config, true);

    tmr_enable_irq(TMR, TMR_CH_RLD_IRQ_MASK(TMR_CH), true);
    intc_m_enable_irq_with_priority(TMR_IRQ, 1);

    gpio_toggle_pin_level(BOARD_LED_GPIO_CTRL, BOARD_LED_GPIO_INDEX,
                        BOARD_LED_GPIO_PIN);
    while(1) {
        __asm volatile("wfi");
    }

    while(1);
    return 0;
}
