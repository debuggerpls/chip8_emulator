#ifndef CHIP8_EMULATOR_CHIP8_H
#define CHIP8_EMULATOR_CHIP8_H

#include <array>
#include <cstdio>
#include <cstdint>

namespace chip8 {

  constexpr int screen_cols = 64;
  constexpr int screen_lines = 32;
  constexpr char filled_pixel = '#';
  constexpr char empty_pixel = ' ';


  class Display {
  public:
    Display();

    ~Display();

    void drawPixel(int line, int col);

    void deletePixel(int line, int col);

    bool drawSprite(int line, int col, int num_bytes, uint8_t *addr);

    void refresh();

    void clear();

  private:
    std::array<uint8_t, screen_lines * screen_cols> screen_{};

  };

  class Chip8 {
  public:
    Chip8();

    ~Chip8() = default;

    void init();

    void print_state();

    void decode_op(uint8_t op);

    Display display;

  protected:
    uint8_t &vf() { return v_[v_.size() - 1]; }

    std::array<uint8_t, 4096> ram_{};
    std::array<uint8_t, 16> v_{};
    std::array<uint16_t, 16> stack_{};
    uint16_t pc_{};
    uint16_t i_{};
    uint8_t sp_{};
    uint8_t delay_timer_{};
    uint8_t sound_timer_{};
  };

} // namespace

#endif //CHIP8_EMULATOR_CHIP8_H
