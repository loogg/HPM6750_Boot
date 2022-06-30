#include "system.h"
#include "boot.h"
#include <board.h>
#include <rthw.h>
#include <dfs_fs.h>
#include <dfs_romfs.h>
#include "iap.h"
#include "sdcard.h"

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

    rc = dfs_mount(RT_NULL, "/", "rom", 0, &(romfs_root));
    if(rc != RT_EOK) {
        LOG_E("rom mount to '/' failed!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

void system_process(void) {
    static rt_tick_t _pre_tick = 0;

    if ((g_system.step == SYSTEM_STEP_WAIT_SYNC) || (g_system.step == SYSTEM_STEP_BOOT_PROCESS)) {
        if (iap_process() != RT_EOK) g_system.step = SYSTEM_STEP_ERROR;
        if (sdcard_check() == RT_EOK) g_system.step = SYSTEM_STEP_SDCARD;
    }

    switch (g_system.step) {
        case SYSTEM_STEP_INIT: {
            int rc = system_init();
            if (rc != RT_EOK) {
                g_system.step = SYSTEM_STEP_ERROR;
                break;
            }

            _pre_tick = rt_tick_get();
            g_system.step = SYSTEM_STEP_WAIT_SYNC;
        } break;

        case SYSTEM_STEP_WAIT_SYNC: {
            if (g_system.is_remain) {
                LOG_I("sync:%u tick, enter boot", rt_tick_get() - _pre_tick);
                g_system.step = SYSTEM_STEP_BOOT_PROCESS;
                break;
            }

            if ((rt_tick_get() - _pre_tick) >= rt_tick_from_millisecond(ENTER_BOOT_TIMEOUT)) {
                LOG_W("wait sync timeout:%u tick, will jump.", rt_tick_get() - _pre_tick);
                g_system.step = SYSTEM_STEP_UPDATE;
                break;
            }
        } break;

        case SYSTEM_STEP_BOOT_PROCESS: {
            if (g_system.is_quit) {
                LOG_I("will jump.");
                g_system.step = SYSTEM_STEP_UPDATE;
                break;
            }
        } break;

        case SYSTEM_STEP_SDCARD: {
            sdcard_update();
            g_system.step = SYSTEM_STEP_UPDATE;
        } break;

        case SYSTEM_STEP_UPDATE: {
            const struct fal_partition *app_part = g_system.app_part;
            const struct fal_partition *download_part = g_system.download_part;
            firm_pkg_t download_header = {0};

            int rc = check_part_firm(download_part, &download_header);
            if (rc == RT_EOK) {
                rc = firm_upgrade(download_part, &download_header, app_part);
                if (rc == RT_EOK) {
                    LOG_I("The partition \'%s\' is erasing.", download_part->name);
                    fal_partition_erase_all(download_part);
                    LOG_I("The partition \'%s\' erase success.", download_part->name);

                    boot_app_enable();
                } else {
                    LOG_E("firm update failed. now restart");
                    rt_hw_interrupt_disable();
                    ppor_sw_reset(HPM_PPOR, 10);
                }
            } else {
                firm_pkg_t app_header = {0};

                LOG_E("Get OTA \"%s\" partition firmware filed!", download_part->name);
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
