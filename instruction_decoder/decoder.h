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

#ifndef DECODER_H
#define DECODER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void decoder_decode_stream(const uint8_t* const inst_stream, const uint32_t inst_stream_len, FILE* output_file);
void decoder_get_displacement(const uint8_t* const inst_stream, uint32_t* inst_stream_index, const bool is_16_bit, uint16_t* const displacement, bool* const displacement_is_negative);
void decoder_get_immediate(const uint8_t* const inst_stream, uint32_t* inst_stream_index, const uint8_t s, const uint8_t w, uint16_t* const immediate, bool* const immediate_is_negative);
const char* decoder_get_reg_name(const uint8_t w, const uint8_t reg);
const char* decoder_get_effective_address(const uint8_t rm);

#endif
