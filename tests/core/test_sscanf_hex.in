#include <stdio.h>
#include <string>
#include <cstdlib>

int main()
{
  unsigned int a, b;
  sscanf("0x12AB 12AB", "%x %x", &a, &b);
  printf("%d %d\n", a, b);

  std::string hexstr("0102037F00FF");
  const char * cstr = hexstr.c_str();
  int len = hexstr.length() / 2;
  char * tmp_data = new char[len];
  for(int i = 0; i < len; i++)
  {
      sscanf(cstr, "%2hhx", &tmp_data[i]);
      cstr += 2 * sizeof(char);
  }

  for (int j = 0; j < len; j++)
      printf("%i, ", tmp_data[j]);
  printf("\n");
  delete[] tmp_data;
}


