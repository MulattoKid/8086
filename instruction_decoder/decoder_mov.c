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

#include "decoder_mov.h"

#include "decoder.h"

#include <stdbool.h>

static void mov_mod_00(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m, FILE* file)
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

static void mov_mod_01_10(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m, const uint8_t mod, FILE* file)
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

static void mov_mod_11(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t r_m, FILE* file)
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

void decode_mov_regmem_tofrom_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
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
            mov_mod_00(inst_stream, inst_stream_index, d, w, reg, r_m, file);
            break;
        }
        case 0b01:
        case 0b10:
        {
            mov_mod_01_10(inst_stream, inst_stream_index, d, w, reg, r_m, mod, file);
            break;
        }
        case 0b11:
        {
            mov_mod_11(inst_stream, inst_stream_index, d, w, reg, r_m, file);
            break;
        }
        default:
        {
            fprintf(file, "[DECODE MOD] Unsupported MOD field (0x%02X)\n", mod);
            return;
        }
    }
}

void decode_mov_imm_to_mem(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
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

void decode_mov_imm_to_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
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

void decode_mov_mem_to_acc(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
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

void decode_mov_acc_to_mem(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
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
