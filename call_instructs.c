/**
 * @file call_instructs.c
 * @author Chris Hazen - group 24
 * @brief Execution handling for vm
 */
#include "call_instructs.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"

#include "immed_instructs.h"
#include "jump_instructs.h"
#include "memory.h"
#include "reg_instructs.h"
#include "registers.h"
#include "syscall.h"
#include "tracing.h"

/** Execute whatever is currently loaded in memory */
void execute()
{
    FILE *file = fopen("test.myo", "w+");

    // Call instructions in memory and increment program counter
    for (PC = 0; memory.words[PC/4] != 0; PC += 4)
    {
        if (tracing)
        {
            writeTrace(stdout, memory.instrs[PC/4]);
        }

        switch(instruction_type(memory.instrs[PC/4]))
        {
        case reg_instr_type:
            callRegInstr(memory.instrs[PC/4].reg);
            break;
        case syscall_instr_type:
            handleSyscall(memory.instrs[PC/4].syscall);
            break;
        case immed_instr_type:
            callImmedInstr(memory.instrs[PC/4].immed);
            break;
        case jump_instr_type:
            callJumpInstr(memory.instrs[PC/4].jump);
            PC -= 4;
            break;
        case error_instr_type:
        default:
            bail_with_error("Error in execute(): Unrecognized instruction type\n");
            break;
        }
    }
    fclose(file);
}