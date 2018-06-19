#include <limits>
#include <sstream>
#include "linearbrightnesscolorscale.h"
#include "heapblock.h"

HeapBlock::HeapBlock() {}

HeapBlock::HeapBlock(uint32_t start_tick, uint32_t size, uint64_t address,
                     const std::string *alloctag = nullptr)
    : start_tick_(start_tick), end_tick_(std::numeric_limits<uint32_t>::max()),
      size_(size), address_(address), highlighted_(false), allocation_tag_(alloctag),
      free_tag_(nullptr) {}

HeapBlock::HeapBlock(uint32_t start_tick, uint32_t end_tick, uint32_t size,
                     uint64_t address)
    : start_tick_(start_tick), end_tick_(end_tick), size_(size),
      address_(address), highlighted_(false) {}

void HeapBlock::toVertices(uint32_t max_tick, std::vector<HeapVertex> *vertices,
                           bool debug) const {

  std::pair<QVector3D, QVector3D> colors =
    highlighted_ ?
    LinearBrightnessColorScale::highlightedColorsFromTick(start_tick_, end_tick_, max_tick)
    : LinearBrightnessColorScale::colorsFromTick(start_tick_, end_tick_, max_tick);

  uint32_t lower_left_x = start_tick_;
  uint64_t lower_left_y = address_;
  uint32_t lower_right_x = end_tick_;

  uint64_t lower_right_y = lower_left_y;
  uint32_t upper_right_x = lower_right_x;
  uint64_t upper_right_y = address_ + size_;
  uint32_t upper_left_x = lower_left_x;
  uint64_t upper_left_y = upper_right_y;

  // Create new vertices.
  if (!debug) {
    vertices->push_back(HeapVertex(lower_left_x, lower_left_y, colors.second));
    vertices->push_back(HeapVertex(lower_right_x, lower_right_y, colors.second));
    vertices->push_back(HeapVertex(upper_left_x, upper_left_y, colors.first));
    vertices->push_back(HeapVertex(lower_right_x, lower_right_y, colors.second));
    vertices->push_back(
        HeapVertex(upper_right_x, upper_right_y, colors.first));
    vertices->push_back(HeapVertex(upper_left_x, upper_left_y, colors.first));
  } else {
    // In debugging mode, make sure that the colors form unit triangles.
    static const QVector3D color_A(0.0, 0.0, 1.0);
    static const QVector3D color_B(1.0, 0.0, 1.0);
    static const QVector3D color_C(1.0, 1.0, 1.0);
    vertices->push_back(HeapVertex(lower_left_x, lower_left_y, color_A));
    vertices->push_back(HeapVertex(lower_right_x, lower_right_y, color_B));
    vertices->push_back(HeapVertex(upper_left_x, upper_left_y, color_C));
  }
}

std::string getBlockInformationAsString(const HeapBlock &block) {
  std::stringstream stringstream;
  stringstream << " Block address: " << std::hex << block.address_;
  stringstream << " Size: " << block.size_ << std::dec << "(" << block.size_
               << ")";
  stringstream << " AllocationTick: " << block.start_tick_;
  if (block.allocation_tag_ != nullptr) {
    stringstream << " AllocationTag: " << *block.allocation_tag_;
  }
  if (block.end_tick_ == std::numeric_limits<uint32_t>::max()) {
    stringstream << " [Currently Alive] ";
  } else {
    stringstream << " FreeTick: " << block.end_tick_;
    if (block.free_tag_ != nullptr) {
      stringstream << " FreeTag: " << *block.free_tag_;
    }
  }
  return stringstream.str();
}
