#include "heaphistory.h"
#include "heapwindow.h"
#include "displayheapwindow.h"

// Emulates uint64_t/int64 addition using vectors of integers. Uses carry
// extraction code from Hackers Delight 2-16.
//
// Function must be valid C++ and valid GLSL!
ivec2 Add64(ivec2 a, ivec2 b) {
  int sum_lower_word = a.x + b.x;
  int carry = ((a.x & b.x) | (((a.x | b.x) & (sum_lower_word ^ 0xFFFFFFFF))));
  int carry_flag = 0;
  // We do not have an easily-available unsigned shift, so we use
  // an IF.
  if ((carry & 0x8000000) != 0) {
    carry_flag = 1;
  }
  int sum_upper_word = a.y + b.y + carry_flag;
  ivec2 result = ivec2(sum_lower_word, sum_upper_word);
  return result;
}

// Function must be valid C++ and valid GLSL!
ivec2 Sub64(ivec2 a, ivec2 b) {
  int sub_lower_word = a.x - b.x;
  // We do not have unsigned shift.
  int borrow = (((a.x ^ 0xFFFFFFFF) & b.x) |
                ((((a.x ^ b.x) ^ 0xFFFFFFFF) & (sub_lower_word))));
  int borrow_flag = 0;
  if (borrow & 0x8000000) {
    borrow_flag = 1;
  }
  int sub_upper_word = a.y - b.y - borrow_flag;
  ivec2 result = ivec2(sub_lower_word, sub_upper_word);
  return result;
}

