#ifndef __BOOT_H
#define __BOOT_H
#include <stdint.h>
#include "common.h"

int check_part_firm(const struct fal_partition *part, firm_pkg_t *firm_pkg);
int firm_upgrade(const struct fal_partition *src_part, firm_pkg_t *src_header,
                 const struct fal_partition *app_part);
void boot_app_enable(void);
void boot_start_application(void);

#endif
