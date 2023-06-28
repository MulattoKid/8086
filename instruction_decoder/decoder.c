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

#include <stdbool.h>
#include <stdio.h>

#define REGISTER_COUNT (uint8_t)8U
#define ADDRESS_CALC_COUNT (uint8_t)8U

typedef enum
{
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


static void mov_mod_00(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m, FILE* file)
{
    /* Handle direct address in displacement */
    if (r_m == 0b110)
    {
        /* Get 16-bit displacement */
        uint16_t addr = inst_stream[*inst_stream_index];
        (*inst_stream_index)++;
        addr |= (uint16_t)inst_stream[*inst_stream_index] << 8;
        (*inst_stream_index)++;

        /* Determine what's source and destination */
        if (d == 0)
        {
            const char* src_name = decoder_get_reg_name_from_reg(w, reg);
            const uint16_t dst_name = addr;

            /* Print decoded instruction */
            fprintf(file, "mov [0x%04X], %s\n", dst_name, src_name);
        }
        else /* d == 1 */
        {
            const uint16_t src_name = addr;
            const char* dst_name = decoder_get_reg_name_from_reg(w, reg);

            /* Print decoded instruction */
            fprintf(file, "mov %s, [0x%04X]\n", dst_name, src_name);
        }

        return;
    }
    
    /* Determine what's source and destination */
    if (d == 0)
    {
        const char* src_name = decoder_get_reg_name_from_reg(w, reg);
        const char* dst_name = decoder_get_effective_address_from_rm(r_m);

        /* Print decoded instruction */
        fprintf(file, "mov [%s], %s\n", dst_name, src_name);
    }
    else /* d == 1 */
    {
        const char* src_name = decoder_get_effective_address_from_rm(r_m);
        const char* dst_name = decoder_get_reg_name_from_reg(w, reg);

        /* Print decoded instruction */
        fprintf(file, "mov %s, [%s]\n", dst_name, src_name);
    }
}

static void mov_mod_01_10(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m, const uint8_t mod, FILE* file)
{
    /* Get address */
    bool address_is_negative = false;
    uint16_t address = (uint16_t)inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    if (mod == 0b10)
    {
        address |= ((uint16_t)inst_stream[*inst_stream_index]) << 8;
        (*inst_stream_index)++;
    }
    else /* mod == 0b01 */
    {
        /* Check if address is negative, and if so sign-extend it to 16 bits*/
        if ((address & 0x80) == 0x80)
        {
            address |= 0xFF00;
            address_is_negative = true;
        }
    }

    /* Determine what's source and destination */
    if (d == 0)
    {
        const char* src_name = decoder_get_reg_name_from_reg(w, reg);
        const char* dst_name = decoder_get_effective_address_from_rm(r_m);

        /* Print decoded instruction */
        if (address == 0x00)
        {
            fprintf(file, "mov [%s], %s\n", dst_name, src_name);
        }
        else
        {
            if (address_is_negative == false)
            {
                fprintf(file, "mov [%s + %u], %s\n", dst_name, address, src_name);
            }
            else
            {
                fprintf(file, "mov [%s + %i], %s\n", dst_name, *((int16_t*)&address), src_name);
            }
        }
    }
    else /* d == 1 */
    {
        const char* src_name = decoder_get_effective_address_from_rm(r_m);
        const char* dst_name = decoder_get_reg_name_from_reg(w, reg);
        
        /* Print decoded instruction */
        if (address == 0x00)
        {
            fprintf(file, "mov %s, [%s]\n", dst_name, src_name);
        }
        else
        {
            if (address_is_negative == false)
            {
                fprintf(file, "mov %s, [%s + %u]\n", dst_name, src_name, address);
            }
            else
            {
                fprintf(file, "mov %s, [%s + %i]\n", dst_name, src_name, *((int16_t*)&address));
            }
        }
    }
}

static void mov_mod_11(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m, FILE* file)
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
    fprintf(file, "mov %s, %s\n", decoder_get_reg_name_from_reg(w, dst_index), decoder_get_reg_name_from_reg(w, src_index));
}

static void decode_mov(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, FILE* file)
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
            mov_mod_00(inst_stream, inst_stream_len, inst_stream_index, d, w, reg, r_m, file);
            break;
        }
        case 0b01:
        case 0b10:
        {
            mov_mod_01_10(inst_stream, inst_stream_len, inst_stream_index, d, w, reg, r_m, mod, file);
            break;
        }
        case 0b11:
        {
            mov_mod_11(inst_stream, inst_stream_len, inst_stream_index, d, w, reg, r_m, file);
            break;
        }
        default:
        {
            fprintf(file, "[DECODE MOD] Unsupported MOD field (0x%02X)\n", mod);
            return;
        }
    }
}

