#version 130
in highp ivec3 position;
in highp vec3 color;

out vec4 vColor;

uniform mat2 scale_heap_to_screen;
uniform int visible_heap_base_A;
uniform int visible_heap_base_B;
uniform int minimum_visible_tick;

// Emulates uint64_t addition using vectors of integers. Uses carry
// extraction code from Hackers Delight 2-16.
ivec2 AddUint64(ivec2 a, ivec2 b) {
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

ivec2 SubUint64(ivec2 a, ivec2 b) {
  int sub_lower_word = a.x - b.x;
  // We do not have unsigned shift.
  int borrow = (
              ((a.x ^ 0xFFFFFFFF) & b.x) |
              ((((a.x ^ b.x) ^ 0xFFFFFFFF) & (sub_lower_word ^ 0xFFFFFFFF))));
  int borrow_flag = 0;
  if (borrow == 1) {
      borrow_flag = 1;
  }
  int sub_upper_word = a.y - b.y - borrow_flag;
  ivec2 result = ivec2(sub_lower_word, sub_upper_word);
  return result;
}

// Code to multiply a float with a uint64_t.
float Multiply64BitWithFloat(ivec2 a, float b) {
  float a0 = float(a.x & 0xFFFF);
  float a1 = float((a.x & 0xFFFF0000) >> 16);
  float a2 = float(a.y & 0xFFFF);
  float a3 = float((a.y & 0xFFFF0000) >> 16);
  float left_shift_16f = float(0x10000);
  float left_shift_32f = left_shift_16f * left_shift_16f;
  float left_shift_48f = left_shift_32f * left_shift_16f;
  float result = a0 * b +
           a1 * b * left_shift_16f +
           a2 * b * left_shift_32f +
           a3 * b * left_shift_48f;
  return result;
}

void main(void)
{
   // Read the X (tick) and Y (address) coordinate.
   int tick = position.x;
   ivec2 address = ivec2(position.y, position.z);
   // Get the base of the heap.
   ivec2 heap_base = ivec2(visible_heap_base_A, visible_heap_base_B);

   // Translate the y / address coordinate of the heap so that the left lower
   // corner of the visible heap window aligns with 0.
   ivec2 address_coordinate_translated = SubUint64(address, heap_base);

   // Translate the x / tick coordinate to be aligned with 0.
   int translated_tick = tick - minimum_visible_tick;

   // Multiply the y coordinate with the y entry of the transformation matrix.
   // To avoid a degenerate matrix, C++ code supplies a matrix containing the
   // square roots of the actual matrix to the shader code, so apply the float
   // twice.
   float final_y = Multiply64BitWithFloat(address_coordinate_translated,
                                          scale_heap_to_screen[1][1])
           * scale_heap_to_screen[1][1];
   float final_x = (tick * scale_heap_to_screen[0][0])
           * scale_heap_to_screen[0][0];

   gl_Position = vec4(final_x, final_y, 0.0, 1.0);
   if (final_x > 0.4) {
       vColor = vec4(1.0, 0.0, 0.0, 1.0);
   } else if (final_y > 0) {
       vColor = vec4(0.0, 1.0, 0.0, 1.0);
   } else {
       vColor = vec4(0.0, 0.0, 1.0, 1.0);
   }
}
