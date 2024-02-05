#include <vector>

int main() {
  std::vector<int> v{0, 1};
  v.reserve(4);
  int *p = &v[0];
  return p[3];
}
