
/************************************************************

The L1 cache is a 64KB, 4-way set associative, write-back cache
for both instructions and data (not separate I-cache and D-cache).
As with the rest of the memory subsystem, a cache line is 8 words,
where each word is 64 bits (8 bytes).

The size of the L1 cache is 64KB of data
                          = 8K words (since there are 8 bytes/word)
                          = 1K cache lines (since there are 8 words/cache line)
                          = 256 sets (since there are 4 cache lines/set)

Each cache entry has: valid bit, reference bit, dirty bit,
                     tag, and cache-line data.

The upper 16 bits of a 64-bit address are ignored, so the 
remaining 48 bits of an address are grouped as follows (from lsb to msb):
3 bits are used for byte offset within a word (bits 0-2)
3 bits are used for word offset within a cache line (bits 3-5)
8 bits are used for the set index, since there are 256 = 2^8 sets per cache (bits 6-13).
34 bits are used for the tag (bits 14-47).

Therefore, a 64-bit address looks like:

     16        34        8         3        3
  ------------------------------------------------
 | unused |   tag   |   set    | word   |  byte  |
 |        |         |  index   | offset | offset |
  ------------------------------------------------

Each cache entry is structured as follows:

    1 1 1    27      34 
    ------------------------------------------------
   |v|r|d|reserved|  tag  |  8-word cache line data |
    ------------------------------------------------

where:
  v is the valid bit
  r is the reference bit
  d is the dirty bit
and the 27 "reserved" bits are an artifact of using C. The
cache hardware would not have those.

**************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "memory_subsystem_constants.h"
#include "l1_cache.h"


/***************************************************
This struct defines the structure of a single cache
entry in the L1 cache. It has the following fields:
  v_r_d_tag: 64-bit unsigned word containing the 
           valid (v) bit at bit 63 (leftmost bit),
           the reference (r) bit at bit 62,
           the dirty bit (d) at bit 61, and the tag 
           in bits 0 through 33 (the 34 rightmost bits)
  cache_line: an array of 8 words, constituting a single
              cache line.
****************************************************/

typedef struct {
  uint64_t v_r_d_tag;
  uint64_t cache_line[WORDS_PER_CACHE_LINE];
} L1_CACHE_ENTRY;

//4-way set-associative cache, so there are
//4 cache lines per set.
#define L1_LINES_PER_SET 4

/***************************************************
  This structure defines an L1 cache set. Its only
  field, lines, is an array of four cache lines.
***************************************************/

typedef struct {
  L1_CACHE_ENTRY lines[L1_LINES_PER_SET];
} L1_CACHE_SET;

// Although addresses are 64 bits, only the lowest 48
// bits are actually used. The upper 16 bits are
// zeroed out, using this mask.

#define LOWER_48_BIT_MASK 0xFFFFFFFFFFFF

//There are 256 sets in the L1 cache
#define L1_NUM_CACHE_SETS 256

//The L1 cache itself is just an array of 256 cache sets.
L1_CACHE_SET l1_cache[L1_NUM_CACHE_SETS];

//Mask for v bit: Bit 63 of v_r_d_tag
#define L1_VBIT_MASK ((uint64_t) 1 << 63)

//Mask for r bit: Bit 62 of v_r_d_tag
#define L1_RBIT_MASK ((uint64_t) 1 << 62)

//Mask for d bit: Bit 61 of v_r_d_tag
#define L1_DIRTYBIT_MASK ((uint64_t) 1 << 61)

//The tag is the low 34 bits of v_r_d_tag
//The mask is just 34 ones, so 3FFFFFFFF hex

#define L1_ENTRY_TAG_MASK 0x3FFFFFFFF

//Bits 3-5 of an address specifies the offset of the addressed
//word within the cache line
//Mask is 111000 in binary = 0x38

#define WORD_OFFSET_MASK 0x38

//After masking to extract the word offset, it needs
//to be shifted to the right by 3.
#define WORD_OFFSET_SHIFT 3

// Bits 14-47 of an address are used as the 34 tag bits
// The mask is 34 ones (see L1_ENTRY_TAG_MASK, above) shifted
// left by 14 bits.
#define L1_ADDRESS_TAG_MASK ((uint64_t) L1_ENTRY_TAG_MASK << 14)

//After masking to extract the tag from the address, it needs to 
//be shifted right by 14.
#define L1_ADDRESS_TAG_SHIFT 14

