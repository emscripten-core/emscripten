#include <mimalloc.h>

int main() {
  mi_option_enable(mi_option_verbose);
  mi_options_print();
  return 0;
}
