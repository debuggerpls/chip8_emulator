#include "chip8.h"
#include "curses.h"

namespace chip8 {

  void Chip8::print_state() {
    for (int i = 0; i < 16; ++i) {
      printf("V%X\t0x%X\n", i, v_[i]);
    }
    printf("I\t0x%X\n", i_);
    printf("PC\tOx%X\n", pc_);
    printf("SP\t0x%X\n", sp_);
    printf("STACK: ");
    for (auto i : stack_) {
      printf("0x%X ", i);
    }
    printf("\n");
    printf("DELAY_TIMER\t0x%X\n", delay_timer_);
    printf("SOUND_TIMER\t0x%X\n", sound_timer_);
  }

  void Chip8::decode_op(uint8_t op) {
    auto inc_pc = true;
    auto ls_nibble = static_cast<uint8_t>(op & 0xF);
    auto ms_nibble = static_cast<uint8_t>(op >> 12);
    auto ls_byte = static_cast<uint8_t>(op & 0xFF);
    uint8_t tmp = ((op >> 8) & 0xF);
    auto &vx = v_[tmp];
    tmp = ((op >> 4) & 0xF);
    auto &vy = v_[tmp];

    switch (ms_nibble) {
      case 0:
        if (ls_byte == 0xE0) {
          // CLS - clear display
          display.clear();
          display.refresh();
        } else if (ls_byte == 0xEE) {
          // RET - return from subroutine
          --sp_;
          pc_ = stack_[sp_];
        }
        break;
      case 1:
        // JP addr - jump to location at addr
        pc_ = (op & 0xFFF);
        inc_pc = false;
        break;
      case 2:
        // CALL addr - call addr
        stack_[sp_] = pc_;
        ++sp_;
        pc_ = (op & 0x0FFF);
        inc_pc = false;
        break;
      case 3:
        // SE Vx, byte
        if (vx == ls_byte) {
          ++pc_;
        }
        break;
      case 4:
        // SNE Vx, byte
        if (vx != ls_byte) {
          ++pc_;
        }
        break;
      case 5:
        // SE Vx, Vy
        if (vx == vy) {
          ++pc_;
        }
        break;
      case 6:
        // LD Vx, byte
        vx = ls_byte;
        break;
      case 7:
        // ADD Vx, byte
        vx += ls_byte;
        break;
      case 8:
        switch (ls_nibble) {
          case 0:
            // LD Vx, Vy
            vx = vy;
            break;
          case 1:
            // OR Vx, Vy
            vx = (vx | vy);
            break;
          case 2:
            // AND Vx, Vy
            vx = (vx & vy);
            break;
          case 3:
            // XOR Vx, Vy
            vx = (vx ^ vy);
            break;
          case 4:
            // ADD Vx, Vy - add vy to vx, set VF as carry
            tmp = vx;
            vx += vy;
            if (vx < tmp) {
              vf() = 1;
            }
            break;
          case 5:
            // SUB Vx, Vy - sub vy from vx, set VF as not borrow
            if (vx > vy) {
              vf() = 1;
            }
            vx -= vy;
            break;
          case 6:
            // SHR Vx {, Vy}
            if (vx & 0x1) {
              vf() = 1;
            }
            vx = vx >> 1;
            break;
          case 7:
            // SUBN Vx, Vy, set VF as not borrow
            if (vy > vx) {
              vf() = 1;
            }
            vx = vy - vx;
            break;
          case 0xE:
            // SHL Vx {, Vy}
            if (ms_nibble & 0x8) {
              vf() = 1;
            }
            vx = vx << 1;
            break;
          default:
            break;
        }
        break;
      case 9:
        // SNE Vx, Vy
        if (vx != vy) {
          ++pc_;
        }
        break;
      case 0xA:
        // LD I, addr
        i_ = (op & 0xFFF);
        break;
      case 0xB:
        // JP V0, addr
        pc_ = v_[0] + (op & 0xFFF);
        inc_pc = false;
        break;
      case 0xC:
        // RND Vx, byte - set vx to random byte and kk
        // TODO: random byte and not magic value
        vx = ls_byte;
        break;
      case 0xD:
        // DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        // TODO: this

        break;
      case 0xE:
        if (ls_byte == 0x9E) {
          // SKP Vx - Skip next instruction if key with the value of Vx is pressed
          // TODO: this
        } else if (ls_byte == 0xA1) {
          // SKNP Vx - Skip next instruction if key with the value of Vx is not pressed.
          // TODO: this
        }
        break;
      case 0xF:
        switch (ls_byte) {
          case 0x7:
            // LD Vx, DT
            vx = delay_timer_;
            break;
          case 0x0A:
            // LD Vx, K - Wait for a key press, store the value of the key in Vx
            // TODO: this
            break;
          case 0x15:
            // LD DT, Vx
            delay_timer_ = vx;
            break;
          case 0x18:
            // LD ST, Vx
            sound_timer_ = vx;
            break;
          case 0x1E:
            // ADD I, Vx
            i_ += vx;
            break;
          case 0x29:
            // LD F, Vx - Set I = location of sprite for digit Vx
            i_ = vx * 5;
            break;
          case 0x33:
            // Fx33 - LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2
            // TODO: this
            break;
          case 0x55:
            // LD [I], Vx - Store registers V0 through Vx in memory starting at location I
            tmp = ((op >> 8) & 0xF);
            for (int i = 0; i <= tmp; ++i) {
              ram_[i_ + i] = v_[i];
            }
            break;
          case 0x65:
            // LD Vx, [I] - Read registers V0 through Vx from memory starting at location I
            tmp = ((op >> 8) & 0xF);
            for (int i = 0; i <= tmp; ++i) {
              v_[i] = ram_[i_ + i];
            }
            break;
          default:
            printf("Unknown op code: 0x%X\n", op);
            break;
        }
        break;
      default:
        printf("Unknown op code: 0x%X\n", op);
        break;
    }

    if (inc_pc) {
      ++pc_;
    }

  }

