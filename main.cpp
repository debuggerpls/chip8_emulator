#include "chip8.h"
#include <csignal>
#include <cstdio>

auto quit = false;

void signal_handler(int /*unused*/) {
  quit = true;
}

int main() {
  using namespace chip8;
  Chip8 chip8;
  //chip8.print_state();

  signal(SIGINT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGKILL, signal_handler);


  while (!quit) {

  }

  return 0;
}
