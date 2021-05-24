#ifndef CHIP8_EMULATOR_CHIP8_H
#define CHIP8_EMULATOR_CHIP8_H

#include "display.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>

constexpr int DISPLAY_WIDTH = 64;
constexpr int DISPLAY_HEIGHT = 32;

struct Stack {
    // NOTE: we trust the application to not overflow
    void push(uint16_t value);
    uint16_t pop();

    uint16_t stack[16]{0};
    int index{0};
};

struct Timer {
    void decr();
    uint8_t get();
    void set(uint8_t value);

private:
    uint8_t _value;
    std::mutex _mutex;
};

struct Instruction {
    // FIXME: is the order right or should it be reversed?
    Instruction(uint8_t b0, uint8_t b1) : value((b0 << 8) | b1) {}

    uint16_t N() const { return (value & 0x000F); }
    uint16_t NN() const { return (value & 0x00FF); }
    uint16_t NNN() const { return (value & 0x0FFF); }
    uint16_t X() const { return ((value >> 8) & 0x0F); }
    uint16_t Y() const { return ((value >> 4) & 0x0F); }
    uint16_t FN() const { return ((value >> 12) & 0x0F); }

    uint16_t value{0};
};

/*
 * TODO:
 * Configurable instruction speed
 */
struct Chip8 {
    Chip8();
    ~Chip8();

    bool init();
    void init_font();
    void fetch_decode_execute();
    Instruction fetch();

    uint8_t memory[4096]{0};
    display::Display display;
    Stack stack;
    Timer delay_timer;
    Timer sound_timer;

    uint16_t PC{0};
    uint16_t I{0};

    /* internal registers */
    uint8_t V0{0};
    uint8_t V1{0};
    uint8_t V2{0};
    uint8_t V3{0};
    uint8_t V4{0};
    uint8_t V5{0};
    uint8_t V6{0};
    uint8_t V7{0};
    uint8_t V8{0};
    uint8_t V9{0};
    uint8_t VA{0};
    uint8_t VB{0};
    uint8_t VC{0};
    uint8_t VD{0};
    uint8_t VE{0};
    uint8_t VF{0};

    std::atomic<int> shutdown{0};

private:
   std::thread _timer_thread;
    void decode_execute(Instruction instruction);
};

void timer_fnc(Chip8 *chip);

#endif//CHIP8_EMULATOR_CHIP8_H
