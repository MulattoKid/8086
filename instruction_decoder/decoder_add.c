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

#include "decoder_add.h"

#include "decoder.h"

static void add_mem_reg_no_displacement(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t rm)
{
    /* Check if direct address */
    if (rm == 0b110)
    {
        /* Get 16-bit displacement (guaranteed to be negative) */
        uint16_t displacement = 0;
        bool _displacement_is_negative = false;
        decoder_get_displacement(inst_stream, inst_stream_index, true, &displacement, &_displacement_is_negative);

        /* Determine source and destination*/
        if (d == 0)
        {
            const char* src_name = decoder_get_reg_name(w, reg);
            const uint16_t dst_name = displacement;
            
            /* Print decoded instruction */
            fprintf(file, "add [%u], %s\n", dst_name, src_name);
        }
        else /* d == 1*/
        {
            const uint16_t src_name = displacement;
            const char* dst_name = decoder_get_reg_name(w, reg);
            
            /* Print decoded instruction */
            fprintf(file, "add %s, [%u]\n", dst_name, src_name);
        }
    }
    else
    {
        /* Determine source and destination*/
        if (d == 0)
        {
            const char* src_name = decoder_get_reg_name(w, reg);
            const char* dst_name = decoder_get_effective_address(rm);
            
            /* Print decoded instruction */
            fprintf(file, "add [%s], %s\n", dst_name, src_name);
        }
        else /* d == 1*/
        {
            const char* src_name = decoder_get_effective_address(rm);
            const char* dst_name = decoder_get_reg_name(w, reg);
            
            /* Print decoded instruction */
            fprintf(file, "add %s, [%s]\n", dst_name, src_name);
        }
    }
}

static void add_mem_reg_with_displacement(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file, const uint8_t d, const uint8_t w, const uint8_t mod, const uint8_t reg, const uint8_t rm)
{
    /* Get displacement */
    uint16_t displacement = 0;
    bool displacement_is_negative = false;
    decoder_get_displacement(inst_stream, inst_stream_index, mod == 0b10, &displacement, &displacement_is_negative);

    /* Determine source and destination */
    if (d == 0)
    {
        const char* src_name = decoder_get_reg_name(w, reg);
        const char* dst_name = decoder_get_effective_address(rm);

        /* Print decoded instruction */
        if (displacement_is_negative == false)
        {
            fprintf(file, "add [%s + %u], %s\n", dst_name, displacement, src_name);
        }
        else /* displacement_is_negative == true */
        {
            fprintf(file, "add [%s %i], %s\n", dst_name, *((int16_t*)&displacement), src_name);
        }
    }
    else /* d == 1 */
    {
        const char* src_name = decoder_get_effective_address(rm);
        const char* dst_name = decoder_get_reg_name(w, reg);

        /* Print decoded instruction */
        if (displacement_is_negative == false)
        {
            fprintf(file, "add %s, [%s + %u]\n", dst_name, src_name, displacement);
        }
        else /* displacement_is_negative == true */
        {
            fprintf(file, "add %s, [%s %i]\n", dst_name, src_name, *((int16_t*)&displacement));
        }
    }
}

static void add_reg_reg(FILE* file, const uint8_t d, const uint8_t w, const uint8_t reg, const uint8_t rm)
{
    /* Determine source and destination*/
    if (d == 0)
    {
        const char* src_name = decoder_get_reg_name(w, reg);
        const char* dst_name = decoder_get_reg_name(w, rm);
        
        /* Print decoded instruction */
        fprintf(file, "add %s, %s\n", src_name, dst_name);
    }
    else /* d == 1*/
    {
        const char* src_name = decoder_get_reg_name(w, rm);
        const char* dst_name = decoder_get_reg_name(w, reg);
        
        /* Print decoded instruction */
        fprintf(file, "add %s, %s\n", src_name, dst_name);
    }
}

static void add_imm_mem_no_displacement(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file, const uint8_t s, const uint8_t w, const uint8_t rm)
{
    /* Check if direct address */
    if (rm == 0b110)
    {
        /* Get 16-bit displacement (guaranteed to be negative) */
        uint16_t displacement = 0;
        bool _displacement_is_negative = false;
        decoder_get_displacement(inst_stream, inst_stream_index, true, &displacement, &_displacement_is_negative);

        /* Get immediate value */
        uint16_t immediate = 0;
        bool immediate_is_negative = false;
        decoder_get_immediate(inst_stream, inst_stream_index, s, w, &immediate, &immediate_is_negative);

        /* Print decoded instruction */
        if (immediate_is_negative == false)
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%u], %u\n", displacement, immediate);
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%u], %u\n", displacement, immediate);
            }
        }
        else /* immediate_is_negative == true */
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%u], %i\n", displacement, *((int16_t*)&immediate));
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%u], %i\n", displacement, *((int16_t*)&immediate));
            }
        }
    }
    else
    {
        /* Get immediate value */
        uint16_t immediate = 0;
        bool immediate_is_negative = false;
        decoder_get_immediate(inst_stream, inst_stream_index, s, w, &immediate, &immediate_is_negative);

        /* Print decoded instruction */
        const char* destination_name = decoder_get_effective_address(rm);
        if (immediate_is_negative == false)
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%s], %u\n", destination_name, immediate);
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%s], %u\n", destination_name, immediate);
            }
        }
        else /* immediate_is_negative == true */
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%s], %i\n", destination_name, *((int16_t*)&immediate));
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%s], %i\n", destination_name, *((int16_t*)&immediate));
            }
        }
    }
}

