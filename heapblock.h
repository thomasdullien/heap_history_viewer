#ifndef HEAPBLOCK_H
#define HEAPBLOCK_H

#include <cstdint>
#include <cstddef>
#include <string>

#include "vertex.h"

// A simple POD class for memory blocks in the heap diagram.
class HeapBlock {
public:
  HeapBlock();
  // Constructor for the most common case: Well-defined start, unknown end.
  HeapBlock(uint32_t start_tick, uint32_t size, uint64_t address, const std::string* alloctag);
  // Constructor for the case that the end is known.
  HeapBlock(uint32_t start_tick, uint32_t end_tick, uint32_t size,
            uint64_t address);
  // Create vertices for the block. Since the block consists of 2 triangles with
  // 3 vertices each, and since the third dimension will be set by the shader,
  // this function will write 6 floats to output_vertices. The correction term
  // will be subtracted from the address value before writing the vertices to
  // align the lowest recorded allocation with uint64_t 0. This should
  // hopefully help reduce rounding errors / issues.
  void toVertices(uint32_t max_tick, std::vector<HeapVertex> *output_vertices,
                  bool debug = false) const;
  // Check if a given point is inside the current block.
  bool contains(uint32_t tick, uint64_t address) {
    return (tick >= start_tick_) && (tick <= end_tick_) &&
           (address >= address_) && (address <= address_ + size_);
  }
  bool wasFreed() const { return end_tick_ != std::numeric_limits<uint32_t>::max(); }

  uint32_t start_tick_;
  uint32_t end_tick_;
  uint32_t size_;
  uint64_t address_;
  bool highlighted_;
  const std::string* allocation_tag_;
  const std::string* free_tag_;
};

std::string getBlockInformationAsString(const HeapBlock& block);

#endif // HEAPBLOCK_H
