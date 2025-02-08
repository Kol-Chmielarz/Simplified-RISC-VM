/**
 * @file reg_instructs.c
 * @author Chris Hazen - group 24
 * @brief Functions for register instructions (see Table 3. -srm-vm.dpf)
 */
#include "reg_instructs.h"

#include "instruction.h"
#include "registers.h"
#include "utilities.h"

/*
| Name | op | rs | rt | rd | shift | func | Explanation                                                                                      |
| ---- | -- | -- | -- | -- | ----- | ---- | ------------------------------------------------------------------------------------------------ |
| ADD  | 0  | s  | t  | d  | -     | 33   | Add: GPR[d] <- GPR[s] + GPR[t]                                                                   |
| SUB  | 0  | s  | t  | d  | -     | 35   | Subtract: GPR[d] <- GPR[s] - GPR[t]                                                              |
| MUL  | 0  | s  | t  | -  | -     | 25   | Multiply: Multiply GPR[s] and GPR[t], putting the least significant bits in LO and the most significant bits in HI.  (HI, LO) <- GPR[s] x GPR[t] |
| DIV  | 0  | s  | t  | -  | -     | 27   | Divide (remainder in HI, quotient in LO): HI <- GPR[s] % GPR[t] LO <- GPR[s]/GPR[t]              |
| MFHI | 0  | -  | -  | d  | -     | 16   | Move from HI: GPR[d] <- HI                                                                       |
| MFLO | 0  | -  | -  | d  | -     | 18   | Move from LO: GPR[d] <- LO                                                                       |
| AND  | 0  | s  | t  | d  | -     | 36   | Bitwise And: GPR[d] <- GPR[s] and GPR[t]                                                         |
| BOR  | 0  | s  | t  | d  | -     | 37   | Bitwise Or: GPR[d] <- GPR[s] or GPR[t]                                                           |
| NOR  | 0  | s  | t  | d  | -     | 39   | Bitwise Not-Or: GPR[d] <- not(GPR[s] or GPR[t])                                                  |
| XOR  | 0  | s  | t  | d  | -     | 38   | Bitwise Exclusive-Or: GPR[d] <- GPR[s] xor GPR[t]                                                |
| SLL  | 0  | -  | t  | d  | h     | 0    | Shift Left Logical: GPR[d] <- GPR[t] << h                                                        |
| SRL  | 0  | -  | t  | d  | h     | 3    | Shift Right Logical: GPR[d] <- GPR[t] >> h                                                       |
| JR   | 0  | s  | 0  | 0  | 0     | 8    | Jump Register: PC <- GPR[s]                                                                      |
*/

/** Call register instruction based on 32bit word */
void callRegInstr(reg_instr_t instr)
{
    // Assert that the op code of the register instruction is 0
    if (instr.op != REG_O)
    {
        bail_with_error("Error in `cal_reg_instr()`: Op code != 0");
    }

    switch(instr.func)
    {
    case ADD_F:
        // Add function code
        setReg(instr.rd, getReg(instr.rs) + getReg(instr.rt));
        break;
    case SUB_F:
        // Subtract function code
        setReg(instr.rd, getReg(instr.rs) - getReg(instr.rt));
        break;
    case MUL_F:
        // Multiply function code
        {
        long long mult = (long long) getReg(instr.rs) * getReg(instr.rt);
        HI = mult & 0xFFFFFFFF; // Least significant bits
        LO = mult >> 32; // Most significant bits
        break;
        }
    case DIV_F:
        // Divide function code
        if(getReg(instr.rt) != 0)
        {
            HI = getReg(instr.rs) % getReg(instr.rt); // Remainder
            LO = getReg(instr.rs) / getReg(instr.rt); // Quotient
        }
        break;
    case MFHI_F:
        // Move from HI function code
        setReg(instr.rd, HI);
        break;
    case MFLO_F:
        // Move from LO function code
        setReg(instr.rd, LO);
        break;
    case AND_F:
        // AND function code
        setReg(instr.rd, getReg(instr.rs) & getReg(instr.rt));
        break;
    case BOR_F:
        // OR function code
        setReg(instr.rd, getReg(instr.rs) | getReg(instr.rt));
        break;
    case NOR_F:
        // NOR function code
        setReg(instr.rd, ~(getReg(instr.rs) | getReg(instr.rt)));
        break;
    case XOR_F:
        // XOR function code
        setReg(instr.rd, getReg(instr.rs) ^ getReg(instr.rt));
        break;
    case SLL_F:
        // Shift left logical function code
        setReg(instr.rd, getReg(instr.rt) << instr.shift);
        break;
    case SRL_F:
        // Shift right logical function code
        setReg(instr.rd, getReg(instr.rt) >> instr.shift);
        break;
    case JR_F:
        // Jump register function code
        PC = getReg(instr.rs) - 4; // Offset to account for PC increasing next instruction
        break;
    default:
        // Default case when no function code matches
        bail_with_error("Error in `callRegInstr()`: Invalid function code");
    }
}

