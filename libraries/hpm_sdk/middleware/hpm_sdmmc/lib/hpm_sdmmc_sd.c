/*
 * Copyright (c) 2021 - 2022 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "hpm_sdmmc_sd.h"
#include "hpm_l1c_drv.h"

#define SPEED_1Kbps (1000U)
#define SPEED_1Mbps (1000UL * 1000UL)

typedef union {
    uint32_t status_words[16];
    struct {
        uint32_t reserved[12];
        uint64_t : 8;
        uint64_t uhs_au_size: 4;
        uint64_t uhs_speed_grade: 4;
        uint64_t erase_offset: 2;
        uint64_t erase_timeout: 6;
        uint64_t erase_size: 16;
        uint64_t : 4;
        uint64_t au_size: 4;
        uint64_t performance_move: 8;
        uint64_t speed_class: 8;

        uint32_t size_of_protected_area;

        uint32_t sd_card_type: 16;
        uint32_t : 6;
        uint32_t : 7;
        uint32_t secured_mode: 1;
        uint32_t data_bus_width: 2;
    };
} sd_raw_status_t;

static uint32_t sd_be2le(uint32_t be);

static void sd_convert_data_endian(uint32_t *word, uint32_t word_count);

static hpm_stat_t sd_send_card_status(sd_card_t *card);


static hpm_stat_t sd_send_rca(sd_card_t *card);

static hpm_stat_t sd_send_csd(sd_card_t *card);

hpm_stat_t sd_switch_function(sd_card_t *card, uint32_t mode, uint32_t group, uint32_t number);

static void sd_decode_scr(sd_card_t *card, uint32_t *raw_scr);

static hpm_stat_t sd_send_scr(sd_card_t *card);

static hpm_stat_t sd_app_cmd_send_cond_op(sd_card_t *card, sd_ocr_t ocr);

static hpm_stat_t sd_send_if_cond(sd_card_t *card);

static hpm_stat_t sd_probe_bus_voltage(sd_card_t *card);

static hpm_stat_t sd_switch_voltage(sd_card_t *card);

static void sd_decode_csd(sd_card_t *card, uint32_t *raw_csd);

static void sd_decode_status(sd_card_t *card, uint32_t *raw_status);

static hpm_stat_t sd_set_bus_width(sd_card_t *card, sdmmc_buswidth_t buswidth);

static hpm_stat_t sd_set_bus_timing(sd_card_t *card);

static void sd_convert_data_endian(uint32_t *word, uint32_t word_count)
{
    for (uint32_t i = 0; i < word_count / 2; i++) {
        uint32_t temp = word[i];
        word[i] = word[word_count - i - 1];
        word[word_count - i - 1] = temp;
    }

    for (uint32_t i = 0; i < word_count; i++) {
        word[i] = sd_be2le(word[i]);
    }
}

static hpm_stat_t sd_send_card_status(sd_card_t *card)
{
    hpm_stat_t status;

    sdmmchost_cmd_t *cmd = &card->host->cmd;
    memset(cmd, 0, sizeof(*cmd));
    cmd->cmd_index = sdmmc_cmd_send_status;
    cmd->resp_type = sdmmc_resp_r1;
    cmd->cmd_argument = (uint32_t) card->relative_addr << 16;
    status = sdmmchost_send_command(card->host, cmd);
    if (status != status_success) {
        return status;
    }

    card->r1_status.status = cmd->response[0];

    return status;
}

static hpm_stat_t sd_switch_voltage(sd_card_t *card)
{
    hpm_stat_t status;

    sdmmchost_cmd_t *cmd = &card->host->cmd;
    memset(cmd, 0, sizeof(*cmd));
    cmd->cmd_index = sd_cmd_switch;
    cmd->resp_type = sdmmc_resp_r1;
    cmd->cmd_argument = 0;
    status = sdmmchost_send_command(card->host, cmd);


    return status;
}

static hpm_stat_t sd_send_if_cond(sd_card_t *card)
{
    sdmmchost_cmd_t *cmd = &card->host->cmd;
    (void) memset(cmd, 0, sizeof(sdmmchost_cmd_t));

    cmd->cmd_index = sd_cmd_send_if_cond;
    cmd->cmd_argument = 0x1aa;
    cmd->resp_type = sdmmc_resp_r7;

    hpm_stat_t status = sdmmchost_send_command(card->host, cmd);
    if (status != status_success) {
        return status;
    }

    if ((cmd->response[0] & 0xFFU) != 0xAAU) {
        return status_sdmmc_card_not_support;
    }

    return status_success;
}

static void sd_decode_csd(sd_card_t *card, uint32_t *raw_csd)
{
    sd_csd_t *csd = &card->csd;

    csd->csd_structure = (uint8_t) ((raw_csd[3U] & 0xC0000000U) >> 30U);
    csd->data_read_access_time1 = (uint8_t) ((raw_csd[3U] & 0xFF0000U) >> 16U);
    csd->data_read_access_time2 = (uint8_t) ((raw_csd[3U] & 0xFF00U) >> 8U);
    csd->transfer_speed = (uint8_t) (raw_csd[3U] & 0xFFU);
    csd->card_command_class = (uint16_t) ((raw_csd[2U] & 0xFFF00000U) >> 20U);
    csd->read_block_len = (uint8_t) ((raw_csd[2U] & 0xF0000U) >> 16U);

    if ((raw_csd[2U] & 0x8000U) != 0U) {
        csd->support_read_block_partial = true;
    }
    if ((raw_csd[2U] & 0x4000U) != 0U) {
        csd->support_write_block_misalignment = true;
    }
    if ((raw_csd[2U] & 0x2000U) != 0U) {
        csd->support_read_block_misalignment = true;
    }
    if ((raw_csd[2U] & 0x1000U) != 0U) {
        csd->is_dsr_implemented = true;
    }
    if (csd->csd_structure == 0U) {
        csd->device_size = (uint32_t) ((raw_csd[2U] & 0x3FFU) << 2U);
        csd->device_size |= (uint32_t) ((raw_csd[1U] & 0xC0000000U) >> 30U);
        csd->read_current_vdd_min = (uint8_t) ((raw_csd[1U] & 0x38000000U) >> 27U);
        csd->read_current_vdd_max = (uint8_t) ((raw_csd[1U] & 0x7000000U) >> 24U);
        csd->write_current_vdd_min = (uint8_t) ((raw_csd[1U] & 0xE00000U) >> 20U);
        csd->write_current_vdd_max = (uint8_t) ((raw_csd[1U] & 0x1C0000U) >> 18U);
        csd->device_size_multiplier = (uint8_t) ((raw_csd[1U] & 0x38000U) >> 15U);

        /* Get card total block count and block size. */
        card->block_count = ((csd->device_size + 1U) << (csd->device_size_multiplier + 2U));
        card->block_size = (1UL << (csd->read_block_len));
        if (card->block_size != SDMMC_BLOCK_SIZE_DEFAULT) {
            card->block_count *= card->block_size;
            card->block_size = SDMMC_BLOCK_SIZE_DEFAULT;
            card->block_count /= card->block_size;

            card->card_size_in_bytes = (csd->device_size + 1U) * (1UL << (csd->device_size_multiplier + 2U));
        }
    } else if (csd->csd_structure == 1U) {
        card->block_size = SDMMC_BLOCK_SIZE_DEFAULT;

        csd->device_size = (uint32_t) ((raw_csd[2U] & 0x3FU) << 16U);
        csd->device_size |= (uint32_t) ((raw_csd[1U] & 0xFFFF0000U) >> 16U);
        if (csd->device_size >= 0xFFFFU) {
            csd->support_sdxc = true;
        }

        card->block_count = ((csd->device_size + 1U) * 1024U);
        card->card_size_in_bytes = (uint64_t) (csd->device_size + 1U) * 512UL * 1024UL;
    } else {
        /* Unsupported csd version */
    }

    if ((uint8_t) ((raw_csd[1U] & 0x4000U) >> 14U) != 0U) {
        csd->is_erase_block_enabled = true;
    }
    csd->erase_sector_size = (1UL + ((raw_csd[1U] & 0x3F80U) >> 7U)) * card->block_size;
    csd->write_protect_group_size = (1UL + (raw_csd[1U] & 0x7FU)) * card->block_size;
    if ((uint8_t) (raw_csd[0U] & 0x80000000U) != 0U) {
        csd->is_write_protection_group_enabled = true;
    }
    csd->write_speed_factor = (uint8_t) ((raw_csd[0U] & 0x1C000000U) >> 26U);
    csd->max_write_block_len = 1UL << ((raw_csd[0U] & 0x3C00000U) >> 22U);
    if ((uint8_t) ((raw_csd[0U] & 0x200000U) >> 21U) != 0U) {
        csd->support_write_block_partial = true;
    }
    if ((uint8_t) ((raw_csd[0U] & 0x8000U) >> 15U) != 0U) {
        csd->support_file_format_group = true;
    }
    if ((uint8_t) ((raw_csd[0U] & 0x4000U) >> 14U) != 0U) {
        csd->support_copy = true;
    }
    if ((uint8_t) ((raw_csd[0U] & 0x2000U) >> 13U) != 0U) {
        csd->support_permanent_write_protect = true;
    }
    if ((uint8_t) ((raw_csd[0U] & 0x1000U) >> 12U) != 0U) {
        csd->support_temporary_write_protect = true;
    }
    csd->file_format = (uint8_t) ((raw_csd[0U] & 0xC00U) >> 10U);


    uint32_t tran_speed = raw_csd[3] & 0xFFU;
    uint32_t bitrate_unit = tran_speed & 0x7U;
    uint32_t time_value = (tran_speed >> 3) & 0xFU;
    const uint32_t bitrate_unit_list[8] = {100UL * SPEED_1Kbps, SPEED_1Mbps, 10U * SPEED_1Mbps, 100U * SPEED_1Mbps,
                                           0, 0, 0, 0};
    const uint32_t time_value_list[16] = {0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
    card->max_freq = bitrate_unit_list[bitrate_unit] / 10U * time_value_list[time_value];
}

