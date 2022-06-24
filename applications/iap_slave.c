#include "iap_slave.h"
#include "common.h"

#define DBG_TAG "IAP_Slave"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define AGILE_MODBUS_FC_IAP 0x50

enum {
    IAP_CMD_NULL = 0,
    IAP_CMD_SYNC,
    IAP_CMD_CHECK,
    IAP_CMD_START,
    IAP_CMD_WRITE,
    IAP_CMD_UPDATE
};

enum { IAP_STEP_NULL = 0, IAP_STEP_START, IAP_STEP_WRITE, IAP_STEP_UPDATE };

static uint8_t _iap_step = IAP_STEP_NULL;
static uint32_t _total_len = 0;
static uint32_t _write_len = 0;
static uint16_t _write_packet = 0;

uint8_t compute_meta_length_after_function_callback(agile_modbus_t *ctx, int function,
                                                    agile_modbus_msg_type_t msg_type) {
    int length;

    if (msg_type == AGILE_MODBUS_MSG_INDICATION) {
        length = 0;
        if (function == AGILE_MODBUS_FC_IAP) length = 4;
    } else {
        /* MSG_CONFIRMATION */
        length = 1;
        if (function == AGILE_MODBUS_FC_IAP) length = 4;
    }

    return length;
}

int compute_data_length_after_meta_callback(agile_modbus_t *ctx, uint8_t *msg, int msg_length,
                                            agile_modbus_msg_type_t msg_type) {
    int function = msg[ctx->backend->header_length];
    int length;

    if (msg_type == AGILE_MODBUS_MSG_INDICATION) {
        length = 0;
        if (function == AGILE_MODBUS_FC_IAP) {
            length =
                (msg[ctx->backend->header_length + 3] << 8) + msg[ctx->backend->header_length + 4];
        }
    } else {
        /* MSG_CONFIRMATION */
        length = 0;
        if (function == AGILE_MODBUS_FC_IAP) {
            length =
                (msg[ctx->backend->header_length + 3] << 8) + msg[ctx->backend->header_length + 4];
        }
    }

    return length;
}

/**
 * @brief   从机回调函数
 * @param   ctx modbus 句柄
 * @param   slave_info 从机信息体
 * @return  =0:正常;
 *          <0:异常
 *             (-AGILE_MODBUS_EXCEPTION_UNKNOW(-255): 未知异常，从机不会打包响应数据)
 *             (其他负数异常码: 从机会打包异常响应数据)
 */