// Function must be valid C++ and valid GLSL!
float Multiply64BitWithFloat(ivec2 a, float b) {
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

DisplayHeapWindow::DisplayHeapWindow() {}

DisplayHeapWindow::DisplayHeapWindow(const ivec2 &minimum_tick,
                                     const ivec2 &maximum_tick,
                                     const ivec3 &minimum_address,
                                     const ivec3 &maximum_address) {
  minimum_tick_ = minimum_tick;
  maximum_tick_ = maximum_tick;
  minimum_address_ = minimum_address;
  maximum_address_ = maximum_address;
}

void DisplayHeapWindow::reset(const HeapWindow &global_window) {
  minimum_tick_ = Load32BitLeftShiftedBy4Into64Bit(global_window.minimum_tick_);
  maximum_tick_ = Load32BitLeftShiftedBy4Into64Bit(global_window.maximum_tick_);
  minimum_address_ = Load64BitLeftShiftedBy4Into96Bit(
      global_window.minimum_address_ & 0xFFFFFFFF,
      global_window.minimum_address_ >> 32);
  maximum_address_ = Load64BitLeftShiftedBy4Into96Bit(
      global_window.maximum_address_ & 0xFFFFFFFF,
      global_window.minimum_address_ >> 32);
}

void DisplayHeapWindow::pan(double dx, double dy) {}

void DisplayHeapWindow::zoomToPoint(double dx, double dy, double how_much_x,
                                    double how_much_y) {}

// Map screen coordinates back to the heap, returns false if the coordinate
// can't fall into the heap because it has a negative component.
bool DisplayHeapWindow::mapDisplayCoordinateToHeap(double dx, double dy,
                                                   uint32_t *tick,
                                                   uint64_t *address) {}

void DisplayHeapWindow::setMinimumTick(ivec2 minimum_tick) {
  minimum_tick_ = minimum_tick;
}

void DisplayHeapWindow::setMaximumTick(ivec2 maximum_tick) {
  maximum_tick_ = maximum_tick;
}

void DisplayHeapWindow::setMinimumAddress(ivec3 address) {
  minimum_address_ = address;
}

void DisplayHeapWindow::setMaximumAddress(ivec3 address) {
  maximum_address_ = address;
}

long double DisplayHeapWindow::getXScalingHeapToScreen() const {
  ivec2 width = Sub64(maximum_tick_, minimum_tick_);
  uint64_t shrinkage = width.getUint64();
  long double factor =
      static_cast<long double>(1.0) / static_cast<long double>(shrinkage);
  long double result = sqrt(static_cast<long double>(factor));
  return result;
}

// Scaling to map the heap Y to the interval [0, 1].
long double DisplayHeapWindow::getYScalingHeapToScreen() const {
  ivec3 height = Sub96(maximum_address_, minimum_address_);
  // If we are only dealing with a 64-bit height now, everything is easy.
  if (height.z == 0) {
    uint64_t shrinkage = height.getLowUint64();
    long double factor = static_cast<long double>(1.0) / shrinkage;
    return sqrt(static_cast<long double>(factor));
  }
  // We are dealing with a value bigger than 64 bit now. Get left-most
  // bit.
  uint32_t high_bit = 32 - __builtin_clz(height.upper_32);
  uint64_t shifted_lower_part = height.getLowUint64() >> high_bit;
  uint64_t shifted_upper_part =
      (static_cast<uint64_t>(height.getUpper32()) << (64 - high_bit));
  uint64_t shifted_height = shifted_lower_part | shifted_upper_part;
  long double factor = static_cast<long double>(1.0) / shifted_height;
  // Now account for the shift.
  long double factor2 = static_cast<long double>(1.0) / (1 << high_bit);
  long double result = sqrt(static_cast<long double>(factor * factor2));

  return result;
}

std::pair<float, float>
DisplayHeapWindow::mapHeapCoordinateToDisplay(uint32_t tick, uint64_t address) {
  ivec3 position(tick, address & 0xFFFFFFFF, address >> 32);

  return internalMapHeapCoordinateToDisplay(
      position, minimum_address_.x, minimum_address_.y, minimum_address_.z,
      minimum_tick_.x, minimum_tick_.y, getXScalingHeapToScreen(),
      getYScalingHeapToScreen());
}

// Keep this code as close as possible to actual GSLS v1.3 shader code, so the
// code can be tested here and then cut/pasted into the shader when it works
// (since debugging GLSL is so horrible).
std::pair<float, float> DisplayHeapWindow::internalMapHeapCoordinateToDisplay(
    ivec3 position, int visible_heap_base_A, int visible_heap_base_B,
    int visible_heap_base_C, int visible_tick_base_A, int visible_tick_base_B,
    float scale_heap_x, float scale_heap_y) {
  float scale_heap_to_screen[2][2] = {{scale_heap_x, 0.0}, {0.0, scale_heap_y}};

  // =========================================================================
  // Everything below should be valid C++ and also valid GLSL!
  // =========================================================================
  //
  // Read the X (tick) and Y (address) coordinate of the current point.
  ivec2 tick = Load32BitLeftShiftedBy4Into64Bit(position.x);
  ivec3 address = Load64BitLeftShiftedBy4Into96Bit(position.y, position.z);

  // Get the base of the heap in the displayed window. This is a 96-bit number
  // where the lowest 4 bit represent a fractional component, the rest is a
  // normal 92-bit integer.
  ivec3 heap_base =
      ivec3(visible_heap_base_A, visible_heap_base_B, visible_heap_base_C);

  // Translate the y / address coordinate of the heap so that the left lower
  // corner of the visible heap window aligns with 0.
  ivec3 address_coordinate_translated = Sub96(address, heap_base);

  // Lowest 4 bit represent fractional component, again.
  ivec2 minimum_visible_tick(visible_tick_base_A, visible_tick_base_B);

  // Translate the x / tick coordinate to be aligned with 0.
  ivec2 tick_coordinate_translated = Sub64(tick, minimum_visible_tick);

  // Multiply the y coordinate with the y entry of the transformation matrix.
  // To avoid a degenerate matrix, C++ code supplies a matrix containing the
  // square roots of the actual matrix to the shader code, so apply the float
  // twice
  float temp_y = Multiply96BitWithFloat(address_coordinate_translated,
                                        scale_heap_to_screen[1][1]);
  float final_y = temp_y * scale_heap_to_screen[1][1];

  float temp_x = Multiply64BitWithFloat(tick_coordinate_translated,
                                        scale_heap_to_screen[0][0]);
  float final_x = temp_x * scale_heap_to_screen[0][0];

  final_y = 2 * final_y - 1;
  final_x = 2 * final_x - 1;
  // ==========================================================================
  // End of mandatory valid GLSL part.
  // ==========================================================================
  return std::make_pair(final_x, final_y);
}
