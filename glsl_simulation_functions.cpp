// A number of functions that are needed to compile the glsl 130 code
// in the shaders as C++ for easier debugging.
#include "glsl_simulation_functions.h"

// =========================================================================
// Everything below should be valid C++ and also valid GLSL! This code is
// shared between this file and the various vertex shaders, so make sure it
// always stays in synch.
// =========================================================================

// Emulates uint64_t/int64 addition using vectors of integers. Uses carry
// extraction code from Hackers Delight 2-16.
// Function must be valid C++ and valid GLSL!
ivec2 Add64(ivec2 a, ivec2 b) {
  int sum_lower_word = a.x + b.x;
  int carry = ((a.x & b.x) | (((a.x | b.x) & (sum_lower_word ^ 0xFFFFFFFF))));
  int carry_flag = 0;
  // We do not have an easily-available unsigned shift, so we use
  // an IF.
  if ((carry & 0x80000000) != 0) {
    carry_flag = 1;
  }
  int sum_upper_word = a.y + b.y + carry_flag;
  ivec2 result = ivec2(sum_lower_word, sum_upper_word);
  return result;
}

// Function must be valid C++ and valid GLSL!
ivec2 Sub64(ivec2 a, ivec2 b) {
  int sub_lower_word = a.x - b.x;
  int not_a_and_b = (a.x ^ 0xFFFFFFFF) & b.x;
  int a_equiv_b = (a.x ^ b.x) ^ 0xFFFFFFFF;
  int a_equiv_b_and_c = a_equiv_b & sub_lower_word;
  int borrow = not_a_and_b | a_equiv_b_and_c;
  int borrow_flag = 0;
  // No unsigned shift-right available.
  if ((borrow & 0x80000000) != 0) {
    borrow_flag = 1;
  }
  int sub_upper_word = a.y - b.y - borrow_flag;
  ivec2 result = ivec2(sub_lower_word, sub_upper_word);
  return result;
}

// Function must be valid C++ and valid GLSL!
float Multiply64BitWithFloat(ivec2 a, float b) {
  bool is_negative = false;
  if ((a.y & 0x80000000) != 0) {
    is_negative = true;
    ivec2 zero = ivec2(0, 0);
    a = Sub64(zero, a);
  }
  float a0 = float(a.x & 0xFFFF);
  float a1 = float((a.x & 0xFFFF0000) >> 16);
  float a2 = float(a.y & 0xFFFF);
  float a3 = float((a.y & 0xFFFF0000) >> 16);
  float left_shift_16f = float(0x10000);
  float left_shift_32f = left_shift_16f * left_shift_16f;
  float left_shift_48f = left_shift_32f * left_shift_16f;
  float result = a0 * b;
  result = result + a1 * b * left_shift_16f;
  result = result + a2 * b * left_shift_32f;
  result = result + a3 * b * left_shift_48f;
  if (is_negative) {
    result = result * (-1.0);
  }
  return result;
}

// Emulates uint96 addition using vectors of integers, uses 64-bit addition
// defined above.
// Function must be valid C++ and valid GLSL!
ivec3 Add96(ivec3 a, ivec3 b) {
  ivec2 temp_a = ivec2(a.x, 0);
  ivec2 temp_b = ivec2(b.x, 0);
  ivec2 temp_ab = Add64(temp_a, temp_b);
  // The lowest int of the result has been calculated.
  int c1 = temp_ab.x;
  ivec2 temp_a2 = ivec2(a.y, 0);
  ivec2 temp_b2 = ivec2(b.y, 0);
  ivec2 temp_carry = ivec2(temp_ab.y, 0);
  ivec2 temp_ab_carry = Add64(Add64(temp_a2, temp_b2), temp_carry);
  // The middle int has been calculated.
  int c2 = temp_ab_carry.x;
  // For the last int, we do not need to be concerned about the carry-out.
  int c3 = a.z + b.z + temp_ab_carry.y;
  return ivec3(c1, c2, c3);
}

