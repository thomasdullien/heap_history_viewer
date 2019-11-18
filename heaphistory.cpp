#include <algorithm>
#include <limits>

#include "json.hpp"
// using json = json;

#include "heaphistory.h"

// Constructors for helper classes.

HeapConflict::HeapConflict(uint32_t tick, uint64_t address, bool alloc)
    : tick_(tick), address_(address), allocation_or_free_(alloc) {}

// The code for the heap history.
HeapHistory::HeapHistory()
    : current_tick_(0),
      global_area_(std::numeric_limits<uint64_t>::max(), 0, 0, 1) {
  setCurrentWindowToGlobal();
}

bool HeapHistory::hasMandatoryJSONElementFields(
    const nlohmann::json &json_element) {
  static std::map<std::string, std::vector<std::string>> mandatory_fields = {
      {"alloc", {"address", "size"}},
      {"free", {"address"}},
      {"filterrange", {"low", "high"}},
      {"event", {}},
      {"rangefree", {"low", "high"}},
      {"address", {"address"}}};
  if (json_element.find("type") == json_element.end()) {
    std::cout << "[E] Failed to find type field" << std::endl;
    return false;
  }
  std::string type = json_element["type"].get<std::string>();
  std::vector<std::string> mandatory = mandatory_fields[type];
  for (const std::string &field : mandatory) {
    if (json_element.find(field) == json_element.end()) {
      std::cout << "[E] Failed to find mandatory field " << field
                << " for type " << type << std::endl;
      return false;
    }
  }
  return true;
}

void HeapHistory::LoadFromJSONStream(std::istream &jsondata) {
  nlohmann::json incoming_data;
  incoming_data << jsondata;

  for (const auto &json_element : incoming_data) {
    if (!hasMandatoryJSONElementFields(json_element)) {
      printf("OH NOES MANDATORY ELEMENT MISSING\n");
      continue;
    }
    // The "type" field is mandatory for every event.
    std::string type = json_element["type"].get<std::string>();
    std::string tag = "";
    if (json_element.find("tag") != json_element.end()) {
      tag = json_element["tag"].get<std::string>();
    }
    // A not-too-intrusive gray by default.
    std::string color = "#B0B0B0";
    if (json_element.find("color") != json_element.end()) {
      color = json_element["color"].get<std::string>();
    }

    auto ret = alloc_or_free_tags_.insert(tag);
    const std::string *de_duped_tag = &*(ret.first);

    if (type == "alloc") {
      recordMalloc(json_element["address"].get<uint64_t>(),
                   json_element["size"].get<uint32_t>(), de_duped_tag, 0);
    } else if (type == "free") {
      recordFree(json_element["address"].get<uint64_t>(), de_duped_tag, 0);
    } else if (type == "event") {
      recordEvent(tag, color);
    } else if (type == "rangefree") {
      uint64_t low = json_element["low"].get<uint64_t>();
      uint64_t high = json_element["high"].get<uint64_t>();
      recordFreeRange(low, high, de_duped_tag, 0);
    } else if (type == "address") {
      recordAddress(json_element["address"].get<uint64_t>(), tag, color);
    } else if (type == "filterrange") {
      uint64_t low = json_element["low"].get<uint64_t>();
      uint64_t high = json_element["high"].get<uint64_t>();
      recordFilterRange(low, high);
    }

    fflush(stdout);
  }
  printf("heap_blocks_.size() is *%d*\n", heap_blocks_.size());
  // Sweep through the existing blocks and dump out the non-freed ones.:w
  for (const auto& block : heap_blocks_) {
    if (!block.wasFreed()) {
      printf("Alive block:\n%s\n", getBlockInformationAsString(block).c_str());
    } else {
      printf("Freed block:\n%s\n", getBlockInformationAsString(block).c_str());
    }
  }
  fflush(stdout);

  // Initialize the internal caches.
  uint64_t height = global_area_.maximum_address_
    - global_area_.minimum_address_;
  active_region_cache_ = ActiveRegionCache(height,
    &heap_blocks_);

}

