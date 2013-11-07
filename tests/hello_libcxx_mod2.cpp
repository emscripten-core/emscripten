#include <iostream>
#include <emscripten.h>

int main()
{
  std::cout << "hello, world!" << std::endl;
  emscripten_jcache_printf("waka %d waka\n", 5);
  return 0;
}

