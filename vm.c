#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine_types.h"
#include "instruction.h"
#include "bof.h"
#include "utilities.h"
#include "memory.h"
#include "call_instructs.h"
#include "registers.h"
#include "regname.h"

// Function prototypes
void loadProgram(BOFFILE bf);
void printAssembly(const char *bofFileName);
void executeProgram();

int main(int argc, char *argv[])
{
    // Check if the correct number of arguments are passed
    if (!(argc == 2) && !(argc == 3 && strncmp(argv[1], "-p", 2) == 0))
    {
        printf("Usage: %s [-p] <filename>\n", argv[0]);
        return 1;
    }

    // Assign the bof filename to a variable
    const char *bofFileName = argc == 2 ? argv[1] : argv[2];

    // Open the file
    BOFFILE bofFile = bof_read_open(bofFileName); // Function exits on failure

    // Check if the -p option is selected
    if (strcmp(argv[1], "-p") == 0)
    {
        printAssembly(bofFileName);
        return 0;
    }
    else
    {
        loadProgram(bofFile);
        execute();
    }

    return 0;
}

/// @brief Load a  `.bof` file into memory
/// @param bf File to read
void loadProgram(BOFFILE bf)
{
    // TODO
    BOFHeader header = bof_read_header(bf);

    // debug_print(
    // "text_start_address, %d\n"
    // "text_length, %d\n"
    // "data_start_address, %d\n"
    // "data_length, %d\n"
    // "stack_bottom_addr %d\n",
    // header.text_start_address,
    // header.text_length,
    // header.data_start_address,
    // header.data_length,
    // header.stack_bottom_addr
    // );

    initRegisters();

    setReg(GP, header.data_start_address);
    setReg(SP, header.stack_bottom_addr);
    setReg(FP, header.stack_bottom_addr);

    // debug_print("sizeof(BOFHeader) %d\n", sizeof(BOFHeader));

    // Check if the header is valid
    if (strncmp(header.magic, "BOF", MAGIC_BUFFER_SIZE) != 0)
    {
        printf("Error in `loadProgram()`: Inputted file is not a .bof file\n");
        return;
    }

    // Find the starting address of the instructions
    // word_type textStartingAddress = header.text_start_address;

    // Load the instructions into memory
    fseek(bf.fileptr, sizeof(BOFHeader), SEEK_SET);
    size_t textLength = header.text_length;
    size_t readingBytes = bof_read_bytes(bf, textLength, memory.bytes);

    // debug_print("Bytes in f: %d\n", textLength);
    // debug_print("Bytes read: %d\n", readingBytes);
    // for (int i = 0; i < textLength+1; i+=4)
    // {
    //     debug_print("%02x %02x %02x %02x ", memory.bytes[i], memory.bytes[i+1], memory.bytes[i+2], memory.bytes[i+3]);
    //     debug_print("%c %c %c %c\n", memory.bytes[i], memory.bytes[i+1], memory.bytes[i+2], memory.bytes[i+3]);
    // }

    if (readingBytes != textLength)
    {
        printf("Error in `loadProgram()`: Bytes read from .bof file does not match size of text section in header\n");
        return;
    }

    // fseek(bf.fileptr, header.data_start_address, SEEK_SET);
    size_t dataLen = header.data_length;
    size_t dataRead = bof_read_bytes(bf, dataLen, memory.bytes + header.data_start_address);

    // debug_print("dataLen  %d\n", dataLen);
    // debug_print("dataRead %d\n", dataRead);

    if (dataRead != dataLen)
    {
        printf("Error in `loadProgram()`: Bytes read from .bof file does not match size of data section in header\n");
        return;
    }

    // Close the file
    bof_close(bf);
}

/// @brief Decompile and print assembly for a `.bof` file
/// @param bofFileName Name of the file to print
/// @param bf File to read
void printAssembly(const char *bofFileName)
{
    // Open the file
    BOFFILE bf = bof_read_open(bofFileName);

    // Read the header
    BOFHeader header = bof_read_header(bf);
    word_type addr = 0;

    // Print header
    printf("Addr Instruction\n");

    // Print text section
    for (int i = 0; i < header.text_length / BYTES_PER_WORD; i++)
    {
        bin_instr_t instr = instruction_read(bf);
        fprintf(stdout, "%4d ", addr);
        printf("%s \n", instruction_assembly_form(instr));
        addr += BYTES_PER_WORD;
    }

    // Read words from the data section of the file
    word_type dataStart = header.data_start_address;
    word_type dataEnd = dataStart + header.data_length;
    word_type currentDataAddress = dataStart;

    int elementsPrinted = 0; // Initialize the element counter

    while (currentDataAddress < dataEnd)
    {
        word_type value = bof_read_word(bf);
        if (value == 0)
        {
            break; // Exit the loop if value is zero
        }

        printf("%8d: %d    ", currentDataAddress, value);
        currentDataAddress += BYTES_PER_WORD;
        elementsPrinted++;

        // Check if we have printed 5 elements, and add a newline if needed
        if (elementsPrinted == 5)
        {
            printf("\n");
            elementsPrinted = 0; // Reset the counter after printing 5 elements
        }
    }

    // Print the final line
    printf("%d: %d ...\n", currentDataAddress, 0);
    // Close file
    bof_close(bf);
}

/// @brief Load and execute a `.bof` file
void executeProgram()
{
    // TODO
}

// We may need to add more functions(TBD)

// Task for h1.p2
// Write a VM that reads binary object files
// -read binary object files
// - loads the program in the bof

// with -p option:
//     prints the loaded program and exits

// with no options:
//     - executes the program
//     - produces tracing output

//     ./vm vm_test0.bof

// More information :
// https://www.cs.ucf.edu/~leavens/COP3402/lectures/hw1-2023/slides.txt