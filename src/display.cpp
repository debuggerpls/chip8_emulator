#include "display.h"

display::Display::Display(int w, int h) : width(w), height(h) {
    pixels.resize(w * h);
}

bool display::Display::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    // TODO: configurable pixel size
    screen.window = SDL_CreateWindow("CHIP8 interpreter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * 10, height * 10, SDL_WINDOW_SHOWN);

    if (!screen.window) {
        return false;
    }

    screen.renderer = SDL_CreateRenderer(screen.window, -1, SDL_RENDERER_ACCELERATED);
    if (!screen.renderer) {
        return false;
    }

    screen.texture = SDL_CreateTexture(screen.renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!screen.texture) {
        return false;
    }

    return true;
}

display::Display::~Display() {
    screen.clean_up();
    SDL_Quit();
}

void display::Display::draw() {
    SDL_UpdateTexture(screen.texture, nullptr, &pixels[0], width * sizeof(Pixel));
    SDL_RenderClear(screen.renderer);
    SDL_RenderCopy(screen.renderer, screen.texture, nullptr, nullptr);
    SDL_RenderPresent(screen.renderer);
}

void display::Display::clear() {
    std::memset(&pixels[0], 0, pixels.size() * sizeof(Pixel));
    draw();
}

void display::Screen::clean_up() {
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
}
