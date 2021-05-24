#include "chip8.h"

#include <cstdio>


int main(int argc, char **argv) {
    Chip8 chip;

    if (!chip.init()) {
        printf("Failed to initialize CHIP8\n");
        return 1;
    }

    chip.display.draw();
    SDL_Delay(3000);
    chip.display.pixels[16].on();
    chip.display.draw();
    SDL_Delay(3000);


    return 0;
}

