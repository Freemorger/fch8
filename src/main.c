#include <stdio.h>
#include <raylib.h>
#include "../include/vm.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: fch8 program\n");
        return 1;
    }

    VM* vm = init_vm();
    if (!load_program(vm, argv[1])) {
        printf("Failed to load file %s\n", argv[1]);
        return 1;
    }

    const int SCALE = 15; // 960x480
    const int TARGET_FPS = 60;
    
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(960, 480, "fch8 Chip-8 Emulator");
    // SetTargetFPS(TARGET_FPS);
    
    double accum = 0.0;
    while (!WindowShouldClose() && vm->running) {
        accum += GetFrameTime();

        vm_iter(vm);
        vm->pc += 2;

        if (vm->drawfl) {
            BeginDrawing();
                for (int y = 0; y < VRAM_HEIGHT; y++) {
                    for (int x = 0; x < VRAM_WIDTH; x++) {
                        Color col = vm->vram[y][x] ? WHITE : BLACK;

                        DrawRectangle(x * SCALE, y * SCALE, SCALE, SCALE, col);
                    }
                }   
            EndDrawing();
            WaitTime(1 / TARGET_FPS);
        }

        if (accum > 1.0) {
            vm->dt = vm->dt > 0 ? vm->dt - 1 : 0;
            accum = 0.0;
        }
    }
    CloseWindow();

    return 0;
}