// Decide whether a block is worth sending to the graphics card.
inline bool HeapHistory::isBlockActive(const HeapBlock &block,
  uint64_t min_size, uint64_t min_address, uint64_t max_address,
  uint64_t min_tick, uint64_t max_tick) const {

  // Is it big enough to be visible on the screen? (at least 1/500th of the screen height).
  if (block.size_ < min_size) {
    return false;
  }
  if (block.address_ > max_address) {
    return false;
  }
  if (block.address_ + block.size_ < min_address) {
    return false;
  }
  if (block.end_tick_ < min_tick) {
    return false;
  }
  if (block.start_tick_ > max_tick) {
    return false;
  }
  return true;
}

void HeapHistory::setCurrentWindow(const HeapWindow &new_window) {
  current_window_.reset(new_window);
}

bool HeapHistory::isEventFiltered(uint64_t address) {
  if (filter_ranges_.empty()) {
    return false;
  }
  for (const auto& filter : filter_ranges_) {
    if ((address >= filter.first) && (address <= filter.second)) {
      return false;
    }
  }
  return true;
}

void HeapHistory::recordMalloc(uint64_t address, size_t size,
                               const std::string *tag, uint8_t heap_id) {
  ++current_tick_;
  if (isEventFiltered(address)) {
    return;
  }

  // Check if there is already a live block at this address.
  if (live_blocks_.find(std::make_pair(address, heap_id)) !=
      live_blocks_.end()) {
    // Record a conflict.
    recordMallocConflict(address, size, heap_id);
    return;
  }
  heap_blocks_.push_back(HeapBlock(current_tick_, size, address, tag));
  this->cached_blocks_sorted_by_address_.clear();

  live_blocks_[std::make_pair(address, heap_id)] = heap_blocks_.size() - 1;

  global_area_.maximum_address_ =
      std::max(address + size, global_area_.maximum_address_);
  global_area_.minimum_address_ =
      std::min(address, global_area_.minimum_address_);
  // Make room for 5% more on the right hand side.
  global_area_.maximum_tick_ =
      (static_cast<double>(current_tick_) * 1.05) + 1.0;
  global_area_.minimum_tick_ = 0;
}

void HeapHistory::recordFree(uint64_t address, const std::string *tag,
                             uint8_t heap_id) {
  // Any event has to clear the sorted HeapBlock cache.
  ++current_tick_;
  if (isEventFiltered(address)) {
    return;
  }
  std::map<std::pair<uint64_t, uint8_t>, size_t>::iterator current_block =
      live_blocks_.find(std::make_pair(address, heap_id));
  if (current_block == live_blocks_.end()) {
    recordFreeConflict(address, heap_id);
    return;
  }
  size_t index = current_block->second;
  heap_blocks_[index].end_tick_ = current_tick_;
  heap_blocks_[index].free_tag_ = tag;
  live_blocks_.erase(current_block);

  // Set the max tick 5% higher than strictly necessary.
  global_area_.maximum_tick_ =
      (static_cast<double>(current_tick_) * 1.05) + 1.0;
}

void HeapHistory::recordFreeRange(uint64_t low_end, uint64_t high_end,
                                  const std::string *tag, uint8_t heap_id) {
  // Find the lower boundary.
  std::map<std::pair<uint64_t, uint8_t>, size_t>::iterator start_block =
      live_blocks_.lower_bound(std::make_pair(low_end, heap_id));
  std::map<std::pair<uint64_t, uint8_t>, size_t>::iterator end_block =
      live_blocks_.upper_bound(std::make_pair(high_end, heap_id));
  std::vector<std::pair<uint64_t, uint8_t>> blocks_to_free;

  while ((start_block != end_block) && (start_block != live_blocks_.end())) {
    uint64_t block_address = start_block->first.first;
    uint8_t block_heap_id = start_block->first.second;
    // We cannot call recordFree inside the loop because modifying the
    // live_block_ map would invalidate our
    // iterators.
    if ((block_heap_id == heap_id) && (block_address >= low_end) &&
        (block_address <= high_end)) {
      blocks_to_free.push_back(std::make_pair(block_address, heap_id));
    }
    ++start_block;
  }
  for (const auto &block : blocks_to_free) {
    recordFree(block.first, tag, block.second);
  }
}

