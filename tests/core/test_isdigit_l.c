#include <iostream>
int main() {
  using namespace std;
  use_facet<num_put<char> >(cout.getloc()).put(cout, cout, '0', 3.14159265);
}
