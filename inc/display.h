#ifndef CHIP8_EMULATOR_DISPLAY_H
#define CHIP8_EMULATOR_DISPLAY_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <SDL.h>

namespace display {
    struct Pixel {
        bool is_on() const { return r; }
        void set_on() { r = g = b = 255; }
        void set_off() { r = g = b = 0; }
        void toggle() { if (r) r = g = b = 0; else r = g = b = 255; }

        uint8_t r {0};
        uint8_t g {0};
        uint8_t b {0};
    };

    struct Screen {
        void clean_up();

        SDL_Window *window {nullptr};
        SDL_Renderer *renderer {nullptr};
        SDL_Texture *texture {nullptr};
    };

    struct Display {
        explicit Display(int w, int h);
        ~Display();

        bool init();
        void draw();
        void clear();

        std::vector<Pixel> pixels{};
        Screen screen;
        int width {0};
        int height {0};
    };

}

#endif//CHIP8_EMULATOR_DISPLAY_H