void HeapHistory::recordFilterRange(uint64_t low, uint64_t high) {
  filter_ranges_.push_back(std::make_pair(low, high));
}

void HeapHistory::recordFreeConflict(uint64_t address, uint8_t heap_id) {
  conflicts_.push_back(HeapConflict(current_tick_, address, false));
}

void HeapHistory::recordMallocConflict(uint64_t address, size_t size,
                                       uint8_t heap_id) {
  conflicts_.push_back(HeapConflict(current_tick_, address, true));
}

void HeapHistory::recordRealloc(uint64_t old_address, uint64_t new_address,
                                size_t size, uint8_t heap_id) {
  // How should realloc relations be visualized?
  auto ret = alloc_or_free_tags_.insert("Free'd on reallocation");
  recordFree(old_address, &*(ret.first), heap_id);
  // Should the address perhaps be remembered here?
  ret = alloc_or_free_tags_.insert("Reallocated block");
  recordMalloc(new_address, size, &*(ret.first), heap_id);
}

void HeapHistory::recordEvent(const std::string &event_label,
                              const std::string &color) {
  tick_to_event_strings_[current_tick_] =
      std::make_pair(ColorStringToUint32(color), event_label);
}

uint32_t HeapHistory::ColorStringToUint32(const std::string &color) {
  const char *str = color.c_str() + 1;
  return strtol(str, nullptr, 16);
}

void HeapHistory::recordAddress(uint64_t address, const std::string &label,
                                const std::string &color) {
  address_to_address_strings_[address] =
      std::make_pair(ColorStringToUint32(color), label);
}

//============================================================================
// Functions to fill HeapVertex vectors for rendering

void colorToFloats(uint32_t color, float* red, float* green, float* blue) {
  *red = static_cast<float>((color & 0xFF0000) >> 16) / 255.0;
  *green = static_cast<float>((color & 0xFF00) >> 8) / 255.0;
  *blue = static_cast<float>(color & 0xFF) / 255.0;
}

void HeapHistory::eventsToVertices(std::vector<HeapVertex> *vertices) const {
  // Add two vertices with 0 or 1 on the y axis, and the proper tick on the
  // x axis.
  for (const auto &event : tick_to_event_strings_) {
    uint32_t color = event.second.first;
    float red, green, blue;
    colorToFloats(color, &red, &green, &blue);

    vertices->push_back(HeapVertex(event.first, 0, QVector3D(red, green, blue)));
    vertices->push_back(HeapVertex(event.first, 1, QVector3D(red, green, blue)));
  }
}

void HeapHistory::addressesToVertices(std::vector<HeapVertex> *vertices) const {
  // Add two vertices with 0 or 1 on the y axis, and the proper tick on the
  // x axis.
  for (const auto &event : address_to_address_strings_) {
    uint32_t color = event.second.first;
    float red, green, blue;
    colorToFloats(color, &red, &green, &blue);

    vertices->push_back(
      HeapVertex(0, event.first, QVector3D(red, green, blue)));
    vertices->push_back(
      HeapVertex(1, event.first, QVector3D(red, green, blue)));
  }
}

void HeapHistory::getActiveRegions(std::map<uint64_t, uint64_t>* regions,
  uint64_t* out_size) const {

  // Calculate what the proper size of a "region" should be at the current zoom
  // level. We take 1/100 of the screen height at the moment.
  long double yscaling = current_window_.getYScalingHeapToScreen();
  long double minimum_size = ((1.0/3.0) / yscaling);
  uint64_t uint_minsize = static_cast<uint64_t>(minimum_size);

  printf("YScaling is %Le, XScaling is %Le\n", current_window_.getYScalingHeapToScreen(),
    current_window_.getXScalingHeapToScreen());
  fflush(stdout);

  const std::map<uint64_t, uint64_t>* current_regions =
    active_region_cache_.getActiveRegions(uint_minsize, out_size);
  *regions = *current_regions;
}