int slave_callback(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info) {
    int function = slave_info->sft->function;
    if (function != AGILE_MODBUS_FC_IAP) {
        *(slave_info->rsp_length) = 0;
        return 0;
    }

    int send_index = slave_info->send_index;
    uint8_t *frame_ptr = slave_info->buf;
    int cmd = (frame_ptr[0] << 8) + frame_ptr[1];
    int data_len = (frame_ptr[2] << 8) + frame_ptr[3];
    uint8_t *data_ptr = frame_ptr + 4;

    if (!g_system.is_remain) {
        if (cmd != IAP_CMD_SYNC) {
            *(slave_info->rsp_length) = 0;
            return 0;
        }
    }

    /* RSP CMD */
    ctx->send_buf[send_index++] = frame_ptr[0];
    ctx->send_buf[send_index++] = frame_ptr[1];

    switch (cmd) {
        case IAP_CMD_SYNC: {
            g_system.sync_cmd_cnt++;
            if (g_system.sync_cmd_cnt >= 3) g_system.is_remain = 1;
            *(slave_info->rsp_length) = 0;
        } break;

        case IAP_CMD_CHECK: {
            ctx->send_buf[send_index++] = 0;
            ctx->send_buf[send_index++] = 0;
            *(slave_info->rsp_length) = send_index;
        } break;

        case IAP_CMD_START: {
            const struct fal_partition *app_part = g_system.app_part;
            uint32_t firm_len;

            if (data_len != 4) return -AGILE_MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

            firm_len = (((uint32_t)data_ptr[0] << 24) + ((uint32_t)data_ptr[1] << 16) +
                        ((uint32_t)data_ptr[2] << 8) + (uint32_t)data_ptr[3]);
            if (firm_len > app_part->len) return -AGILE_MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

            _total_len = firm_len;
            _write_len = 0;
            _write_packet = 0;

            LOG_I("start iap, frm_size:%u", _total_len);

            ctx->send_buf[send_index++] = 0;
            ctx->send_buf[send_index++] = 1;

            if (_iap_step == IAP_STEP_START) {
                LOG_I("has been erased.");
                ctx->send_buf[send_index++] = 1;
                *(slave_info->rsp_length) = send_index;
                break;
            }

            if (fal_partition_erase_all(app_part) <= 0) {
                LOG_E("erase %s partition failed.", app_part->name);
                ctx->send_buf[send_index++] = 0;
                *(slave_info->rsp_length) = send_index;
                break;
            }

            LOG_I("erase success.");
            _iap_step = IAP_STEP_START;
            ctx->send_buf[send_index++] = 1;
            *(slave_info->rsp_length) = send_index;
        } break;

        case IAP_CMD_WRITE: {
            const struct fal_partition *app_part = g_system.app_part;
            uint16_t packet_num = 0;
            uint16_t firm_len = 0;
            uint8_t *firm_ptr = RT_NULL;

            if (data_len < 4) return -AGILE_MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

            packet_num = (((uint16_t)data_ptr[0] << 8) + (uint16_t)data_ptr[1]);
            if (packet_num == 0) return -AGILE_MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

            firm_len = (((uint16_t)data_ptr[2] << 8) + (uint16_t)data_ptr[3]);
            if (data_len != (4 + firm_len)) return -AGILE_MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            firm_ptr = data_ptr + 4;

            ctx->send_buf[send_index++] = 0;
            ctx->send_buf[send_index++] = 3;

            if (packet_num == _write_packet) {
                LOG_W("packet_num(%u) has been written.", packet_num);
                ctx->send_buf[send_index++] = (_write_packet >> 8) & 0xff;
                ctx->send_buf[send_index++] = _write_packet & 0xff;
                ctx->send_buf[send_index++] = 1;
                *(slave_info->rsp_length) = send_index;
                break;
            }

            if (packet_num < _write_packet) {
                LOG_E("packet_num(%u) is less than write_packet(%u)", packet_num, _write_packet);
                ctx->send_buf[send_index++] = (_write_packet >> 8) & 0xff;
                ctx->send_buf[send_index++] = _write_packet & 0xff;
                ctx->send_buf[send_index++] = 0;
                *(slave_info->rsp_length) = send_index;
                break;
            }

            _write_packet = packet_num;

            if (_write_len >= _total_len) {
                LOG_W("write_len(%u) is >= total_len(%u)", _write_len, _total_len);
                ctx->send_buf[send_index++] = (_write_packet >> 8) & 0xff;
                ctx->send_buf[send_index++] = _write_packet & 0xff;
                ctx->send_buf[send_index++] = 1;
                *(slave_info->rsp_length) = send_index;
                break;
            }

            if ((_total_len - _write_len) < firm_len) {
                LOG_W("write_len(%u) + firm_len(%u) is greater than total_len(%u)", _write_len,
                      firm_len, _total_len);
                firm_len = _total_len - _write_len;
            }

            _iap_step = IAP_STEP_WRITE;

            if (fal_partition_write(app_part, _write_len, firm_ptr, firm_len) <= 0) {
                LOG_E("write %s partition failed.", app_part->name);
                ctx->send_buf[send_index++] = (_write_packet >> 8) & 0xff;
                ctx->send_buf[send_index++] = _write_packet & 0xff;
                ctx->send_buf[send_index++] = 0;
                *(slave_info->rsp_length) = send_index;
                break;
            }

            _write_len += firm_len;

            ctx->send_buf[send_index++] = (_write_packet >> 8) & 0xff;
            ctx->send_buf[send_index++] = _write_packet & 0xff;
            ctx->send_buf[send_index++] = 1;
            *(slave_info->rsp_length) = send_index;
        } break;

        case IAP_CMD_UPDATE: {
            g_system.is_quit = 1;
            _iap_step = IAP_STEP_UPDATE;
            ctx->send_buf[send_index++] = 0;
            ctx->send_buf[send_index++] = 0;
            *(slave_info->rsp_length) = send_index;
        } break;

        default: {
            *(slave_info->rsp_length) = 0;
        } break;
    }

    return 0;
}
