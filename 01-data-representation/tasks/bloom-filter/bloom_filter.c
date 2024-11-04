#include "bloom_filter.h"
#include "malloc.h"
  
uint64_t calc_hash(const char* str, uint64_t modulus, uint64_t seed) {
  // TODO: Write any hash function for further usage
  
  uint64_t hash = 0;
  while (*str) {
    hash = (hash * seed + *str) % modulus;
    ++str;
  }
  
  return hash;
}
  
void bloom_init(struct BloomFilter* bloom_filter, uint64_t set_size, hash_fn_t hash_fn, uint64_t hash_fn_count) {
  // TODO: Initialize bloom filter
  
  bloom_filter->set_size = set_size;
  bloom_filter->hash_fn_count = hash_fn_count;
  bloom_filter->hash_fn = hash_fn;
  
  uint64_t set_elements_count = (bloom_filter->set_size >> 6) + (bloom_filter->set_size % 64 != 0);
  bloom_filter->set = calloc(set_elements_count, sizeof(uint64_t));
}
  
void bloom_destroy(struct BloomFilter* bloom_filter) {
  // TODO: Free memory if needed
  
  free(bloom_filter->set);
  bloom_filter->set = NULL;
  
}
  
void bloom_insert(struct BloomFilter* bloom_filter, Key key) {
  // TODO: Insert key into set
  
  for (uint64_t i = 0; i < bloom_filter->hash_fn_count; ++i) {
    uint64_t index = calc_hash(key, bloom_filter->set_size, 31 + i);
    bloom_filter->set[index / 64] |= (1ULL << (index % 64));
  }
}
  
bool bloom_check(struct BloomFilter* bloom_filter, Key key) {
  // TODO: Check if key exists in set
  
  for (uint64_t i = 0; i < bloom_filter->hash_fn_count; ++i) {
    uint64_t index = calc_hash(key, bloom_filter->set_size, 31 + i);
    if ((bloom_filter->set[index / 64] & (1ULL << (index % 64))) == 0) {
      return 0;
    }
  }
  
  return 1;
}

