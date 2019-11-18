#include "displayheapwindow.h"
#include "glsl_simulation_functions.h"
#include "heapwindow.h"

#include <cinttypes>

DisplayHeapWindow::DisplayHeapWindow() = default;

DisplayHeapWindow::DisplayHeapWindow(const ivec2 &minimum_tick,
  const ivec2 &maximum_tick,
  const ivec3 &minimum_address,
  const ivec3 &maximum_address) {
  setMinAndMaxTick(minimum_tick, maximum_tick);
  setMinAndMaxAddress(minimum_address, maximum_address);
}

void DisplayHeapWindow::reset(const HeapWindow &global_window) {
  setMinAndMaxTick(
    Load32BitLeftShiftedBy4Into64Bit(global_window.minimum_tick_),
    Load32BitLeftShiftedBy4Into64Bit(global_window.maximum_tick_));
  setMinAndMaxAddress(
    Load64BitLeftShiftedBy4Into96Bit(
      global_window.minimum_address_ & 0xFFFFFFFF,
      global_window.minimum_address_ >> 32u),
    Load64BitLeftShiftedBy4Into96Bit(
      global_window.maximum_address_ & 0xFFFFFFFF,
      global_window.maximum_address_ >> 32u));

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

  setMinAndMaxTick(new_minimum_tick, new_maximum_tick);
  setMinAndMaxAddress(new_minimum_address, new_maximum_address);
}

void DisplayHeapWindow::zoomToPoint(double dx, double dy, double how_much_x,
                                    double how_much_y, long double max_height,
                                    long double max_width) {
  (void) max_height; // TODO(patricia-gallardo): Is the parameter needed?
  (void) max_width; // TODO(patricia-gallardo): Is the parameter needed?
  long double epsilon = 0.05;
  long double height = getHeightAsLongDouble();
  long double width = getWidthAsLongDouble();
  if ((height < 0) || (width < 0)) {
    printf("[!] Something is going wrong zooming!\n");
    return;
  }
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

  setMinAndMaxTick(new_minimum_tick, new_maximum_tick);
  setMinAndMaxAddress(new_minimum_address, new_maximum_address);
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
  if ((tentative_tick < std::numeric_limits<uint32_t>::min()) ||
      (tentative_tick > std::numeric_limits<uint32_t>::max()) ||
      (tentative_address.z > 0xF) || (tentative_address.z < 0)) {
    return false;
  }
  // Return the values.
  uint64_t final_address = tentative_address.getLowUint64() >> 4u;
  uint64_t tentative_address_z = tentative_address.z;
  tentative_address_z = tentative_address_z << 60u;
  final_address |= tentative_address_z;
  *address = final_address;
  *tick = static_cast<uint32_t>(tentative_tick) >> 4u;
  return true;
}

bool DisplayHeapWindow::setMinAndMaxTick(ivec2 min_tick, ivec2 max_tick) {
  maximum_tick_ = max_tick;
  minimum_tick_ = min_tick;
  if (min_tick.getUint64() & 0x8000000000000000L) {
    printf("[Alert!] Setting min tock negative??\n");
    return false;
  }
  uint64_t width = maximum_tick_.getUint64() - minimum_tick_.getUint64();
  if (width & 0x8000000000000000L) {
    printf("[Alert!] Invalid max/min tick combination!\n");
    return false;
  }
  return true;
}

bool DisplayHeapWindow::setMinAndMaxAddress(ivec3 min_address, ivec3 max_address) {
  ivec3 height = Sub96(max_address, min_address);
  if (height.isNegative() && (max_address.x != 0)) {
    printf("[Alert!] Invalid max/min address combination!\n");
    return false;
  }
  maximum_address_ = max_address;
  minimum_address_ = min_address;
  height = Sub96(maximum_address_, minimum_address_);
  if (height.isNegative() && (max_address.x != 0)) {
    printf("[Alert!] Invalid max/min address combination!\n");
    return false;
  }
  return true;
}

void DisplayHeapWindow::checkInternalValuesForSanity() const {
  // Is maximum_tick_ and minimum_tick_ positive?
  if ((maximum_tick_.y < 0) || (minimum_tick_.y < 0)) {
    printf("[Alert!] Something is wrong with maximum_tick_ or minimum_tick_:\n"
      "%s and %s!\n",
      ivec2ToHex(maximum_tick_).c_str(), ivec2ToHex(minimum_tick_).c_str());
  }
  // Is maximum_address_ and minimum_address_ positive?
  if ((maximum_address_.z < 0) || (minimum_address_.z < 0)) {
    printf("[Alert!] Something is wrong with maximum_address_ or minimum_address_:\n"
      "%s and %s!\n",
      ivec3ToHex(maximum_address_).c_str(), ivec3ToHex(maximum_address_).c_str());
  }
  // Is maximum_tick_ - minimum_tick_ positive?
  ivec3 height = Sub96(maximum_address_, minimum_address_);
  if (height.z < 0) {
    printf("[Alert!] Something is wrong with height:\n"
      "%s!\n", ivec3ToHex(height).c_str());
  }
  // Is maximum_address_ - minimum_address_ positive?
  uint64_t width = maximum_tick_.getUint64() - minimum_tick_.getUint64();
  if (width & 0x8000000000000000L) {
    printf("[Alert!] Something is wrong with width:\n%" PRIx64 "\n", width);
  }
}

