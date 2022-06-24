/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HPM_ADC16_DRV_H
#define HPM_ADC16_DRV_H

/*---------------------------------------------------------------------*
 * Includes
 *---------------------------------------------------------------------*/
#include "hpm_common.h"
#include "hpm_adc16_regs.h"
#include "hpm_soc_feature.h"

/**
 * @brief ADC16 driver APIs
 * @defgroup adc16_interface ADC16 driver APIs
 * @ingroup adc_interfaces
 * @{
 */

/*---------------------------------------------------------------------*
 *  Typedef Enum Declarations
 *---------------------------------------------------------------------*/

/** @brief Define ADC16 conversion modes. */
typedef enum {
   adc16_conv_mode_oneshot = 0,
    adc16_conv_mode_period,
    adc16_conv_mode_sequence,
    adc16_conv_mode_preempt
} adc16_conversion_mode_t;

/** @brief Define a trigger event of one trigger conversion. */
#define adc16_event_trig_complete       ADC16_INT_STS_TRIG_CMPT_MASK

/** @brief Define a DMA abort event in sequence mode . */
#define adc16_event_seq_dma_abort       ADC16_INT_STS_SEQ_DMAABT_MASK

/** @brief Define a complete event of the full of sequence conversions. */
#define adc16_event_seq_full_complete   ADC16_INT_STS_SEQ_CMPT_MASK

/** @brief Define a complete event of one of the full of sequence conversions. */
#define adc16_event_seq_single_complete ADC16_INT_STS_SEQ_CVC_MASK

/*---------------------------------------------------------------------*
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------*/
/** @brief ADC16 common configuration struct. */
typedef struct {
    uint8_t ch;
    uint8_t sample_cycle_shift;
    uint8_t conv_mode;
    uint8_t wait_dis;
    uint16_t thshdh;
    uint16_t thshdl;
    uint32_t sample_cycle;
    uint32_t adc_clk_div;
    uint16_t conv_duration;
    bool port3_rela_time;
    bool sel_sync_ahb;
    bool adc_ahb_en;
} adc16_config_t;

/** @brief ADC16 DMA configuration struct. */
typedef struct {
    uint32_t *start_addr;
    uint32_t size_in_4bytes;
    uint32_t stop_pos;
    bool stop_en;
} adc16_dma_config_t;

/** @brief ADC16 DMA configuration struct for sequence mode. */
typedef struct {
    uint32_t result    :16;
    uint32_t seq_num   :4;
    uint32_t           :4;
    uint32_t adc_ch    :5;
    uint32_t           :2;
    uint32_t cycle_bit :1;
} adc16_seq_dma_data_t;

/** @brief ADC16 DMA configuration struct for preemption mode. */
typedef struct {
    uint32_t result     :16;
    uint32_t trig_ch    :2;
    uint32_t            :2;
    uint32_t trig_index :4;
    uint32_t adc_ch     :5;
    uint32_t            :2;
    uint32_t cycle_bit  :1;
} adc16_preempt_dma_data_t;

/** @brief ADC16 configuration struct for period mode. */
typedef struct {
    uint32_t clk_src_in_hz;
    uint8_t ch;
    uint8_t prescale;
    uint8_t period_ms;
} adc16_prd_config_t;

/** @brief ADC16 queue configuration struct for sequence mode. */
typedef struct {
    bool seq_int_en;
    uint8_t ch;
} adc16_seq_queue_config_t;

/** @brief ADC16 configuration struct for sequence mode. */
typedef struct {
    adc16_seq_queue_config_t queue[ADC_SOC_MAX_SEQ_LEN];
    bool restart_en;
    bool cont_en;
    bool sw_trig_en;
    bool hw_trig_en;
    uint8_t seq_len;
} adc16_seq_config_t;

/** @brief ADC16 trigger configuration struct for preempt mode. */
typedef struct {
    bool    inten[ADC_SOC_MAX_TRIG_CH_LEN];
    uint8_t adc_ch[ADC_SOC_MAX_TRIG_CH_LEN];
    uint8_t trig_ch;
    uint8_t trig_len;
} adc16_trig_config_t;

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
 * @brief Get a default configuration for an adc16 instance.
 *
 * @param[out] config A pointer to the configuration struct of "adc16_config_t".
 *
 */
void adc16_init_default_config(adc16_config_t *config);

/**
 * @brief Initialize an adc16 instance.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] config A pointer to the configuration struct of "adc16_config_t".
 * @retval status_success Initialize an ADC instance successfully.
 * @retval status_invalid_argument Initialize an ADC instance unsuccessfully because of passing one or more invalid arguments.
 */
hpm_stat_t adc16_init(ADC16_Type *ptr, adc16_config_t *config);

/**
 * @brief Configure the periodic mode for an adc16 instance.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] config A pointer to the configuration struct of "adc16_prd_config_t".
 * @retval status_success Configure the periodic mode for an ADC instance successfully.
 * @retval status_invalid_argument Configure the periodic mode for an ADC instance unsuccessfully because of passing one or more invalid arguments.
 *
 */
hpm_stat_t adc16_set_period_config(ADC16_Type *ptr, adc16_prd_config_t *config);

/**
 * @brief Configure the sequence mode for an adc16 instance.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] config A pointer to configuration struct of "adc16_seq_config_t".
 * @retval status_success Configure the sequence mode for an ADC instance successfully.
 * @retval status_invalid_argument Configure the sequence mode for an ADC instance unsuccessfully because of passing one or more invalid arguments.
 */