// Determines all active pages ranges, and then provides rectangles covering the
// active areas of memory in a light color.
void HeapHistory::activeRegionsToVertices(std::vector<HeapVertex> *vertices)
  const {
  std::map<uint64_t, uint64_t> address_ranges;
  uint64_t region_size;
  // Determine the correct active regions on this zoom level.
  getActiveRegions(&address_ranges, &region_size);
  printf("Got %d ranges at granularity %lx\n", address_ranges.size(),
    region_size);

  QVector3D color = QVector3D(0.0, 0.7, 0.0);
  uint32_t lower_left_x = 0; // Minimum Tick.
  uint32_t lower_right_x = getMaximumTick();
  for (std::pair<uint64_t, uint64_t> range : address_ranges) {
    uint64_t lower_left_y = range.first;
    uint64_t lower_right_y = lower_left_y;
    uint32_t upper_right_x = lower_right_x;
    uint64_t upper_right_y = range.second;
    uint32_t upper_left_x = lower_left_x;
    uint64_t upper_left_y = upper_right_y;

    vertices->push_back(HeapVertex(lower_left_x, lower_left_y, color));
    vertices->push_back(HeapVertex(lower_right_x, lower_right_y, color));
    vertices->push_back(HeapVertex(upper_left_x, upper_left_y, color));
    vertices->push_back(HeapVertex(lower_right_x, lower_right_y, color));
    vertices->push_back(
      HeapVertex(upper_right_x, upper_right_y, color));
    vertices->push_back(HeapVertex(upper_left_x, upper_left_y, color));
  }
}

// Write out 6 vertices (for two triangles) into the buffer.
void HeapHistory::HeapBlockToVertices(const HeapBlock &block,
                                      std::vector<HeapVertex> *vertices) const {
  block.toVertices(current_tick_, vertices);
}

inline uint64_t HeapHistory::getMinimumBlockSize() const {
  long double yscaling = current_window_.getYScalingHeapToScreen();
  long double minimum_size = ((1.0/1000.0) / yscaling);
  uint64_t uint_min_size = uint64_t(minimum_size);
  return uint_min_size;
}

void HeapHistory::highlightBySize(uint32_t highlight_size) {
  for (HeapBlock& block : heap_blocks_) {
    block.highlighted_ = false;
    if (block.size_ == highlight_size) {
      block.highlighted_ = true;
    }
  }
}

// Converts the vector of heap blocks in the current heap history to
// heap vertices. Filters out elements that are too small to be rendered
// or fall outside of the current screen, unless all is "true".
size_t HeapHistory::heapBlockVerticesForActiveWindow(
    std::vector<HeapVertex> *vertices, bool all) const {
  uint64_t uint_min_size = getMinimumBlockSize();
  uint64_t minimum_address = current_window_.getMinimumAddressUint64();
  uint64_t maximum_address = current_window_.getMaximumAddressUint64();
  uint64_t minimum_tick = current_window_.getMinimumTickUint32();
  uint64_t maximum_tick = current_window_.getMaximumTickUint32();

  size_t active_block_count = 0;
  for (std::vector<HeapBlock>::const_iterator iter = heap_blocks_.begin();
       iter != heap_blocks_.end(); ++iter) {
    bool active = isBlockActive(*iter, uint_min_size, minimum_address,
      maximum_address, minimum_tick, maximum_tick) || all;

    if (active) {
      HeapBlockToVertices(*iter, vertices);
      ++active_block_count;
    }
  }

  return active_block_count;
}


bool HeapHistory::getEventAtTick(uint32_t tick, std::string *eventstring) {
  const auto iterator = tick_to_event_strings_.find(tick);
  if (iterator == tick_to_event_strings_.end()) {
    // Try an approximate search.
    auto iterator_approx = tick_to_event_strings_.lower_bound(tick-300);
    uint32_t minimum = std::numeric_limits<uint32_t>::max();

    while ((iterator_approx != tick_to_event_strings_.end() &&
           (abs(static_cast<int32_t>(iterator_approx->first - tick)) < 300))) {
      if (abs(static_cast<int32_t>(iterator_approx->first - tick)) <= minimum) {
        minimum = abs(static_cast<int32_t>(iterator_approx->first - tick));
        *eventstring = iterator_approx->second.second;
      }
      ++iterator_approx;
    }
    if (minimum != std::numeric_limits<uint32_t>::max()) {
      return true;
    } else {
      return false;
    }
  } else {
    *eventstring = iterator->second.second;
    return true;
  }
}