// Function must be valid C++ and valid GLSL!
ivec3 Sub96(ivec3 a, ivec3 b) {
  ivec2 temp_a = ivec2(a.x, 0);
  ivec2 temp_b = ivec2(b.x, 0);
  ivec2 temp_ab = Sub64(temp_a, temp_b);
  // The lowest int of the result has been calculated.
  int c1 = temp_ab.x;
  ivec2 temp_a2 = ivec2(a.y, 0);
  ivec2 temp_b2 = ivec2(b.y, 0);
  ivec2 temp_borrow = ivec2(-temp_ab.y, 0);
  ivec2 temp_ab_with_borrow = Sub64(Sub64(temp_a2, temp_b2), temp_borrow);
  // The middle int has been calculated.
  int c2 = temp_ab_with_borrow.x;
  // For the last int, we do not need to be concerned about the carry-out.
  int c3 = a.z - b.z - (-temp_ab_with_borrow.y);
  return ivec3(c1, c2, c3);
}

// Function must be valid C++ and valid GLSL!
float Multiply96BitWithFloat(ivec3 a, float b) {
  // First check if the value-to-be-multiplied is negative.
  bool is_negative = false;
  if ((a.z & 0x80000000) != 0) {
    is_negative = true;
    ivec3 zero = ivec3(0, 0, 0);
    // Turn the number positive.
    a = Sub96(zero, a);
  }
  float a0 = float(a.x & 0xFFFF);
  float a1 = float((a.x & 0xFFFF0000) >> 16);
  float a2 = float(a.y & 0xFFFF);
  float a3 = float((a.y & 0xFFFF0000) >> 16);
  float a4 = float(a.z & 0xFFFF);
  float a5 = float((a.z & 0xFFFF0000) >> 16);
  float left_shift_16f = float(0x10000);
  float left_shift_32f = left_shift_16f * left_shift_16f;
  float left_shift_48f = left_shift_32f * left_shift_16f;
  float left_shift_64f = left_shift_48f * left_shift_16f;
  float left_shift_80f = left_shift_64f * left_shift_16f;
  float result = a0 * b;
  result = result + a1 * b * left_shift_16f;
  result = result + a2 * b * left_shift_32f;
  result = result + a3 * b * left_shift_48f;
  result = result + a4 * b * left_shift_64f;
  result = result + a5 * b * left_shift_80f;
  if (is_negative) {
    result = result * (-1.0);
  }
  return result;
}

// Function must be valid C++ and valid GLSL!
int TopNibble(int value) { return ((value & 0xF0000000) >> 28 & 0xF); }

ivec3 Load64BitLeftShiftedBy4Into96Bit(int low, int high) {
  int c3 = TopNibble(high);
  int c2 = (high << 4) | TopNibble(low);
  int c1 = low << 4;
  return ivec3(c1, c2, c3);
}

// Function must be valid C++ and valid GLSL!
ivec2 Load32BitLeftShiftedBy4Into64Bit(int low) {
  int c1 = low << 4;
  int c2 = TopNibble(low);
  return ivec2(c1, c2);
}
// =========================================================================
// End of valid C++ and valid GLSL part.
// =========================================================================

ivec2 LongDoubleTo64Bits(long double value) {
  bool negative = (value < 0);
  long double absolute = fabs(value);
  uint32_t highest_bit = log2(absolute);
  ivec2 result;

  // Find the biggest power-of-two smaller than the value.
  for (int32_t current_bit = highest_bit; current_bit >= 0; --current_bit) {
    long double current_exponential = exp2(current_bit);
    if (absolute >= current_exponential) {
      absolute -= current_exponential;
      result.flipBit(current_bit);
    }
  }
  if (negative) {
    ivec2 zero;
    return Sub64(zero, result);
  }
  return result;
}

ivec3 LongDoubleTo96Bits(long double value) {
  bool negative = (value < 0);
  long double absolute = fabs(value);
  uint32_t highest_bit = log2(absolute);
  ivec3 result;

  // Find the biggest power-of-two smaller than the value.
  for (int32_t current_bit = highest_bit; current_bit >= 0; --current_bit) {
    long double current_exponential = exp2(current_bit);
    if (absolute >= current_exponential) {
      absolute -= current_exponential;
      result.flipBit(current_bit);
    }
  }
  if (negative) {
    ivec3 zero;
    return Sub96(zero, result);
  }
  return result;
}
