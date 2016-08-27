#include "heaphistory.h"
#include "heapwindow.h"
#include "displayheapwindow.h"

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

  maximum_width_ = Sub64(maximum_tick_, minimum_tick_);
  maximum_height_ = Sub96(maximum_address_, minimum_address_);
}

void DisplayHeapWindow::checkHorizontalCenter(ivec2 *new_minimum_tick,
                                              ivec2 *new_maximum_tick) const {
  ivec2 width_64 = Sub64(*new_maximum_tick, *new_minimum_tick);
  long double width = width_64.getUint64();
  long double half_width = width / 2;
  ivec2 half_width_64 = LongDoubleTo64Bits(half_width);
  ivec2 horizontal_center = Add64(*new_minimum_tick, half_width_64);

  // The horizontal center should not fall below zero.
  if (horizontal_center.y < 0) {
    *new_minimum_tick = LongDoubleTo64Bits(-half_width);
    *new_maximum_tick = LongDoubleTo64Bits(half_width);
  } else if (horizontal_center.y > 0x17) {
    ivec2 maximal_horizontal_center(0, 0x17);
    *new_maximum_tick = Add64(maximal_horizontal_center, half_width_64);
    *new_minimum_tick = Sub64(maximal_horizontal_center, half_width_64);
  }
}

void DisplayHeapWindow::checkVerticalCenter(ivec3 *new_minimum_address,
                                            ivec3 *new_maximum_address) const {
  ivec3 height_96 = Sub96(*new_maximum_address, *new_minimum_address);
  long double height = height_96.getLongDouble();
  long double half_height = height / 2;
  ivec3 half_height_96 = LongDoubleTo96Bits(half_height);
  ivec3 vertical_center = Add96(*new_minimum_address, half_height_96);

  // The vertical center should not fall below zero.
  if (vertical_center.z < 0) {
    *new_minimum_address = LongDoubleTo96Bits(-half_height);
    *new_maximum_address = LongDoubleTo96Bits(half_height);
  } else if (vertical_center.z > 0x17) {
    ivec3 maximal_vertical_center(0, 0, 0x17);
    *new_maximum_address = Add96(maximal_vertical_center, half_height_96);
    *new_minimum_address = Sub96(maximal_vertical_center, half_height_96);
  }
}

void DisplayHeapWindow::pan(double dx, double dy) {
  // Calculate the height and width of the window as long doubles.
  long double height = getHeightAsLongDouble();
  long double width = getWidthAsLongDouble();
  long double pan_x = -dx * width;
  long double pan_y = dy * height;
  ivec2 pan_x_64 = LongDoubleTo64Bits(pan_x);
  ivec2 new_minimum_tick = Add64(minimum_tick_, pan_x_64);
  ivec2 new_maximum_tick = Add64(maximum_tick_, pan_x_64);
  ivec3 pan_y_96 = LongDoubleTo96Bits(pan_y);
  ivec3 new_maximum_address = Add96(maximum_address_, pan_y_96);
  ivec3 new_minimum_address = Add96(minimum_address_, pan_y_96);
  // Ensure that the center of the screen definitely stays in bounds.
  // Horizontal dimension.
  checkHorizontalCenter(&new_minimum_tick, &new_maximum_tick);
  // Vertical dimension.
  checkVerticalCenter(&new_minimum_address, &new_maximum_address);

  maximum_tick_ = new_maximum_tick;
  minimum_tick_ = new_minimum_tick;
  maximum_address_ = new_maximum_address;
  minimum_address_ = new_minimum_address;
}

void DisplayHeapWindow::zoomToPoint(double dx, double dy, double how_much_x,
                                    double how_much_y, long double max_height,
                                    long double max_width) {
  long double epsilon = 0.05;
  long double height = getHeightAsLongDouble();
  long double width = getWidthAsLongDouble();
  long double target_height = height * how_much_y;
  long double target_width = width * how_much_x;
  if (target_height > maximum_height_.getLongDouble()) {
    target_height = maximum_height_.getLongDouble();
  }
  if (target_width > maximum_width_.getLongDouble()) {
    target_width = maximum_width_.getLongDouble();
  }
  double extra_width = target_width - width;
  long double extra_height = target_height - height;

  // Do not allow the height or width to be more than 2x total heap size.

  // Make sure we move a little more toward the point than we need to keep the
  // point constant on screen.
  if (dx > 0.5) {
    dx += epsilon;
  } else {
    dx -= epsilon;
  }
  if (dy > 0.5) {
    dy += epsilon;
  } else {
    dy -= epsilon;
  }

  long double extra_width_right = (1.0 - dx) * extra_width;
  long double extra_width_left = dx * extra_width;
  long double extra_height_top = dy * extra_height;
  long double extra_height_bottom = (1.0 - dy) * extra_height;

  ivec3 new_maximum_address =
      Add96(maximum_address_, LongDoubleTo96Bits(extra_height_top));
  ivec3 new_minimum_address =
      Sub96(minimum_address_, LongDoubleTo96Bits(extra_height_bottom));
  ivec2 new_minimum_tick =
      Sub64(minimum_tick_, LongDoubleTo64Bits(extra_width_left));
  ivec2 new_maximum_tick =
      Add64(maximum_tick_, LongDoubleTo64Bits(extra_width_right));

  // Now ensure that the center is not outside of bounds.
  checkHorizontalCenter(&new_minimum_tick, &new_maximum_tick);
  checkVerticalCenter(&new_minimum_address, &new_maximum_address);

  maximum_tick_ = new_maximum_tick;
  minimum_tick_ = new_minimum_tick;
  maximum_address_ = new_maximum_address;
  minimum_address_ = new_minimum_address;
}