static hpm_stat_t sd_send_csd(sd_card_t *card)
{
    sdmmchost_cmd_t *cmd = &card->host->cmd;
    (void) memset(cmd, 0, sizeof(sdmmchost_cmd_t));

    cmd->cmd_index = sd_cmd_send_csd;
    cmd->cmd_argument = (uint32_t) card->relative_addr << 16;
    cmd->resp_type = sdmmc_resp_r2;

    hpm_stat_t status = sdmmchost_send_command(card->host, cmd);
    if (status != status_success) {
        return status;
    }

    uint32_t temp_buf[4];
    memcpy(temp_buf, cmd->response, sizeof(temp_buf));
    sd_decode_csd(card, temp_buf);

    return status_success;
}


static hpm_stat_t sd_all_send_cid(sd_card_t *card)
{
    sdmmchost_cmd_t *cmd = &card->host->cmd;
    (void) memset(cmd, 0, sizeof(sdmmchost_cmd_t));

    cmd->cmd_index = sd_cmd_all_send_cid;
    cmd->resp_type = sdmmc_resp_r2;

    hpm_stat_t status = sdmmchost_send_command(card->host, cmd);
    if (status != status_success) {
        return status;
    }
    card->cid.cid_words[0] = cmd->response[0];
    card->cid.cid_words[1] = cmd->response[1];

    return status;
}

