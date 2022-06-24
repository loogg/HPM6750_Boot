/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "hpm_ov5640.h"

hpm_stat_t camera_device_init(camera_context_t *camera_context, camera_config_t *camera_config)
{
    hpm_stat_t stat = status_success;

#ifdef CAMREA_RESET_PWDN_CONFIGURABLE
    /* power  scequence */
    camera_context->reset_pin(true);
    camera_context->power_down_pin(true);
    camera_context->delay_ms(10);
    camera_context->power_down_pin(false);
    camera_context->delay_ms(10);
    camera_context->reset_pin(false);
    camera_context->delay_ms(20);
#endif

    stat = ov5640_soft_reset(camera_context);
    if (stat != status_success) {
        return stat;
    }
    camera_context->delay_ms(20);

    stat = ov5640_init(camera_context, camera_config);

    return stat;
}