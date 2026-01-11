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
#include <raylib.h>

#define MEMORY_SIZE 4096
#define GPR_COUNT 16 // general purpose registers
#define STACK_SIZE 16
#define VRAM_WIDTH 64
#define VRAM_HEIGHT 32
#define KEYPAD_COUNT 16
#define FONT_COUNT 80 // in bytes

typedef struct VM {
    uint16_t pc; // program counter 
    uint16_t i; // index register
    uint16_t stack[STACK_SIZE];
    uint8_t  memory[MEMORY_SIZE];
    uint8_t  vram[VRAM_HEIGHT][VRAM_WIDTH];
    uint8_t  gpr[GPR_COUNT]; // v0-ve
    uint8_t  dt; // timer 
    uint8_t  st; // sound 
    uint8_t  sp; // stack pointer

    bool running;
    bool drawfl; // draw flag for internal usage
                 
    KeyboardKey keymap[KEYPAD_COUNT];
    uint8_t     fontmap[FONT_COUNT];
} VM;

VM* init_vm();
bool load_program(VM* vm, char* filename);
void vm_iter(VM* vm);
uint8_t await_key(VM* vm);
