#include "chip8.c"

int main() {
    FILE* f = fopen("1-chip8-logo.ch8", "rb");
    fpos_t fSize;
    fseek(f, 0, SEEK_END);
    fgetpos(f, &fSize);
    rewind(f);
    chip8* ch8 = chip8_init();
    fread(&ch8->memory[ch8->PC], 1, fSize, f);
    while (ch8) {
        opcode(ch8);
    }
    fclose(f);
    return 0;
}