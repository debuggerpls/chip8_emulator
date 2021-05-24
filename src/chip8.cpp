#include "chip8.h"
#include "font.h"
#include <cstring>
#include <chrono>

Chip8::Chip8() : _timer_thread(timer_fnc, this) {

}

Chip8::~Chip8() {
    shutdown = 1;
    _timer_thread.join();
}

void Chip8::init_font() {
    int index = 0x50;
    std::memcpy(&memory[index], font, 5 * 16);
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
