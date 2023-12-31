/*
Copyright (c) 2023 Daniel Fedai Larsen

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "decoder.h"

#include "decoder_add.h"
#include "decoder_mov.h"

#include <stdio.h>
#include <stdlib.h>

#define REGISTER_COUNT (uint8_t)8U
#define ADDRESS_CALC_COUNT (uint8_t)8U

typedef enum
{
    OPCODE_ADD                   = 0b00000000,
    OPCODE_ADD_IMM_TO_REG_OR_MEM = 0b10000000,
    OPCODE_ADD_IMM_TO_ACC        = 0b00000100,
    OPCODE_MOV                   = 0b10001000,
    OPCODE_MOV_IMM_TO_REG_OR_MEM = 0b11000110,
    OPCODE_MOV_IMM_TO_REG        = 0b10110000,
    OPCODE_MOV_MEM_TO_ACC        = 0b10100000,
    OPCODE_MOV_ACC_TO_MEM        = 0b10100010,
} opcode_t;

static const char* reg_to_reg_name[2][REGISTER_COUNT] = {
    /* W=0 */
    {
        "al",
        "cl",
        "dl",
        "bl",
        "ah",
        "ch",
        "dh",
        "bh",
    },
    /* W=1 */
    {
        "ax",
        "cx",
        "dx",
        "bx",
        "sp",
        "bp",
        "si",
        "di"
    }
};

static const char* r_m_to_addr_calc_name[ADDRESS_CALC_COUNT] = {
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx"
};

/**
 * See page 4-18 in the manual.
 * 
 * Multibyte instructions:
 *  - The first six bytes of a multibyte instruction generally contain an opcode that identifies the basic instruction type
 *  - The 7th bit called 'D' generally specifies the direction of the operation
 *    - 1 = the REG field in the second byte identifies the destination operand
 *    - 0 = the REG field in the second byte identifies the source operand
 *  - The 8th bit called 'W' distinguishes between operation sizes
 *    - 1 = instruction operates on word data
 *    - 0 = instruction operates on byte data
 *  - One of three additional single-bit fields, 'S', 'V' or 'Z', appears in some instructions
 *    - 'S' is used in conjunction with 'W' to indicate sign extension of immediate fields in arithmetic instructions
 *      - 1 = Sign extend 8-bit immediate data to 16 bits if W=1
 *      - 0 = No sign extension
 *    - 'V' distiguishes between single- and variable-bit shifts and rotates
 *      - 1 = Shift/rotate count is specified in CL register
 *      - 0 = Shift/rotate count is one
 *    - 'Z' is ued as a compare bit with the zero flag in conditional repeat and loop instructions
 *      - 1 = Repeat/loop while zero flag is set
 *      - 0 = Repeat/loop while zero flag is clear
 *  - The second byte of the instruction usually identifies the instruction's operands
 *    - The MOD field indicates whether
 *      - 00 = Memory mode, no displacement follows (except when R/M=110, then 16-bit displacement follows)
 *      - 01 = Memory mode, 8-bit displacement follows
 *      - 10 = Memory mode, 16-bit displacement follows
 *      - 11 = Register mode (no displacement)
 *    - The REG field identifies a register that is one of the instruction operands (in some instructions, mainly
 *       immediate-to-memory, REG is used as an extension of the opcode to identify the type of operation)
 *              W=0  W=1
 *      - 000 = AL   AX
 *      - 001 = CL   CX
 *      - 010 = DL   DX
 *      - 011 = BL   BX
 *      - 100 = AH   SP
 *      - 101 = CH   BP
 *      - 110 = DH   SI
 *      - 111 = BH   DI
 *  - The encoding of the R/M (register/memory) field depends on how the MOD field is set
 *    - If MOD=11, R/M is treated as the REG field
 *    - If MOD!=11, the value in R/M determines how to compute the effective address (see Table 4-10)
 *  - Bytes three through siz of an instruction are optional fields that usually contain the displacement value of a memory
 *    operand and/or the actual value of an immediate constant operand
*/
void decoder_decode_stream(const uint8_t* const inst_stream, const uint32_t inst_stream_len, FILE* output_file)
{
    fprintf(output_file, "bits 16\n\n");

    uint32_t index = 0;
    while (index < inst_stream_len)
    {
        /* Determine OPCODE */
        const opcode_t opcode = inst_stream[index];
        if ((opcode & 0b11111100) == OPCODE_ADD)
        {
            decode_add_regmem_reg(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11111100) == OPCODE_ADD_IMM_TO_REG_OR_MEM)
        {
            decode_add_imm_to_regmem(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_ADD_IMM_TO_ACC)
        {
            decode_add_imm_to_acc(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11111100) == OPCODE_MOV)
        {
            decode_mov_regmem_tofrom_reg(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_MOV_IMM_TO_REG_OR_MEM)
        {
            decode_mov_imm_to_mem(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11110000) == OPCODE_MOV_IMM_TO_REG)
        {
            decode_mov_imm_to_reg(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_MOV_MEM_TO_ACC)
        {
            decode_mov_mem_to_acc(inst_stream, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_MOV_ACC_TO_MEM)
        {
            decode_mov_acc_to_mem(inst_stream, &index, output_file);
        }
        else
        {
            fprintf(output_file, "[DECODE] Unknown opcode (0x%02X)\n", opcode);
            break;
        }

        fflush(output_file);
    }
}

void decoder_get_displacement(const uint8_t* const inst_stream, uint32_t* inst_stream_index, const bool is_16_bit, uint16_t* const displacement, bool* const displacement_is_negative)
{
    *displacement_is_negative = false;
    *displacement = inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    if (is_16_bit == true)
    {
        *displacement |= (uint16_t)inst_stream[*inst_stream_index] << 8;
        (*inst_stream_index)++;
    }
    else /* is_16_bit == false */
    {
        // Sign-extend if negative
        if (*displacement & 0x80)
        {
            *displacement |= 0xFF00;
            *displacement_is_negative = true;
        }
    }
}

void decoder_get_immediate(const uint8_t* const inst_stream, uint32_t* inst_stream_index, const uint8_t s, const uint8_t w, uint16_t* const immediate, bool* const immediate_is_negative)
{
    *immediate_is_negative = false;
    *immediate = inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    /* 16-bit value */
    if ((s == 0) && (w == 1))
    {
        *immediate |= (uint16_t)inst_stream[*inst_stream_index] << 8;
        (*inst_stream_index)++;
    }
    else if ((s == 1) && (w == 1) && (*immediate & 0x80)) /* Sign-extend if negative */
    {
        *immediate |= 0xFF00;
        *immediate_is_negative = true;
    }
}

const char* decoder_get_reg_name(const uint8_t w, const uint8_t reg)
{
    return reg_to_reg_name[w][reg];
}

const char* decoder_get_effective_address(const uint8_t rm)
{
    return r_m_to_addr_calc_name[rm];
}