static hpm_stat_t sd_send_rca(sd_card_t *card)
{
    sdmmchost_cmd_t *cmd = &card->host->cmd;
    (void) memset(cmd, 0, sizeof(sdmmchost_cmd_t));

    cmd->cmd_index = sd_cmd_send_relative_addr;
    cmd->resp_type = sdmmc_resp_r6;

    hpm_stat_t status = sdmmchost_send_command(card->host, cmd);
    if (status != status_success) {
        return status;
    }
    card->relative_addr = cmd->response[0] >> 16;

    return status;
}

static hpm_stat_t sd_app_cmd_send_cond_op(sd_card_t *card, sd_ocr_t ocr)
{
    hpm_stat_t status = sdmmc_send_application_command(card->host, card->relative_addr);
    card->host->delay_ms(1);
    sdmmchost_cmd_t *cmd = &card->host->cmd;
    if (status == status_success) {
        (void) memset(cmd, 0, sizeof(sdmmchost_cmd_t));

        cmd->cmd_index = sd_acmd_sd_send_op_cond;
        cmd->cmd_argument = ocr.ocr_word;
        cmd->resp_type = sdmmc_resp_r3;
        status = sdmmchost_send_command(card->host, cmd);
    }
    return status;
}

static hpm_stat_t sd_probe_bus_voltage(sd_card_t *card)
{
    hpm_stat_t status = status_invalid_argument;

    do {
        card->host->delay_ms(1);
        status = sdmmc_go_idle_state(card->host);
        HPM_BREAK_IF(status != status_success);

        card->host->delay_ms(2);
        status = sd_send_if_cond(card);
        if (status == status_sdmmc_card_not_support) {
            status = sdmmc_go_idle_state(card->host);
            HPM_BREAK_IF(status != status_success);
        }

        sd_ocr_t ocr = {.ocr_word = 0x40ff8000};

        if (card->host->support_1v8) {
            ocr.switching_to_1v8_accepted = 1;
        }

        status = sd_app_cmd_send_cond_op(card, ocr);
        HPM_BREAK_IF(status != status_success);
        sdmmchost_cmd_t *cmd = &card->host->cmd;
        sd_ocr_t recv_ocr;
        recv_ocr.ocr_word = cmd->response[0];
        do {
            status = sd_app_cmd_send_cond_op(card, ocr);
            HPM_BREAK_IF(status != status_success);
            recv_ocr.ocr_word = cmd->response[0];

        } while (recv_ocr.card_power_up_status == 0);

        card->ocr.ocr_word = recv_ocr.ocr_word;


        if (card->host->support_1v8 && (card->ocr.switching_to_1v8_accepted != 0U)) {
            status = sd_switch_voltage(card);
            HPM_BREAK_IF(status != status_success);

        }
        if (card->host->support_1v8) {
            card->operation_voltage = sdmmc_operation_voltage_1v8;
        } else {
            card->operation_voltage = sdmmc_operation_voltage_3v3;
        }

    } while (false);

    return status;
}

