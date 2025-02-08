/**
 * @file tracing.h
 * @author Chris Hazen - group 24
 * @brief Provides methods for tracing output
 */
#ifndef _TRACING
#define _TRACING
#include <stdio.h>
#include <stdbool.h>

#include "instruction.h"

extern bool tracing;

/// @brief Write the current contents of the registers and current instruction to a `.out` file
/// @param file File to output trace to ; Note file must be open
/// @param instr Current raw binary instruction to write
void writeTrace(FILE *file, bin_instr_t instr);

#endif