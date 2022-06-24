/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HPM_ADC12_DRV_H
#define HPM_ADC12_DRV_H

/*---------------------------------------------------------------------*
 * Includes
 *---------------------------------------------------------------------*/
#include "hpm_common.h"
#include "hpm_adc12_regs.h"
#include "hpm_soc_feature.h"

/**
 * @brief ADC12 driver APIs
 * @defgroup adc12_interface ADC12 driver APIs
 * @ingroup adc_interfaces
 * @{
 */

/*---------------------------------------------------------------------*
 *  Typedef Enum Declarations
 *---------------------------------------------------------------------*/

/** @brief Define ADC12 sample signal types. */
typedef enum {
    adc12_sample_signal_single_ended = 0,
    adc12_sample_signal_differential
} adc12_sample_signal_t;

/** @brief Define ADC12 resolutions. */
typedef enum {
    adc12_res_6_bits = 0,
    adc12_res_8_bits,
    adc12_res_10_bits,
    adc12_res_12_bits
} adc12_resolution_t;

/** @brief Define ADC12 conversion modes. */
typedef enum {
    adc12_conv_mode_oneshot = 0,
    adc12_conv_mode_period,
    adc12_conv_mode_sequence,
    adc12_conv_mode_preempt
} adc12_conversion_mode_t;

/** @brief Define a trigger event of one trigger conversion. */
#define adc12_event_trig_complete       ADC12_INT_STS_TRIG_CMPT_MASK

/** @brief Define a DMA abort event in sequence mode . */
#define adc12_event_seq_dma_abort       ADC12_INT_STS_SEQ_DMAABT_MASK

/** @brief Define a complete event of the full of sequence conversions. */
#define adc12_event_seq_full_complete   ADC12_INT_STS_SEQ_CMPT_MASK

/** @brief Define a complete event of one of the full of sequence conversions. */

#define adc12_event_seq_single_complete ADC12_INT_STS_SEQ_CVC_MASK

/*---------------------------------------------------------------------*
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------*/

/** @brief ADC12 common configuration struct. */
typedef struct {
    uint8_t ch;
    uint8_t diff_sel;
    uint8_t res;
    uint8_t sample_cycle_shift;
    uint8_t conv_mode;
    uint8_t wait_dis;
    uint16_t thshdh;
    uint16_t thshdl;
    uint32_t sample_cycle;
    uint32_t adc_clk_div;
    bool sel_sync_ahb;
    bool adc_ahb_en;
} adc12_config_t;

/** @brief ADC12 DMA configuration struct. */
typedef struct {
    uint32_t *start_addr;
    uint32_t size_in_4bytes;
    uint32_t stop_pos;
    bool stop_en;
} adc12_dma_config_t;

/** @brief ADC12 DMA configuration struct for sequence mode. */
typedef struct {
    uint32_t           :4;
    uint32_t result    :12;
    uint32_t seq_num   :4;
    uint32_t           :4;
    uint32_t adc_ch    :5;
    uint32_t           :2;
    uint32_t cycle_bit :1;
} adc12_seq_dma_data_t;

/** @brief ADC12 DMA configuration struct for preemption mode. */
typedef struct {
    uint32_t            :4;
    uint32_t result     :12;
    uint32_t trig_ch    :2;
    uint32_t            :2;
    uint32_t trig_index :4;
    uint32_t adc_ch     :5;
    uint32_t            :2;
    uint32_t cycle_bit  :1;
} adc12_preempt_dma_data_t;

/** @brief ADC12 configuration struct for period mode. */
typedef struct {
    uint32_t clk_src_freq_in_hz;
    uint8_t ch;
    uint8_t prescale;
    uint8_t period;
} adc12_prd_config_t;

/** @brief ADC12 queue configuration struct for sequence mode. */
typedef struct {
    bool seq_int_en;
    uint8_t ch;
} adc12_seq_queue_config_t;

/** @brief ADC12 configuration struct for sequence mode. */
typedef struct {
    adc12_seq_queue_config_t queue[ADC_SOC_MAX_SEQ_LEN];
    bool restart_en;
    bool cont_en;
    bool sw_trig_en;
    bool hw_trig_en;
    uint8_t seq_len;
} adc12_seq_config_t;

/** @brief ADC12 trigger configuration struct for preempt mode. */
typedef struct {
    bool    inten[ADC_SOC_MAX_TRIG_CH_LEN];
    uint8_t adc_ch[ADC_SOC_MAX_TRIG_CH_LEN];
    uint8_t trig_ch;
    uint8_t trig_len;
} adc12_trig_config_t;

/*---------------------------------------------------------------------*
 *  API Declarations
 *---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @name Initialization and Deinitialization
 * @{
 */

/**
 * @brief Get a default configuration for an ADC12 instance.
 *
 * @param[out] config A pointer to the configuration struct of "adc12_config_t".
 *
 */
void adc12_init_default_config(adc12_config_t *config);

/**
 * @brief Initialize an ADC12 instance.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] config A pointer to the configuration struct of "adc12_config_t".
 * @retval status_success Initialize an ADC instance successfully.
 * @retval status_invalid_argument Initialize an ADC instance unsuccessfully because of passing one or more invalid arguments.
 */
hpm_stat_t adc12_init(ADC12_Type *ptr, adc12_config_t *config);

/**
 * @brief Configure the periodic mode for an ADC12 instance.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] config A pointer to the configuration struct of "adc12_prd_config_t".
 * @retval status_success Configure the periodic mode for an ADC instance successfully.
 * @retval status_invalid_argument Configure the periodic mode for an ADC instance unsuccessfully because of passing one or more invalid arguments.
 *
 */