uint32_t sd_be2le(uint32_t be)
{
    uint32_t value = be;

    uint8_t *be_u8 = (uint8_t *) &value;

    for (uint32_t i = 0; i < 2; i++) {
        uint8_t tmp = be_u8[i];
        be_u8[i] = be_u8[3 - i];
        be_u8[3 - i] = tmp;
    }

    return value;
}

static hpm_stat_t sd_send_scr(sd_card_t *card)
{
    hpm_stat_t status = status_invalid_argument;

    status = sdmmc_send_application_command(card->host, card->relative_addr);
    if (status != status_success) {
        return status;
    }

    sdmmchost_cmd_t *cmd = &card->host->cmd;
    (void) memset(cmd, 0, sizeof(*cmd));
    cmd->cmd_index = sd_acmd_send_scr;
    cmd->resp_type = sdmmc_resp_r1;
    sdmmchost_data_t *data = &card->host->data;
    memset(data, 0, sizeof(*data));
    data->block_size = 8;
    data->block_cnt = 1;
    data->rx_data = card->host->buffer;
    sdmmchost_xfer_t *content = &card->host->xfer;
    content->data = data;
    content->command = cmd;
    status = sdmmchost_transfer(card->host, content);
    if (status != status_success) {
        return status;
    }

    // Get SCR value
    sd_scr_t scr;
    scr.scr_word[0] = data->rx_data[0];
    scr.scr_word[1] = data->rx_data[1];
    sd_convert_data_endian(&scr.scr_word[0], ARRAY_SIZE(scr.scr_word));

    sd_decode_scr(card, &scr.scr_word[0]);

    return status;
}

void sd_decode_scr(sd_card_t *card, uint32_t *raw_scr)
{
    sd_scr_t *scr = &card->scr;

    scr->scr_word[0] = raw_scr[0];
    scr->scr_word[1] = raw_scr[1];

    if ((scr->sd_bus_widths & 0x04) != 0) {
        card->sd_flags.support_4bit_width = 1;
    }
    if (scr->support_cmd20 == 1) {
        card->sd_flags.support_speed_class_control_cmd = 1;
    }
    if (scr->support_cmd23) {
        card->sd_flags.support_set_block_count_cmd = 1;
    }
}