static void add_imm_mem_with_displacement(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file, const uint8_t s, const uint8_t w, const uint8_t mod, const uint8_t rm)
{
    /* Get displacement */
    uint16_t displacement = 0;
    bool displacement_is_negative = false;
    decoder_get_displacement(inst_stream, inst_stream_index, mod == 0b10, &displacement, &displacement_is_negative);

    /* Get immediate value */
    uint16_t immediate = 0;
    bool immediate_is_negative = false;
    decoder_get_immediate(inst_stream, inst_stream_index, s, w, &immediate, &immediate_is_negative);

    /* Print decoded instruction */
    const char* destination_name = decoder_get_effective_address(rm);
    if (displacement_is_negative == false)
    {
        if (immediate_is_negative == false)
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%s + %u], %u\n", destination_name, displacement, immediate);
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%s + %u], %u\n", destination_name, displacement, immediate);
            }
        }
        else /* immediate_is_negative == true */
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%s + %u], %i\n", destination_name, displacement, *((int16_t*)&immediate));
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%s + %u], %i\n", destination_name, displacement, *((int16_t*)&immediate));
            }
        }
    }
    else /* displacement_is_negative == true */
    {
        
        if (immediate_is_negative == false)
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%s %i], %u\n", destination_name,  *((int16_t*)&displacement), immediate);
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%s %i], %u\n", destination_name,  *((int16_t*)&displacement), immediate);
            }
        }
        else /* immediate_is_negative == true */
        {
            if (w == 0)
            {
                fprintf(file, "add byte [%s %i], %i\n", destination_name,  *((int16_t*)&displacement), *((int16_t*)&immediate));
            }
            else /* w == 1 */
            {
                fprintf(file, "add word [%s %i], %i\n", destination_name,  *((int16_t*)&displacement), *((int16_t*)&immediate));
            }
        }
    }
}

static void add_imm_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file, const uint8_t s, const uint8_t w, const uint8_t rm)
{
    /* Get immediate value */
    uint16_t immediate = 0;
    bool immediate_is_negative = false;
    decoder_get_immediate(inst_stream, inst_stream_index, s, w, &immediate, &immediate_is_negative);

    /* Print decoded instruction */
    const char* destination_name = decoder_get_reg_name(w, rm);
    if (immediate_is_negative == false)
    {
        fprintf(file, "add %s, %u\n", destination_name, immediate);
    }
    else /* immediate_is_negative == true */
    {
        fprintf(file, "add %s, %i\n", destination_name, *((int16_t*)&immediate));
    }
}

void decode_add_regmem_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
{
    /* Get fields */
    const uint8_t d = (inst_stream[*inst_stream_index] & 0b10) >> 1;
    const uint8_t w = inst_stream[*inst_stream_index] & 0b1;
    (*inst_stream_index)++;
    const uint8_t mod = (inst_stream[*inst_stream_index] & 0b11000000) >> 6;
    const uint8_t reg = (inst_stream[*inst_stream_index] & 0b00111000) >> 3;
    const uint8_t rm = inst_stream[*inst_stream_index] & 0b00000111;
    (*inst_stream_index)++;

    switch (mod)
    {
        case 0b00:
        {
            add_mem_reg_no_displacement(inst_stream, inst_stream_index, file, d, w, reg, rm);
            break;
        }
        case 0b01:
        case 0b10:
        {
            add_mem_reg_with_displacement(inst_stream, inst_stream_index, file, d, w, mod, reg, rm);
            break;
        }
        case 0b11:
        {
            add_reg_reg(file, d, w, reg, rm);
            break;
        }
        default:
        {
            fprintf(file, "[DECODE MOD] Unsupported MOD field (0x%02X)\n", mod);
            return;
        }
    }
}

void decode_add_imm_to_regmem(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
{
     /* Get fields */
    const uint8_t s = (inst_stream[*inst_stream_index] & 0b10) >> 1;
    const uint8_t w = inst_stream[*inst_stream_index] & 0b1;
    (*inst_stream_index)++;
    const uint8_t mod = (inst_stream[*inst_stream_index] & 0b11000000) >> 6;
    const uint8_t rm = inst_stream[*inst_stream_index] & 0b00000111;
    (*inst_stream_index)++;

    switch (mod)
    {
        case 0b00:
        {
            add_imm_mem_no_displacement(inst_stream, inst_stream_index, file, s, w, rm);
            break;
        }
        case 0b01:
        case 0b10:
        {
            add_imm_mem_with_displacement(inst_stream, inst_stream_index, file, s, w, mod, rm);
            break;
        }
        case 0b11:
        {
            add_imm_reg(inst_stream, inst_stream_index, file, s, w, rm);
            break;
        }
        default:
        {
            fprintf(file, "[DECODE MOD] Unsupported MOD field (0x%02X)\n", mod);
            return;
        }
    }
}

void decode_add_imm_to_acc(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file)
{
    printf("decode_add_imm_to_acc not supported\n");
    exit(1);
}