hpm_stat_t adc16_set_sequence_config(ADC16_Type *ptr, adc16_seq_config_t *config);

/**
 * @brief Configure the preemption mode for an adc16 instance.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] config a pointer to configuration struct of "adc16_trig_config_t".
 * @retval status_success Configure the preemption mode for an ADC instance successfully.
 * @retval status_invalid_argument Configure the preemption mode for an ADC instance unsuccessfully because of passing one or more invalid arguments.
 */
hpm_stat_t adc16_set_preempt_config(ADC16_Type *ptr, adc16_trig_config_t *config);

/** @} */

/**
 * @name DMA Control
 * @{
 */

/**
 * @brief Configure the stop position offset in the specified memory for DMA write operation for sequence mode.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] stop_pos The stop position offset.
 */
static inline void adc16_set_seq_stop_pos(ADC16_Type *ptr, uint16_t stop_pos)
{
    ptr->SEQ_DMA_CFG = (ptr->SEQ_DMA_CFG & ~ADC16_SEQ_DMA_CFG_STOP_POS_MASK)
                     | ADC16_SEQ_DMA_CFG_STOP_POS_SET(stop_pos);
}

/**
 * @brief Configure the start address of DMA write operation for preemption mode.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] addr The start address of DMA write operation.
 */
static inline void adc16_init_preempt_dma(ADC16_Type *ptr, uint32_t addr)
{
    ptr->TRG_DMA_ADDR = addr & ADC16_TRG_DMA_ADDR_TRG_DMA_ADDR_MASK;
}

/**
 * @brief Configure the start address of DMA write operation for preemption mode.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] config A pointer to configuration struct of "adc16_dma_config_t".
 */
void adc16_init_seq_dma(ADC16_Type *ptr, adc16_dma_config_t *config);

/** @} */

/**
 * @name Status
 * @{
 */

/**
 * @brief Get ADC status flags.
 *
 * This function gets all ADC status flags.
 * @param[in] ptr An adc16 peripheral base address.
 * @retval Status The adc16 interrupt status flags.
 */
static inline uint32_t adc16_get_status_flags(ADC16_Type *ptr)
{
    return ptr->INT_STS;
}

/**
 * @brief Get the setting value of wait disable.
 *
 * This status flag is only used when wait_dis is set to disable.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @retval Status It means whether the current setting of wait disable is disable.
 */
static inline bool adc16_get_wait_dis_status(ADC16_Type *ptr)
{
    return ADC16_BUF_CFG0_WAIT_DIS_GET(ptr->BUF_CFG0);
}

/**
 * @brief Get status flag of a Conversion.
 *
 * This status flag is only used when wait_dis is set to disable.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] ch An adc16 peripheral channel.
 * @retval Status It means  the current conversion is valid.
 */
static inline bool adc16_get_conv_valid_status(ADC16_Type *ptr, uint8_t ch)
{
    return ADC16_BUS_RESULT_VALID_GET(ptr->BUS_RESULT[ch]);
}

/**
 * @brief Clear status flags.
 *
 * Only the specified flags can be cleared by writing INT_STS register.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] mask Mask value for flags to be cleared. Refer to "_adc16_status_flags".
 */
static inline void adc16_clear_status_flags(ADC16_Type *ptr, uint32_t mask)
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
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] mask Mask value for interrupt events. Refer to "_adc16_interrupt_enable".
 */
static inline void adc16_enable_interrupts(ADC16_Type *ptr, uint32_t mask)
{
    ptr->INT_EN |= mask;
}

/**
 * @brief Disable interrupts.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] mask Mask value for interrupt events. Refer to "_adc16_interrupt_enable".
 */
static inline void adc16_disable_interrupts(ADC16_Type *ptr, uint32_t mask)
{
    ptr->INT_EN &= ~mask;
}

/** @} */

/**
 * @name Trigger and Conversion
 * @{
 */

/**
 * @brief Do a software trigger for sequence mode.
 *
 * @param[in] ptr An adc16 peripheral base address.
 *
 */
void adc16_trigger_seq_by_sw(ADC16_Type *ptr);

/**
 * @brief Get the result in oneshot mode.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] ch An adc16 peripheral channel.
 * @param[out] result The result of an adc16 conversion.
 *
 * @retval status_success Get the result of an adc16 conversion in oneshot mode successfully.
 * @retval status_invalid_argument Get the result of an adc16 conversion in oneshot mode unsuccessfully because of passing invalid arguments.
 */
hpm_stat_t adc16_get_oneshot_result(ADC16_Type *ptr, uint8_t ch, uint16_t *result);

/**
 * @brief Get the result in periodic mode.
 *
 * @param[in] ptr An adc16 peripheral base address.
 * @param[in] ch An adc16 peripheral channel.
 * @param[out] result The result of an adc16 conversion.
 *
 * @retval status_success Get the result of an adc16 conversion in periodic mode successfully.
 * @retval status_invalid_argument Get the result of an adc16 conversion in periodic mode unsuccessfully because of passing invalid arguments.
 */
hpm_stat_t adc16_get_prd_result(ADC16_Type *ptr, uint8_t ch, uint16_t *result);

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* HPM_ADC16_DRV_H */