//Bits 6-13 are used to extract the set index from an address.
//The mask is 8 ones (so FF hex) shifted left by 6.
#define L1_SET_INDEX_MASK (0xff << 6)

//After masking to extract the set index from an address, it
//needs to be shifted to the right by 6
#define L1_SET_INDEX_SHIFT 6

//This can be used to set or clear the lowest bit of the status
//register to indicate a cache hit or miss.
#define L1_CACHE_HIT_MASK 0x1


/************************************************
            l1_initialize()

This procedure initializes the L1 cache by clearing
the valid bit of each cache entry in each set in
the cache.
************************************************/

void l1_initialize() {
  for (int set = 0; set < L1_NUM_CACHE_SETS; set++) {
    for (int line = 0; line < L1_LINES_PER_SET; line++) {
      l1_cache[set].lines[line].v_r_d_tag = 0;  // Clearing the entire v_r_d_tag field
    }
  }
}


/**********************************************************

             l1_cache_access()

This procedure implements the reading or writing of a single word
to the L1 cache. 

The parameters are:

address:  unsigned 64-bit address. This address can be anywhere within
          a cache line.

write_data: a 64-bit word. On a write operation, if there is a cache
          hit, write_data is copied to the appropriate word in the
          appropriate cache line.

control:  an unsigned byte (8 bits), of which only the two lowest bits
          are meaningful, as follows:
          -- bit 0:  read enable (1 means read, 0 means don't read)
          -- bit 1:  write enable (1 means write, 0 means don't write)

read_data: a 64-bit ouput parameter (thus, a pointer to it is passed).
         On a read operation, if there is a cache hit, the appropriate
         word of the appropriate cache line in the cache is written
         to read_data.

status: this in an 8-bit output parameter (thus, a pointer to it is 
        passed).  The lowest bit of this byte should be set to 
        indicate whether a cache hit occurred or not:
              cache hit: bit 0 of status = 1
              cache miss: bit 0 of status = 0

If the access results in a cache miss, then the only
effect is to set the lowest bit of the status byte to 0.

**********************************************************/

void l1_cache_access(uint64_t address, uint64_t write_data, 
                     uint8_t control, uint64_t *read_data, uint8_t *status) {
  address &= LOWER_48_BIT_MASK;
  uint64_t set_index = (address & L1_SET_INDEX_MASK) >> L1_SET_INDEX_SHIFT;
  uint64_t tag = (address & L1_ADDRESS_TAG_MASK) >> L1_ADDRESS_TAG_SHIFT;
  uint64_t word_offset = (address & WORD_OFFSET_MASK) >> WORD_OFFSET_SHIFT;

  *status = 0;  // Assume a cache miss initially

  for (int line = 0; line < L1_LINES_PER_SET; line++) {
    uint64_t v_r_d_tag = l1_cache[set_index].lines[line].v_r_d_tag;
    uint64_t entry_tag = v_r_d_tag & L1_ENTRY_TAG_MASK;

    if ((v_r_d_tag & L1_VBIT_MASK) && (entry_tag == tag)) {
      // Cache hit
      *status = L1_CACHE_HIT_MASK;
      l1_cache[set_index].lines[line].v_r_d_tag |= L1_RBIT_MASK; // Set reference bit

      if (control & READ_ENABLE_MASK) {
        *read_data = l1_cache[set_index].lines[line].cache_line[word_offset];
      }

      if (control & WRITE_ENABLE_MASK) {
        l1_cache[set_index].lines[line].cache_line[word_offset] = write_data;
        l1_cache[set_index].lines[line].v_r_d_tag |= L1_DIRTYBIT_MASK; // Set dirty bit
      }

      break;
    }
  }
}


// This (all 1's) is used in l1_insert_line(), below, to indicate a value that
// is uninitialized.

#define UNINITIALIZED ~0x0


