#include <webnet.h>
#include <wn_module.h>
#include <string.h>
#include <stdio.h>
#include <fal.h>
#include "common.h"

#define DBG_TAG "web.firm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static int file_size = 0;
static uint8_t update_ok = 0;

static const char *get_file_name(struct webnet_session *session) {
    const char *path = RT_NULL, *path_last = RT_NULL;

    path_last = webnet_upload_get_filename(session);
    if (path_last == RT_NULL) {
        LOG_W("file name err!!");
        return RT_NULL;
    }

    path = strrchr(path_last, '\\');
    if (path != RT_NULL) {
        path++;
        path_last = path;
    }

    path = strrchr(path_last, '/');
    if (path != RT_NULL) {
        path++;
        path_last = path;
    }

    return path_last;
}

static int upload_open(struct webnet_session *session) {
    const char *file_name = RT_NULL;
    const struct fal_partition *using_part = RT_NULL;

    file_size = 0;
    update_ok = 0;

    file_name = get_file_name(session);
    if (file_name == RT_NULL) return RT_NULL;

    LOG_D("Upload FileName: %s", file_name);
    LOG_D("Content-Type   : %s", webnet_upload_get_content_type(session));

    if (strstr(file_name, ".bin")) {
        LOG_I("using app part");
        using_part = g_system.app_part;
    } else if (strstr(file_name, ".rbl")) {
        LOG_I("using download part");
        using_part = g_system.download_part;
    } else {
        LOG_W("Unsupported file type.");
        return RT_NULL;
    }

    LOG_I("The partition \'%s\' is erasing.", using_part->name);
    int len = fal_partition_erase_all(using_part);
    if (len <= 0) {
        LOG_W("The partition \'%s\' erase failed.", using_part->name);
        return RT_NULL;
    }
    LOG_I("The partition \'%s\' erase success.", using_part->name);

    update_ok = 1;

    return (int)using_part;
}

static int upload_close(struct webnet_session *session) { return 0; }

static int upload_write(struct webnet_session *session, const void *data, rt_size_t length) {
    if (update_ok == 0) return 0;

    const struct fal_partition *using_part =
        (const struct fal_partition *)webnet_upload_get_userdata(session);

    if (using_part == RT_NULL) {
        LOG_W("using partition NULL");
        update_ok = 0;
        return 0;
    }

    if (file_size + length > using_part->len) {
        LOG_W("file size is too large");
        update_ok = 0;
        return 0;
    }

    int len = fal_partition_write(using_part, file_size, data, length);
    if (len <= 0) {
        LOG_W("write error");
        update_ok = 0;
        return 0;
    }

    file_size += length;

    return length;
}

static int upload_done(struct webnet_session *session) {
    const char *mimetype;

    LOG_I("Upload done.");

    char tmp[100] = "";
    snprintf(tmp, sizeof(tmp), "{\"code\":%d,\"filesize\":%d}", update_ok ? 0 : -1, file_size);

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", rt_strlen(tmp));
    webnet_session_printf(session, tmp);

    g_system.is_quit = 1;

    return 0;
}

const struct webnet_module_upload_entry upload_entry_firm = {"/firm", upload_open, upload_close,
                                                             upload_write, upload_done};
