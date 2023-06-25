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

#include <stdio.h>

#define REGISTER_COUNT (uint8_t)8U
#define ADDRESS_CALC_COUNT (uint8_t)8U

typedef enum
{
    OPCODE_MOV     = 0b10001000,
    OPCODE_MOV_IMM = 0b10110000,
} opcode_t;

static const char* reg_to_register_name[2][REGISTER_COUNT] = {
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

static const char* r_m_to_addr_calc_name[3][ADDRESS_CALC_COUNT] = {
    /* MOD=00 */
    {
        "bx + si",
        "bx + di",
        "bp + si",
        "bp + di",
        "si",
        "di",
        "direct address",
        "bx"
    },
    /* MOD=01 */
    {
        "bx + si + ",
        "bx + di + ",
        "bp + si + ",
        "bp + di + ",
        "si + ",
        "di + ",
        "bp + ",
        "bx + "
    },
    /* MOD=10 */
    {
        "bx + si + ",
        "bx + di + ",
        "bp + si + ",
        "bp + di + ",
        "si + ",
        "di + ",
        "bp + ",
        "bx + "
    }
};

static void mov_mod_00(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m)
{
    /* Handle direct address in displacement */
    if (r_m == 0b110)
    {
        /* Get 16-bit displacement */
        uint16_t addr = inst_stream[*inst_stream_index];
        (*inst_stream_index)++;
        addr |= (uint16_t)inst_stream[*inst_stream_index] << 8;
        (*inst_stream_index)++;

        /* Print decoded instruction */
        printf("mov %s, [0x%04X]\n", reg_to_register_name[w][reg], addr);

        return;
    }
    
    /* Determine what's source and destination */
    if (d == 0)
    {
        const char* src_name = reg_to_register_name[w][reg];
        const char* dst_name = r_m_to_addr_calc_name[0b00][r_m];

        /* Print decoded instruction */
        printf("mov %s, [%s]\n", dst_name, src_name);
    }
    else /* d == 1 */
    {
        const char* src_name = r_m_to_addr_calc_name[0b00][r_m];
        const char* dst_name = reg_to_register_name[w][reg];

        /* Print decoded instruction */
        printf("mov %s, [%s]\n", dst_name, src_name);
    }
}

static void mov_mod_11(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m)
{
    /* Determine what's source and destination */
    uint8_t src_index;
    uint8_t dst_index;
    if (d == 0)
    {
        src_index = reg;
        dst_index = r_m;
    }
    else /* d == 1 */
    {
        src_index = r_m;
        dst_index = reg;
    }

    /* Print decoded instruction */
    printf("mov %s, %s\n", reg_to_register_name[w][dst_index], reg_to_register_name[w][src_index]);
}

static void decode_mov(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index)
{
    /* Get fields */
    const uint8_t d = (inst_stream[*inst_stream_index] & 0b10) >> 1;
    const uint8_t w = inst_stream[*inst_stream_index] & 0b1;
    (*inst_stream_index)++;
    const uint8_t mod = (inst_stream[*inst_stream_index] & 0b11000000) >> 6;
    const uint8_t reg = (inst_stream[*inst_stream_index] & 0b00111000) >> 3;
    const uint8_t r_m = inst_stream[*inst_stream_index] & 0b00000111;
    (*inst_stream_index)++;

    /* Check fields */
    switch (mod)
    {
        case 0b00:
        {
            mov_mod_00(inst_stream, inst_stream_len, inst_stream_index, d, w, reg, r_m);
            break;
        }
        case 0b01:
        {
            return;
            break;
        }
        case 0b10:
        {
            return;
            break;
        }
        case 0b11:
        {
            mov_mod_11(inst_stream, inst_stream_len, inst_stream_index, d, w, reg, r_m);
            break;
        }
        default:
        {
            printf("[DECODE MOD] Unsupported MOD field (0x%02X)\n", mod);
            return;
        }
    }
}

static void decode_mov_imm(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index)
{
    /* Get fields */
    const uint8_t w = (inst_stream[*inst_stream_index] & 0b1000) >> 3;
    const uint8_t reg = inst_stream[*inst_stream_index] & 0b111;
    (*inst_stream_index)++;

    /* Get data */
    const uint8_t data_low = inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    uint8_t data_high = 0x00;
    if (w == 1)
    {
        data_high = inst_stream[*inst_stream_index];
        (*inst_stream_index)++;
    }

    /* Print decoded instruction */
    printf("mov %s, %u\n", reg_to_register_name[w][reg], ((uint16_t)data_high << 8) | (uint16_t)data_low);
}

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
void inst_decode(const uint8_t* const inst_stream, const uint32_t inst_stream_len)
{
    uint32_t index = 0;
    while (index < inst_stream_len)
    {
        /* Determine OPCODE */
        const opcode_t opcode = inst_stream[index] & 0xFC;
        if ((opcode & 0b11111100) == OPCODE_MOV)
        {
            decode_mov(inst_stream, inst_stream_len, &index);
        }
        else if ((opcode & 0b11110000) == OPCODE_MOV_IMM)
        {
            decode_mov_imm(inst_stream, inst_stream_len, &index);
        }
        else
        {
            printf("[DECODE] Unknown opcode (0x%02X)\n", opcode);
            return;
        }
    }
}