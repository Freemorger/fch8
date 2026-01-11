/*
 * =====================================================================================
 *
 *       Filename:  vm.c
 *
 *    Description: chip8 vm  
 *
 *        Version:  1.1
 *        Created:  09.01.2026 18:08:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  (freemorger), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "../include/vm.h"

uint16_t get_be_db(uint8_t c1, uint8_t c2);
KeyboardKey* get_keymap();
uint16_t get_bcd(uint8_t n);
uint8_t* load_font();

VM* init_vm() {
    VM* vm = malloc(sizeof(VM));
    vm->pc = 0x200;
    vm->i = 0;
    vm->dt = 0;
    vm->st = 0;
    vm->sp = 0;
    vm->running = true;
    vm->drawfl = false;

    KeyboardKey* kmap = get_keymap();
    for (int i = 0; i < KEYPAD_COUNT; i++) {
        vm->keymap[i] = kmap[i];
    }
    uint8_t* fmap = load_font();
    memcpy(vm->fontmap, fmap, FONT_COUNT);
    
    return vm;
}

bool load_program(VM* vm, char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) return false;

    size_t read = fread(vm->memory + vm->pc, 1, MEMORY_SIZE, f);
    printf("was read %lu bytes\n", read);

    fclose(f);
    return true;
}

void vm_iter(VM* vm) {
    if (vm->pc >= MEMORY_SIZE) {
        vm->running = false;
        return;
    }

    vm->drawfl = false;
    uint16_t instr = get_be_db(
        vm->memory[vm->pc], vm->memory[vm->pc + 1]
    );
    //printf("pc: 0x%x, instr: 0x%x\n", vm->pc, instr);
    uint8_t nibble = (uint8_t)(instr >> 12);

    switch (nibble) {
        case 0: 
            if (instr == 0x00E0) { // clear bg 
                ClearBackground(BLACK);
            } else if (instr == 0x00EE) { // ret 
                if (vm->sp == 0) {
                    fprintf(stderr, "At PC %d: stack underflow!\n", vm->pc - 2);
                    exit(1);
                } 
                vm->sp--;
                uint16_t retaddr = vm->stack[vm->sp];
                vm->pc = retaddr - 2; // -2 for loop incr compensation
            }
            break;
        case 1: // 0x1NNN - jump at 0xNNN
            uint16_t jmp_addr = instr & 0x0FFF;
            vm->pc = jmp_addr - 2;
            return;
        case 2: // 0x2NNN - call func at 0xNNN  
            uint16_t ret_addr = vm->pc + 2;
            vm->stack[vm->sp] = ret_addr;
            vm->sp++;
            
            uint16_t func_addr = instr & 0x0FFF;
            vm->pc = func_addr - 2;
            return;
        case 3: // 0x3XNN - Vx == NN condition
            size_t reg_idx = (instr & 0x0F00) >> 8;
            uint8_t      val  = instr & 0x00FF;
            if (vm->gpr[reg_idx] == val) {
                vm->pc += 2; 
            }
            break;
        case 4: {// 0x4XNN - Vx != NN condition
            size_t reg_idx = (instr & 0x0F00) >> 8;
            uint8_t      val  = instr & 0x00FF;
            if (vm->gpr[reg_idx] != val) {
                vm->pc += 2; 
            }
            break;
        }
        case 5: {// 0x5XY0 - Vx == Vy condition
            size_t reg1_idx = (instr & 0x0F00) >> 8;
            size_t reg2_idx = (instr & 0x00F0) >> 4;
            if (vm->gpr[reg1_idx] == vm->gpr[reg2_idx]) {
                vm->pc += 2; 
            }
            break;
        }
        case 6: {// 0x6XNN: Vx = NN
            size_t reg_idx  = (instr & 0x0F00) >> 8;
            uint8_t    val   = instr & 0x00FF;
            vm->gpr[reg_idx] = val;
            break;
        }
        case 7: {// 0x7XNN: Vx += NN
            size_t reg_idx  = (instr & 0x0F00) >> 8;
            uint8_t    val   = instr & 0x00FF;
            vm->gpr[reg_idx] += val;
            break;
        }
        case 8: {
            uint8_t low_nib = instr & 0x000F;
            switch (low_nib) {
                case 0: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] = vm->gpr[rsrc_idx];
                    break;
                }
                case 1: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] |= vm->gpr[rsrc_idx];
                    break;
                }
                case 2: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] &= vm->gpr[rsrc_idx];
                    break;
                }
                case 3: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] ^= vm->gpr[rsrc_idx];
                    break;
                }
                case 4: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] += vm->gpr[rsrc_idx];
                    break;
                }
                case 5: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] -= vm->gpr[rsrc_idx];
                    break;
                }
                case 6: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] >>= 1;
                    break;
                }
                case 7: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] = vm->gpr[rsrc_idx] - vm->gpr[rdst_idx];
                    break;
                }
                case 0xE: {
                    size_t rdst_idx = (instr & 0x0F00) >> 8;
                    size_t rsrc_idx = (instr & 0x00F0) >> 4;

                    vm->gpr[rdst_idx] <<= 1;
                    break;
                }
                 
                default: 
                    fprintf(stderr, "Illegal opcode 0x%x\n", instr);
                    exit(2);
                    break;
            }
            break;
        }
        case 9: {
            size_t r1_idx = (instr & 0x0F00) >> 8;
            size_t r2_idx = (instr & 0x00F0) >> 4;

            if (vm->gpr[r1_idx] != vm->gpr[r2_idx]) {
                vm->pc += 2;
            }
            break;
        }
        case 0xA: {
            uint16_t val = instr & 0x0FFF;

            vm->i = val;
            break;
        }
        case 0xB: {
            size_t jmp_addr = instr & 0x0FFF;
            
            vm->pc = jmp_addr + vm->gpr[0] - 2;
            break;
        }
        case 0xC: {
            size_t rdst_idx = (instr & 0x0F00) >> 8;
            uint8_t        val = instr & 0x00FF;

            // gratefully clamping in [0; 255]
            vm->gpr[rdst_idx] = (rand() % 255) & val;
            break;
        }
        case 0xD: {
            vm->drawfl = true;

            size_t reg1_idx = (instr & 0x0F00) >> 8;
            size_t reg2_idx = (instr & 0x00F0) >> 4;
            size_t        n = instr & 0x000F;

            size_t vx = vm->gpr[reg1_idx];
            size_t vy = vm->gpr[reg2_idx];

            uint8_t* spt_ptr = &vm->memory[vm->i]; // sprite ptr  
            
            vm->gpr[0xf] = 0; // carry flag
             
            for (uint8_t y = 0; y < n; y++) { // row 
                uint8_t sum_y = vy + y;
                uint8_t tgt_y = sum_y % VRAM_HEIGHT;
                for (uint8_t x = 0; x < 8; x++) { // col 
                    uint8_t sum_x = vx + x;
                    uint8_t tgt_x = sum_x % VRAM_WIDTH;

                    uint8_t* vram_ptr = &vm->vram[tgt_y][tgt_x];

                    uint8_t       inv = 7 - x;
                    uint8_t spritebit = (*spt_ptr  >> inv) & 1;
                    uint8_t   vrambit = (*vram_ptr >> inv) & 1;

                    uint8_t  resbit = spritebit ^ vrambit;
                    *(vram_ptr) = resbit;
                    bool changed = (resbit == 1) && (vrambit == 0); // changed
                                                                    // to 1

                    if (changed) vm->gpr[0xf] = 1;


                }
                spt_ptr++;
            } 
            break;
        }
        case 0xE: {
            uint8_t low_byte = instr & 0xFF;
            uint8_t   r1_idx = (instr & 0x0F00) >> 8;
            uint8_t      key = vm->gpr[r1_idx] & 0xFF;

            switch (low_byte) {
                case 0x9E: {
                    if (IsKeyDown(key)) {
                        vm->pc += 2;
                    }
                    break;
                }
                case 0xA1: {
                    if (!IsKeyDown(key)) {
                        vm->pc += 2;
                    }
                    break;
                }
                default: {
                    fprintf(stderr, "Illegal opcode 0x%x\n", instr);
                    exit(2);
                    break;
                }
            }
            break;
        }
        case 0xF: {
            uint8_t low_byte = instr & 0xFF;
            uint8_t   r1_idx = (instr & 0x0F00) >> 8;

            switch (low_byte) {
                case 0x7: {
                    vm->gpr[r1_idx] = vm->dt;
                    break;
                }
                case 0x0A: {
                    vm->gpr[r1_idx] = await_key(vm);
                    break;
                }
                case 0x15: {
                    vm->dt = vm->gpr[r1_idx];
                    break;
                }
                case 0x18: {
                    vm->st = vm->gpr[r1_idx];
                    break;
                }
                case 0x1E: {
                    vm->i += vm->gpr[r1_idx];
                    break;
                }
                case 0x29: {
                    uint8_t digit = vm->gpr[r1_idx] & 0xF; // low nib 
                    
                    vm->i = 0x50 + (digit * 5);

                    break;
                }
                case 0x33: {
                    uint16_t bcd = get_bcd(vm->gpr[r1_idx]);
                    printf("BCD for %d is 0x%x \n", vm->gpr[r1_idx], bcd);

                    uint8_t* ptr = &vm->memory[vm->i];

                    *ptr = bcd & 0xF;
                    *(ptr + 1) = (bcd >> 4) & 0xF;
                    *(ptr + 2) = (bcd >> 8) & 0xF;
                    break;
                }
                case 0x55: {
                    uint8_t* ptr = &vm->memory[vm->i];

                    for (int i = 0; i <= r1_idx; i++) {
                        *(ptr + i) = vm->gpr[i];
                    }
                    break;
                }
                case 0x65: {
                    uint8_t* ptr = &vm->memory[vm->i];
                    
                    for (int i = 0; i <= r1_idx; i++) {
                        vm->gpr[i] = *(ptr + i);
                    }
                    break;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Illegal opcode 0x%x\n", instr);
            exit(2);
            break;
    }
}

uint16_t get_be_db(uint8_t c1, uint8_t c2) {
    uint16_t res = (c1 << 8) | c2;
    return res;
}

KeyboardKey* get_keymap() {
    static KeyboardKey kmap[KEYPAD_COUNT] = { 
        KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, 
        KEY_Q, KEY_W, KEY_E, KEY_R, 
        KEY_A, KEY_S, KEY_D, KEY_F, 
        KEY_Z, KEY_X, KEY_C, KEY_V
    };
    return kmap;
}

/// Blocking op that would wait until some of chip8 
/// key would be pressed
uint8_t await_key(VM* vm) {
    for (;;) {
        for (uint8_t i = 0; i < 16; i++) {
            if (IsKeyDown(vm->keymap[i])) {
                return i;
            }
        } 
    }
}

uint16_t get_bcd(uint8_t n) {
    uint16_t ones = n % 10;
    uint16_t hundreds = (n / 100);
    uint16_t tens = ((n - hundreds * 100) / 10) << 4;

    return ones | tens | (hundreds << 8);
}

uint8_t* load_font() {
    static uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    
    return fontset;
}
