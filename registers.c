/**
 * @file registers.c
 * @author Chris Hazen - group 24
 * @brief Registers for Simplified RISC VM
 */
#include "registers.h"

#include "string.h"

#include "instruction.h"
#include "regname.h"
#include "utilities.h"

/*********************************************************************************
 * General Purpose Registers
 *********************************************************************************/

word_type PC;
word_type HI, LO;

static word_type registers[NUM_REGISTERS];

// From `regname.c`
// Copied to make `char *` to `reg idx` util
static const char *regnames[NUM_REGISTERS] = {
    "$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };

/*********************************************************************************
 * Util Functions
 *********************************************************************************/

/** Find the index of a register based on its name */
unsigned short strToReg(char *regName)
{
    // Simple linear search is sufficient for if this utility is needed
    for (int regIdx = 0; regIdx < NUM_REGISTERS; regIdx++)
    {
        if (strcmp(regnames[regIdx], regName) == 0)
        {
            return regIdx;
        }
    }

    // Value of `NUM_REGISTERS` is not valid index
    // Only returned when name is not found
    return NUM_REGISTERS;
}

/** Get the names of the registers as a char* array*/
const char **getRegnames()
{
    return regnames;
}

/** Initialize registers */
void initRegisters()
{
    // Set values of all registers to 0
    // Mostly a sanity check to ensure clean initial values
    for (int reg = 0; reg < NUM_REGISTERS; reg++)
    {
        registers[reg] = 0;
    }
}

/*********************************************************************************
 * Register Getter/Setter
 *********************************************************************************/

/** Set the value of a register */
void setReg(unsigned reg, word_type val)
{
    // Ensure valid register
    if (reg > NUM_REGISTERS)
    {
        bail_with_error("Error in `setReg()`: Invalid register access");
    }

    registers[reg] = val;
}

/** Get the value of a register */
word_type getReg(unsigned reg)
{
    // Ensure valid register
    if (reg > NUM_REGISTERS)
    {
        bail_with_error("Error in `getReg()`: Invalid register access");
    }

    return registers[reg];
}