hpm_stat_t adc12_set_period_config(ADC12_Type *ptr, adc12_prd_config_t *config);

/**
 * @brief Configure the sequence mode for an ADC12 instance.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] config A pointer to configuration struct of "adc12_seq_config_t".
 * @retval status_success Configure the sequence mode for an ADC instance successfully.
 * @retval status_invalid_argument Configure the sequence mode for an ADC instance unsuccessfully because of passing one or more invalid arguments.
 */
hpm_stat_t adc12_set_sequence_config(ADC12_Type *ptr, adc12_seq_config_t *config);

/**
 * @brief Configure the preemption mode for an ADC12 instance.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] config a pointer to configuration struct of "adc12_trig_config_t".
 * @retval status_success Configure the preemption mode for an ADC instance successfully.
 * @retval status_invalid_argument Configure the preemption mode for an ADC instance unsuccessfully because of passing one or more invalid arguments.
 */
hpm_stat_t adc12_set_preempt_config(ADC12_Type *ptr, adc12_trig_config_t *config);

/** @} */

/**
 * @name DMA Control
 * @{
 */

/**
 * @brief Configure the stop position offset in the specified memory for DMA write operation for sequence mode.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] stop_pos The stop position offset.
 */
static inline void adc12_set_seq_stop_pos(ADC12_Type *ptr, uint16_t stop_pos)
{
    ptr->SEQ_DMA_CFG = (ptr->SEQ_DMA_CFG & ~ADC12_SEQ_DMA_CFG_STOP_POS_MASK)
                     | ADC12_SEQ_DMA_CFG_STOP_POS_SET(stop_pos);
}

/**
 * @brief Configure the start address of DMA write operation for preemption mode.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] addr The start address of DMA write operation.
 */
static inline void adc12_init_pmt_dma(ADC12_Type *ptr, uint32_t addr)
{
    ptr->TRG_DMA_ADDR = addr & ADC12_TRG_DMA_ADDR_TRG_DMA_ADDR_MASK;
}

/**
 * @brief Configure the start address of DMA write operation for preemption mode.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] config A pointer to configuration struct of "adc12_dma_config_t".
 */
void adc12_init_seq_dma(ADC12_Type *ptr, adc12_dma_config_t *config);

/** @} */

/**
 * @name Status
 * @{
 */

/**
 * @brief Get ADC status flags.
 *
 * This function gets all ADC status flags.
 * @param[in] ptr An ADC12 peripheral base address.
 * @retval Status The ADC12 interrupt status flags.
 */
static inline uint32_t adc12_get_status_flags(ADC12_Type *ptr)
{
    return ptr->INT_STS;
}

/**
 * @brief Get the setting value of wait disable.
 *
 * This status flag is only used when wait_dis is set to disable.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @retval Status It means whether the current setting of wait disable is disable.
 */
static inline bool adc12_get_wait_dis_status(ADC12_Type *ptr)
{
    return ADC12_BUF_CFG0_WAIT_DIS_GET(ptr->BUF_CFG0);
}

/**
 * @brief Get status flag of a conversion.
 *
 * This status flag is only used when wait_dis is set to disable.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] ch An ADC12 peripheral channel.
 * @retval Status It means  the current conversion is valid.
 */
static inline bool adc12_get_conv_valid_status(ADC12_Type *ptr, uint8_t ch)
{
    return ADC12_BUS_RESULT_VALID_GET(ptr->BUS_RESULT[ch]);
}

/**
 * @brief Clear status flags.
 *
 * Only the specified flags can be cleared by writing INT_STS register.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] mask Mask value for flags to be cleared. Refer to "_adc12_status_flags".
 */
static inline void adc12_clear_status_flags(ADC12_Type *ptr, uint32_t mask)
{
    ptr->INT_STS |= mask;
}

/** @} */

/**
 * @name Interrupts
 * @{
 */

/**
 * @brief Enable interrupts.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] mask Mask value for interrupt events. Refer to "_adc12_interrupt_enable".
 */
static inline void adc12_enable_interrupts(ADC12_Type *ptr, uint32_t mask)
{
    ptr->INT_EN |= mask;
}

/**
 * @brief Disable interrupts.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] mask Mask value for interrupt events. Refer to "_adc12_interrupt_enable".
 */
static inline void adc12_disable_interrupts(ADC12_Type *ptr, uint32_t mask)
{
    ptr->INT_EN &= ~mask;
}

/** @} */

/**
 * @name Trigger and Conversion
 * @{
 */

/**
 * @brief Get the result in oneshot mode.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] ch An ADC12 peripheral channel.
 * @param[out] result The result of an ADC12 conversion.
 *
 * @retval status_success Get the result of an ADC12 conversion in oneshot mode successfully.
 * @retval status_invalid_argument Get the result of an ADC12 conversion in oneshot mode unsuccessfully because of passing invalid arguments.
 */
hpm_stat_t adc12_get_oneshot_result(ADC12_Type *ptr, uint8_t ch, uint16_t *result);

/**
 * @brief Get the result in periodic mode.
 *
 * @param[in] ptr An ADC12 peripheral base address.
 * @param[in] ch An ADC12 peripheral channel.
 * @param[out] result The result of an ADC12 conversion.
 *
 * @retval status_success Get the result of an ADC12 conversion in periodic mode successfully.
 * @retval status_invalid_argument Get the result of an ADC12 conversion in periodic mode unsuccessfully because of passing invalid arguments.
 */
hpm_stat_t adc12_get_prd_result(ADC12_Type *ptr, uint8_t ch, uint16_t *result);

void adc12_trigger_seq_by_sw(ADC12_Type *ptr);

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* HPM_ADC12_DRV_H */
