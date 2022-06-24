/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HPM_UART_DRV_H
#define HPM_UART_DRV_H
#include "hpm_common.h"
#include "hpm_uart_regs.h"

/**
 * 
 * @brief UART driver APIs
 * @defgroup uart_interface UART driver APIs
 * @ingroup io_interfaces
 * @{
 */

/**
 * @brief UART status
 */
enum {
    status_uart_no_suitable_baudrate_parameter_found = MAKE_STATUS(status_group_uart, 1),
};

/* @brief Parity */
typedef enum parity {
    parity_none = 0,
    parity_odd,
    parity_even,
    parity_always_1,
    parity_always_0,
} parity_setting_t;

/* @brief Stop bits */
typedef enum num_of_stop_bits {
    stop_bits_1 = 0,
    stop_bits_1_5,
    stop_bits_2,
} num_of_stop_bits_t;

/* @brief Word length */
typedef enum word_length {
    word_length_5_bits = 0,
    word_length_6_bits,
    word_length_7_bits,
    word_length_8_bits,
} word_length_t;

/* @brief UART fifo trigger levels */
typedef enum uart_fifo_trg_lvl {
    uart_rx_fifo_trg_not_empty = 0,
    uart_rx_fifo_trg_gt_one_quarter = 1,
    uart_rx_fifo_trg_gt_half = 2,
    uart_rx_fifo_trg_gt_three_quarters = 3,

    uart_tx_fifo_trg_not_full = 0,
    uart_tx_fifo_trg_lt_three_quarters = 1,
    uart_tx_fifo_trg_lt_half = 2,
    uart_tx_fifo_trg_lt_one_quarter = 3,
} uart_fifo_trg_lvl_t;

/* @brief UART signals */
typedef enum uart_signal {
    uart_signal_rts = UART_MCR_RTS_MASK,
} uart_signal_t;

/* @brief UART signal levels */
typedef enum uart_signal_level {
    uart_signal_level_high,
    uart_signal_level_low,
} uart_signal_level_t;

/* @brief UART modem status */
typedef enum uart_modem_stat {
    uart_modem_stat_cts = UART_MSR_CTS_MASK,
    uart_modem_stat_dcts_changed = UART_MSR_DCTS_MASK,
} uart_modem_stat_t;

/* @brief UART interrupt enable masks */
typedef enum uart_intr_enable {
    uart_intr_rx_data_avail_or_timeout = UART_IER_ERBI_MASK,
    uart_intr_tx_slot_avail = UART_IER_ETHEI_MASK,
    uart_intr_rx_line_stat = UART_IER_ELSI_MASK,
    uart_intr_modem_stat = UART_IER_EMSI_MASK,
} uart_intr_enable_t;

/* @brief UART interrupt IDs */
typedef enum uart_intr_id {
    uart_intr_id_modem_stat = 0x0,
    uart_intr_id_tx_slot_avail = 0x2,
    uart_intr_id_rx_data_avail = 0x4,
    uart_intr_id_rx_line_stat = 0x6,
    uart_intr_id_rx_timeout = 0xc,
} uart_intr_id_t;

/* @brief UART status */
typedef enum uart_stat {
    uart_stat_data_ready = UART_LSR_DR_MASK,
    uart_stat_overrun_error = UART_LSR_OE_MASK,
    uart_stat_parity_error = UART_LSR_PE_MASK,
    uart_stat_framing_error = UART_LSR_FE_MASK,
    uart_stat_line_break = UART_LSR_LBREAK_MASK,
    uart_stat_tx_slot_avail = UART_LSR_THRE_MASK,
    uart_stat_transmitter_empty = UART_LSR_TEMT_MASK,
    uart_stat_rx_fifo_error = UART_LSR_ERRF_MASK,
} uart_stat_t;

/**
 * @brief UART modem config 
 */
typedef struct uart_modem_config {
    bool auto_flow_ctrl_en;     /**< Auto flow control enable flag */
    bool loop_back_en;          /**< Loop back enable flag */
    bool set_rts_high;          /**< Set signal RTS level high flag */
} uart_modem_config_t;

