#include "system.h"
#include "boot.h"
#include <board.h>
#include "iap.h"
#include <rthw.h>

#define DBG_TAG "system"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

g_system_t g_system = {0};

static int system_init(void) {
    int rc = fal_init();
    if (rc <= 0) return -RT_ERROR;

    g_system.app_part = fal_partition_find(APP_PART_NAME);
    if (g_system.app_part == RT_NULL) {
        LOG_E("App partition not find.");
        return -RT_ERROR;
    }

    g_system.download_part = fal_partition_find(DOWNLOAD_PART_NAME);
    if (g_system.download_part == RT_NULL) {
        LOG_E("Download partition not find.");
        return -RT_ERROR;
    }

    return RT_EOK;
}

void system_process(void) {
    static rt_tick_t _pre_tick = 0;

    if ((g_system.step == SYSTEM_STEP_WAIT_SYNC) || (g_system.step == SYSTEM_STEP_BOOT_PROCESS)) {
        if (iap_process() != RT_EOK) g_system.step = SYSTEM_STEP_ERROR;
    }

    switch (g_system.step) {
        case SYSTEM_STEP_INIT: {
            int rc = system_init();
            if (rc != RT_EOK) {
                g_system.step = SYSTEM_STEP_ERROR;
                break;
            }

            g_system.step++;
        } break;

        case SYSTEM_STEP_VERIFY_DOWNLOAD: {
            const struct fal_partition *app_part = g_system.app_part;
            const struct fal_partition *download_part = g_system.download_part;
            firm_pkg_t *download_header = &g_system.download_header;
            g_system.download_verify_rc = -RT_ERROR;

            int rc = check_part_firm(download_part, download_header);
            if (rc != RT_EOK) {
                LOG_E("Get OTA \"%s\" partition firmware filed!", download_part->name);
                _pre_tick = rt_tick_get();
                g_system.step++;
            } else {
                if ((download_header->raw_size + sizeof(firm_pkg_t)) > app_part->len) {
                    LOG_E("The partition \'%s\' length is (%d), need (%d)!", app_part->name,
                          app_part->len, download_header->raw_size + sizeof(firm_pkg_t));

                    LOG_I("The partition \'%s\' is erasing.", download_part->name);
                    fal_partition_erase_all(download_part);
                    LOG_I("The partition \'%s\' erase success.", download_part->name);

                    _pre_tick = rt_tick_get();
                    g_system.step++;
                    break;
                }

                g_system.download_verify_rc = RT_EOK;
                g_system.step = SYSTEM_STEP_UPDATE;
            }
        } break;

        case SYSTEM_STEP_WAIT_SYNC: {
            if (g_system.is_remain) {
                LOG_I("sync:%u tick, sync cmd cnt:%u, enter boot", rt_tick_get() - _pre_tick,
                      g_system.sync_cmd_cnt);
                g_system.step++;
                break;
            }

            if ((rt_tick_get() - _pre_tick) >= rt_tick_from_millisecond(ENTER_BOOT_TIMEOUT)) {
                LOG_W("wait sync timeout:%u tick, sync cmd cnt:%u, will jump.",
                      rt_tick_get() - _pre_tick, g_system.sync_cmd_cnt);
                g_system.step = SYSTEM_STEP_UPDATE;
                break;
            }
        } break;

        case SYSTEM_STEP_BOOT_PROCESS: {
            if (g_system.is_quit) {
                LOG_I("sync cmd cnt:%u, will jump.", g_system.sync_cmd_cnt);
                g_system.step++;
                break;
            }
        } break;

        case SYSTEM_STEP_UPDATE: {
            const struct fal_partition *app_part = g_system.app_part;
            const struct fal_partition *download_part = g_system.download_part;
            firm_pkg_t *download_header = &g_system.download_header;

            if (g_system.download_verify_rc == RT_EOK) {
                int rc = firm_upgrade(download_part, download_header, app_part);
                if (rc == RT_EOK) {
                    LOG_I("The partition \'%s\' is erasing.", download_part->name);
                    fal_partition_erase_all(download_part);
                    LOG_I("The partition \'%s\' erase success.", download_part->name);

                    boot_app_enable();
                } else {
                    rt_hw_interrupt_disable();
                    ppor_sw_reset(HPM_PPOR, 10);
                }
            } else {
                firm_pkg_t app_header = {0};
                int rc = check_part_firm(app_part, &app_header);
                if (rc != RT_EOK) LOG_W("Force the %s partition to run!", app_part->name);

                boot_app_enable();
            }
        } break;

        case SYSTEM_STEP_ERROR: {
            LOG_E("init error.");
            rt_thread_mdelay(1000);
        } break;

        default: {
            g_system.step = SYSTEM_STEP_ERROR;
        } break;
    }
}
