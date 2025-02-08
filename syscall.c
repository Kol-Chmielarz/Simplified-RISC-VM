/**
 * @file reg_instructs.c
 * @author Chris Hazen - group 24
 * @brief Functions for System Calls
 */
#include "syscall.h"

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "registers.h"
#include "utilities.h"

/*
| Code | Name | arg. reg. | Effect (in terms of C std. library)       |
| ---- | ---- | --------- | ----------------------------------------- |
| 10   | EXIT | -         | exit(0) // halt                           |
| 4    | PSTR | $a0       | GPR[$v0] <-printf("%s",&memory[GPR[$a0]]) |
| 11   | PCH  | $a0       | GPR[$v0] <-fputc(GPR[$a0],stdout)         |
| 12   | RCH  | -         | GPR[$v0] <-getc(stdin)                    |
| 256  | STRA | -         | start VM tracing; start tracing output    |
| 257  | NOTR | -         | no VM tracing; stop the tracing output    |
*/

/** Handle a system call */
void handleSyscall(syscall_instr_t instr)
{
    switch(instr.code)
    {
    case exit_sc:
        exit(0);
        break;
    case print_str_sc:
        setReg(strToReg("$v0"), printf("%s", (char*)&memory.bytes[getReg(strToReg("$a0"))]));
        break;
    case print_char_sc:
        setReg(strToReg("$v0"), fputc(getReg(strToReg("$a0")), stdout));
        break;
    case read_char_sc:
        setReg(strToReg("$v0"),  getc(stdin));
        break;
    case start_tracing_sc:
        // Start VM tracing
        tracing = true;
        break;
    case stop_tracing_sc:
        // No VM tracing
        tracing = false;
        break;
    default:
        bail_with_error("Invalid instruction code: %d\n", instr.func);
        break;
    }
}