/**
 * @brief UART config
 */
typedef struct uart_config {
 
    uint32_t src_freq_in_hz;            /**< Source clock frequency in Hz */
    uint32_t baudrate;                  /**< Baudrate */
    uint8_t num_of_stop_bits;           /**< Number of stop bits */
    uint8_t word_length;                /**< Word length */
    uint8_t parity;                     /**< Parity */
    uint8_t tx_fifo_level;              /**< TX Fifo level */
    uint8_t rx_fifo_level;              /**< RX Fifo level */
    bool dma_enable;                    /**< DMA Enable flag */
    bool fifo_enable;                   /**< Fifo Enable flag */
    uart_modem_config_t modem_config;   /**< Modem config */
} uart_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get fifo size
 *
 * @param ptr UART base address
 * @retval size of Fifo
 */
static inline uint8_t uart_get_fifo_size(UART_Type *ptr)
{
    return 16 << ((ptr->CFG & UART_CFG_FIFOSIZE_MASK) >> UART_CFG_FIFOSIZE_SHIFT);
}

/**
 * @brief Reset TX Fifo
 *
 * @param ptr UART base address
 */
static inline void uart_reset_tx_fifo(UART_Type *ptr)
{
    if (ptr->FCR & UART_FCR_FIFOE_MASK) {
        ptr->FCR |= UART_FCR_TFIFORST_MASK;
    }
}

/**
 * @brief Reset RX Fifo
 *
 * @param ptr UART base address
 */
static inline void uart_reset_rx_fifo(UART_Type *ptr)
{
    if (ptr->FCR & UART_FCR_FIFOE_MASK) {
        ptr->FCR |= UART_FCR_RFIFORST_MASK;
    }
}

/**
 * @brief Reset both TX and RX Fifo
 *
 * @param ptr UART base address
 */
static inline void uart_reset_all_fifo(UART_Type *ptr)
{
    if (ptr->FCR & UART_FCR_FIFOE_MASK) {
        ptr->FCR |= UART_FCR_RFIFORST_MASK | UART_FCR_TFIFORST_MASK;
    }
}

/**
 * @brief Enable modem loopback
 *
 * @param ptr UART base address
 */
static inline void uart_modem_enable_loopback(UART_Type *ptr)
{
    ptr->MCR |= UART_MCR_LOOP_MASK;
}

/**
 * @brief Disable modem loopback
 *
 * @param ptr UART base address
 */
static inline void uart_modem_disable_loopback(UART_Type *ptr)
{
    ptr->MCR &= ~UART_MCR_LOOP_MASK;
}

/**
 * @brief Disable modem auto flow control
 *
 * @param ptr UART base address
 */

static inline void uart_modem_disable_auto_flow_control(UART_Type *ptr)
{
    ptr->MCR &= ~UART_MCR_AFE_MASK;
}

/**
 * @brief Enable modem auto flow control
 *
 * @param ptr UART base address
 */
static inline void uart_modem_enable_auto_flow_control(UART_Type *ptr)
{
    ptr->MCR |= UART_MCR_AFE_MASK;
}

/**
 * @brief Configure modem
 *
 * @param ptr UART base address
 * @param config Pointer to modem config struct
 */
static inline void uart_modem_config(UART_Type *ptr, uart_modem_config_t *config)
{
    ptr->MCR = UART_MCR_AFE_SET(config->auto_flow_ctrl_en)
        | UART_MCR_LOOP_SET(config->loop_back_en)
        | UART_MCR_RTS_SET(!config->set_rts_high);
}

/**
 * @brief Get modem status
 *
 * @param ptr UART base address
 * @retval Current modem status
 */
static inline uint8_t uart_get_modem_status(UART_Type *ptr)
{
    return ptr->MSR;
}


/**
 * @brief Check modem status with given mask
 *
 * @param ptr UART base address
 * @param mask Status mask value to be checked against
 * @retval true if any bit in given mask is set
 * @retval false if none of any bit in given mask is set
 */
