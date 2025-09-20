#include "chip8.h"
#include ".\include\raylib.h"

#define SCALE 10

int main() {
    FILE* f = fopen("1-chip8-logo.ch8", "rb");
    fpos_t fSize;
    fseek(f, 0, SEEK_END);
    fgetpos(f, &fSize);
    rewind(f);
    chip8* ch8 = chip8_init();
    fread(&ch8->memory[ch8->PC], 1, fSize, f);

    InitWindow(WIDTH*SCALE, HEIGHT*SCALE, "raylib [core] example - basic window");
    while (!WindowShouldClose())
    {
        opcode(ch8);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (int posX = 0; posX<WIDTH; posX++)
            {
                for (int posY = 0; posY<HEIGHT; posY++) 
                {
                    if (ch8->draw && ch8->gfx[posY][posX]) {
                        DrawRectangle(posX*SCALE, posY*SCALE, SCALE, SCALE, PINK);
                    }   
                }
            }
        EndDrawing();
    }

    CloseWindow();
    fclose(f);
    return 0;
}