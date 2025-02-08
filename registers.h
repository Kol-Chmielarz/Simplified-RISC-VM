/**
 * @file registers.h
 * @author Chris Hazen - group 24
 * @brief Register utilities for Simplified RISC VM
 */
#ifndef _REGISTERS
#define _REGISTERS

#include "instruction.h"

/*********************************************************************************
 * Special Registers
 *********************************************************************************/

extern word_type PC;
extern word_type HI, LO;

/*********************************************************************************
 * Util Functions
 *********************************************************************************/

/// @brief Find the index of a register based on its name
/// @param regName Name of register to find
/// @return Index of register if found ; Value of `NUM_REGISTERS` if not found
unsigned short strToReg(char *regName);

/// @brief Get the names of the registers as a list
/// @return Char* array containing list of registers
const char **getRegnames();

/// @brief Initialize register values
void initRegisters();

/*********************************************************************************
 * Register Getter/Setter
 *********************************************************************************/

/// @brief Set the value of a register
/// @param reg Register to set
/// @param val word_type value to set register to
void setReg(unsigned reg, word_type val);

/// @brief Get the value of a register
/// @param reg Register to get value of
/// @return word_type value of register
word_type getReg(unsigned reg);

#endif