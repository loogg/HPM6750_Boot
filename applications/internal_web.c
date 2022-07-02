#include "internal_web.h"
#include <webnet.h>
#include <wn_module.h>

void internal_web_init(void) {
    extern const struct webnet_module_upload_entry upload_entry_firm;

    webnet_upload_add(&upload_entry_firm);

    webnet_init();
}
