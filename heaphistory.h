#ifndef HEAPHISTORY_H
#define HEAPHISTORY_H

#include <cstdint>
#include <map>
#include <vector>

#include <QVector3D>

#include "heapblock.h"
#include "vertex.h"

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

class ContinuousHeapWindow {
public:
  ContinuousHeapWindow() {}
  ContinuousHeapWindow(uint64_t min, uint64_t max, uint32_t mintick,
                       uint32_t maxtick);
  double height() { return maximum_address_ - minimum_address_; }
  double width() { return maximum_tick_ - minimum_tick_; }
  void reset(const HeapWindow &window) {
    minimum_address_ = window.minimum_address_;
    maximum_address_ = window.maximum_address_;
    minimum_tick_ = window.minimum_tick_;
    maximum_tick_ = window.maximum_tick_;
  }
  double minimum_address_;
  double maximum_address_;
  double minimum_tick_;
  double maximum_tick_;
};

class HeapConflict {
public:
  HeapConflict(uint32_t tick, uint64_t address, bool alloc);
  uint32_t tick_;
  uint64_t address_;
  bool allocation_or_free_;
};

class HeapHistory {
public:
  HeapHistory();
  size_t
  getActiveBlocks(std::vector<std::vector<HeapBlock>::iterator> *active_blocks);
  void setCurrentWindow(const HeapWindow &new_window);
  void setCurrentWindowToGlobal() { current_window_.reset(global_area_); }
  const ContinuousHeapWindow &getCurrentWindow() const {
    return current_window_;
  }
  const ContinuousHeapWindow &getGridWindow(uint32_t number_of_lines);

  // Input reading.
  void LoadFromJSONStream(std::istream& jsondata);

  // Attempts to find a block at a given address and tick. Currently broken,
  // hence the two implementations (slow works).
  // TODO(thomasdullien): Debug and fix the fast version.
  bool getBlockAt(uint64_t address, uint32_t tick, HeapBlock *result,
                  uint32_t *index);
  bool getBlockAtSlow(uint64_t address, uint32_t tick, HeapBlock *result,
                      uint32_t *index);

  // Record a memory allocation event. The code supports up to 256 different
  // heaps.
  void recordMalloc(uint64_t address, size_t size, uint8_t heap_id = 0);
  void recordFree(uint64_t address, uint8_t heap_id = 0);
  void recordRealloc(uint64_t old_address, uint64_t new_address, size_t size,
                     uint8_t heap_id);

  // Dump out triangles for the current window of heap events.
  size_t dumpVerticesForActiveWindow(std::vector<HeapVertex> *vertices);
  uint64_t getMinimumAddress() { return global_area_.minimum_address_; }
  uint64_t getMaximumAddress() { return global_area_.maximum_address_; }
  uint32_t getMinimumTick() { return global_area_.minimum_tick_; }
  uint32_t getMaximumTick() { return global_area_.maximum_tick_; }

  double getXProjectionEntry();
  double getYProjectionEntry();
  double getXTranslationEntry();
  double getYTranslationEntry();

  // Functions for moving the currently visible window around.
  void panCurrentWindow(double dx, double dy);
  void zoomToPoint(double dx, double dy, double how_much_x, double how_much_y);

private:
  void recordMallocConflict(uint64_t address, size_t size, uint8_t heap_id);
  void recordFreeConflict(uint64_t address, uint8_t heap_id);
  bool isBlockActive(const HeapBlock &block);
  // Dumps 6 vertices for 2 triangles for a block into the output vector.
  // TODO(thomasdullien): Optimize this to only dump 4 vertices?
  void HeapBlockToVertices(const HeapBlock &block,
                           std::vector<HeapVertex> *vertices);
  // When a new block has been put into the vector, this function needs to be
  // called to update the internal data structures for fast block search.
  void updateCachedSortedIterators();

  std::vector<std::vector<HeapBlock>::iterator>
      cached_blocks_sorted_by_address_;
  // Running counter to keep track of heap events.
  uint32_t current_tick_;
  // The currently active (visible, to-be-displayed) part of the heap history.
  ContinuousHeapWindow current_window_;
  // The rectangle for the grid drawing.
  ContinuousHeapWindow grid_rectangle_;

  // The global size of all heap events.
  HeapWindow global_area_;
  // The vector of all heap blocks. This vector will be sorted by the minimum
  // tick of their allocation.
  std::vector<HeapBlock> heap_blocks_;
  // A map to keep track of blocks that are "currently live".
  std::map<std::pair<uint64_t, uint8_t>, size_t> live_blocks_;
  // A vector of ticks that records the conflicts in heap logic.
  std::vector<HeapConflict> conflicts_;
};

#endif // HEAPHISTORY_H
