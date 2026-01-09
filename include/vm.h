/*
 * =====================================================================================
 *
 *       Filename:  vm.h
 *
 *    Description: chip8 header 
 *
 *        Version:  1.0
 *        Created:  09.01.2026 18:09:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  (freemorger), 
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once 
#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define GPR_COUNT 15 // general purpose registers
#define STACK_SIZE 16

typedef struct VM {
    uint16_t pc; // program counter 
    uint16_t i; // index register
    uint16_t stack[STACK_SIZE];
    char memory[MEMORY_SIZE];
    char gpr[GPR_COUNT]; // v0-ve
    char vf; // carry flag 
    char dt; // timer 
    char st; // sound 
    char sp; // stack pointer
} VM;

VM* init_vm();
bool load_program(VM* vm, char* filename);
void vm_run(VM* vm);
