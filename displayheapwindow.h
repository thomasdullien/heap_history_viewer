#ifndef DISPLAYHEAPWINDOW_H
#define DISPLAYHEAPWINDOW_H

#include "heapwindow.h"

// A simple implementation of an ivec2 and ivec 3 to allow the C++ code
// to resemble GLSL code more.
class ivec2 {
public:
  ivec2() {
    x = 0;
    y = 0;
  }
  ivec2(int32_t a, int32_t b) {
    x = a;
    y = b;
  }
  void setUint64(uint64_t a) {
    x = a & 0xFFFFFFFF;
    y = a >> 32;
  }
  void setInt64(int64_t a) {
    x = a & 0xFFFFFFFF;
    y = a >> 32;
  }
  uint64_t getUint64() const {
    uint64_t temp = (static_cast<uint64_t>(static_cast<uint32_t>(y))) << 32;
    uint64_t result = temp | static_cast<uint64_t>(static_cast<uint32_t>(x));
    return result;
  }
  int64_t getInt64() const {
    return static_cast<int64_t>(getUint64());
  }
  void flipBit(uint32_t index) {
    if (index >= 32) {
      index -= 32;
      y ^= 1 << index;
      return;
    }
    x ^= 1 << index;
    return;
  }

  union {
    struct {
      int32_t x;
      int32_t y;
    };
    uint64_t val;
  };
};

class ivec3 {
public:
  ivec3() {
    x = 0;
    y = 0;
    z = 0;
  }
  ivec3(int32_t a, int32_t b, int32_t c) {
    x = a;
    y = b;
    z = c;
  }

  uint64_t getLowUint64() const {
    return (static_cast<uint64_t>(static_cast<uint32_t>(y)) << 32) | x;
  }
  void setLowUint64(uint64_t val) {
    x = val & 0xFFFFFFFF;
    y = val >> 32;
  }
  uint32_t getUpper32() const { return z; }
  long double getLongDouble() const {
    long double shift32 = static_cast<long double>(0x100000000);
    long double result = z;
    result *= shift32;
    long double y2 = y;
    result += y2;
    result *= shift32;
    long double x2 = x;
    result += x2;
    return result;
  };

  void flipBit(uint32_t index) {
    if (index >= 64) {
      index -= 64;
      z ^= 1 << index;
      return;
    }
    if (index >= 32) {
      index -= 32;
      y ^= 1 << index;
      return;
    }
    x ^= 1 << index;
    return;
  }

  union {
    struct {
      int32_t z;
      int32_t y;
      int32_t x;
    };
    struct {
      uint32_t upper_32;
      uint64_t lower_64;
    };
  };
};

// Utility functions to emulate 64-bit and 96-bit arithmetic in GLSL. Since
// GLSL is so horrible to debug, make sure the code for these functions stays
// both valid C++ and valid GLSL, so the unit tests for this class can help
// debug the shader issues.
ivec2 Add64(ivec2 a, ivec2 b);
ivec2 Sub64(ivec2 a, ivec2 b);
float Multiply64BitWithFloat(ivec2 a, float b);
float Multiply64BitWithFloat(ivec2 a, float b);
ivec3 Add96(ivec3 a, ivec3 b);
ivec3 Sub96(ivec3 a, ivec3 b);
float Multiply96BitWithFloat(ivec3 a, float b);
int TopNibble(int value);
ivec3 LongDoubleTo96Bits(long double value);
ivec3 Load64BitLeftShiftedBy4Into96Bit(int low, int high);
ivec2 Load32BitLeftShiftedBy4Into64Bit(int low);

// A window into the heap for display. Since such a window needs to be pannable
// and zoomable, it can't simply use uint64_t for the corner coordinates - sub-
// integer increments need to be possible. Furthermore, negative numbers need
// to be allowed as well.
//
// The x coordinates need to have a range of [-2^32, 2^33], the y coordinates
// need to have a range of [-2^64, 2^65 ].
//
// Lastly, arithmetic on these coordinates needs to be possible on the GPU
// which means that no doubles nor "true" uint64_t arithmetic - int32_t is all
// we have, and everything else needs to be emulated.
//
// The "solution" to this is the following:
// - Use 64-bit integers for the x coordinate. The lowest 4 bits are the frac-
//   tional component.
// - Use 96-bit integers for the y coordinate. Again, the lowest 4 bits are the
//   fractional component.
//
class DisplayHeapWindow {
public:
  DisplayHeapWindow();
  DisplayHeapWindow(const ivec2 &minimum_tick, const ivec2 &maximum_tick,
                    const ivec3 &minimum_address, const ivec3 &maximum_address);
  void reset(const HeapWindow &global_window);
  void pan(double dx, double dy);
  void zoomToPoint(double dx, double dy, double how_much_x, double how_much_y);
  std::pair<float, float> mapHeapCoordinateToDisplay(uint32_t tick,
                                                     uint64_t address) const;
  // Map screen coordinates back to the heap, returns false if the coordinate
  // does not fall into the heap.
  bool mapDisplayCoordinateToHeap(double dx, double dy, uint32_t *tick,
                                  uint64_t *address) const;
  void setMinimumTick(ivec2 minimum_tick);
  void setMaximumTick(ivec2 maximum_tick);
  void setMinimumAddress(ivec3 address);
  void setMaximumAddress(ivec3 address);

  ivec2 getMinimumTick() const { return minimum_tick_; }
  ivec3 getMinimumAddress() const { return minimum_address_; }

  long double getXScalingHeapToScreen() const;
  long double getYScalingHeapToScreen() const;

  long double getHeightAsLongDouble() const;
  long double getWidthAsLongDouble() const;
private:
  // The internal version of the above mapping function. The code should be
  // kept in a state so that it can be cut & pasted into the GLSL files with-
  // out much modification.
  std::pair<float, float> internalMapHeapCoordinateToDisplay(
      ivec3 position, int visible_heap_base_A, int visible_heap_base_B,
      int visible_heap_base_C, int visible_tick_base_A, int visible_tick_base_B,
      float squash_x, float squash_y) const;

  void checkHorizontalCenter(ivec2 *new_minimum_tick, ivec2 *new_maximum_tick) const;
  void checkVerticalCenter(ivec3 *new_minimum_address, ivec3 *new_maximum_address) const;

  ivec2 minimum_tick_;
  ivec2 maximum_tick_;
  ivec3 minimum_address_;
  ivec3 maximum_address_;
};

#endif // DISPLAYHEAPWINDOW_H