static inline bool uart_check_modem_status(UART_Type *ptr, uart_modem_stat_t mask)
{
    return (ptr->MSR & mask);
}

/**
 * @brief Disable IRQ with mask
 *
 * @param ptr UART base address
 * @param irq_mask IRQ mask value to be disabled
 */
static inline void uart_disable_irq(UART_Type *ptr, uart_intr_enable_t irq_mask)
{
    ptr->IER &= ~irq_mask;
}

/**
 * @brief Enable IRQ with mask
 *
 * @param ptr UART base address
 * @param irq_mask IRQ mask value to be enabled
 */
static inline void uart_enable_irq(UART_Type *ptr, uart_intr_enable_t irq_mask)
{
    ptr->IER |= irq_mask;
}

/**
 * @brief Get Enabled IRQ
 *
 * @param ptr UART base address
 * @return enabled irq
 */
static inline uint32_t uart_get_enabled_irq(UART_Type *ptr)
{
    return ptr->IER;
}

/**
 * @brief Get interrupt identification
 *
 * @param ptr UART base address
 * @retval interrupt id
 */
static inline uint8_t uart_get_irq_id(UART_Type *ptr)
{
    return (ptr->IIR & UART_IIR_INTRID_MASK);
}

/**
 * @brief Get status
 *
 * @param ptr UART base address
 * @retval current status
 */
static inline uint8_t uart_get_status(UART_Type *ptr)
{
    return ptr->LSR;
}

/**
 * @brief Check uart status according to the given status mask
 *
 * @param ptr UART base address
 * @param mask Status mask value to be checked against
 * @retval true if any bit in given mask is set
 * @retval false if none of any bit in given mask is set
 */
static inline bool uart_check_status(UART_Type *ptr, uart_stat_t mask)
{
    return (ptr->LSR & mask);
}

/**
 * @brief Get default config
 *
 * @param ptr UART base address
 * @param config Pointer to the buffer to save default values
 */
void uart_default_config(UART_Type *ptr, uart_config_t *config);

/**
 * @brief Initialization
 *
 * @param ptr UART base address
 * @param config Pointer to config struct
 * @retval status_success only if it succeeds
 */
hpm_stat_t uart_init(UART_Type *ptr, uart_config_t *config);

/**
 * @brief Send one byte
 *
 * @param ptr UART base address
 * @param c Byte to be sent
 * @retval status_success only if it succeeds
 */
hpm_stat_t uart_send_byte(UART_Type *ptr, uint8_t c);

/**
 * @brief Receive one byte
 *
 * @param ptr UART base address
 * @param c Pointer to buffer to save the byte received on UART
 * @retval status_success only if it succeeds
 */
hpm_stat_t uart_receive_byte(UART_Type *ptr, uint8_t *c);

/**
 * @brief Set uart signal output level
 *
 * @param ptr UART base address
 * @param signal Target signal
 * @param level Target signal level
 */
void uart_set_signal_level(UART_Type *ptr,
                           uart_signal_t signal,
                           uart_signal_level_t level);

/**
 * @brief Flush sending buffer/fifo
 *
 * @param ptr UART base address
 * @retval status_success only if it succeeds
 */
hpm_stat_t uart_flush(UART_Type *ptr);

/**
 * @brief Receive bytes blocking
 *
 * @param ptr UART base address
 * @param buf Pointer to the buffer to save received data
 * @param size_in_byte Size in byte to be sent
 * @retval status_success only if it succeeds
 */
hpm_stat_t uart_receive_data(UART_Type *ptr, uint8_t *buf, uint32_t size_in_byte);

/**
 * @brief Send bytes blocking
 *
 * @param ptr UART base address
 * @param buf Pointer to the buffer to be sent
 * @param size_in_byte Size in byte to be sent
 * @retval status_success only if it succeeds
 */
hpm_stat_t uart_send_data(UART_Type *ptr, uint8_t *buf, uint32_t size_in_byte);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif    /* HPM_UART_DRV_H */
