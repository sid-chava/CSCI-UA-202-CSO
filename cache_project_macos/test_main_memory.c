
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "memory_subsystem_constants.h"
#include "main_memory.h"

//main() can take a command-line argument specifying the size of main memory.
//If none is provided, 32MB (= 2^25) is the default.

#define DEFAULT_MEMORY_SIZE_IN_BYTES (1 << 25)


int main(int argc, char *argv[])
{
  uint64_t size_in_bytes;
  if (argc != 2) 
    size_in_bytes = DEFAULT_MEMORY_SIZE_IN_BYTES;
  else
    size_in_bytes = (uint64_t) atoi(argv[1]);

  printf("Memory size is %llu bytes\n", size_in_bytes);

  main_memory_initialize(size_in_bytes);

  uint64_t i,j;
  uint64_t size_in_words = size_in_bytes >> BYTES_TO_WORDS_SHIFT;

  printf("Memory size in words =  %llu\n", size_in_words);

  uint64_t read_data[WORDS_PER_CACHE_LINE];
  uint64_t write_data[WORDS_PER_CACHE_LINE];

  printf("Pass 1: Writing to every memory location, a cache line at a time\n");

  for(i=0; i < size_in_words; i += WORDS_PER_CACHE_LINE) {
    for(j=0; j < WORDS_PER_CACHE_LINE; j++) 
      write_data[j] = i+j;

    //will crash if an output value is attempted to be written
    main_memory_access(i << WORDS_TO_BYTES_SHIFT, write_data, WRITE_ENABLE_MASK, NULL);
  }

  printf("Pass 2: Reading from every location, a cache line at a time,\n");
  printf("        and checking the value read against the value previously written.\n");

  for(i=0; i < size_in_words; i += WORDS_PER_CACHE_LINE) {
    main_memory_access(i << WORDS_TO_BYTES_SHIFT, NULL, READ_ENABLE_MASK, read_data);  

    for(j=0; j < WORDS_PER_CACHE_LINE; j++) 
      if (read_data[j] != (i+j)) { // the i+jth word in memory should contain i+j
	printf("Memory read error: memory[%llu] contains %llu\n", i+j, read_data[j]);
	exit(1);
      }
  }


  printf("Pass 3: Testing reading and writing at the same time\n");


  for(i=0; i < size_in_words; i += WORDS_PER_CACHE_LINE) {
    for(j=0; j < WORDS_PER_CACHE_LINE; j++) 
      write_data[j] = size_in_words - (i+j);

    main_memory_access(i << WORDS_TO_BYTES_SHIFT, write_data, READ_ENABLE_MASK | WRITE_ENABLE_MASK, read_data);  
	
    for(j=0; j < WORDS_PER_CACHE_LINE; j++) 
      if (read_data[j] !=  (i+j)) {
	printf("Error: On read_enable and write_enable, the read should return old value\n");
	exit(1);
      }


    main_memory_access(i << WORDS_TO_BYTES_SHIFT, NULL, READ_ENABLE_MASK, read_data);  

    for(j=0; j < WORDS_PER_CACHE_LINE; j++) 
      if (read_data[j] != size_in_words - (i+j)) {
	printf("Error: After read_enable and write_enable, the next read should return new value\n");
	exit(1);
      }
  }
  printf("Passed\n");

}


    
