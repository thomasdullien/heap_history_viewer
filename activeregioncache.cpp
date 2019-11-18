#include "activeregioncache.h"

ActiveRegionCache::ActiveRegionCache() {
  cached_regions_.resize(0);
}

ActiveRegionCache::ActiveRegionCache(uint64_t maximum_height,
  const std::vector<HeapBlock>* blocks) {
  uint64_t caches = calculateNumberOfCacheEntries(maximum_height);
  cached_regions_.resize(caches);
  printf("[!] Calculating active region caches...\n");
  for (const HeapBlock& block : *blocks) {
    for (uint64_t cache_index = 0; cache_index < caches; ++cache_index) {
      insertRegionsForBlock(&cached_regions_[cache_index],
        cacheIndexToSize(cache_index), block.address_, block.size_);
    }
  }
  printf("[!] Coalescing active region caches...\n");
  fflush(stdout);
  for (uint64_t cache_index = 0; cache_index < caches; ++cache_index) {
    coalesceCache(cache_index);
  }
  printf("[!] Done initializing active region caches.\n");
  fflush(stdout);
}

static int num_leading_zero_bits(uint64_t value) {
#ifdef _MSC_VER
#pragma message ( "WARNING: TODO num_leading_zero_bits not implemented on Windows" )
    return 0; // TODO(patricia-gallardo): Implement on Windows
#else
    return __builtin_clzl(value);
#endif
}

const std::map<uint64_t, uint64_t>* ActiveRegionCache::getActiveRegions(
 uint64_t region_minsize, uint64_t *region_size) const {
 int shift_value = std::max(64 - num_leading_zero_bits(region_minsize), 12);
 int index = std::min(
   static_cast<int>(cached_regions_.size()) - 1, shift_value - 12);
 *region_size = 1L << (index + 12);
 return &cached_regions_[index];
}

void ActiveRegionCache::insertPointer(std::map<uint64_t, uint64_t>* region,
  uint64_t region_size, uint64_t pointer) {
  region->insert(std::make_pair( pointer & (~(region_size - 1)), 0));
}

void ActiveRegionCache::insertRegionsForBlock(
  std::map<uint64_t, uint64_t>* region, uint64_t region_size,
    uint64_t block_address, uint64_t block_size) {

  for (uint64_t address = block_address; address <= block_address + block_size;
    address += region_size) {
    insertPointer(region, region_size, address);
  }
  insertPointer(region, region_size, block_address + block_size);
}

void ActiveRegionCache::coalesceCache(uint64_t cache_index) {
  std::map<uint64_t, uint64_t>& cache = cached_regions_[cache_index];
  uint64_t region_size = cacheIndexToSize(cache_index);
  std::vector<uint64_t> keys_to_remove;

  for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
    uint64_t current_address = iter->first;
    uint64_t forward_index = 1;
    auto forward_iter = iter;
    ++forward_iter;
    while (forward_iter != cache.end() &&
           forward_iter->first == current_address + (
           forward_index * region_size)) {
      keys_to_remove.push_back(forward_iter->first);
      ++forward_iter;
      ++forward_index;
    }
    iter->second = (current_address + (forward_index * region_size)) - 1;
    iter = --forward_iter;
  }
  for (const auto key : keys_to_remove) {
    cache.erase(key);
  }
}

uint64_t ActiveRegionCache::cacheIndexToSize(uint64_t index) {
  return (1L << (index + 12));
}

uint64_t ActiveRegionCache::calculateNumberOfCacheEntries(
  uint64_t maximum_height) {
  uint64_t number_of_caches = 1; // We need the 4k page cache for sure.
  while (cacheIndexToSize(number_of_caches) < (maximum_height / 100.0)) {
    number_of_caches++;
  }
  return number_of_caches;
}
