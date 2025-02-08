/**
 * @file memory.h
 * @brief Holds memory for VM
 */
#ifndef _MEMORY
#define _MEMORY

#include "instruction.h"

// Sizes for the memory (2^16 bytes = 64K)
#define MEMORY_SIZE_IN_BYTES (65536 - BYTES_PER_WORD)
#define MEMORY_SIZE_IN_WORDS (MEMORY_SIZE_IN_BYTES / BYTES_PER_WORD)

// Professor hint
typedef union mem_u
{
    byte_type bytes[MEMORY_SIZE_IN_BYTES];
    word_type words[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} _memory;

extern _memory memory;

#endif