#include "chip8.h"
#include "font.h"
#include <cstring>
#include <chrono>
#include <fstream>

// TODO: configurable display size
Chip8::Chip8() : display(DISPLAY_WIDTH, DISPLAY_HEIGHT), _timer_thread(timer_fnc, this)  {

}

Chip8::~Chip8() {
    shutdown = 1;
    _timer_thread.join();
}

void Chip8::init_font() {
    int index = 0x50;
    std::memcpy(&memory[index], font, 5 * 16);
}

void Chip8::fetch_decode_execute() {
    if (PC >= 4096) {
        shutdown = 1;
        return;
    }
    // fetch() increments PC by 2
    auto instruction = fetch();
    decode_execute(instruction);
}

Instruction Chip8::fetch() {
    Instruction instruction {memory[PC], memory[PC + 1]};
    PC += 2;
    return instruction;
}

void Chip8::decode_execute(Instruction instruction) {
    switch (instruction.FN()) {
        case 0: op_0RRR(instruction); break;
        case 1: op_1NNN(instruction); break;
        case 6: op_6XNN(instruction); break;
        case 7: op_7XNN(instruction); break;
        case 0xA: op_ANNN(instruction); break;
        case 0xD: op_DXYN(instruction); break;
        default: printf("Unknown instruction: 0x%X\n", instruction.value); break;
    }
}

bool Chip8::init() {
    if (!display.init())
        return false;

    init_font();

    PC = 0x200;

    return true;
}

void Chip8::op_0RRR(Instruction instruction) {
    // NOTE: R - means any value (N is taken)
    // NOTE: 0NNN is not supported
    if (instruction.value == 0x00E0) {
        /* Clear screen */
        display.clear();
    } else if (instruction.value == 0x00EE) {
        /* Return from subroutine */
        PC = stack.pop();
    }
}

void Chip8::op_1NNN(Instruction instruction) {
    /* Jump */
    PC = instruction.NNN();
}

void Chip8::op_6XNN(Instruction instruction) {
    /* Set */
    V[instruction.X()] = instruction.NN();
}

void Chip8::op_7XNN(Instruction instruction) {
    /* Add */
    V[instruction.X()] += instruction.NN();
}

void Chip8::op_ANNN(Instruction instruction) {
    /* Set index */
    I = instruction.NNN();
}

void Chip8::op_DXYN(Instruction instruction) {
    /* Display */
    auto x = V[instruction.X()] % display.width;
    auto y = V[instruction.Y()] % display.height;
    V[0xF] = 0;

    for (int row = 0; row < instruction.N() && y < display.height; ++row, ++y) {
        auto byte = memory[I + row];
        for (int bit = 7, cur_x = x; bit >= 0 && cur_x < display.width; --bit, ++cur_x) {
            bool pixel_on = (byte & (1 << bit));
            auto pixel_index = cur_x + y * display.width;
            auto &pixel = display.pixels[pixel_index];
            if (pixel_on && pixel.is_on()) {
                pixel.set_off();
                V[0xF] = 1;
            } else if (pixel_on && !pixel.is_on()) {
                pixel.set_on();
            }
        }
    }

    // TODO: should we draw here or in seperate thread at 60Hz ??
    display.draw();
}
bool Chip8::load_program(const std::string &path) {
    std::ifstream inputFile(path, std::ios_base::binary);
    if (!inputFile.is_open()) {
        return false;
    }

    int program_index = 0x200;

    char c = inputFile.get();
    while (inputFile.good()) {
        memory[program_index++] = c;
        c = inputFile.get();
    }

    inputFile.close();

    return true;
}

void Stack::push(uint16_t value) {
    stack[index++] = value;
}

uint16_t Stack::pop() {
    return stack[--index];
}

void Timer::decr() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_value > 0) {
        _value--;
    }
}

uint8_t Timer::get() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _value;
}

void Timer::set(uint8_t value) {
    std::lock_guard<std::mutex> lock(_mutex);
    _value = value;
}

void timer_fnc(Chip8 *chip) {
    using namespace std::chrono_literals;

    while (!chip->shutdown) {
        // TODO: do we need higher resolution than 16ms?
        std::this_thread::sleep_for(16ms);
        chip->delay_timer.decr();
        chip->sound_timer.decr();
    }
}
