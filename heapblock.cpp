#include <limits>
#include "heapblock.h"

HeapBlock::HeapBlock() {}

HeapBlock::HeapBlock(uint32_t start_tick, uint32_t size, uint64_t address)
    : start_tick_(start_tick), end_tick_(std::numeric_limits<uint32_t>::max()),
      size_(size), address_(address) {}

HeapBlock::HeapBlock(uint32_t start_tick, uint32_t end_tick, uint32_t size,
                     uint64_t address)
    : start_tick_(start_tick), end_tick_(end_tick), size_(size),
      address_(address) {}

void HeapBlock::toVertices(uint32_t max_tick,
                           std::vector<HeapVertex> *vertices) const {
  static const QVector3D color_allocated_light(0.0f, 0.5f, 0.0f);
  static const QVector3D color_allocated_dark(0.0f, 0.3f, 0.0f);
  static const QVector3D color_freed_light(0.5f, 0.5f, 0.5f);
  static const QVector3D color_freed_dark(0.3f, 0.3f, 0.3f);

  QVector3D const *current_color_light = &color_freed_light;
  QVector3D const *current_color_dark = &color_freed_dark;

  float lower_left_x = start_tick_;
  float lower_left_y = address_;
  float lower_right_x = end_tick_;
  // The block is not free
  if (end_tick_ == std::numeric_limits<uint32_t>::max()) {
    lower_right_x = max_tick * 1.1;
    current_color_light = &color_allocated_light;
    current_color_dark = &color_allocated_dark;
  }
  float lower_right_y = lower_left_y;
  float upper_right_x = lower_right_x;
  float upper_right_y = address_ + size_;
  float upper_left_x = lower_left_x;
  float upper_left_y = upper_right_y;

  // Create new vertices.
  vertices->push_back(HeapVertex(lower_left_x, lower_left_y, *current_color_light));
  vertices->push_back(
      HeapVertex(lower_right_x, lower_right_y, *current_color_dark));
  vertices->push_back(HeapVertex(upper_left_x, upper_left_y, *current_color_dark));
  vertices->push_back(
      HeapVertex(lower_right_x, lower_right_y, *current_color_dark));
  vertices->push_back(
      HeapVertex(upper_right_x, upper_right_y, *current_color_light));
  vertices->push_back(HeapVertex(upper_left_x, upper_left_y, *current_color_dark));
}
