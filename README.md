# [Chip-8](https://en.wikipedia.org/wiki/CHIP-8) emulator in C 
## Get && build
```bash
git clone --depth 1 https://github.com/Freemorger/fch8.git
```
Go to dir:
```bash 
cd fch8/ 
```

> To build fch8, you would need raylib installed   

This project now uses [MAD build system](https://github.com/Freemorger/mad).
Build using MAD:
```bash 
mad release # build optimized, recommended for normal usage
# mad debug # build unoptimized with debug symbols 
# mad clean # clean obj files if there so 
```

## Running
Run chip8 binary game from file:
```bash 
./fch8 game.bin 
```
## Notes
- Currently won't play sounds, but may work with games that are doing it.   
- Idk if I will continue this. I like VMs building but chip8 architecture 
is... nvm.   
- Tested on corax+ test suite.   