  void Chip8::init() {
    int i = 0;
    // 0
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0x90;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    // 1
    ram_[i++] = 0x20;
    ram_[i++] = 0x60;
    ram_[i++] = 0x20;
    ram_[i++] = 0x20;
    ram_[i++] = 0x70;
    // 2
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    // 3
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0xF0;
    // 4
    ram_[i++] = 0x90;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0x10;
    // 5
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0xF0;
    // 6
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    // 7
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0x20;
    ram_[i++] = 0x40;
    ram_[i++] = 0x40;
    // 8
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    // 9
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x10;
    ram_[i++] = 0xF0;
    // A
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x90;
    ram_[i++] = 0x90;
    // B
    ram_[i++] = 0xE0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xE0;
    ram_[i++] = 0x90;
    ram_[i++] = 0xE0;
    // C
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0x80;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    // D
    ram_[i++] = 0xE0;
    ram_[i++] = 0x90;
    ram_[i++] = 0x90;
    ram_[i++] = 0x90;
    ram_[i++] = 0xE0;
    // E
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    // F
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0xF0;
    ram_[i++] = 0x80;
    ram_[i++] = 0x80;
  }

  Chip8::Chip8() {
    init();
  }

  Display::Display() {
    initscr();
    wresize(stdscr, screen_lines, screen_cols);
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
  }

  Display::~Display() {
    endwin();
  }

  void Display::refresh() {
    move(0, 0);
    for (auto i: screen_) {
      addch(i ? filled_pixel : empty_pixel);
    }
    ::refresh();
  }

  void Display::clear() {
    std::fill(screen_.begin(), screen_.end(), empty_pixel);
    // could use ::clear(), but we update screen from buffer either way
  }

  bool Display::drawSprite(int line, int col, int num_bytes, uint8_t *addr) {
    auto collision = false;

    return collision;
  }

}