hpm_stat_t sd_init(sd_card_t *card)
{
    hpm_stat_t status = status_invalid_argument;
    do {
        HPM_BREAK_IF(card == NULL);

        if (!card->is_host_ready) {
            status = sd_host_init(card);
        } else {
            sdmmchost_reset(card->host);
        }

        while (!sd_is_card_present(card)) {}

        status = sd_card_init(card);

    } while (false);

    return status;
}

void sd_deinit(sd_card_t *card)
{

}


hpm_stat_t sd_card_init(sd_card_t *card)
{
    hpm_stat_t status = status_invalid_argument;
    do {
        sdmmchost_set_card_bus_width(card->host, sdxc_bus_width_1bit);
        sdmmchost_set_card_clock(card->host, SDMMC_CLOCK_400KHZ);
        card->host->delay_ms(10);
        status = sd_probe_bus_voltage(card);
        HPM_BREAK_IF(status != status_success);
        status = sd_all_send_cid(card);
        HPM_BREAK_IF(status != status_success);
        status = sd_send_rca(card);
        HPM_BREAK_IF(status != status_success);
        status = sd_send_csd(card);
        HPM_BREAK_IF(status != status_success);
        status = sd_select_card(card, true);
        sdmmchost_set_card_clock(card->host, SD_CLOCK_25MHZ);
        status = sd_send_scr(card);

        if (card->sd_flags.support_4bit_width != 0) {
            status = sd_set_bus_width(card, sdmmc_bus_width_4bit);
            if (status != status_success) {
                return status;
            }
            sdmmchost_set_card_bus_width(card->host, sdmmc_bus_width_4bit);
        }

        /* Try to get Card current status */

        /* Set block size */
        status = sdmmc_set_block_size(card->host, SDMMC_BLOCK_SIZE_DEFAULT);
        if (status != status_success) {
            return status;
        }
        /* Set bus timing  */
        status = sd_set_bus_timing(card);

    } while (false);

    return status;
}


static hpm_stat_t sd_set_bus_width(sd_card_t *card, sdmmc_buswidth_t buswidth)
{
    hpm_stat_t status = sdmmc_send_application_command(card->host, card->relative_addr);
    if (status != status_success) {
        return status;
    }

    sdmmchost_cmd_t *cmd = &card->host->cmd;
    memset(cmd, 0, sizeof(*cmd));
    cmd->cmd_index = sd_acmd_set_bus_width;
    cmd->resp_type = sdmmc_resp_r1;

    if (buswidth == sdmmc_bus_width_1bit) {
        cmd->cmd_argument = 0;
    } else if (buswidth == sdmmc_bus_width_4bit) {
        cmd->cmd_argument = 2;
    } else {
        return status_invalid_argument;
    }
    status = sdmmchost_send_command(card->host, cmd);

    return status;
}

static hpm_stat_t sd_set_bus_timing(sd_card_t *card)
{
    hpm_stat_t status = sd_switch_function(card, (uint32_t) sd_switch_function_mode_set,
                                           (uint32_t) sd_switch_function_group_access_mode,
                                           (uint32_t) sd_timing_sdr25_highspeed);
    if (status != status_success) {
        return status;
    }
    sdmmchost_set_card_clock(card->host, SD_CLOCK_50MHZ);
    return status;
}


void sd_card_deinit(sd_card_t *card);

hpm_stat_t sd_host_init(sd_card_t *card)
{
    hpm_stat_t status = status_success;
    assert(card != NULL);

    if (!card->is_host_ready) {
        card->host = sdmmchost_get_host();
        if (card->host == NULL) {
            return status_invalid_argument;
        }
        status = sdmmchost_init(card->host);
        if (status != status_success) {
            return status_fail;
        }
    }
    card->is_host_ready = true;

    return status_success;
}


bool sd_is_card_present(sd_card_t *card)
{
    return sdmmchost_is_card_detected(card->host);
}

bool sd_check_readonly(sd_card_t *card);

hpm_stat_t sd_select_card(sd_card_t *card, bool is_selected)
{
    return sdmmc_select_card(card->host, card->relative_addr, is_selected);
}

