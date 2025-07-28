#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"

static uint8_t chip8_fontset[80] =
{ 
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


chip8* chip8_init() {
    chip8* vm = calloc(1, sizeof(chip8));
    if (!vm) {
        fprintf(stderr, "Not enough memory available, exiting...\n");
        exit(1);
    }

    for (int i = 0; i<80; i++) {
        vm->memory[i + 0x200] = chip8_fontset[i];
    }

    vm->running = 1;
    vm->PC = 0x200;
    vm->opcode = 0;
    vm->I = 0;
    vm->draw = 0;
    vm->SP = 0;
    memset(vm->V, 0, sizeof(uint8_t));

    srand(time(NULL));

    return vm;
}

void chip8_destroy(chip8* vm) {
    free(vm);
    fflush(stdout);
}

void opcode(chip8* vm) {

    vm->opcode = vm->memory[vm->PC] << 8 | vm->memory[vm->PC + 1];
    vm->PC += 2;

    switch (vm->opcode & 0xF000) {

        case 0x0000:
            switch (vm->opcode & 0x000F) {

                /* 00E0 - CLS
                Clear the display.*/
                case 0x0000:
                    memset(vm->memory + 0x200, 0, sizeof(uint8_t));
                    break;

                /* 00EE - RET
                Return from a subroutine.

                The interpreter sets the program counter to the address at the top of the stack, 
                then subtracts 1 from the stack pointer.*/    
                case 0x000E:
                    vm->PC = vm->SP & 0x00FF;
                    vm->SP--;
                    break;

                default:
                    fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);
                    break;
            }
            break;
        
        /* 1nnn - JP addr
        Jump to location nnn.

        The interpreter sets the program counter to nnn.*/
        case 0x1000:
            vm->PC = vm->opcode & 0x0FFF;  
            break;
        
        /* 2nnn - CALL addr
        Call subroutine at nnn.

        The interpreter increments the stack pointer, then puts the current PC on the top of the stack. 
        The PC is then set to nnn.*/    
        case 0x2000:
            vm->SP++;
            vm->stack[vm->SP] = vm->PC;
            vm->PC = vm->opcode & 0x0FFF;
            break;
        
        /* 3xkk - SE Vx, byte
        Skip next instruction if Vx = kk.

        The interpreter compares register Vx to kk, and if they are equal, 
        increments the program counter by 2.*/    
        case 0x3000:
            uint8_t x = (vm->opcode & 0x0F00) >> 8, kk = vm->opcode & 0x00FF;
            if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else if (vm->V[x] == kk) {vm->PC += 2;}
            break;

        /* 4xkk - SNE Vx, byte
        Skip next instruction if Vx != kk.

        The interpreter compares register Vx to kk, and if they are not equal, 
        increments the program counter by 2.*/    
        case 0x4000:
            x = (vm->opcode & 0x0F00) >> 8, kk = vm->opcode & 0x00FF;
            if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else if (vm->V[x] != kk) {vm->PC += 2;}
            break;

        /* 5xy0 - SE Vx, Vy
        Skip next instruction if Vx = Vy.

        The interpreter compares register Vx to register Vy, and if they are equal, 
        increments the program counter by 2.*/    
        case 0x5000:
            x = (vm->opcode & 0x0F00) >> 8;
            uint8_t y = (vm->opcode & 0x00F0) >> 4;
            if (x == 0x0F || y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else if (vm->V[x] == vm->V[y]) {vm->PC += 2;}
            break;

        /* 6xkk - LD Vx, byte
        Set Vx = kk.

        The interpreter puts the value kk into register Vx.*/    
        case 0x6000:
            x = (vm->opcode & 0x0F00) >> 8, kk = vm->opcode & 0x00FF;
            if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else {vm->V[x] = kk;}
            break;

        /* 7xkk - ADD Vx, byte
        Set Vx = Vx + kk.

        Adds the value kk to the value of register Vx, then stores the result in Vx.*/
        case 0x7000:
            x = (vm->opcode & 0x0F00) >> 8, kk = vm->opcode & 0x00FF;
            if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else {vm->V[x] = (vm->V[x] + kk) & 0x00FF;}
            break;

        case 0x8000:
            switch (vm->opcode & 0x000F) {
                /* 8xy0 - LD Vx, Vy
                Set Vx = Vy.

                Stores the value of register Vy in register Vx.*/
                case 0x0000:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0E || y == 0x0E) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {vm->V[x] = vm->V[y];}
                    break;
        
                /* 8xy1 - OR Vx, Vy
                Set Vx = Vx OR Vy.

                Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
                A bitwise OR compares the corresponding bits from two values, and if either bit is 1, 
                then the same bit in the result is also 1. Otherwise, it is 0.*/
                case 0x0001:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0F || y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {vm->V[x] = vm->V[x] | vm->V[y];}
                    break;

                /* 8xy2 - AND Vx, Vy
                Set Vx = Vx AND Vy.

                Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx. 
                A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, 
                then the same bit in the result is also 1. Otherwise, it is 0.*/
                case 0x0002:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0F || y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {vm->V[x] = vm->V[x] & vm->V[y];}
                    break;
                
                /* 8xy3 - XOR Vx, Vy
                Set Vx = Vx XOR Vy.

                Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx. 
                An exclusive OR compares the corrseponding bits from two values, 
                and if the bits are not both the same, then the corresponding bit in the result is set to 1. 
                Otherwise, it is 0. */
                case 0x0003:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0F && y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {vm->V[x] = vm->V[x] ^ vm->V[y];}
                    break;

                /* 8xy4 - ADD Vx, Vy
                Set Vx = Vx + Vy, set VF = carry.

                The values of Vx and Vy are added together. 
                If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. 
                Only the lowest 8 bits of the result are kept, and stored in Vx.*/
                case 0x0004:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0F && y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {vm->V[x] = vm->V[x] + vm->V[y];}
                    break;  
                
                /* 8xy5 - SUB Vx, Vy
                Set Vx = Vx - Vy, set VF = NOT borrow.

                If Vx > Vy, then VF is set to 1, otherwise 0. 
                Then Vy is subtracted from Vx, and the results stored in Vx.*/
                case 0x0005:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0F && y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {
                        vm->V[0xF] = vm->V[x] > vm->V[y] ? 1: 0;
                        vm->V[x] = vm->V[x] - vm->V[y];
                    }
                    break;
                
                /* 8xy6 - SHR Vx {, Vy}
                Set Vx = Vx SHR 1.

                If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. 
                Then Vx is divided by 2.*/
                case 0x0006:
                    x = (vm->opcode & 0x0F00) >> 8;
                    if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {
                        vm->V[0xF] = vm->V[x] & 0x01;
                        vm->V[x] >>= 1;
                    }
                    break; 

                /* 8xy7 - SUBN Vx, Vy
                Set Vx = Vy - Vx, set VF = NOT borrow.

                If Vy > Vx, then VF is set to 1, otherwise 0. 
                Then Vx is subtracted from Vy, and the results stored in Vx.*/
                case 0x0007:
                    x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
                    if (x == 0x0F && y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {
                        vm->V[0xF] = vm->V[y] > vm->V[x] ? 1: 0;
                        vm->V[x] = vm->V[y] - vm->V[x];
                    }
                    break;
                
                /* 8xyE - SHL Vx {, Vy}
                Set Vx = Vx SHL 1.

                If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. 
                Then Vx is multiplied by 2.*/
                case 0x000E:
                    x = (vm->opcode & 0x0F00) >> 8;
                    if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
                    else {
                        vm->V[0xF] = vm->V[x] & 0x10;
                        vm->V[x] <<= 1;
                    }
                    break;

                default:
                    fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);
                    break;
            }
            break;
        
        /* 9xy0 - SNE Vx, Vy
        Skip next instruction if Vx != Vy.

        The values of Vx and Vy are compared, and if they are not equal, 
        the program counter is increased by 2.*/
        case 0x9000:
            x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
            if (x == 0x0F && y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else if (vm->V[x] != vm->V[y]) {vm->PC += 2;}
            break;

        /* Annn - LD I, addr
        Set I = nnn.

        The value of register I is set to nnn.*/
        case 0xA000:
            vm->I = vm->opcode & 0x0FFF;
            break;
        
        /* Bnnn - JP V0, addr
        Jump to location nnn + V0.

        The program counter is set to nnn plus the value of V0.*/
        case 0xB000:
            vm->PC = (vm->opcode & 0x0FFF) + vm->V[0];
            break;
        
        /* Cxkk - RND Vx, byte
        Set Vx = random byte AND kk.

        The interpreter generates a random number from 0 to 255, 
        which is then ANDed with the value kk. The results are stored in Vx. 
        See instruction 8xy2 for more information on AND.*/    
        case 0xC000:
            x = (vm->opcode & 0x0F00) >> 8, kk = vm->opcode & 0x00FF;
            if (x == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            else {vm->V[x] = (uint8_t) rand() & kk;}
            break;
        
        /* Dxyn - DRW Vx, Vy, nibble
        Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

        The interpreter reads n bytes from memory, starting at the address stored in I. 
        These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). 
        Sprites are XORed onto the existing screen. If this causes any pixels to be erased, 
        VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is
        outside the coordinates of the display, it wraps around to the opposite side of the screen. 
        See instruction 8xy3 for more information on XOR, and section 2.4, Display, 
        for more information on the Chip-8 screen and sprites.*/
        case 0xD000:
            x = (vm->opcode & 0x0F00) >> 8, y = (vm->opcode & 0x00F0) >> 4;
            uint8_t n = vm->opcode & 0x000F;
            if (x == 0x0F && y == 0x0F) {fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);}
            break;
        
        default:
            fprintf(stderr, "Invalid opcode: 0x%X\n", vm->opcode);
            break;
    }
}