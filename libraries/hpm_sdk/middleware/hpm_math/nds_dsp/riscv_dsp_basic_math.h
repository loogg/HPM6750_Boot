/**
 * Copyright (C) 2021 Andes Technology Corporation
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __RISCV_DSP_BASIC_MATH_H__
#define __RISCV_DSP_BASIC_MATH_H__

/**
 * @defgroup basic Basic Functions
 */
#ifdef  __cplusplus
extern "C"
{
#endif

#include "riscv_dsp_math_types.h"

// Absolute value
/**
 * @brief Absolute value of floating-potint vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 */
void riscv_dsp_abs_f32(float32_t *src, float32_t *dst, uint32_t size);

/**
 * @brief Absolute value of q31 vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The Q31 value INT32_MIN (0x80000000) will be saturated to the maximum
 * allowable positive value INT32_MAX.
 */
void riscv_dsp_abs_q31(q31_t *src, q31_t *dst, uint32_t size);

/**
 * @brief Absolute value of q15 vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The Q15 value INT16_MIN (0x8000) will be saturated to the maximum
 * allowable positive value INT16_MAX.
 */
void riscv_dsp_abs_q15(q15_t *src, q15_t *dst, uint32_t size);

/**
 * @brief Absolute value of q7 vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The Q7 value INT8_MIN (0x8000) will be saturated to the maximum
 * allowable positive value INT8_MAX.
 */
void riscv_dsp_abs_q7(q7_t *src, q7_t *dst, uint32_t size);

// Addition
/**
 * @brief Addition of floating-potint vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 */
void riscv_dsp_add_f32(float32_t *src1, float32_t *src2, float32_t *dst, uint32_t size);

/**
 * @brief Addition of q31 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q31 range [0x80000000 0x7FFFFFFF].
 */
void riscv_dsp_add_q31(q31_t *src1, q31_t *src2, q31_t *dst, uint32_t size);

/**
 * @brief Addition of q15 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * The output results will be saturated in Q15 range [0x8000 0x7FFF].
 */
void riscv_dsp_add_q15(q15_t *src1, q15_t *src2, q15_t *dst, uint32_t size);

/**
 * @brief Addition of q7 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q7 range [0x80 0x7F].
 */
void riscv_dsp_add_q7(q7_t *src1, q7_t *src2, q7_t *dst, uint32_t size);

/**
 * @brief Addition of U8 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in U16 range [0x0000 0xFFFF].
 */
void riscv_dsp_add_u8_u16(uint8_t *src1, uint8_t *src2, uint16_t *dst, uint32_t size);

// Subtraction
/**
 * @brief Subtraction of floating-point vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 */
void riscv_dsp_sub_f32(float32_t *src1, float32_t *src2, float32_t *dst, uint32_t size);

/**
 * @brief Subtraction of q31 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q31 range [0x80000000 0x7FFFFFFF].
 */
void riscv_dsp_sub_q31(q31_t *src1, q31_t *src2, q31_t *dst, uint32_t size);

/**
 * @brief Subtraction of q15 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * The output results will be saturated in Q15 range [0x8000 0x7FFF].
 */
void riscv_dsp_sub_q15(q15_t *src1, q15_t *src2, q15_t *dst, uint32_t size);

/**
 * @brief Subtraction of q7 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q7 range [0x80 0x7F].
 */
void riscv_dsp_sub_q7(q7_t *src1, q7_t *src2, q7_t *dst, uint32_t size);

/**
 * @brief Subtraction of u8 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q7 range [0x80 0x7F].
 */
void riscv_dsp_sub_u8_q7(uint8_t *src1, uint8_t *src2, q7_t *dst, uint32_t size);

// Multiplication
/**
 * @brief Multiplication of floating-point vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 */
void riscv_dsp_mul_f32(float32_t *src1, float32_t *src2, float32_t *dst, uint32_t size);

/**
 * @brief Multiplication of q31 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q31 range [0x80000000 0x7FFFFFFF].
 */
void riscv_dsp_mul_q31(q31_t *src1, q31_t *src2, q31_t *dst, uint32_t size);

/**
 * @brief Multiplication of q15 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Output results will be saturated in Q15 range [0x8000 0x7FFF].
 */
void riscv_dsp_mul_q15(q15_t *src1, q15_t *src2, q15_t *dst, uint32_t size);

/**
 * @brief Multiplication of q7 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be saturated in Q7 range [0x80 0x7F].
 */
void riscv_dsp_mul_q7(q7_t *src1, q7_t *src2, q7_t *dst, uint32_t size);

/**
 * @brief Multiplication of u8 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 *
 * Ouput results will be in U16 range [0x00 0xFFFF].
 */
void riscv_dsp_mul_u8_u16(uint8_t *src1, uint8_t *src2, uint16_t *dst, uint32_t size);

// Division
/**
 * @brief Division of floating-point vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[out]      *dst  points to the output vector.
 * @param[in]       size  size of the vectors.
 * @return none.
 */
void riscv_dsp_div_f32(float32_t *src1, float32_t *src2, float32_t *dst, uint32_t size);

/**
 * @brief Division of q31 inputs.
 * @param[in]       src1 the smaller input value.
 * @param[in]       src2 the larger input value.
 * @return division of two inputs.
 */
q31_t riscv_dsp_div_q31(q31_t src1, q31_t src2);

/**
 * @brief Division of q63 inputs divided by a positive 32 bits.
 * @param[in]       src1 the q63 input value.
 * @param[in]       src2 the positive 32 bits input value.
 * @return division of two inputs.
 */
q31_t riscv_dsp_div_s64_u32(q63_t src1, uint32_t src2);

/**
 * @brief Division of positive 64-bits inputs divided by a positive 32-bits.
 * @param[in]       src1 the positive 64-bits input value.
 * @param[in]       src2 the positive 32-bits input value.
 * @return division of two inputs.
 */
q31_t riscv_dsp_div_u64_u32(uint64_t src1, uint32_t src2);

// Negation
/**
 * @brief Negation of floating-potint vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 */
void riscv_dsp_neg_f32(float32_t *src, float32_t *dst, uint32_t size);

/**
 * @brief Negation of q31 vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The Q31 value INT32_MIN (0x80000000) will be saturated to the maximum
 * allowable positive value INT32_MAX.
 */
void riscv_dsp_neg_q31(q31_t *src, q31_t *dst, uint32_t size);

/**
 * @brief Negation of q15 vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The Q15 value INT16_MIN (0x8000) will be saturated to the maximum
 * allowable positive value INT16_MAX.
 */
void riscv_dsp_neg_q15(q15_t *src, q15_t *dst, uint32_t size);

/**
 * @brief Negation of q15 vectors.
 * @param[in]       *src points to the input vector.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The Q7 value INT8_MIN (0x80) will be saturated to the maximum allowable
 * positive value INT8_MAX.
 */
void riscv_dsp_neg_q7(q7_t *src, q7_t *dst, uint32_t size);

// Dot Production
/**
 * @brief Dot production of floating-point vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 */
float32_t riscv_dsp_dprod_f32(float32_t *src1, float32_t *src2,
                             uint32_t size);

/**
 * @brief Dot production of q31 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 *
 * The output of multiplications is truncated from 2.62 to 2.48 format and
 * then added without saturation to a 64-bit accumulator. The return value
 * is in 16.48 format. When the size of the vectors less than 2^16, there is
 * no risk to overflow.
 */
q63_t riscv_dsp_dprod_q31(q31_t *src1, q31_t *src2, uint32_t size);

/**
 * @brief Dot production of q15 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 *
 * The output of multiplications is in 2.30 format and then added to a
 * 64-bit accumulator in 34.30 format. The return value is in 34.30 format.
 */
q63_t riscv_dsp_dprod_q15(q15_t *src1, q15_t *src2, uint32_t size);

/**
 * @brief Dot production of u8 * q15 vectors.
 * @param[in]       *src1 points to the uint8_t format input vector.
 * @param[in]       *src2 points to the q15 format input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 *
 * The output of multiplications is in 1.23 format and then added to an
 * accumulator in 9.23 format. The return result is in 9.23 format.
 */

q31_t riscv_dsp_dprod_u8xq15(uint8_t *src1, q15_t *src2, uint32_t size);


/**
 * @brief Dot production of q7 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 *
 * The output of multiplications is in 2.14 format and then added to an
 * accumulator in 18.14 format. The return result is in 18.14 format.
 */
q31_t riscv_dsp_dprod_q7(q7_t *src1, q7_t *src2, uint32_t size);

/**
 * @brief Dot production of q7 * q15 vectors.
 * @param[in]       *src1 points to the q7_t format input vector.
 * @param[in]       *src2 points to the q15 format input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 *
 * The output of multiplications is in 1.22 format and then added to an
 * accumulator in 10.22 format. The return result is in 10.22 format.
 */
q31_t riscv_dsp_dprod_q7xq15(q7_t *src1, q15_t *src2, uint32_t size);

/**
 * @brief Dot production of U8 vectors.
 * @param[in]       *src1 points to the first input vector.
 * @param[in]       *src2 points to the second input vector.
 * @param[in]       size  size of the vectors.
 * @return dot product of two input vectors.
 *
 * The output of multiplications is in 0.16 format and then added to an
 * accumulator in 16.16 format. The return result is in 16.16 format.
 */
uint32_t riscv_dsp_dprod_u8(uint8_t *src1, uint8_t *src2, uint32_t size);

// Offset
/**
 * @brief The offset of floating-point vectors.
 * @param[in]       *src points to the input vector.
 * @param[in]       offset is the value to be added.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 */
void riscv_dsp_offset_f32(float32_t *src, float32_t offset, float32_t *dst,
                      uint32_t size);

/**
 * @brief The offset of q31 vectors.
 * @param[in]       *src points to the input vector.
 * @param[in]       offset is the value to be added.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * Output results are saturated in Q31 range [0x80000000 0x7FFFFFFF].
 */
void riscv_dsp_offset_q31(q31_t *src, q31_t offset, q31_t *dst, uint32_t size);

/**
 * @brief The offset of q15 vectors.
 * @param[in]       *src points to the input vector.
 * @param[in]       offset is the value to be added.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * Output results are saturated in Q15 range [0x8000 0x7FFF].
 */
void riscv_dsp_offset_q15(q15_t *src, q15_t offset, q15_t *dst, uint32_t size);

/**
 * @brief The offset of q7 vectors.
 * @param[in]       *src points to the input vector.
 * @param[in]       offset is the value to be added.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * Output results are saturated in Q7 range [0x80 0x7F].
 */
void riscv_dsp_offset_q7(q7_t *src, q7_t offset, q7_t *dst, uint32_t size);

/**
 * @brief The offset of U8 vectors.
 * @param[in]       *src points to the input vector.
 * @param[in]       offset is the value to be added.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * Output results are saturated in U8 range [0x00 0xFF].
 */
void riscv_dsp_offset_u8(uint8_t *src, q7_t offset, uint8_t *dst, uint32_t size);

// Scale
/**
 * @brief To multiply a floating-point vectors by a floating-point scale.
 * @param[in]       *src points to the input vector.
 * @param[in]       scale is the value to be multiplied.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 */
void riscv_dsp_scale_f32(float32_t *src, float32_t scale, float32_t *dst,
                     uint32_t size);

/**
 * @brief To multiply a q31 vectors by a q31 scale.
 * @param[in]       *src points to the input vector.
 * @param[in]       scalefract is the fractional portion value
 *                             to be multiplied.
 * @param[in]       shift      number of bits to shift.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * These are multiplied to yield a 2.62 output and then is shift with
 * saturation to 1.31 format.
 */
void riscv_dsp_scale_q31(q31_t *src, q31_t scalefract, int8_t shift,
                     q31_t *dst, uint32_t size);

/**
 * @brief To multiply a q15 vectors by a q15 scale.
 * @param[in]       *src points to the input vector.
 * @param[in]       scalefract is the fractional portion value
 *                             to be multiplied.
 * @param[in]       shift      number of bits to shift.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * These are multiplied to yield a 2.30 output and then is shifted with
 * saturation to 1.15 format.
 */
void riscv_dsp_scale_q15(q15_t *src, q15_t scalefract, int8_t shift,
                     q15_t *dst, uint32_t size);

/**
 * @brief To multiply a q7 vectors by a q7 scale.
 * @param[in]       *src points to the input vector.
 * @param[in]       scalefract is the fractional portion value
 *                             to be multiplied.
 * @param[in]       shift      number of bits to shift.
 * @param[out]      *dst points to the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * These are multiplied to yield a 2.14 output and then is shifted with
 * saturation to 1.7 format.
 */
void riscv_dsp_scale_q7(q7_t *src, q7_t scalefract, int8_t shift,
                    q7_t *dst, uint32_t size);

/**
 * @brief To multiply a u8 vectors by a q7 scale.
 * @param[in]  *src points to the input vector.
 * @param[in]  scalefract: is the fractional portion value to be multiplied.
 * @param[in]  shift: number of bits to shift.
 * @param[out] *dst points to the output vector.
 * @param[in]  size size of the vectors.
 * @return none.
 *
 * The inputs are multiplied to yield a 1.15 output and then are shift with
 * saturation to 8-bit formats.
 */
void riscv_dsp_scale_u8(uint8_t *src, q7_t scalefract, int8_t shift, uint8_t *dst, uint32_t size);

// Shift
/**
 * @brief Shifts a q15 vector with a specified shift number.
 * @param[in]       *src the input vector.
 * @param[in]      shift number of shift bits. If (shift > 0) means shifts
 *                       left; (shift < 0) means shifts right.
 * @param[out]      *dst the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The input and output are all saturated to q15 range [0x8000 0x7FFF].
 */
void riscv_dsp_shift_q15(q15_t *src, int8_t shift, q15_t *dst, uint32_t size);

/**
 * @brief Shifts a q31 vector with a specified shift number.
 * @param[in]       *src the input vector.
 * @param[in]      shift number of shift bits. If (shift > 0) means shifts
 *                       left; (shift < 0) means shifts right.
 * @param[out]      *dst the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The input and output are all saturated to q31 range [0x80000000 0x7FFFFFFF].
 */
void riscv_dsp_shift_q31(q31_t *src, int8_t shift, q31_t *dst, uint32_t size);

/**
 * @brief Shifts a q7 vector with a specified shift number.
 * @param[in]       *src the input vector.
 * @param[in]      shift number of shift bits. If (shift > 0) means shifts
 *                       left; (shift < 0) means shifts right.
 * @param[out]      *dst the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The input and output are all saturated to q7 range [0x80 0x7F].
 */
void riscv_dsp_shift_q7(q7_t *src, int8_t shift, q7_t *dst, uint32_t size);

/**
 * @brief Shifts a u8 vector for a specified shift number.
 * @param[in]       *src the input vector.
 * @param[in]      shift number of shift bits. If (shift > 0) means shifts
 *                       left; (shift < 0) means shifts right.
 * @param[out]      *dst the output vector.
 * @param[in]       size size of the vectors.
 * @return none.
 *
 * The input and output are all saturated to u8 range [0x00 0xFF].
 */
void riscv_dsp_shift_u8(uint8_t *src, int8_t shift, uint8_t *dst, uint32_t size);

#ifdef  __cplusplus
}
#endif
#endif // __RISCV_DSP32_BASIC_MATH_H__
