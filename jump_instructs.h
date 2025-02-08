#ifndef JUMP_INSTRUCTS_H
#define JUMP_INSTRUCTS_H

#include "instruction.h"
#include "machine_types.h"

/// @brief Call a jump instruction
/// @param instr Binary instruction to call
void callJumpInstr(jump_instr_t instr);

#endif
