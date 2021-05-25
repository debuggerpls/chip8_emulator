#include "chip8.h"

#include <cstdio>


int main(int argc, char **argv) {
//    if (argc != 2) {
//        printf("Usage:\n");
//        printf("  %s <rom>\n", argv[0]);
//        return 1;
//    }

    Chip8 chip;
//    std::string program(argv[1]);
//    std::string program("../roms/IBM_Logo.ch8");
//    std::string program("../roms/BC_test.ch8");
    std::string program("../roms/test_opcode.ch8");

    if (!chip.load_program(program)) {
        printf("Failed to load program: %s\n", program.c_str());
        return 1;
    }

    if (!chip.init()) {
        printf("Failed to initialize CHIP8\n");
        return 1;
    }

    while (!chip.shutdown) {
        chip.fetch_decode_execute();
        SDL_Delay(2); // 700 instructions should be 1,42 ms
    }

    SDL_Delay(5000);


    return 0;
}

