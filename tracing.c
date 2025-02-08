/**
 * @file tracing.c
 * @author Chris Hazen - group 24
 * @brief Provides methods for tracing output
 */
#include "tracing.h"

#include "instruction.h"
#include "regname.h"

#include "memory.h"
#include "registers.h"

bool tracing = true;

/// @brief Print a set of bytes at an address
/// @param file File to print to
/// @param addr Address to get bytes from
inline static void printBytesAtAddr(FILE *file, word_type address)
{
    int outputCounter = 0;
    for (int addr = getReg(address); ; addr += 4)
    {
        fprintf(file, "%8d: %d\t", addr, memory.bytes[addr]);
        outputCounter++;
        if (memory.bytes[addr] == 0)
        {
            fprintf(file, "...\n");
            break;
        }
        if (outputCounter % 5 == 0)
        {
            printf("\n");
        }
    }
}

/// @brief Print everything between the stack and frame pointers
/// @param file File to print to
inline static void printStack(FILE *file)
{
    int outputCounter = 0;
    for (int addr = getReg(SP); addr <= getReg(FP); addr += 4)
    {
        fprintf(file, "%8d: %d\t", addr, memory.bytes[addr]);
        outputCounter++;
        if (memory.bytes[addr] == 0)
        {
            fprintf(file, "...");
            while (memory.bytes[addr] == 0
                   && memory.bytes[addr+4] == 0
                   && addr < getReg(FP))
            {
                addr += 4;
            }
        }
        if (outputCounter % 5 == 0 || addr == getReg(FP))
        {
            printf("\n");
        }
    }
}

/** Write instruction and register contents to file */
void writeTrace(FILE *file, bin_instr_t instr)
{
    // Print the program counter
    fprintf(file, "      PC: %d\t", PC);

    // Trace HI and LO
    if (HI != 0 || LO != 0)
    {
        fprintf(file, "      HI: %d\t", HI);
        fprintf(file, "      LO: %d", LO);
    }

    // Newline after registers
    fprintf(file, "\n");

    // Print the contents of each register
    const char **names = getRegnames();
    for (int reg = 0; reg < NUM_REGISTERS; reg++) {
        fprintf(file, "GPR[%-3s]: %-4d\t", names[reg], getReg(reg));
        if ((reg + 1) % 6 == 0)
        {
            fprintf(file, "\n");
        }
    }
    fprintf(file, "\n"); // Print final newline of the registers

    // Print globals and stack
    printBytesAtAddr(file, GP);
    printStack(file);
    // printBytesAtAddr(file, SP);

    // Print assembly  instruction
    const char *asmInstrString = instruction_assembly_form(instr);
    fprintf(file, "==> addr: %4d %s\n", PC, asmInstrString);
}