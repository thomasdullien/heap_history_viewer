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

  uint32_t lower_left_x = start_tick_;
  uint64_t lower_left_y = address_;
  uint32_t lower_right_x = end_tick_;
  // The block is not free
  if (end_tick_ == std::numeric_limits<uint32_t>::max()) {
    lower_right_x = max_tick * 1.1;
    if (lower_right_x == max_tick) {
      // Sometimes max_tick is too small still.
      lower_right_x = max_tick+5;
    }
    current_color_light = &color_allocated_light;
    current_color_dark = &color_allocated_dark;
  }
  uint64_t lower_right_y = lower_left_y;
  uint32_t upper_right_x = lower_right_x;
  uint64_t upper_right_y = address_ + size_;
  uint32_t upper_left_x = lower_left_x;
  uint64_t upper_left_y = upper_right_y;

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
/*
  static const QVector3D color_A(0.0, 0.0, 1.0);
  static const QVector3D color_B(1.0, 0.0, 1.0);
  static const QVector3D color_C(1.0, 1.0, 1.0);
  vertices->push_back(HeapVertex(lower_left_x, lower_left_y,
                                 color_A));
  vertices->push_back(
      HeapVertex(lower_right_x, lower_right_y,
                 color_B));
  vertices->push_back(HeapVertex(upper_left_x, upper_left_y,
                                 color_C));*/
  /*vertices->push_back(
      HeapVertex(lower_right_x, lower_right_y, color_A));
  vertices->push_back(
      HeapVertex(upper_right_x, upper_right_y, color_B));
  vertices->push_back(HeapVertex(upper_left_x, upper_left_y, color_C));*/
}
