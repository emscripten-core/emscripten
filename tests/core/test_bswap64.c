#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <sstream>

typedef unsigned long long quint64;

using namespace std;

inline quint64 qbswap(quint64 source) {
  return 0 | ((source & quint64(0x00000000000000ffLL)) << 56) |
         ((source & quint64(0x000000000000ff00LL)) << 40) |
         ((source & quint64(0x0000000000ff0000LL)) << 24) |
         ((source & quint64(0x00000000ff000000LL)) << 8) |
         ((source & quint64(0x000000ff00000000LL)) >> 8) |
         ((source & quint64(0x0000ff0000000000LL)) >> 24) |
         ((source & quint64(0x00ff000000000000LL)) >> 40) |
         ((source & quint64(0xff00000000000000LL)) >> 56);
}

int main() {
  quint64 v = strtoull("4433ffeeddccbb00", NULL, 16);
  printf("%lld\n", v);

  const string string64bitInt = "4433ffeeddccbb00";
  stringstream s(string64bitInt);
  quint64 int64bitInt = 0;
  printf("1\n");
  s >> hex >> int64bitInt;
  printf("2\n");

  stringstream out;
  out << hex << qbswap(int64bitInt);

  cout << out.str() << endl;
  cout << hex << int64bitInt << endl;
  cout << string64bitInt << endl;

  if (out.str() != "bbccddeeff3344") {
    cout << "Failed!" << endl;
  } else {
    cout << "Succeeded!" << endl;
  }

  return 0;
}
