/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "hpm_common.h"
#include "tusb.h"
#include "diskio.h"
#include "ff.h"
#include "file_op.h"

ATTR_PLACE_AT_NONCACHEABLE_INIT char wbuff[50] = {"USB Host MSC FatFs Demo!"};
ATTR_PLACE_AT_NONCACHEABLE_INIT char rbuff[50] = {0};

#if CFG_TUH_MSC
ATTR_PLACE_AT_NONCACHEABLE static scsi_inquiry_resp_t inquiry_resp;
ATTR_PLACE_AT_NONCACHEABLE FATFS fatfs[CFG_TUSB_HOST_DEVICE_MAX];
ATTR_PLACE_AT_NONCACHEABLE FIL file;

FRESULT set_timestamp (
    char *obj,   /* Pointer to the file name */
    int year,
    int month,
    int mday,
    int hour,
    int min,
    int sec
)
{
    FILINFO fno;

    fno.fdate = (WORD)(((year - 1980) * 512U) | month * 32U | mday);
    fno.ftime = (WORD)(hour * 2048U | min * 32U | sec / 2U);

    return f_utime(obj, &fno);
}

bool file_system_mount(uint8_t dev_addr)
{
    char *filename = {"USBHost.txt"};
    char logic_drv_num[10] = {0};
    uint8_t phy_disk = dev_addr - 1;
    unsigned int cnt = 0;

    /* file system */
    disk_initialize(phy_disk);

    if (disk_is_ready(phy_disk)) {
        sprintf(logic_drv_num, "%d", phy_disk);

        if (f_mount(&fatfs[phy_disk], _T(logic_drv_num), 0) != FR_OK)
        {
            printf("FatFs mount failed!\n");
            return false;
        } else {
            printf("FatFs mount succeeded!\n");
        }

        f_chdrive(logic_drv_num); /* change to newly mounted drive */
        f_chdir("/");             /* set root directory as current directory */

        if(f_open(&file, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ) != FR_OK) {
            printf("Can't Open the %s file!\n", filename);
        } else {
            printf("The %s is open.\n", filename);

            /* write the specified file to U disk */
            f_write(&file, wbuff, strlen(wbuff), &cnt);
            f_close(&file);

            /* read the specified file from U disk */
            if(f_open(&file, filename, FA_READ) == FR_OK) {
                f_read(&file, rbuff, strlen(wbuff), &cnt);
                set_timestamp(filename, 2021, 8, 20, 13, 28, 30);
                f_close(&file);

                /* check the write file */
                if (memcmp(wbuff, rbuff, strlen(wbuff)) == 0) {
                    /* scan files in the specified directory of a U disk */
                    f_scan("/");
                    printf("Write the %s file successfully!\n", filename);
                } else {
                    printf("Write the %s file unsuccessfully!\n", filename);
                }
            }
        }
    }

    return true;
}

bool inquiry_complete_cb(uint8_t dev_addr, msc_cbw_t const* cbw, msc_csw_t const* csw)
{
    uint32_t block_count;
    uint32_t block_size;


    if (csw->status != 0)
    {
        printf("Inquiry failed\r\n");
        return false;
    }

    /* Print out Vendor ID, Product ID and Rev */
    printf("%.8s %.16s rev %.4s\r\n", inquiry_resp.vendor_id, inquiry_resp.product_id, inquiry_resp.product_rev);

    /* Get capacity of device */
    block_count = tuh_msc_get_block_count(dev_addr, cbw->lun);
    block_size = tuh_msc_get_block_size(dev_addr, cbw->lun);

    printf("Disk Size: %lu MB\r\n", block_count / ((1024*1024)/block_size));
    printf("Block Count = %lu, Block Size: %lu\r\n", block_count, block_size);

    file_system_mount(dev_addr);

  return true;
}

void tuh_msc_mount_cb(uint8_t dev_addr)
{
    uint8_t const lun = 0;

    printf("\nA MassStorage device is mounted.\r\n");
    tuh_msc_inquiry(dev_addr, lun, &inquiry_resp, inquiry_complete_cb);
}

void tuh_msc_unmount_cb(uint8_t dev_addr)
{
    uint8_t phy_disk = dev_addr-1;
    char logic_drv_num[10] = {0};

    printf("A MassStorage device is unmounted.\r\n");
    sprintf(logic_drv_num, "%d", phy_disk);
    if ( FR_OK == f_unmount(logic_drv_num)) {
        printf("FatFs unmount succeeded!\n");
        disk_deinitialize(phy_disk);
    }
}
#endif /* End of CFG_TUH_MSC */
