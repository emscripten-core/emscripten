#include <ostream>

struct Colors {
  static void normal(std::ostream& stream) {
#if __linux__
    stream << "\033[0m";
#endif
  }
  static void red(std::ostream& stream) {
#if __linux__
    stream << "\033[31m";
#endif
  }
  static void orange(std::ostream& stream) {
#if __linux__
    stream << "\033[33m";
#endif
  }
  static void grey(std::ostream& stream) {
#if __linux__
    stream << "\033[37m";
#endif
  }
};

