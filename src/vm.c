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
#include "../include/vm.h"

uint16_t get_be_db(char c1, char c2);

VM* init_vm() {
    VM* vm = malloc(sizeof(VM));
    vm->pc = 0x200;
    vm->i = 0;
    vm->vf = 0;
    vm->dt = 0;
    vm->st = 0;
    vm->sp = 0;

    return vm;
}

bool load_program(VM* vm, char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) return false;

    uint16_t tmp = vm->pc;
    char b;

    // Bounds check, then load byte from file into memory[tmp]
    while ((tmp <= MEMORY_SIZE) && (((b = fgetc(f)) != EOF) && \
        (vm->memory[tmp++] = b)));

    fclose(f);
    return true;
}

void vm_run(VM* vm) {
    for (vm->pc; vm->pc <= MEMORY_SIZE; vm->pc += 2) {
        uint16_t instr = get_be_db(
            vm->memory[vm->pc], vm->memory[vm->pc + 1]
        );
        char nibble = (char)(instr >> 12);

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
                continue;
            case 2: // 0x2NNN - call func at 0xNNN  
                uint16_t ret_addr = vm->pc + 2;
                vm->stack[vm->sp] = ret_addr;
                vm->sp++;
                
                uint16_t func_addr = instr & 0x0FFF;
                vm->pc = func_addr - 2;
                continue;
            case 3: // 0x3XNN - Vx == NN condition
                size_t reg_idx = (instr & 0x0F00) >> 8;
                char      val  = instr & 0x00FF;
                if (vm->gpr[reg_idx] == val) {
                    vm->pc += 2; 
                }
                break;
            case 4: {// 0x4XNN - Vx != NN condition
                size_t reg_idx = (instr & 0x0F00) >> 8;
                char      val  = instr & 0x00FF;
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
                char    val   = instr & 0x00FF;
                vm->gpr[reg_idx] = val;
                break;
            }
            case 7: {// 0x7XNN: Vx += NN
                size_t reg_idx  = (instr & 0x0F00) >> 8;
                char    val   = instr & 0x00FF;
                vm->gpr[reg_idx] += val;
                break;
            }
            case 8: {
                char low_nib = instr & 0x000F;
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

                        vm->gpr[rdst_idx] >>= vm->gpr[rsrc_idx];
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

                        vm->gpr[rdst_idx] <<= vm->gpr[rsrc_idx];
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
                char        val = instr & 0x00FF;

                // gratefully clamping in [0; 255]
                vm->gpr[rdst_idx] = (rand() % 255) & val;
                break;
            }
            default:
                fprintf(stderr, "Illegal opcode 0x%x\n", instr);
                exit(2);
                break;
        }
    }
}

uint16_t get_be_db(char c1, char c2) {
    uint16_t res = (c1 << 8) | c2;
    return res;
}

