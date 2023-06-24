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

//
typedef enum
{
    OPCODE_MOV = 0b10001000,
} opcode_t;

static void decode_mov(const uint8_t* const inst_stream, const uint32_t inst_stream_len, uint32_t* const inst_stream_index)
{
    // Check that there's enough remaining data to decode the instruction
    if ((*inst_stream_index + 2) > inst_stream_len)
    {
        printf("[DECODE MOV] Not enough data to decode instruction\n");
        return;
    }

    // MOV instruction is two bytes
    *inst_stream_index += 2;
}

/**
 * See page 4-18 in the manual.
*/
void inst_decode(const uint8_t* const inst_stream, const uint32_t inst_stream_len)
{
    uint32_t index = 0;
    while (index < inst_stream_len)
    {
        // Extract 6 MSbs
        const opcode_t opcode = inst_stream[index] & 0xFC;
        switch (opcode)
        {
            case OPCODE_MOV:
            {
                decode_mov(inst_stream, inst_stream_len, &index);
                break;
            }
            
            default:
            {
                printf("[DECODE] Unknown opcode (0x%01X)\n", opcode);
                break;
            }
        }
    }
}