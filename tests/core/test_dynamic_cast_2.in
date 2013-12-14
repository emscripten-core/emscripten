#include <stdio.h>
#include <typeinfo>

class Class {};

int main() {
  const Class* dp = dynamic_cast<const Class*>(&typeid(Class));
  // should return dp == NULL,
  printf("pointer: %p\n", dp);
}
