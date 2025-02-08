#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "memory.h"
#include "registers.h"
#include "utilities.h"
#include "immed_instructs.h"
#include "machine_types.h"

void callImmedInstr(immed_instr_t instr) {
    address_type address;
    byte_type loadedByte;
    byte_type leastSignificantByte;
    word_type loadedWord;

    switch (instr.op)
    {
    case ADDI_O:
        // ADDI instruction: GPR[t] <- GPR[s] + sgnExt(i)
        setReg(instr.rt, getReg(instr.rs) + machine_types_sgnExt(instr.immed));
        break;

    case ANDI_O:
        // ANDI instruction: GPR[t] <- GPR[s] & zeroExt(i)
         setReg(instr.rt, getReg(instr.rs) & machine_types_zeroExt(instr.immed));
        break;

    case BORI_O:
        // Bitwise Or immediate: GPR[t] <- GPR[s] ∨ zeroExt(i)
        setReg(instr.rt, getReg(instr.rs) | machine_types_zeroExt(instr.immed));
        break;

    case XORI_O:
        // Bitwise Xor immediate: GPR[t] <- GPR[s] xor zeroExt(i)
         setReg(instr.rt, getReg(instr.rs) ^ machine_types_zeroExt(instr.immed));
        break;

    case BEQ_O:
        //Branch on Equal: if GPR[s] = GPR[t] then PC <- PC + formOffset(o)
        if (getReg(instr.rs) == getReg(instr.rt))
        {
            PC += machine_types_formOffset(instr.immed);
        }
        break;

    case BGEZ_O:
        // Branch >= 0: if GPR[s] >= 0 then PC <- PC + formOffset(o)
        if (getReg(instr.rs) >= 0)
        {
            PC += machine_types_formOffset(instr.immed);
        }
        break;

    case BGTZ_O:
        //Branch > 0: if GPR[s] > 0 then PC <- PC + formOffset(o)
        if (getReg(instr.rs) > 0)
        {
            PC += machine_types_formOffset(instr.immed);
        }
        break;

    case BLEZ_O:
        // Branch <= 0: if GPR[s] <= 0 then PC <- PC + formOffset(o)
         setReg(instr.rt, getReg(instr.rs) & machine_types_zeroExt(instr.immed));
        break;

    case BLTZ_O:
        // Branch < 0: if GPR[s] < 0 then PC <- PC + formOffset(o)
        if (getReg(instr.rs) < 0)
        {
            PC += machine_types_formOffset(instr.immed);
        }
        break;

    case BNE_O:
        // BNE instruction: if GPR[s] != GPR[t] then PC <- PC + formOffset(o)
        if (getReg(instr.rs) != getReg(instr.rt))
        {
            PC += machine_types_formOffset(instr.immed);
        }
        break;

    case LBU_O:
        //Load Byte Unsigned: GPR[t] <- zeroExt(memory[GPR[b] + formOffset(o)])
        address = getReg(instr.rs) + machine_types_formOffset(instr.immed);
        loadedByte = memory.bytes[address];
        setReg(instr.rt, machine_types_zeroExt((immediate_type)loadedByte));
        break;

    case LW_O:
        //Load Word (4 bytes): GPR[t] <- memory[GPR[b] + formOffset(o)]
        address = getReg(instr.rs) + machine_types_formOffset(instr.immed);
        loadedWord = memory.words[address / BYTES_PER_WORD];
        setReg(instr.rt, loadedWord);
        break;

    case SB_O:
        //Store Byte (least significant byte of GPR[t]): memory[GPR[b] + formOffset(o)] <- GPR[t]
        address = getReg(instr.rs) + machine_types_formOffset(instr.immed);
        leastSignificantByte = (byte_type)(getReg(instr.rt) & 0xFF);
        memory.bytes[address] = leastSignificantByte;
        break;

    case SW_O:
        //Store Word (4 bytes): memory[GPR[b] + formOffset(o)] <- GPR[t]
        address = getReg(instr.rs) + machine_types_formOffset(instr.immed);
        memory.words[address / BYTES_PER_WORD] = getReg(instr.rt);
        break;

    default:
        // Handle unknown opcode
        printf("Unknown opcode: %d\n", instr.op);
        exit(EXIT_FAILURE);
    }
}
