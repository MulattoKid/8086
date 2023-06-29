#ifndef DECODER_MOV_H
#define DECODER_MOV_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Decode register/memory to/from register instruction (0b100010xx)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_mov_regmem_tofrom_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

/**
 * @brief Decode immediate to memory instruction (0b1100011x)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_mov_imm_to_mem(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

/**
 * @brief Decode immediate to register instruction (0b1011xxxx)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_mov_imm_to_reg(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

/**
 * @brief Decode memory to accumulator instruction (0b1010000x)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_mov_mem_to_acc(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

/**
 * @brief Decode accumulator to memory instruction (0b1010001x)
 * 
 * @param instr_stream Stream of bytes with encoded instructions
 * @param instr_stream_index Current index into 'instr_stream'
 * @param file File to write decoded instruction into
*/
void decode_mov_acc_to_mem(const uint8_t* const inst_stream, uint32_t* const inst_stream_index, FILE* file);

#endif
