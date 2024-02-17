#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "memory_subsystem_constants.h"
#include "l2_cache.h"

#define L2_NUM_CACHE_ENTRIES (1<<15)

typedef struct {
  uint32_t v_d_tag;
  uint64_t cache_line[WORDS_PER_CACHE_LINE];
} L2_CACHE_ENTRY;

#define LOWER_48_BIT_MASK 0xFFFFFFFFFFFF
#define L2_VBIT_MASK (0x1 << 31)
#define L2_DIRTYBIT_MASK (0x1 << 30)
#define L2_ENTRY_TAG_MASK 0x7FFFFFF

L2_CACHE_ENTRY l2_cache[L2_NUM_CACHE_ENTRIES];

void l2_initialize() {
  for (int i = 0; i < L2_NUM_CACHE_ENTRIES; i++) {
    l2_cache[i].v_d_tag = 0;
  }
}

#define L2_ADDRESS_TAG_MASK ((uint64_t) 0x7ffffff << 21)
#define L2_ADDRESS_TAG_SHIFT 21
#define L2_INDEX_MASK (0x7fff << 6)
#define L2_INDEX_SHIFT 6
#define L2_HIT_STATUS_MASK 0x1

void l2_cache_access(uint64_t address, uint64_t write_data[], 
                     uint8_t control, uint64_t read_data[], uint8_t *status) {
  address = address & LOWER_48_BIT_MASK;
  uint64_t index = (address & L2_INDEX_MASK) >> L2_INDEX_SHIFT;
  uint64_t tag = (address & L2_ADDRESS_TAG_MASK) >> L2_ADDRESS_TAG_SHIFT;

  uint32_t entry_v_d_tag = l2_cache[index].v_d_tag;
  uint32_t entry_tag = entry_v_d_tag & L2_ENTRY_TAG_MASK;

  if (!(entry_v_d_tag & L2_VBIT_MASK) || (entry_tag != tag)) {
    *status = 0;  // Cache miss
  } else {
    *status = L2_HIT_STATUS_MASK;  // Cache hit
    if (control & 0x1) {  // Read
      memcpy(read_data, l2_cache[index].cache_line, sizeof(uint64_t) * WORDS_PER_CACHE_LINE);
    }
    if (control & 0x2) {  // Write
      memcpy(l2_cache[index].cache_line, write_data, sizeof(uint64_t) * WORDS_PER_CACHE_LINE);
      l2_cache[index].v_d_tag |= L2_DIRTYBIT_MASK;  // Set dirty bit
    }
  }
}

void l2_insert_line(uint64_t address, uint64_t write_data[], 
                    uint64_t *evicted_writeback_address, 
                    uint64_t evicted_writeback_data[], 
                    uint8_t *status) {
  address = address & LOWER_48_BIT_MASK;
  uint64_t index = (address & L2_INDEX_MASK) >> L2_INDEX_SHIFT;
  uint64_t tag = (address & L2_ADDRESS_TAG_MASK) >> L2_ADDRESS_TAG_SHIFT;

  uint32_t entry_v_d_tag = l2_cache[index].v_d_tag;

  if (!(entry_v_d_tag & L2_VBIT_MASK) || !(entry_v_d_tag & L2_DIRTYBIT_MASK)) {
    *status = 0;  // No write-back needed
  } else {
    *evicted_writeback_address = ((entry_v_d_tag & L2_ENTRY_TAG_MASK) << L2_ADDRESS_TAG_SHIFT) | (index << L2_INDEX_SHIFT);
    memcpy(evicted_writeback_data, l2_cache[index].cache_line, sizeof(uint64_t) * WORDS_PER_CACHE_LINE);
    *status = 1;  // Write-back needed
  }

  memcpy(l2_cache[index].cache_line, write_data, sizeof(uint64_t) * WORDS_PER_CACHE_LINE);
  l2_cache[index].v_d_tag = (tag | L2_VBIT_MASK) & ~L2_DIRTYBIT_MASK;
}