// Extremely slow O(n) version of testing if a given point lies within any
// block.
bool HeapHistory::getBlockAtSlow(uint64_t address, uint32_t tick,
                                 HeapBlock *result, uint32_t *index) {
  for (std::vector<HeapBlock>::iterator iter = heap_blocks_.begin();
       iter != heap_blocks_.end(); ++iter) {
    if (iter->contains(tick, address)) {
      *result = *iter;
      *index = iter - heap_blocks_.begin();
      return true;
    }
  }
  return false;
}

void HeapHistory::updateCachedSortedIterators() {
  // Makes sure there is a sorted iterator array.
  if (cached_blocks_sorted_by_address_.size() == 0) {
    // Fill the iterator cache.
    for (std::vector<HeapBlock>::iterator iter = heap_blocks_.begin();
         iter != heap_blocks_.end(); ++iter) {
      cached_blocks_sorted_by_address_.push_back(iter);
    }
    auto comparison = [](const std::vector<HeapBlock>::iterator &left,
                         const std::vector<HeapBlock>::iterator &right) {
      // Sort blocks in descending order by address, then tick.
      return (left->address_ > right->address_) ||
             (left->start_tick_ > right->start_tick_);
    };
    std::sort(cached_blocks_sorted_by_address_.begin(),
              cached_blocks_sorted_by_address_.end(), comparison);
  }
}

// Attempts to find a block on the heap at a given address and tick.
// If successful, the provided pointer is filled, and an index into
// the internal heap block vector is provided (this is useful for
// calculating vertex ranges).
//
// XXX: This code is still buggy, and does not seem to find all blocks.
// TODO(thomasdullien): Debug and fix.
bool HeapHistory::getBlockAt(uint64_t address, uint32_t tick, HeapBlock *result,
                             uint32_t *index) {
  updateCachedSortedIterators();
  std::pair<uint64_t, uint32_t> val(address, tick);

  // Find the block whose lower left corner is the first (by address, then by
  // tick, descending) to come after the requested point.
  const std::vector<std::vector<HeapBlock>::iterator>::iterator candidate =
      std::lower_bound(cached_blocks_sorted_by_address_.begin(),
                       cached_blocks_sorted_by_address_.end(), val,
                       [this](const std::vector<HeapBlock>::iterator &iterator,
                              const std::pair<uint64_t, uint32_t> &pair) {
                         fflush(stdout);
                         return (pair.first < iterator->address_) ||
                                (pair.second < iterator->start_tick_);
                       });
  if (candidate == cached_blocks_sorted_by_address_.end()) {
    return false;
  }
  // Check that the point lies within the candidate block.
  std::vector<HeapBlock>::iterator block = *candidate;
  if ((address > (block->address_ + block->size_)) ||
      (address < block->address_) || (tick > block->end_tick_) ||
      (tick < block->start_tick_)) {
    return false;
  }
  *result = *block;
  *index = *candidate - heap_blocks_.begin();
  return true;
}

// Provided a displacement (percentage of size of the current window in x and y
// direction), pan the window accordingly.
void HeapHistory::panCurrentWindow(double dx, double dy) {
  current_window_.pan(dx, dy);
}

// Zoom toward a given point on the screen. The point is given in relative
// height / width of the current window, e.g. the center is 0.5, 0.5.
void HeapHistory::zoomToPoint(double dx, double dy, double how_much_x,
                              double how_much_y, long double max_height,
                              long double max_width) {
  current_window_.zoomToPoint(dx, dy, how_much_x, how_much_y, max_height,
                              max_width);
}

