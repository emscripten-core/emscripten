#include <iostream>
#include <emscripten.h>

int main()
{
  std::cout << "hello, world!" << std::endl;
  emscripten_jcache_printf("waka %d waka\n", 5);
  emscripten_jcache_printf("yet another printf %.2f %d\n", 5.5, 66);
  return 0;
}

