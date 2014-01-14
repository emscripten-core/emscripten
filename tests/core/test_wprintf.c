#include <wchar.h>

int main()
{
   wprintf (L"Characters: %lc %lc \n", L'a', 65);
   wprintf (L"Decimals: %d %ld\n", 1977, 650000L);
   wprintf (L"Preceding with blanks: %10d \n", 1977);
   wprintf (L"Preceding with zeros: %010d \n", 1977);
   wprintf (L"Some different radixes: %d %x %o %#x %#o \n", 100, 100, 100, 100, 100);
   wprintf (L"floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
   wprintf (L"Width trick: %*d \n", 5, 10);
   wprintf (L"%ls \n", L"A wide string");
   return 0;
}