/************************************************************

                 l1_insert_line()

This procedure inserts a new cache line into the L1 cache.

The parameters are:

address: 64-bit memory address for the new cache line.

write_data: an array of unsigned 64-bit words containing the 
            cache line data to be inserted into the cache.

evicted_writeback_address: a 64-bit output parameter (thus,
          a pointer to it is passed) that, if the cache line
          being evicted needs to be written back to memory,
          should be assigned the memory address for the evicted
          cache line.
          
evicted_writeback_data: an array of 64-bit words that, if the cache 
          line being evicted needs to be written back to memory,
          should be assigned the cache line data for the evicted
          cache line. Since there are 8 words per cache line, the
          actual parameter should be an array of at least 8 words.

status: this in an 8-bit output parameter (thus, a pointer to it is 
        passed).  The lowest bit of this byte should be set to 
        indicate whether the evicted cache line needs to be
        written back to memory or not, as follows:
            0: no write-back required
            1: evicted cache line needs to be written back.


 The cache replacement algorithm uses a simple NRU
 algorithm. A cache entry (among the cache entries in the set) is 
 chosen to be written to in the following order of preference:
    - valid bit = 0
    - reference bit = 0 and dirty bit = 0
    - reference bit = 0 and dirty bit = 1
    - reference bit = 1 and dirty bit = 0
    - reference bit = 1 and dirty bit = 1
*********************************************************/


void l1_insert_line(uint64_t address, uint64_t write_data[], 
                    uint64_t *evicted_writeback_address, 
                    uint64_t evicted_writeback_data[], 
                    uint8_t *status) {
  address &= LOWER_48_BIT_MASK;
  uint64_t set_index = (address & L1_SET_INDEX_MASK) >> L1_SET_INDEX_SHIFT;
  uint64_t tag = (address & L1_ADDRESS_TAG_MASK) >> L1_ADDRESS_TAG_SHIFT;

  uint64_t r0_d0_index = UNINITIALIZED, r0_d1_index = UNINITIALIZED, r1_d0_index = UNINITIALIZED;
  int chosen_line = -1;

  for (int line = 0; line < L1_LINES_PER_SET; line++) {
    uint64_t v_r_d_tag = l1_cache[set_index].lines[line].v_r_d_tag;

    if (!(v_r_d_tag & L1_VBIT_MASK)) { // valid bit = 0
      chosen_line = line;
      break;
    } else {
      BOOL is_r_set = (v_r_d_tag & L1_RBIT_MASK) != 0;
      BOOL is_d_set = (v_r_d_tag & L1_DIRTYBIT_MASK) != 0;

      if (!is_r_set && !is_d_set && r0_d0_index == UNINITIALIZED) {
        r0_d0_index = line;
      } else if (!is_r_set && is_d_set && r0_d1_index == UNINITIALIZED) {
        r0_d1_index = line;
      } else if (is_r_set && !is_d_set && r1_d0_index == UNINITIALIZED) {
        r1_d0_index = line;
      }
    }
  }

  if (chosen_line == -1) {
    if (r0_d0_index != UNINITIALIZED) {
      chosen_line = r0_d0_index;
    } else if (r0_d1_index != UNINITIALIZED) {
      chosen_line = r0_d1_index;
    } else if (r1_d0_index != UNINITIALIZED) {
      chosen_line = r1_d0_index;
    } else {
      chosen_line = 0; // Evict the first line if all are recently used
    }
  }

  uint64_t evict_v_r_d_tag = l1_cache[set_index].lines[chosen_line].v_r_d_tag;
  BOOL evict_is_dirty = (evict_v_r_d_tag & L1_DIRTYBIT_MASK) != 0;

  if (evict_is_dirty) {
    *status = 1; // Write-back is needed
    uint64_t evict_tag = evict_v_r_d_tag & L1_ENTRY_TAG_MASK;
    *evicted_writeback_address = (evict_tag << L1_ADDRESS_TAG_SHIFT) | (set_index << L1_SET_INDEX_SHIFT);
    for (int i = 0; i < WORDS_PER_CACHE_LINE; i++) {
      evicted_writeback_data[i] = l1_cache[set_index].lines[chosen_line].cache_line[i];
    }
  } else {
    *status = 0; // No write-back needed
  }

  // Insert the new line
  l1_cache[set_index].lines[chosen_line].v_r_d_tag = (tag & L1_ENTRY_TAG_MASK) | L1_VBIT_MASK;
  for (int i = 0; i < WORDS_PER_CACHE_LINE; i++) {
    l1_cache[set_index].lines[chosen_line].cache_line[i] = write_data[i];
  }
}


/************************************************

       l1_clear_r_bits()

This procedure clears the r bit of each entry in each set of the L1
cache. It is called periodically to support the the NRU algorithm.

***********************************************/
    
void l1_clear_r_bits() {
  for (int set = 0; set < L1_NUM_CACHE_SETS; set++) {
    for (int line = 0; line < L1_LINES_PER_SET; line++) {
      l1_cache[set].lines[line].v_r_d_tag &= ~L1_RBIT_MASK; // Clear the reference bit
    }
  }
}

