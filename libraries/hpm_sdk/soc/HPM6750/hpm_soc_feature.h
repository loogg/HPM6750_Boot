/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HPM_SOC_FEATURE_H
#define HPM_SOC_FEATURE_H

/*
 * I2C Section
 */
#define I2C_SOC_FIFO_SIZE (4U)

/*
 * PMIC Section
 */
#define PCFG_SOC_LDO1P1_MIN_VOLTAGE_IN_MV (900U)
#define PCFG_SOC_LDO1P1_MAX_VOLTAGE_IN_MV (1100U)
#define PCFG_SOC_LDO2P5_MIN_VOLTAGE_IN_MV (2100U)
#define PCFG_SOC_LDO2P5_MAX_VOLTAGE_IN_MV (2500U)

/*
 * I2S Section
 */
#define I2S_SOC_MAX_CHANNEL_NUM (16U)
#define I2S_SOC_MAX_TX_CHANNEL_NUM (8U)
#define PDM_I2S HPM_I2S0
#define DAO_I2S HPM_I2S1
#define PDM_SOC_SAMPLE_RATE_IN_HZ (16000U)
#define VAD_SOC_SAMPLE_RATE_IN_HZ (16000U)
#define DAO_SOC_SAMPLE_RATE_IN_HZ (48000U)
#define DAO_SOC_PDM_SAMPLE_RATE_RATIO (3U)
#define DAO_SOC_VAD_SAMPLE_RATE_RATIO (3U)

/*
 * PLLCTL Section
 */
#define PLLCTL_SOC_PLL_MAX_COUNT (5U)
/* PLL reference clock in hz */
#define PLLCTL_SOC_PLL_REFCLK_FREQ (24U * 1000000UL)
/* only PLL1 and PLL2 have DIV0, DIV1 */
#define PLLCTL_SOC_PLL_HAS_DIV0(x) ((((x) == 1) || ((x) == 2)) ? 1 : 0)
#define PLLCTL_SOC_PLL_HAS_DIV1(x) ((((x) == 1) || ((x) == 2)) ? 1 : 0)


/*
 * PWM Section
 */
#define PWM_SOC_PWM_MAX_COUNT (8U)
#define PWM_SOC_CMP_MAX_COUNT (24U)
#define PWM_SOC_OUTPUT_TO_PWM_MAX_COUNT (8U)
#define PWM_SOC_OUTPUT_MAX_COUNT (24U)

/*
 * DMA Section
 */
#define DMA_SOC_TRANSFER_WIDTH_MAX DMA_TRANSFER_WIDTH_DOUBLE_WORD
#define DMA_SOC_BUS_NUM (1U)
#define DMA_SOC_CHANNEL_NUM (8U)

/*
 * PDMA Section
 */
#define PDMA_SOC_PS_MAX_COUNT (2U)

/*
 * LCDC Section
 */
#define LCDC_SOC_MAX_LAYER_COUNT         (8U)
#define LCDC_SOC_MAX_CSC_LAYER_COUNT     (2U)
#define LCDC_SOC_LAYER_SUPPORTS_CSC(x) ((x) < 2)
#define LCDC_SOC_LAYER_SUPPORTS_YUV(x) ((x) < 2)

/*
* USB Section
*/
#define USB_SOC_MAX_COUNT                          (2U)

#define USB_SOC_DCD_QTD_NEXT_INVALID               (1U)
#define USB_SOC_DCD_QHD_BUFFER_COUNT               (5U)
#define USB_SOC_DCD_QTD_ALIGNMENT                  (32U)
#define USB_SOC_DCD_QHD_ALIGNMENT                  (64U)
#define USB_SOC_DCD_MAX_ENDPOINT_COUNT             (8U)
#define USB_SOC_DCD_MAX_QTD_COUNT                  (USB_SOC_DCD_MAX_ENDPOINT_COUNT * 2U)
#define USB_SOS_DCD_MAX_QHD_COUNT                  (USB_SOC_DCD_MAX_ENDPOINT_COUNT * 2U)
#define USB_SOC_DCD_DATA_RAM_ADDRESS_ALIGNMENT     (2048U)

#define USB_SOC_HCD_QTD_BUFFER_COUNT               (5U)
#define USB_SOC_HCD_QTD_ALIGNMENT                  (32U)
#define USB_SOC_HCD_QHD_ALIGNMENT                  (32U)
#define USB_SOC_HCD_MAX_ENDPOINT_COUNT             (8U)
#define USB_SOC_HCD_MAX_XFER_ENDPOINT_COUNT        (USB_SOC_HCD_MAX_ENDPOINT_COUNT * 2U)
#define USB_SOC_HCD_FRAMELIST_MAX_ELEMENTS         (1024U)
#define USB_SOC_HCD_DATA_RAM_ADDRESS_ALIGNMENT     (4096U)

/*
* ENET Section
*/
#define ENET_SOC_DESC_ADDR_ALIGNMENT               (16U)
#define ENET_SOC_BUFF_ADDR_ALIGNMENT               (4U)
#define ENET_SOC_ADDR_MAX_COUNT                    (5U)
#define ENET_SOC_ALT_EHD_DES_MIN_LEN               (4U)
#define ENET_SOC_ALT_EHD_DES_MAX_LEN               (8U)
#define ENET_SOC_ALT_EHD_DES_LEN                   (8U)
/*
* ADC Section
*/
#define ADC_SOC_MAX_SEQ_LEN         (16U)
#define ADC_SOC_MAX_TRIG_CH_LEN     (4U)
#define ADC_SOC_DMA_ADDR_ALIGNMENT  (4U)

/*
 * SYSCTL Section
 */
#define SYSCTL_SOC_CPU_GPR_COUNT (14U)
#define SYSCTL_SOC_MONITOR_SLICE_COUNT (4U)

/*
 * PTPC Section
 */
#define PTPC_SOC_TIMER_MAX_COUNT       (2U)

/*
 * CAN Section
 */
#define CAN_SOC_MAX_COUNT       (4U)

#endif /* HPM_SOC_FEATURE_H */
