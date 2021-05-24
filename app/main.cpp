#include <SDL.h>
#include <cstdio>
#include <cstring>

struct Screen {
    void clean_up() {
        if (texture)
            SDL_DestroyTexture(texture);
        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);
    }

    SDL_Window *window {nullptr};
    SDL_Renderer *renderer {nullptr};
    SDL_Texture *texture {nullptr};
};

struct DisplayPixel {
    void white() { r = g = b = 255; }
    void black() { r = g = b = 0; }

    uint8_t r {0};
    uint8_t g {0};
    uint8_t b {0};
};

int exit_sdl_error(const char* msg, Screen &screen);

int exit_sdl_error(const char* msg, Screen &screen) {
    printf("ERROR: %s - %s\n", msg, SDL_GetError());
    screen.clean_up();
    return EXIT_FAILURE;
}

int main(int argc, char **argv) {
    Screen screen;
    DisplayPixel pixels[64 * 32];
    std::memset(pixels, 255, 64 * 16 * sizeof(DisplayPixel));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return exit_sdl_error("SDL_Init()", screen);
    }

    screen.window = SDL_CreateWindow("CHIP8 interpreter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);

    if (!screen.window) {
        return exit_sdl_error("SDL_CreateWindow()", screen);
    }

    screen.renderer = SDL_CreateRenderer(screen.window, -1, SDL_RENDERER_ACCELERATED);
    if (!screen.renderer) {
        return exit_sdl_error("SDL_CreateRenderer()", screen);
    }

    screen.texture = SDL_CreateTexture(screen.renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 64, 32);
    if (!screen.texture) {
        return exit_sdl_error("SDL_CreateTexture()", screen);
    }

    SDL_UpdateTexture(screen.texture, nullptr, pixels, 64 * sizeof(DisplayPixel));

    SDL_RenderClear(screen.renderer);
    SDL_RenderCopy(screen.renderer, screen.texture, nullptr, nullptr);
    SDL_RenderPresent(screen.renderer);

    SDL_Delay(3000);

    screen.clean_up();
    SDL_Quit();

    return 0;
}