// Map screen coordinates back to the heap, returns false if the coordinate
// can't fall into the heap because it has a negative component.
bool DisplayHeapWindow::mapDisplayCoordinateToHeap(double dx, double dy,
                                                   uint32_t *tick,
                                                   uint64_t *address) const {
  long double height = getHeightAsLongDouble();
  long double width = getWidthAsLongDouble();
  long double relative_x = dx * width;
  long double relative_y = (1.0 - dy) * height;

  ivec3 tentative_address =
      Add96(LongDoubleTo96Bits(relative_y), minimum_address_);
  int64_t tentative_tick = relative_x + minimum_tick_.getInt64();

  // Check if the numbers are in bounds.
  if ((tentative_tick < 0) || (tentative_tick > 0xFFFFFFFFFUL) ||
      (tentative_address.z > 0xF) || (tentative_address.z < 0)) {
    return false;
  }
  // Return the values.
  uint64_t final_address = tentative_address.getLowUint64() >> 4;
  final_address |= tentative_address.z >> 4;
  *address = final_address;
  *tick = tentative_tick >> 4;
  return true;
}

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

long double DisplayHeapWindow::getHeightAsLongDouble() const {
  long double shift32 = static_cast<long double>(0x100000000);
  ivec3 height = Sub96(maximum_address_, minimum_address_);
  return height.getLongDouble();
}

long double DisplayHeapWindow::getWidthAsLongDouble() const {
  long double result = static_cast<long double>(maximum_tick_.getUint64() -
                                                minimum_tick_.getUint64());
  return result;
}

std::pair<float, float>
DisplayHeapWindow::mapHeapCoordinateToDisplay(uint32_t tick,
                                              uint64_t address) const {
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
    float scale_heap_x, float scale_heap_y) const {
  float scale_heap_to_screen[2][2] = {{scale_heap_x, 0.0}, {0.0, scale_heap_y}};

  // =========================================================================
  // Everything below should be valid C++ and also valid GLSL! This code is
  // shared between displayheapwindow.cpp and simple.vert, so make sure it
  // always stays in synch!!
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
  ivec2 minimum_visible_tick = ivec2(visible_tick_base_A, visible_tick_base_B);

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
  // XXX:DEBUG CODE

  if (debug_mode_) {
    printf("[Debug]   address is %08lx%08lx%08lx\n", address.z, address.y,
           address.x);
    printf("[Debug]   tick is %08lx%08lx\n", tick.y, tick.x);
    printf("[Debug]   minimum_visible_tick is %08lx%08lx\n",
           minimum_visible_tick.y, minimum_visible_tick.x);
    printf("[Debug]   heap_base is %08lx%08lx%08lx\n", heap_base.z, heap_base.y,
           heap_base.x);
    printf("[Debug]   address_coordinate_translated is %08lx%08lx%08lx\n",
           address_coordinate_translated.z, address_coordinate_translated.y,
           address_coordinate_translated.x);
    printf("[Debug]   tick_coordinate_translated is %08lx%08lx\n",
           tick_coordinate_translated.y, tick_coordinate_translated.x);
    printf("[Debug]   temp_x is %f, temp_y is %f\n", temp_x, temp_y);
    printf("[Debug]   scale_heap_to_screen[0][0] is %f, "
           "scale_heap_to_screen[1][1] is %f\n ",
           scale_heap_to_screen[0][0], scale_heap_to_screen[1][1]);
    printf("[Debug]   final_x is %f, final_y is %f\n", final_x, final_y);
  }

  return std::make_pair(final_x, final_y);
}
