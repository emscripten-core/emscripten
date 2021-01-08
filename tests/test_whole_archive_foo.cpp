#include "test_whole_archive_foo.h"
#include <string>
#include <map>

bug::Foo f("baz", 4);

namespace bug {
static std::map<std::string, int>* thing = nullptr;

int get_thing(std::string name) {
  return (*thing)[name];
}
void init(std::string name, int count) {
  if (thing == nullptr) thing = new std::map<std::string, int>;
  (*thing)[name] = count;
}
} // namespace bug
