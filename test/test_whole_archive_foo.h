#ifndef FOO_H
#define FOO_H

#include <string>
#include <iostream>

namespace bug {
void init(std::string name, int count);
int get_thing(std::string name);

class Foo {
 public:
  Foo(std::string name, int count) {
    init(name, count);
  }
  ~Foo() { std::cout<<"foo?"; }
};

} // namespace bug
#endif // FOO_H
