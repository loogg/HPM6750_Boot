#include "boot.h"
#include <board.h>
#include <fal.h>
#include <string.h>
#include "crc32.h"
#include <rthw.h>

#define DBG_TAG "boot"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

enum {
    BOOT_CRYPT_ALGO_NONE = 0x0L,      /**< no encryption algorithm and no compression algorithm */
    BOOT_CRYPT_ALGO_XOR = 0x1L,       /**< XOR encryption */
    BOOT_CRYPT_ALGO_AES256 = 0x2L,    /**< AES256 encryption */
    BOOT_CMPRS_ALGO_GZIP = 0x1L << 8, /**< Gzip: zh.wikipedia.org/wiki/Gzip */
    BOOT_CMPRS_ALGO_QUICKLZ = 0x2L << 8, /**< QuickLZ: www.quicklz.com */
    BOOT_CMPRS_ALGO_FASTLZ = 0x3L << 8,  /**< FastLZ: fastlz.org/ */

    BOOT_CRYPT_STAT_MASK = 0xFL,
    BOOT_CMPRS_STAT_MASK = 0xFL << 8,
};

#define FIRM_BUF_SIZE 4096
static uint8_t _firm_buf[FIRM_BUF_SIZE];

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

static int get_firm_header(const struct fal_partition *part, uint32_t off, firm_pkg_t *firm_pkg) {
    if (fal_partition_read(part, off, (uint8_t *)firm_pkg, sizeof(firm_pkg_t)) < 0) {
        LOG_E("Partition[%s] read head error!", part->name);
        return -RT_ERROR;
    }

    uint32_t calc_crc;
    crc32_ctx ctx;
    crc32_init(&ctx);
    crc32_update(&ctx, (uint8_t *)firm_pkg, sizeof(firm_pkg_t) - 4);
    crc32_final(&ctx, &calc_crc);
    if (calc_crc != firm_pkg->hdr_crc32) {
        LOG_E("Partition[%s] head CRC32 error!", part->name);
        return -RT_ERROR;
    }

    if (strncmp(firm_pkg->type, "RBL", 3) != 0) {
        LOG_E("Partition[%s] type[%s] not surport.", part->name, firm_pkg->type);
        return -RT_ERROR;
    }

    if ((firm_pkg->algo & BOOT_CRYPT_STAT_MASK) != BOOT_CRYPT_ALGO_NONE) {
        LOG_E("Not surpport crypt!");
        return -RT_ERROR;
    }

    if ((firm_pkg->algo & BOOT_CMPRS_STAT_MASK) != BOOT_CRYPT_ALGO_NONE) {
        LOG_E("Not surpport compress!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static int calc_part_firm_crc32(const struct fal_partition *part, uint32_t firm_len,
                                uint32_t firm_off, uint32_t *calc_crc) {
    int total_length = 0, length = 0;
    crc32_ctx ctx;
    if (((uint64_t)firm_len + firm_off) > part->len) return -RT_ERROR;

    crc32_init(&ctx);
    do {
        length = fal_partition_read(
            part, firm_off + total_length, _firm_buf,
            firm_len - total_length > FIRM_BUF_SIZE ? FIRM_BUF_SIZE : firm_len - total_length);
        if (length <= 0) return -RT_ERROR;
        crc32_update(&ctx, _firm_buf, length);
        total_length += length;

    } while (total_length < firm_len);

    crc32_final(&ctx, calc_crc);
    return RT_EOK;
}

int check_part_firm(const struct fal_partition *part, firm_pkg_t *firm_pkg) {
    uint32_t calc_crc, header_off, firm_off;
    header_off = 0;
    firm_off = sizeof(firm_pkg_t);
    if (strcmp(part->name, APP_PART_NAME) == 0) {
        header_off = part->len - sizeof(firm_pkg_t);
        firm_off = 0;
    }

    if (get_firm_header(part, header_off, firm_pkg) != RT_EOK) return -RT_ERROR;

    if (calc_part_firm_crc32(part, firm_pkg->raw_size, firm_off, &calc_crc) != RT_EOK)
        return -RT_ERROR;

    if (firm_pkg->body_crc32 != calc_crc) {
        LOG_E(
            "Get firmware header occur CRC32(calc.crc: %08X != hdr.info_crc32: %08X) error on "
            "\'%s\' partition!",
            calc_crc, firm_pkg->body_crc32, part->name);
        return -RT_ERROR;
    }
    LOG_I("Verify \'%s\' partiton(fw ver: %s, timestamp: %d) success.", part->name,
          firm_pkg->version_name, firm_pkg->time_stamp);
    return RT_EOK;
}

int firm_upgrade(const struct fal_partition *src_part, firm_pkg_t *src_header,
                 const struct fal_partition *app_part) {
    rt_err_t result = RT_EOK;
    firm_pkg_t app_header = {0};
    uint32_t total_length = 0;
    int length = 0;

    if ((src_header->raw_size + sizeof(firm_pkg_t)) > app_part->len) {
        LOG_W("The partition \'%s\' length is (%d), need (%d)!", app_part->name, app_part->len,
              src_header->raw_size + sizeof(firm_pkg_t));

        return RT_EOK;
    }

    result = check_part_firm(app_part, &app_header);

    if (result == RT_EOK)
        LOG_I("OTA firmware(%s) upgrade(%s->%s) startup.", app_part->name, app_header.version_name,
              src_header->version_name);
    else
        LOG_I("OTA firmware(%s) upgrade startup.", app_part->name);
    LOG_I("The partition \'%s\' is erasing.", app_part->name);

    result = fal_partition_erase_all(app_part);

    if (result < 0) return -RT_ERROR;
    LOG_I("The partition \'%s\' erase success.", app_part->name);

    do {
        length = fal_partition_read(src_part, sizeof(firm_pkg_t) + total_length, _firm_buf,
                                    src_header->raw_size - total_length > FIRM_BUF_SIZE
                                        ? FIRM_BUF_SIZE
                                        : src_header->raw_size - total_length);
        if (length <= 0) return -RT_ERROR;

        result = fal_partition_write(app_part, total_length, _firm_buf, length);

        if (result <= 0) return -RT_ERROR;

        total_length += length;
        print_progress(total_length, src_header->raw_size);
    } while (total_length < src_header->raw_size);

    if (fal_partition_write(app_part, app_part->len - sizeof(firm_pkg_t), (uint8_t *)src_header,
                            sizeof(firm_pkg_t)) < 0)
        return -RT_ERROR;

    result = check_part_firm(app_part, &app_header);
    return result;
}

void boot_app_enable(void) {
    rt_hw_interrupt_disable();
    BOOT_BKP = 0xA5A5;
    ppor_sw_reset(HPM_PPOR, 10);
}

void boot_start_application(void) {
    uint32_t bkp_data = BOOT_BKP;
    BOOT_BKP = 0;

    if (bkp_data != 0xA5A5) return;

    l1c_dc_writeback_all();
    l1c_dc_disable();
    l1c_ic_disable();
    fencei();
    __asm("la a0, %0" ::"i"(BOOT_APP_ADDR));
    __asm("jr a0");
}
