/**
 * @file reg_instructs.h
 * @author Chris Hazen - group 24
 * @brief Functions for System Calls
 */
#ifndef _SYSCALL
#define _SYSCALL

#include "instruction.h"
#include "tracing.h"


/// @brief Handle a system call
/// @param instr Syscall to handle
void handleSyscall(syscall_instr_t instr);

#endif