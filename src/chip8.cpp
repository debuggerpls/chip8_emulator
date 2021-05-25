#include "chip8.h"
#include "font.h"
#include <cstring>
#include <chrono>
#include <cstdlib>
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
        case 2: op_2NNN(instruction); break;
        case 3: op_3XNN(instruction); break;
        case 4: op_4XNN(instruction); break;
        case 5: op_5XY0(instruction); break;
        case 6: op_6XNN(instruction); break;
        case 7: op_7XNN(instruction); break;
        case 8: op_8XYR(instruction); break;
        case 9: op_9XY0(instruction); break;
        case 0xA: op_ANNN(instruction); break;
        case 0xB: op_BNNN(instruction); break;
        case 0xC: op_CXNN(instruction); break;
        case 0xD: op_DXYN(instruction); break;
        case 0xE: op_EXRR(instruction); break;
        case 0xF: op_FXRR(instruction); break;
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

void Chip8::op_2NNN(Instruction instruction) {
    /* Call subroutine */
    stack.push(PC);
    PC = instruction.NNN();
}

void Chip8::op_3XNN(Instruction instruction) {
    /* Skip if equal */
    if (V[instruction.X()] == instruction.NN()) {
        PC += 2;
    }
}

void Chip8::op_4XNN(Instruction instruction) {
    /* Skip if not equal */
    if (V[instruction.X()] != instruction.NN()) {
        PC += 2;
    }
}

void Chip8::op_5XY0(Instruction instruction) {
    /* Skip if VX == VY */
    if (V[instruction.X()] == V[instruction.Y()]) {
        PC += 2;
    }
}

void Chip8::op_9XY0(Instruction instruction) {
    /* Skip if VX != VY */
    if (V[instruction.X()] != V[instruction.Y()]) {
        PC += 2;
    }
}

// FIXME: configurable for 8XY6 & 8XYE operations ( not its original implementation )
void Chip8::op_8XYR(Instruction instruction) {
    /* Arithmetic instructions */
    uint16_t tmp = 0;
    switch (instruction.N()) {
        case 0: V[instruction.X()] = V[instruction.Y()]; break;
        case 1: V[instruction.X()] |= V[instruction.Y()]; break;
        case 2: V[instruction.X()] &= V[instruction.Y()]; break;
        case 3: V[instruction.X()] ^= V[instruction.Y()]; break;
        case 4:
            tmp = V[instruction.X()];
            V[instruction.X()] += V[instruction.Y()];
            V[0xF] = tmp > V[instruction.X()] ? 1 : 0;
            break;
        case 5:
            V[0xF] = V[instruction.X()] > V[instruction.Y()] ? 1 : 0;
            V[instruction.X()] -= V[instruction.Y()];
            break;
        case 7:
            V[0xF] = V[instruction.Y()] > V[instruction.X()] ? 1 : 0;
            V[instruction.X()] = V[instruction.Y()] - V[instruction.X()];
            break;
        case 6:
            V[instruction.X()] = V[instruction.Y()];
            V[0xF] = V[instruction.X()] & 1;
            V[instruction.X()] = V[instruction.X()] >> 1;
            break;
        case 0xE:
            V[instruction.X()] = V[instruction.Y()];
            V[0xF] = V[instruction.X()] & (1 << 15) ? 1 : 0;
            V[instruction.X()] = V[instruction.X()] << 1;
            break;
        default: printf("Unknown instruction: 0x%X\n", instruction.value); break;
    }
}

// FIXME: configurable - original or Chip-48/SUPER-CHIP
void Chip8::op_BNNN(Instruction instruction) {
    /* Jump with offset */
    PC = instruction.NNN() + V[0];
}

void Chip8::op_CXNN(Instruction instruction) {
    /* Random */
    V[instruction.X()] = rand() & instruction.NN();
}

void Chip8::op_EXRR(Instruction instruction) {
    /* Skip if key */
    switch (instruction.NN()) {
        case 0x9E:
            // if key in VX(0-F) is pressed, inc PC by 2
            break;
        case 0xA1:
            // if key in VX(0-F) is not pressed, inc PC by 2
            break;
        default: printf("Unknown instruction: 0x%X\n", instruction.value); break;
    }
}

// FIXME: add configurable FX55 & FX65 ( now original implemented )
void Chip8::op_FXRR(Instruction instruction) {
    uint16_t temp = 0;
    switch (instruction.NN()) {
        case 0x07: // Set VX to the current value of delay timer
            V[instruction.X()] = delay_timer.get();
            break;
        case 0x15: // Set the delay timer to the value in VX
            delay_timer.set(V[instruction.X()]);
            break;
        case 0x18: // Set the sound timer to the value in VX
            sound_timer.set(V[instruction.X()]);
            break;
        case 0x1E: // Add to index
            I += V[instruction.X()];
            V[0xF] = I >= 0x1000 ? 1 : 0; // like Amiga interpreter
            break;
        case 0x0A: // Get key (blocking)
            // get which key was pressed and released and save it in VX (BLOCK)
            break;
        case 0x29: // Font character
            I = 0x50 + (instruction.X() & 0xf) * 5;
            break;
        case 0x33: // Binary-coded decimal conversion
            memory[I] = V[instruction.X()] / 100;
            memory[I + 1] = (V[instruction.X()] / 10) % 10;
            memory[I + 2] = V[instruction.X()] % 10;
            break;
        case 0x55: // Store registers to memory
            temp = V[instruction.X()];
            for (int i = 0; i <= temp && i < 16; ++i) {
//                memory[I + i] = V[i];
                memory[I] = V[i];
                ++I;
            }
            break;
        case 0x65: // Load registers from memory
            temp = V[instruction.X()];
            for (int i = 0; i <= temp && i < 16; ++i) {
//                V[i] = memory[I + i];
                V[i] = memory[I];
                ++I;
            }
            break;
        default: printf("Unknown instruction: 0x%X\n", instruction.value); break;
    }
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
