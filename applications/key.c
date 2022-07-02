#include "common.h"
#include <rtdevice.h>
#include "drv_gpio.h"

#define DBG_TAG "Key"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define KEY_PIN            GET_PIN(F, 1)
#define PRESS_DOWN_TIMEOUT 100

int key_process(void) {
    static uint8_t _init_ok = 0;
    static rt_tick_t tick_timeout = 0;
    static int pre_state = PIN_HIGH;

    if (!_init_ok) {
        rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
        _init_ok = 1;
    }

    int state = rt_pin_read(KEY_PIN);
    if (state == PIN_LOW) {
        if (pre_state == PIN_HIGH) {
            tick_timeout = rt_tick_get() + rt_tick_from_millisecond(PRESS_DOWN_TIMEOUT);
        }

        if (!g_system.is_remain) {
            if ((rt_tick_get() - tick_timeout) < (RT_TICK_MAX / 2)) {
                LOG_I("will enter boot");
                g_system.is_remain = 1;
            }
        }
    }
    pre_state = state;

    return RT_EOK;
}
