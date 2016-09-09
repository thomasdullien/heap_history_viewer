#version 130
in highp ivec3 position;
in highp vec3 color;

out vec4 vColor;

uniform mat2 scale_heap_to_screen;
uniform int visible_heap_base_A;
uniform int visible_heap_base_B;
uniform int visible_heap_base_C;
uniform int visible_tick_base_A;
uniform int visible_tick_base_B;

// =========================================================================
// Everything below should be valid C++ and also valid GLSL! This code is
// shared between displayheapwindow.cpp and simple.vert, so make sure it
// always stays in synch!!
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

vec4 IntToColor(int argument) {
  return vec4((argument & 0xFF) / 255.0,
              ((argument & 0xFF00) >> 8) / 255.0,
              ((argument & 0xFF0000) >> 16) / 255.0,
              1.0);
}

int FloatToInt(float argument) {
   argument = argument;// * 256.0;
   return int(argument);
}

// This shader draws vertical lines to signify events.
void main(void)
{
  // =========================================================================
  // Everything below should be valid C++ and also valid GLSL! This code is
  // shared between displayheapwindow.cpp and simple.vert, so make sure it
  // always stays in synch!!
  // =========================================================================
  //
  // Read the X (tick) and Y (address) coordinate of the current point.
  ivec3 address = Load64BitLeftShiftedBy4Into96Bit(position.y, position.z);

  // Get the base of the heap in the displayed window. This is a 96-bit number
  // where the lowest 4 bit represent a fractional component, the rest is a
  // normal 92-bit integer.
  ivec3 heap_base =
      ivec3(visible_heap_base_A, visible_heap_base_B, visible_heap_base_C);

  // Translate the y / address coordinate of the heap so that the left lower
  // corner of the visible heap window aligns with 0.
  ivec3 address_coordinate_translated = Sub96(address, heap_base);

  // Multiply the y coordinate with the y entry of the transformation matrix.
  // To avoid a degenerate matrix, C++ code supplies a matrix containing the
  // square roots of the actual matrix to the shader code, so apply the float
  // twice
  float temp_y = Multiply96BitWithFloat(address_coordinate_translated,
                                        scale_heap_to_screen[1][1]);
  float final_y = temp_y * scale_heap_to_screen[1][1];

  final_y = 2 * final_y - 1;

  float final_x = -1.0;
  if (position.x != 0) {
    final_x = 1.0;
  }
  // ==========================================================================
  // End of mandatory valid GLSL part.
  // ==========================================================================

  gl_Position = vec4(final_x, final_y, 0.0, 1.0);
  vColor = vec4(color, 0.5);
}