hpm_stat_t sd_read_status(sd_card_t *card)
{
    hpm_stat_t status = sdmmc_send_application_command(card->host, card->relative_addr);
    if (status != status_success) {
        return status;
    }

    sdmmchost_cmd_t *cmd = &card->host->cmd;
    sdmmchost_data_t *data = &card->host->data;
    sdmmchost_xfer_t *content = &card->host->xfer;
    cmd->cmd_index = sd_acmd_sd_status;
    cmd->resp_type = sdmmc_resp_r1;
    data->block_size = 64;
    data->block_cnt = 1;
    data->rx_data = card->host->buffer;
    content->data = data;
    content->command = cmd;
    status = sdmmchost_transfer(card->host, content);
    if (status != status_success) {
        return status;
    }
    uint32_t raw_status[16];
    memcpy(raw_status, card->host->buffer, 64);
    sd_convert_data_endian(raw_status, ARRAY_SIZE(raw_status));

    sd_decode_status(card, raw_status);

    return status;
}

static void sd_decode_status(sd_card_t *card, uint32_t *raw_status)
{
    sd_raw_status_t *sd_raw_status = (sd_raw_status_t *) raw_status;

    card->status.bus_width = sd_raw_status->data_bus_width;
    card->status.secure_mode = sd_raw_status->secured_mode;
    card->status.card_type = sd_raw_status->sd_card_type;
    card->status.protected_size = sd_raw_status->size_of_protected_area;
    switch (sd_raw_status->speed_class) {
    case 0:
        card->status.speed_class = 0;
        break;
    case 1:
        card->status.speed_class = 2;
        break;
    case 2:
        card->status.speed_class = 4;
        break;
    case 3:
        card->status.speed_class = 6;
        break;
    case 4:
        card->status.speed_class = 10;
        break;
    default:
        card->status.speed_class = 0;
        break;
    }
    card->status.performance_move = sd_raw_status->performance_move;
    card->status.au_size = (0x4000UL << sd_raw_status->au_size);
    card->status.erase_size = sd_raw_status->au_size * sd_raw_status->erase_size;
    card->status.uhs_au_size = (0x4000UL << sd_raw_status->uhs_au_size);
    card->status.erase_timeout = sd_raw_status->erase_timeout;
    card->status.uhs_speed_grade = sd_raw_status->uhs_speed_grade;
}

hpm_stat_t sd_read_blocks(sd_card_t *card, uint8_t *buffer, uint32_t start_block, uint32_t block_count)
{
    hpm_stat_t status;

    do {
        sdmmchost_cmd_t *cmd = &card->host->cmd;
        sdmmchost_data_t *data = &card->host->data;
        sdmmchost_xfer_t *content = &card->host->xfer;
        memset(cmd, 0, sizeof(*cmd));
        memset(data, 0, sizeof(*data));
        memset(content, 0, sizeof(*content));

        if (card->sd_flags.support_set_block_count_cmd != 0) {
            while (block_count > 0) {
                uint32_t read_block_count = (block_count >= 64) ? 64 : block_count;

                status = sdmmc_set_block_count(card->host, read_block_count);
                HPM_BREAK_IF(status != status_success);

                cmd->cmd_index = sdmmc_cmd_read_multiple_block;
                cmd->resp_type = sdmmc_resp_r1;
                cmd->cmd_argument = start_block;
                data->block_size = SDMMC_BLOCK_SIZE_DEFAULT;
                data->block_cnt = read_block_count;
                data->rx_data = (uint32_t *) sdmmc_get_sys_addr((uint32_t) buffer);
                content->data = data;
                content->command = cmd;
                uint32_t aligned_start = HPM_L1C_CACHELINE_ALIGN_DOWN((uint32_t) data->rx_data);
                uint32_t aligned_end = HPM_L1C_CACHELINE_ALIGN_UP((uint32_t) data->rx_data + card->block_size * block_count);
                uint32_t aligned_size = aligned_end - aligned_start;
                l1c_dc_flush(aligned_start, aligned_size);
                status = sdmmchost_transfer(card->host, content);
                l1c_dc_invalidate(aligned_start, aligned_size);
                if (status != status_success) {
                    break;
                }
                block_count -= read_block_count;
                start_block += read_block_count;
                buffer += SDMMC_BLOCK_SIZE_DEFAULT * read_block_count;
            }
        } else {
            while (block_count > 0) {
                cmd->cmd_index = sdmmc_cmd_read_single_block;
                cmd->resp_type = sdmmc_resp_r1;
                cmd->cmd_argument = start_block;
                data->block_size = SDMMC_BLOCK_SIZE_DEFAULT;
                data->block_cnt = 1;
                data->rx_data = (uint32_t *) sdmmc_get_sys_addr((uint32_t) buffer);
                content->data = data;
                content->command = cmd;
                uint32_t aligned_start = HPM_L1C_CACHELINE_ALIGN_DOWN((uint32_t) data->rx_data);
                uint32_t aligned_end = HPM_L1C_CACHELINE_ALIGN_UP((uint32_t) data->rx_data + card->block_size);
                uint32_t aligned_size = aligned_end - aligned_start;
                l1c_dc_flush(aligned_start, aligned_size);
                status = sdmmchost_transfer(card->host, content);
                l1c_dc_invalidate(aligned_start, aligned_size);
                if (status != status_success) {
                    break;
                }
                block_count--;
                start_block++;
                buffer += SDMMC_BLOCK_SIZE_DEFAULT;
            }
        }
    } while (false);

    return status;
}