long double DisplayHeapWindow::getXScalingHeapToScreen() const {
  checkInternalValuesForSanity();
  ivec2 width = Sub64(maximum_tick_, minimum_tick_);
  uint64_t shrinkage = width.getUint64();
  long double factor =
      static_cast<long double>(1.0) / static_cast<long double>(shrinkage);
  long double result = sqrt(static_cast<long double>(factor));
  return result;
}

static int num_leading_zero_bits(uint32_t value) {
#ifdef _MSC_VER
#pragma message ( "WARNING: TODO num_leading_zero_bits not implemented on Windows" )
    return 0; // TODO(patricia-gallardo) - Implement on Windows
#else
    return __builtin_clz(value);
#endif
}

// Scaling to map the heap Y to the interval [0, 1].
long double DisplayHeapWindow::getYScalingHeapToScreen() const {
  checkInternalValuesForSanity();
  ivec3 height = Sub96(maximum_address_, minimum_address_);
  // If we are only dealing with a 64-bit height now, everything is easy.
  if (height.z == 0) {
    uint64_t shrinkage = height.getLowUint64();
    long double factor = static_cast<long double>(1.0) / shrinkage;
    return sqrt(static_cast<long double>(factor));
  }
  // We are dealing with a value bigger than 64 bit now. Get left-most
  // bit.
  uint32_t high_bit = 32 - num_leading_zero_bits(height.upper_32);
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
  ivec3 height = Sub96(maximum_address_, minimum_address_);
  return height.getLongDouble();
}

long double DisplayHeapWindow::getWidthAsLongDouble() const {
  return static_cast<long double>(maximum_tick_.getUint64() -
                                  minimum_tick_.getUint64());
}

std::pair<float, float>
DisplayHeapWindow::mapHeapCoordinateToDisplay(uint32_t tick,
                                              uint64_t address) const {
  ivec3 position(tick, address & 0xFFFFFFFF, address >> 32u);

  return internalMapHeapCoordinateToDisplay(
      position, minimum_address_.x, minimum_address_.y, minimum_address_.z,
      minimum_tick_.x, minimum_tick_.y, getXScalingHeapToScreen(),
      getYScalingHeapToScreen());
}

void DisplayHeapWindow::debugDumpHeapVertex(const HeapVertex& vertex) const {
  ivec3 position(vertex.getX(), vertex.getY() & 0xFFFFFFFF, vertex.getY() >> 32u);

  internalMapAddressCoordinateToDisplay(
      position, minimum_address_.x, minimum_address_.y, minimum_address_.z,
      minimum_tick_.x, minimum_tick_.y, getXScalingHeapToScreen(),
      getYScalingHeapToScreen());
}

void DisplayHeapWindow::debugDumpHeapVerticesToAddressMapper(
  const std::vector<HeapVertex>* vertices) const {
  printf("(");
  for (const HeapVertex& vertex : *vertices) {
    debugDumpHeapVertex(vertex);
    printf(",");
  }
  printf(")\n");
  fflush(stdout);
}

// Keep this code as close as possible to actual GSLS v1.3 shader code, so the
// code can be tested here and then cut/pasted into the shader when it works
// (since debugging GLSL is so horrible).
void DisplayHeapWindow::internalMapAddressCoordinateToDisplay(
  ivec3 position, int visible_heap_base_A, int visible_heap_base_B,
  int visible_heap_base_C, int visible_tick_base_A, int visible_tick_base_B,
  float scale_heap_x, float scale_heap_y) const {

  Q_UNUSED(visible_tick_base_A);
  Q_UNUSED(visible_tick_base_B);

  float scale_heap_to_screen[2][2] = {{scale_heap_x, 0.0}, {0.0, scale_heap_y}};
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

  Q_UNUSED(final_x);
  printf("%f ", final_y);
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
    printf("[Debug]   (%08x%08x%08x, %08x%08x) -> (%f, %f)\n", address.z,
      address.y, address.x, tick.y, tick.x, final_x, final_y);
    printf("[Debug]   minimum_visible_tick: %08x%08x, heap_base: %08x%08x%08x\n",
      minimum_visible_tick.y, minimum_visible_tick.x, heap_base.z,
      heap_base.y, heap_base.x);
    printf("[Debug] address_coordinate_translated is %08x%08x%08x, "
      "tick_coordinate_translated is %08x%08x\n", address_coordinate_translated.z,
      address_coordinate_translated.y, address_coordinate_translated.x,
      tick_coordinate_translated.y, tick_coordinate_translated.x);
    printf("[Debug]   temp_x is %f, temp_y is %f\n", temp_x, temp_y);
    printf("[Debug]   scale_heap_to_screen[0][0] is %f, "
      "scale_heap_to_screen[1][1] is %f\n",
      scale_heap_to_screen[0][0], scale_heap_to_screen[1][1]);
    printf("[Debug]-------------------------------\n");
    fflush(stdout);
  }

  return std::make_pair(final_x, final_y);
}