static void decode_mov_imm_to_reg_or_mem(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, FILE* file)
{
    /* Get fields */
    const uint8_t w = inst_stream[*inst_stream_index] & 0b1;
    (*inst_stream_index)++;
    const uint8_t mod = (inst_stream[*inst_stream_index] & 0b11000000) >> 6;
    const uint8_t r_m = inst_stream[*inst_stream_index] & 0b111;
    (*inst_stream_index)++;

    /* Get address */
    uint16_t address = 0;
    if (mod != 0b00)
    {
        address = (uint16_t)inst_stream[*inst_stream_index];
        (*inst_stream_index)++;
    }
    if (mod == 0b10)
    {
        address |= ((uint16_t)inst_stream[*inst_stream_index]) << 8;
        (*inst_stream_index)++;
    }

    /* Get immediate value */
    uint16_t imm = (uint16_t)inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    if (w == 1)
    {
        imm |= ((uint16_t)inst_stream[*inst_stream_index]) << 8;
        (*inst_stream_index)++;
    }

    /* Determine what's source and destination */
    const uint16_t src_name = imm;
    const char* dst_name = decoder_get_effective_address_from_rm(r_m);

    /* Print decoded instruction */
    if (address == 0x00)
    {
        if (w == 0)
        {
            fprintf(file, "mov [%s], byte %u\n", dst_name, src_name);
        }
        else /* w == 1*/
        {
            fprintf(file, "mov [%s], word %u\n", dst_name, src_name);
        }
    }
    else
    {
        if (w == 0)
        {
            fprintf(file, "mov [%s + %u], byte %u\n", dst_name, address, src_name);
        }
        else /* w == 1*/
        {
            fprintf(file, "mov [%s + %u], word %u\n", dst_name, address, src_name);
        }
    }
}

static void decode_mov_imm_to_reg(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, FILE* file)
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
    fprintf(file, "mov %s, %u\n", decoder_get_reg_name_from_reg(w, reg), ((uint16_t)data_high << 8) | (uint16_t)data_low);
}

static void decode_mov_mem_to_acc(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, FILE* file)
{
    /* Get fields */
    const uint8_t w = inst_stream[*inst_stream_index] & 0b1;
    (*inst_stream_index)++;

    /* Get address */
    uint16_t address = (uint16_t)inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    if (w == 1)
    {
        address |= ((uint16_t)inst_stream[*inst_stream_index]) << 8;
        (*inst_stream_index)++;
    }

    /* Print decoded instruction */
    fprintf(file, "mov ax, [0x%04X]\n", address);
}

static void decode_mov_acc_to_mem(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index, FILE* file)
{
    /* Get fields */
    const uint8_t w = inst_stream[*inst_stream_index] & 0b1;
    (*inst_stream_index)++;

    /* Get address */
    uint16_t address = (uint16_t)inst_stream[*inst_stream_index];
    (*inst_stream_index)++;
    if (w == 1)
    {
        address |= ((uint16_t)inst_stream[*inst_stream_index]) << 8;
        (*inst_stream_index)++;
    }

    /* Print decoded instruction */
    fprintf(file, "mov [0x%04X], ax\n", address);
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
void decoder_decode_stream(const uint8_t* const inst_stream, const uint32_t inst_stream_len, FILE* output_file)
{
    uint32_t index = 0;
    while (index < inst_stream_len)
    {
        /* Determine OPCODE */
        const opcode_t opcode = inst_stream[index];
        if ((opcode & 0b11111100) == OPCODE_MOV)
        {
            decode_mov(inst_stream, inst_stream_len, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_MOV_IMM_TO_REG_OR_MEM)
        {
            decode_mov_imm_to_reg_or_mem(inst_stream, inst_stream_len, &index, output_file);
        }
        else if ((opcode & 0b11110000) == OPCODE_MOV_IMM_TO_REG)
        {
            decode_mov_imm_to_reg(inst_stream, inst_stream_len, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_MOV_MEM_TO_ACC)
        {
            decode_mov_mem_to_acc(inst_stream, inst_stream_len, &index, output_file);
        }
        else if ((opcode & 0b11111110) == OPCODE_MOV_ACC_TO_MEM)
        {
            decode_mov_acc_to_mem(inst_stream, inst_stream_len, &index, output_file);
        }
        else
        {
            fprintf(output_file, "[DECODE] Unknown opcode (0x%02X)\n", opcode);
            return;
        }
    }
}

const char* decoder_get_reg_name_from_reg(const uint8_t w, const uint8_t reg)
{
    return reg_to_reg_name[w][reg];
}

const char* decoder_get_effective_address_from_rm(const uint8_t rm)
{
    return r_m_to_addr_calc_name[rm];
}