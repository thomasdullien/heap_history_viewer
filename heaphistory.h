#ifndef HEAPHISTORY_H
#define HEAPHISTORY_H

#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include <QVector3D>

#include "displayheapwindow.h"
#include "heapblock.h"
#include "heapwindow.h"
#include "vertex.h"

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

  const DisplayHeapWindow &getCurrentWindow() const {
    return current_window_;
  }

  // Input reading.
  void LoadFromJSONStream(std::istream &jsondata);

  // Attempts to find a block at a given address and tick. Currently broken,
  // hence the two implementations (slow works).
  // TODO(thomasdullien): Debug and fix the fast version.
  bool getBlockAt(uint64_t address, uint32_t tick, HeapBlock *result,
                  uint32_t *index);
  bool getBlockAtSlow(uint64_t address, uint32_t tick, HeapBlock *result,
                      uint32_t *index);

  // Record a memory allocation event. The code supports up to 256 different
  // heaps.
  void recordMalloc(uint64_t address, size_t size, const std::string* alloc_tag, uint8_t heap_id = 0);
  void recordFree(uint64_t address, const std::string* tag, uint8_t heap_id = 0);
  void recordRealloc(uint64_t old_address, uint64_t new_address, size_t size,
                     uint8_t heap_id);
  void recordEvent(const std::string& event_label);
  void recordAddress(uint64_t address, const std::string& label);

  // Dump out triangles for the current window of heap events.
  size_t heapBlockVerticesForActiveWindow(std::vector<HeapVertex> *vertices);
  uint64_t getMinimumAddress() { return global_area_.minimum_address_; }
  uint64_t getMaximumAddress() { return global_area_.maximum_address_; }
  uint32_t getMinimumTick() { return global_area_.minimum_tick_; }
  uint32_t getMaximumTick() { return global_area_.maximum_tick_; }

  // Functions for moving the currently visible window around.
  void panCurrentWindow(double dx, double dy);
  void zoomToPoint(double dx, double dy, double how_much_x, double how_much_y,
                   long double max_height, long double max_width);

  void eventsToVertices(std::vector<HeapVertex> *vertices);
  void addressesToVertices(std::vector<HeapVertex> *vertices);
  bool getEventAtTick(uint32_t tick, std::string* eventstring);
private:
  void recordMallocConflict(uint64_t address, size_t size, uint8_t heap_id);
  void recordFreeConflict(uint64_t address, uint8_t heap_id);
  void recordFreeRange(uint64_t low_end, uint64_t high_end, const std::string *tag, uint8_t heap_id);
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
  DisplayHeapWindow current_window_;

  // The global size of all heap events.
  HeapWindow global_area_;
  // The vector of all heap blocks. This vector will be sorted by the minimum
  // tick of their allocation.
  std::vector<HeapBlock> heap_blocks_;
  // A map to keep track of blocks that are "currently live".
  std::map<std::pair<uint64_t, uint8_t>, size_t> live_blocks_;
  // A vector of ticks that records the conflicts in heap logic.
  std::vector<HeapConflict> conflicts_;

  // A tick-to-string mapping for events.
  std::map<uint32_t, std::string> tick_to_event_strings_;

  // An address-to-string mapping for horizontal lines.
  std::map<uint64_t, std::string> address_to_address_strings_;

  std::set<std::string> alloc_or_free_tags_;
};

#endif // HEAPHISTORY_H
