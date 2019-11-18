#ifndef ACTIVEREGIONCACHE_H
#define ACTIVEREGIONCACHE_H

#include <map>
#include <vector>

#include "heapblock.h"

// An in-memory cache for "active regions" at different zoom levels,
// from round_to_power_of_two(max_height / 100) down to 4k pages.
//
class ActiveRegionCache {
public:
  ActiveRegionCache();
  ActiveRegionCache(uint64_t maximum_height,
    const std::vector<HeapBlock>* blocks);

  const std::map<uint64_t, uint64_t>* getActiveRegions(
    uint64_t region_minsize, uint64_t* outsize) const;
private:
  // Makes the test class a friend to permit testing private functions.
  friend class TestActiveRegionCache;

  static uint64_t cacheIndexToSize(uint64_t index);
  void coalesceCache(uint64_t cache_index);
  static void insertRegionsForBlock(std::map<uint64_t, uint64_t>* region,
    uint64_t region_size, uint64_t block_address, uint64_t block_size);
  static void insertPointer(std::map<uint64_t, uint64_t>* region,
    uint64_t region_size, uint64_t pointer);
  static uint64_t calculateNumberOfCacheEntries(uint64_t maximum_height);

  // Vector of active regions of the form "start address, upper limit".
  std::vector<std::map<uint64_t, uint64_t>> cached_regions_;

};

#endif // ACTIVEREGIONCACHE_H
