/**
 * @file immediate_instructs.h
 * @author Kol Chmielarz - group 24
 * @brief Functions for immediate instructions (see Table 4. -srm-vm.dpf)
 */
#ifndef INSTRUCTION_HANDLER_H
#define INSTRUCTION_HANDLER_H

#include "instruction.h"

/// @brief Call a register instruction
/// @param instr Binary instruction to call
void callImmedInstr(immed_instr_t instr);



#endif