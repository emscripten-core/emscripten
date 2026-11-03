/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <thread>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <unistd.h>
#endif

using namespace std;

void main_loop() {
  static bool first_line = true;
  string str;

  errno = 0;
  while (errno != EAGAIN) {
    if (first_line) {
      char c;
      if (std::cin.get(c)) {
        std::cout.put(c);
        if (c == '\n') {
          first_line = false;
        }
      }
    } else {
      if (getline(std::cin, str)) {
        std::cout << str << "\n";
      }
    }

    if (std::cin.fail() && !std::cin.eof()) {
      if (errno != EAGAIN) {
        cerr << "error " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
      }
      std::cin.clear();
    }

    if (std::cin.eof()) {
      std::cout << "eof\n";
      exit(EXIT_SUCCESS);
    }
  }
}

int main(int argc, char const *argv[]) {
  // SM shell doesn't implement an event loop and therefore doesn't support
  // emscripten_set_main_loop. However, its stdin reads are sync so it
  // should exit out after calling main_loop once.
  main_loop();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 60, 0);
#else
  while (1) main_loop(); sleep(1);
#endif
  return 0;
}
