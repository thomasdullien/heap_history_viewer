#ifndef HEAPBLOCK_H
#define HEAPBLOCK_H

#include <cstdint>
#include <cstddef>

#include "vertex.h"

// A simple POD class for memory blocks in the heap diagram.
class HeapBlock {
public:
  HeapBlock();
  // Constructor for the most common case: Well-defined start, unknown end.
  HeapBlock(uint32_t start_tick, uint32_t size, uint64_t address);
  // Constructor for the case that the end is known.
  HeapBlock(uint32_t start_tick, uint32_t end_tick, uint32_t size,
            uint64_t address);
  // Create vertices for the block. Since the block consists of 2 triangles with
  // 3 vertices each, and since the third dimension will be set by the shader,
  // this function will write 6 floats to output_vertices.
  void toVertices(uint32_t max_tick, std::vector<HeapVertex>* output_vertices) const;
  // Check if a given point is inside the current block.
  bool contains(uint32_t tick, uint64_t address) {
    return (tick >= start_tick_) && (tick <= end_tick_) &&
           (address >= address_) && (address <= address_ + size_);
  }
  bool wasFreed() {
    return end_tick_ != std::numeric_limits<uint32_t>::max();
  }

  uint32_t start_tick_;
  uint32_t end_tick_;
  uint32_t size_;
  uint64_t address_;
};

#endif // HEAPBLOCK_H