hpm_stat_t sd_write_blocks(sd_card_t *card, const uint8_t *buffer, uint32_t start_block, uint32_t block_count)
{
    hpm_stat_t status;

    do {
        sdmmchost_cmd_t *cmd = &card->host->cmd;
        sdmmchost_data_t *data = &card->host->data;
        sdmmchost_xfer_t *content = &card->host->xfer;
        memset(cmd, 0, sizeof(*cmd));
        memset(data, 0, sizeof(*data));
        memset(content, 0, sizeof(*content));

        sd_polling_card_status_busy(card, 1000);

        if (card->sd_flags.support_set_block_count_cmd != 0) {
            while (block_count > 0) {

                uint32_t write_block_count = (block_count >= 64) ? 64 : block_count;

                status = sdmmc_set_block_count(card->host, write_block_count);
                HPM_BREAK_IF(status != status_success);

                cmd->cmd_index = sdmmc_cmd_write_multiple_block;
                cmd->resp_type = sdmmc_resp_r1;
                cmd->cmd_argument = start_block;
                data->block_size = SDMMC_BLOCK_SIZE_DEFAULT;
                data->block_cnt = write_block_count;
                data->tx_data = (const uint32_t *) sdmmc_get_sys_addr((uint32_t) buffer);
                content->data = data;
                content->command = cmd;
                uint32_t aligned_start = HPM_L1C_CACHELINE_ALIGN_DOWN((uint32_t) data->tx_data);
                uint32_t aligned_end = HPM_L1C_CACHELINE_ALIGN_UP((uint32_t) data->tx_data + card->block_size * write_block_count);
                uint32_t aligned_size = aligned_end - aligned_start;
                l1c_dc_flush(aligned_start, aligned_size);
                status = sdmmchost_transfer(card->host, content);
                if (status != status_success) {
                    break;
                }
                sd_polling_card_status_busy(card, 1000);
                block_count -= write_block_count;
                start_block += write_block_count;
                buffer += SDMMC_BLOCK_SIZE_DEFAULT * write_block_count;
            }
        } else {
            while (block_count > 0) {
                cmd->cmd_index = sdmmc_cmd_write_single_block;
                cmd->resp_type = sdmmc_resp_r1;
                cmd->cmd_argument = start_block;
                data->block_size = SDMMC_BLOCK_SIZE_DEFAULT;
                data->block_cnt = 1;
                data->tx_data = (const uint32_t *) sdmmc_get_sys_addr((uint32_t) buffer);
                content->data = data;
                content->command = cmd;
                uint32_t aligned_start = HPM_L1C_CACHELINE_ALIGN_DOWN((uint32_t) data->tx_data);
                uint32_t aligned_end = HPM_L1C_CACHELINE_ALIGN_UP((uint32_t) data->tx_data + card->block_size);
                uint32_t aligned_size = aligned_end - aligned_start;
                l1c_dc_flush(aligned_start, aligned_size);
                status = sdmmchost_transfer(card->host, content);
                if (status != status_success) {
                    break;
                }
                sd_polling_card_status_busy(card, 1000);
                block_count--;
                start_block++;
                buffer += SDMMC_BLOCK_SIZE_DEFAULT;
            }
        }
    } while (false);

    return status;
}

