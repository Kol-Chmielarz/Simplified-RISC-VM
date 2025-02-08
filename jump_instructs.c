#include <stdio.h>
#include <stdlib.h>

#include "jump_instructs.h"
#include "instruction.h"
#include "registers.h"
#include "utilities.h"
#include "machine_types.h"
#include "regname.h"

/*
Name|op|addr| (Explanation)
JMP |2 | a  |Jump: PC <- formAddress(P C, a)
JAL |3 | a  |Jump and Link: GPR[$ra] <- PC; PC <- formAddress(PC, a)
*/

void callJumpInstr(jump_instr_t instr)
{
    switch (instr.op)
    {
    case JMP_O:
        // Jump: PC <- formAddress(PC, a)
        PC = machine_types_formAddress(PC, instr.addr);
        break;
    case JAL_O:
        setReg(RA, PC+4);
        PC = machine_types_formAddress(PC, instr.addr);
        break;
    default:
        printf("Unknown opcode: %d\n", instr.op);
        exit(EXIT_FAILURE);
    }
}
