/**
 * @file reg_instructs.h
 * @author Chris Hazen - group 24
 * @brief Functions for register instructions (see Table 3. -srm-vm.dpf)
 */
#ifndef _REG_INSTRUCTS
#define _REG_INSTRUCTS

#include "instruction.h"
#include "machine_types.h"

/// @brief Call a register instruction
/// @param instr Binary instruction to call
void callRegInstr(reg_instr_t instr);

#endif