#include "chip8.h"
#include <csignal>
#include <cstdio>

auto quit = false;

void signal_handler(int /*unused*/) {

  quit = true;
}

int main() {
  Chip8 chip8;
  //chip8.print_state();

  signal(SIGINT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGKILL, signal_handler);

  chip8.display.drawPixel(2, 2);
  chip8.display.drawPixel(0, 2);
  chip8.display.refresh();
  chip8.display.deletePixel(0, 2);
  chip8.display.refresh();

  while (!quit) {

  }

  return 0;
}
