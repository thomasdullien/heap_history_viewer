#ifndef HEAPWINDOW_H
#define HEAPWINDOW_H
#include "stdint.h"

class HeapWindow {
public:
  HeapWindow(uint64_t min, uint64_t max, uint32_t mintick, uint32_t maxtick);
  uint64_t height() { return maximum_address_ - minimum_address_; }
  uint32_t width() { return maximum_tick_ - minimum_tick_; }
  void reset(const HeapWindow &window) { *this = window; }
  uint64_t minimum_address_;
  uint64_t maximum_address_;
  uint32_t minimum_tick_;
  uint32_t maximum_tick_;
};

#endif // HEAPWINDOW_H
