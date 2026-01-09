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

    InitWindow(640, 480, "fch8 Chip-8 Emulator");
    
    vm_run(vm);

    CloseWindow();

    return 0;
}