hpm_stat_t sd_erase_blocks(sd_card_t *card, uint32_t start_block, uint32_t block_count)
{
    hpm_stat_t status;

    do {
        sdmmchost_cmd_t *cmd = &card->host->cmd;
        memset(cmd, 0, sizeof(*cmd));
        sd_polling_card_status_busy(card, 1000);
        // Send erase start
        cmd->cmd_index = sd_cmd_erase_start;
        cmd->cmd_argument = start_block;
        cmd->resp_type = sdmmc_resp_r1;
        status = sdmmchost_send_command(card->host, cmd);
        HPM_BREAK_IF(status != status_success);
        // Send Erase end
        cmd->cmd_index = sd_cmd_erase_end;
        cmd->cmd_argument = start_block + block_count - 1U;
        status = sdmmchost_send_command(card->host, cmd);
        HPM_BREAK_IF(status != status_success);

        // Send erase command
        cmd->cmd_index = sdmmc_cmd_erase;
        cmd->cmd_argument = 0xFF;
        cmd->resp_type = sdmmc_resp_r1b;
        status = sdmmchost_send_command(card->host, cmd);
        HPM_BREAK_IF(status != status_success);

        // Wait until erase completed.
        sd_polling_card_status_busy(card, 1000);

    } while (false);

    return status;
}

hpm_stat_t sd_set_driver_strength(sd_card_t *card, sd_drive_strength_t driver_strength)
{

    return sd_switch_function(card, (uint32_t) sd_switch_function_mode_set,
                              (uint32_t) sd_switch_function_group_drive_strength, (uint32_t) driver_strength);
}

hpm_stat_t sd_set_max_current(sd_card_t *card, sd_max_current_t max_current)
{
    return sd_switch_function(card, (uint32_t) sd_switch_function_mode_set,
                              (uint32_t) sd_switch_function_group_power_limit, (uint32_t) max_current);
}

hpm_stat_t sd_polling_card_status_busy(sd_card_t *card, uint32_t timeout_ms)
{
    hpm_stat_t status = status_invalid_argument;
    bool is_busy = true;
    do {
        HPM_BREAK_IF(card == NULL);

        status = sd_send_card_status(card);

        if ((card->r1_status.status == sdmmc_state_program) || (card->r1_status.ready_for_data == 0U)) {
            is_busy = true;
            card->host->delay_ms(1);
        } else {
            is_busy = false;
        }

    } while (is_busy);

    return status;
}

hpm_stat_t sd_switch_function(sd_card_t *card, uint32_t mode, uint32_t group, uint32_t number)
{
    hpm_stat_t error = status_invalid_argument;

    do {

        HPM_BREAK_IF((card == NULL) || (group > 6) || (group < 1) || (number > 15U));

        sdmmchost_cmd_t *cmd = &card->host->cmd;
        sdmmchost_data_t *data = &card->host->data;
        sdmmchost_xfer_t *content = &card->host->xfer;
        memset(cmd, 0, sizeof(*cmd));
        memset(data, 0, sizeof(*data));
        memset(content, 0, sizeof(*content));

        cmd->cmd_index = sd_cmd_switch;
        cmd->resp_type = sdmmc_resp_r1;
        cmd->cmd_argument =
            (mode << 31) | (0x00FFFFFFUL & (~(0xFUL << ((group - 1U) * 4)))) | (number << ((group - 1U) * 4));
        data->block_size = sizeof(switch_function_status_t);
        data->block_cnt = 1;
        data->rx_data = (uint32_t *) sdmmc_get_sys_addr((uint32_t) &card->host->buffer);
        content->data = data;
        content->command = cmd;
        error = sdmmchost_transfer(card->host, content);
        if (error != status_success) {
            break;
        }

        memcpy(card->sfs.status_word, card->host->buffer, sizeof(switch_function_status_t));
        sd_convert_data_endian(card->sfs.status_word, ARRAY_SIZE(card->sfs.status_word));

    } while (false);

    return error;
}
