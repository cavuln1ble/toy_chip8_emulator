#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifndef CHIP8_H
#define CHIP8_H

typedef struct chip8 {
    uint8_t running;

    //The Chip-8 supports 36 2-bytes long Chip-8 opcodes
    uint16_t opcode;

    //Chip-8 can hold a whopping 4 kilobytes of memory
    uint8_t memory[4096];

    // General-purpose V registers, VF is instruction-only
    uint8_t V[16];

    //16-bit I register for memory addresses
    uint16_t I;

    //Program counter register for the currently executing address
    uint16_t PC;

    //Stack pointer
    uint8_t SP;
    uint16_t stack[16];

    //Delay timer 
    uint8_t DT;
    //Sound timer
    uint8_t ST;

    //monochrome display 64x32 resolution
    unsigned char gfx[64][32];

    //screen update register
    uint8_t draw;

    //keyboard
    unsigned char key[16];
}chip8;

chip8* chip8_init();
void chip8_destroy(chip8* vm);
void opcode(chip8* vm);

#endif