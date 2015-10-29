#include <cstdlib>
#include <ostream>

struct Colors {
  static bool use;

  static void normal(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[0m";
#endif
  }
  static void red(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[31m";
#endif
  }
  static void magenta(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[35m";
#endif
  }
  static void orange(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[33m";
#endif
  }
  static void grey(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[37m";
#endif
  }
  static void green(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[32m";
#endif
  }
  static void blue(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[34m";
#endif
  }
  static void bold(std::ostream& stream) {
    if (!use) return;
#if defined(__linux__) || defined(__apple__)
    stream << "\033[1m";
#endif
  }

  Colors() {
    use = !getenv("COLORS") || getenv("COLORS")[0] != '0';
  }
} colors;

bool Colors::use;

