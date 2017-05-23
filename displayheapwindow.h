#ifndef DISPLAYHEAPWINDOW_H
#define DISPLAYHEAPWINDOW_H

#include "glsl_simulation_functions.h"
#include "heapwindow.h"
#include "vertex.h"

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
  void zoomToPoint(double dx, double dy, double how_much_x, double how_much_y,
                   long double max_height, long double max_width);
  std::pair<float, float> mapHeapCoordinateToDisplay(uint32_t tick,
                                                     uint64_t address) const;
  // Map screen coordinates back to the heap, returns false if the coordinate
  // does not fall into the heap.
  bool mapDisplayCoordinateToHeap(double dx, double dy, uint32_t *tick,
                                  uint64_t *address) const;
  bool setMinAndMaxTick(ivec2 min_tick, ivec2 max_tick);
  bool setMinAndMaxAddress(ivec3 min_address, ivec3 max_address);

  ivec2 getMinimumTick() const { return minimum_tick_; }
  ivec3 getMinimumAddress() const { return minimum_address_; }

  long double getXScalingHeapToScreen() const;
  long double getYScalingHeapToScreen() const;

  long double getHeightAsLongDouble() const;
  long double getWidthAsLongDouble() const;

  void checkInternalValuesForSanity() const;
  void setDebug(bool mode) const { debug_mode_ = mode; }
  // Debugging functions to help debug the GLSL shader code in C++.
  void internalMapAddressCoordinateToDisplay(ivec3 position,
    int visible_heap_base_A, int visible_heap_base_B,
    int visible_heap_base_C, int visible_tick_base_A, int visible_tick_base_B,
    float scale_heap_x, float scale_heap_y) const;
  void debugDumpHeapVertex(const HeapVertex& vertex) const;
  void debugDumpHeapVerticesToAddressMapper(
    const std::vector<HeapVertex>* vertices) const;

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

  ivec2 maximum_width_;
  ivec3 maximum_height_;

  mutable bool debug_mode_;
};

#endif // DISPLAYHEAPWINDOW_H
