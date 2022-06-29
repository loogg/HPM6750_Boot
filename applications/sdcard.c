#include <rtthread.h>
#include <dfs_fs.h>
#include <unistd.h>
#include "sdcard.h"
#include "common.h"

#define DBG_TAG "sdcard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define SDCARD_DEVICE_NAME "sd0"
#define SDCARD_ROOT        "/"
#define SDCARD_FIRM_PATH   "/rtthread.rbl"

enum {
    SDCARD_CHECK_STEP_NULL = 0,
    SDCARD_CHECK_STEP_FIND,
    SDCARD_CHECK_STEP_MOUNT_SUCCESS,
    SDCARD_CHECK_STEP_MOUNT_FAIL
};

#define FIRM_BUF_SIZE 512
static uint8_t _firm_buf[FIRM_BUF_SIZE];
static uint8_t _check_step = SDCARD_CHECK_STEP_FIND;

static void print_progress(size_t cur_size, size_t total_size) {
    static uint8_t progress_sign[100 + 1];
    uint8_t i, per = cur_size * 100 / total_size;

    if (per > 100) {
        per = 100;
    }

    for (i = 0; i < 100; i++) {
        if (i < per) {
            progress_sign[i] = '=';
        } else if (per == i) {
            progress_sign[i] = '>';
        } else {
            progress_sign[i] = ' ';
        }
    }

    progress_sign[sizeof(progress_sign) - 1] = '\0';

    LOG_I("\033[2A");
    LOG_I("OTA Write: [%s] %d%%", progress_sign, per);
}

int sdcard_update(void) {
    const struct fal_partition *download_part = g_system.download_part;
    firm_pkg_t *download_header = &g_system.download_header;
    int rc = RT_EOK, ret = RT_EOK;
    struct stat s = {0};
    uint32_t total_length = 0;
    int length = 0;

    if (_check_step != SDCARD_CHECK_STEP_MOUNT_SUCCESS) {
        LOG_W("sdcard is not mounted.");
        return -RT_ERROR;
    }

    if (stat(SDCARD_FIRM_PATH, &s) != 0) {
        LOG_W("get file information failed.");
        return -RT_ERROR;
    }

    if (s.st_size <= 0) {
        LOG_W("%s is a empty file.", SDCARD_FIRM_PATH);
        return -RT_ERROR;
    }

    if (s.st_size > download_part->len) {
        LOG_W("firm size (%d) is greater than (%d)", s.st_size, download_part->len);
        return -RT_ERROR;
    }

    LOG_I("firm file %s size: %d", SDCARD_FIRM_PATH, s.st_size);

    int fd = open(SDCARD_FIRM_PATH, O_RDONLY);
    if (fd < 0) {
        LOG_W("open %s file failed.", SDCARD_FIRM_PATH);
        return -RT_ERROR;
    }

    ret = -RT_ERROR;
    do {
        LOG_I("The partition \'%s\' is erasing.", download_part->name);

        rc = fal_partition_erase_all(download_part);

        if (rc < 0) break;
        LOG_I("The partition \'%s\' erase success.", download_part->name);

        do {
            length = read(fd, _firm_buf,
                          s.st_size - total_length > FIRM_BUF_SIZE ? FIRM_BUF_SIZE
                                                                   : s.st_size - total_length);
            if (length <= 0) break;

            rc = fal_partition_write(download_part, total_length, _firm_buf, length);
            if (rc <= 0) break;

            total_length += length;
            print_progress(total_length, s.st_size);
        } while (total_length < s.st_size);

        if (total_length < s.st_size) break;

        rc = check_part_firm(download_part, download_header);
        if (rc != RT_EOK) break;

        ret = RT_EOK;
    } while (0);

    close(fd);

    return ret;
}

int sdcard_check(void) {
    switch (_check_step) {
        case SDCARD_CHECK_STEP_FIND: {
            rt_device_t dev = rt_device_find(SDCARD_DEVICE_NAME);
            if (dev == RT_NULL) break;

            int rc = dfs_mount(SDCARD_DEVICE_NAME, SDCARD_ROOT, "elm", 0, 0);
            if (rc == RT_EOK) {
                LOG_I("sd card mount to '%s'", SDCARD_ROOT);
                _check_step = SDCARD_CHECK_STEP_MOUNT_SUCCESS;
            } else {
                LOG_W("sd card mount to '%s' failed!", SDCARD_ROOT);
                _check_step = SDCARD_CHECK_STEP_MOUNT_FAIL;
            }
        } break;

        case SDCARD_CHECK_STEP_MOUNT_SUCCESS:
            return RT_EOK;

        default:
            break;
    }

    return -RT_ERROR;
}
