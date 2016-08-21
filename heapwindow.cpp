#include "heapwindow.h"

HeapWindow::HeapWindow(uint64_t min, uint64_t max, uint32_t mintick,
                       uint32_t maxtick)
    : minimum_address_(min), maximum_address_(max), minimum_tick_(mintick),
      maximum_tick_(maxtick) {}
