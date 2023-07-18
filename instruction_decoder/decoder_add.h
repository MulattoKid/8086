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

#ifndef DECODER_ADD_H
#define DECODER_ADD_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Decode register/memory plus register instruction (0b000000xx)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_add_regmem_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

/**
 * @brief Decode immediate plus register/memory instruction (0b100000xx)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_add_imm_to_regmem(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

/**
 * @brief Decode immediate plus accumulator instruction (0b0000001x)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_add_imm_to_acc(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

#endif
