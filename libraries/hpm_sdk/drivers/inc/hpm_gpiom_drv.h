/*
 * Copyright (c) 2021 hpmicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HPM_GPIOM_DRV_H
#define HPM_GPIOM_DRV_H

#include "hpm_gpiom_regs.h"

/**
 * 
 * @brief GPIOM driver APIs
 * @defgroup gpiom_interface GPIOM driver APIs
 * @ingroup io_interfaces
 * @{
 */

/* @brief gpiom control module */
typedef enum gpiom_gpio {
    gpiom_soc_gpio0 = 0,
    gpiom_soc_gpio1 = 1,
    gpiom_core0_fast = 2,
    gpiom_core1_fast = 3,
} gpiom_gpio_t;

/* @brief pin visibility */
typedef enum gpiom_pin_visibility {
    gpiom_pin_visible = 0,
    gpiom_pin_invisible = 1,
} gpiom_pin_visibility_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get pin's controler
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 * 
 * @retval pin control module
 */
static inline gpiom_gpio_t gpiom_get_pin_controler(GPIOM_Type *ptr,
                                                      uint8_t gpio_index,
                                                      uint8_t pin_index)
{
    return (gpiom_gpio_t)((ptr->ASSIGN[gpio_index].PIN[pin_index]
            & (GPIOM_PIN_SELECT_MASK)) >> GPIOM_PIN_SELECT_SHIFT);
}

/**
 * @brief set pin's controler
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 * @param gpio gpio module index
 */
static inline void gpiom_set_pin_controler(GPIOM_Type *ptr,
                              uint8_t gpio_index,
                              uint8_t pin_index,
                              gpiom_gpio_t gpio)
{
    ptr->ASSIGN[gpio_index].PIN[pin_index] = 
        (ptr->ASSIGN[gpio_index].PIN[pin_index] & ~(GPIOM_PIN_SELECT_MASK))
      | GPIOM_PIN_SELECT_SET(gpio);
}

/**
 * @brief Check if pin is visibility for specificed module
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 * @param gpio gpio module index
 * 
 * @retval true if pin is visible by specificed module
 * @retval false if pin is not visible by specificed module
 */
static inline bool gpiom_check_pin_visibility(GPIOM_Type *ptr,
                              uint8_t gpio_index,
                              uint8_t pin_index,
                              gpiom_gpio_t gpio)
{
    return (ptr->ASSIGN[gpio_index].PIN[pin_index] & ((1 << gpio) << GPIOM_PIN_HIDE_SHIFT))
        >> GPIOM_PIN_HIDE_SHIFT >> gpio == gpiom_pin_visible;
}

/**
 * @brief enable pin visibility for specificed module
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 * @param gpio gpio module index
 */
static inline void gpiom_enable_pin_visibility(GPIOM_Type *ptr,
                              uint8_t gpio_index,
                              uint8_t pin_index,
                              gpiom_gpio_t gpio)
{
    ptr->ASSIGN[gpio_index].PIN[pin_index] = 
        (ptr->ASSIGN[gpio_index].PIN[pin_index] & ~((1 << gpio) << GPIOM_PIN_HIDE_SHIFT));
}

/**
 * @brief disable pin visibility for specificed module
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 * @param gpio gpio module index
 */
static inline void gpiom_disable_pin_visibility(GPIOM_Type *ptr,
                              uint8_t gpio_index,
                              uint8_t pin_index,
                              gpiom_gpio_t gpio)
{
    ptr->ASSIGN[gpio_index].PIN[pin_index] = 
        (ptr->ASSIGN[gpio_index].PIN[pin_index] & ~((1 << gpio) << GPIOM_PIN_HIDE_SHIFT))
      | GPIOM_PIN_HIDE_SET(1 << gpio);
}

/**
 * @brief Check if pin management is locked
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 * 
 * @retval true if pin management is locked
 * @retval false if pin management is not locked
 */
static inline bool gpiom_pin_is_locked(GPIOM_Type *ptr,
                              uint8_t gpio_index,
                              uint8_t pin_index)
{
    return (ptr->ASSIGN[gpio_index].PIN[pin_index] & GPIOM_PIN_LOCK_MASK)
        == GPIOM_PIN_LOCK_MASK;
}

/**
 * @brief lock pin management
 *
 * @param ptr GPIOM base address
 * @param gpio_index gpio port index
 * @param pin_index pin index
 */
static inline void gpiom_lock_pin(GPIOM_Type *ptr,
                              uint8_t gpio_index,
                              uint8_t pin_index)
{
    ptr->ASSIGN[gpio_index].PIN[pin_index] |= GPIOM_PIN_LOCK_MASK;
}

#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif /* HPM_GPIOM_DRV_H */

