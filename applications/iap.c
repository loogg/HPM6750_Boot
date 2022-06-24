#include "common.h"
#include <board.h>
#include <agile_modbus.h>
#include "iap_slave.h"
#include <string.h>

#define DBG_TAG "IAP"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* 自动收发芯片 MAX13487 */
#define RS485_EN_RX()
#define RS485_EN_TX()

#define RS485_DEVICE_NAME "uart4"

static rt_sem_t _rx_notice = RT_NULL;
static rt_device_t _rs485_dev = RT_NULL;

static rt_err_t rs485_rx_ind(rt_device_t dev, rt_size_t size) {
    rt_sem_release(_rx_notice);

    return RT_EOK;
}

static int rs485_init(void) {
    /* RS485 EN GPIO */
    /* 自动收发芯片 MAX13487 */

    RS485_EN_RX();

    /* device init */
    _rx_notice = rt_sem_create("485_rx", 0, RT_IPC_FLAG_FIFO);
    if (_rx_notice == RT_NULL) {
        LOG_E("create rx sem failed.");
        return -RT_ERROR;
    }

    _rs485_dev = rt_device_find(RS485_DEVICE_NAME);
    if (_rs485_dev == RT_NULL) {
        LOG_E("find rs485 device (%s) failed.", RS485_DEVICE_NAME);
        rt_sem_delete(_rx_notice);
        return -RT_ERROR;
    }
    rt_device_set_rx_indicate(_rs485_dev, rs485_rx_ind);

    if (rt_device_open(_rs485_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK) {
        LOG_E("open rs485 device (%s) failed.", RS485_DEVICE_NAME);
        rt_sem_delete(_rx_notice);
        return -RT_ERROR;
    }

    LOG_I("init ok.");

    return RT_EOK;
}

static int rs485_send(uint8_t *buf, int len) {
    RS485_EN_TX();
    rt_device_write(_rs485_dev, 0, buf, len);
    RS485_EN_RX();

    return len;
}

static int rs485_receive(uint8_t *buf, int bufsz, int timeout) {
    int len = 0;

    while (1) {
        rt_sem_control(_rx_notice, RT_IPC_CMD_RESET, RT_NULL);

        int rc = rt_device_read(_rs485_dev, 0, buf + len, bufsz);
        if (rc > 0) {
            len += rc;
            bufsz -= rc;
            if (bufsz == 0) break;

            continue;
        }

        if (rt_sem_take(_rx_notice, rt_tick_from_millisecond(timeout)) != RT_EOK) break;
    }

    return len;
}

int iap_process(void) {
    static uint8_t _init_ok = 0;
    static uint8_t _ctx_send_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
    static uint8_t _ctx_read_buf[5000];
    static agile_modbus_rtu_t _ctx_rtu;
    static int _remain_length = 0;

    agile_modbus_t *ctx = &_ctx_rtu._ctx;

    if (!_init_ok) {
        if (rs485_init() != RT_EOK) return -RT_ERROR;

        agile_modbus_rtu_init(&_ctx_rtu, _ctx_send_buf, sizeof(_ctx_send_buf), _ctx_read_buf,
                              sizeof(_ctx_read_buf));
        agile_modbus_set_slave(ctx, 1);
        agile_modbus_set_compute_meta_length_after_function_cb(
            ctx, compute_meta_length_after_function_callback);
        agile_modbus_set_compute_data_length_after_meta_cb(ctx,
                                                           compute_data_length_after_meta_callback);

        _init_ok = 1;
    }

    uint8_t tmp_buf[100];
    int rt = rs485_receive(tmp_buf, sizeof(tmp_buf), 15);
    int read_len = rt;
    if (rt > (ctx->read_bufsz - _remain_length)) read_len = ctx->read_bufsz - _remain_length;

    if (read_len > 0) memcpy(ctx->read_buf + _remain_length, tmp_buf, read_len);

    int total_len = read_len + _remain_length;
    int is_reset = 0;

    if (total_len > 0 && read_len == 0) is_reset = 1;

    while (total_len > 0) {
        int frame_length = 0;
        int rc = agile_modbus_slave_handle(ctx, total_len, 1, slave_callback, &frame_length);
        if (rc >= 0) {
            ctx->read_buf = ctx->read_buf + frame_length;
            ctx->read_bufsz = ctx->read_bufsz - frame_length;
            _remain_length = total_len - frame_length;
            total_len = _remain_length;

            if (rc > 0) rs485_send(ctx->send_buf, rc);
        } else {
            int max_frame = 50;
            if (g_system.is_remain) max_frame = 4200;

            if (total_len > max_frame || is_reset == 1) {
                ctx->read_buf++;
                ctx->read_bufsz--;
                total_len--;
                continue;
            }

            if (ctx->read_buf != _ctx_read_buf) {
                for (int i = 0; i < total_len; i++) {
                    _ctx_read_buf[i] = ctx->read_buf[i];
                }
            }

            ctx->read_buf = _ctx_read_buf;
            ctx->read_bufsz = sizeof(_ctx_read_buf);

            _remain_length = total_len;

            break;
        }
    }

    if (total_len == 0) {
        _remain_length = 0;
        ctx->read_buf = _ctx_read_buf;
        ctx->read_bufsz = sizeof(_ctx_read_buf);
    }

    return RT_EOK;
}
