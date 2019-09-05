// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>

void PrintWide ( const wchar_t * format, ... )
{
  int max_chars = 256;
  wchar_t buffer[max_chars];
  memset(buffer, 0, max_chars);
  va_list args;
  va_start ( args, format );
  wprintf(L"format    starts with 0x%x\n", *(int*)format);
  wprintf(L"fmt    continues with 0x%x\n", *(((int*)format) + 1));
  wprintf(L"fmt    continues with 0x%x\n", *(((int*)format) + 2));
  int r = vswprintf ( buffer, max_chars-1, format, args );
  wprintf(L"vswprintf told us %d\n", r);
  wprintf(L"vswoutput st-rts with 0x%x\n", *(int*)buffer);
  wprintf(L"vsw    continues with 0x%x\n", *(((int*)buffer) + 1));
  wprintf(L"vsw    continues with 0x%x\n", *(((int*)buffer) + 2));
  wprintf(buffer);
  va_end ( args );
}

void PrintBigWide ( const wchar_t * format, ... )
{
  int max_chars = 8096;
	wchar_t buffer[max_chars] = { 0 };
  va_list args;
  va_start ( args, format );
  int ret = vswprintf ( buffer, max_chars-1, format, args );
  if (ret >= max_chars) {
		ret = max_chars - 1;
	}
	if (ret > 0) {
		wprintf(L"PrintBigWide wrote %d wchars:\n", ret);
	}
  wprintf(buffer);
  va_end ( args );
}

int main ()
{
   FILE *f = fopen("test.dat", "wb");
   int num = fwprintf(f, L"hello %d", 5);
   wprintf(L"fwprintf told us %d\n", num);
   fclose(f);
   f = fopen("test.dat", "rb");
   fseek(f, 0, SEEK_END);
   int size = ftell(f);
   fclose(f);
   wprintf(L"file size is %d\n", size);

   wchar_t str[] = L"test string has %d wide characters.\n";
   wprintf(L"str starts with 0x%x\n", *(int*)str);
   wprintf(L"str continues with 0x%x\n", *(((int*)str) + 1));
   wprintf(L"str continues with 0x%x\n", *(((int*)str) + 2));
   PrintWide ( str, wcslen(str) );
   PrintWide ( str, wcslen(str) );
   PrintWide ( str, wcslen(str) );
  
  
   wchar_t long_str[] = L"test string has %d wide characters.\n"
     "Internally the variadic print functions use a 256 char buffer, so this is a string that's longer than 256 chars, "
     "so in case this breaks we have a test case. As discovered in #9305 vswprintf had been broken for some time, "
     "but was never picked up as the test strings were all shorter then 256 chars. So hopefully this long rambly string "
     "will help guard against that bug being re-introduced.\n";
   PrintWide ( str, wcslen(str) );
  
   wprintf (L"Characters: %lc %lc \n", L'a', 65);
   wprintf (L"Decimals: %d %ld\n", 1977, 650000L);
   wprintf (L"Preceding with blanks: %10d \n", 1977);
   wprintf (L"Preceding with zeros: %010d \n", 1977);
   wprintf (L"Some different radixes: %d %x %o %#x %#o \n", 100, 100, 100, 100, 100);
   wprintf (L"floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
   wprintf (L"Width trick: %*d \n", 5, 10);
   wprintf (L"%ls \n", L"A wide string");

   wchar_t buffer [100];
   memset(buffer, 0, sizeof(buffer));
   int cx;
   cx = swprintf(buffer, 100, L"The half of %d is %d", 80, 80/2);
   wprintf(L"swprintf told us %d\n", cx);
   for (int i = 0; i < 10; i++) wprintf(L"pre %d\n", ((int*)buffer)[i]);
   swprintf (buffer+cx, 100-cx-1, L", and the half of that is %d.\n", 80/2/2);
   for (int i = 0; i < 10; i++) wprintf(L"post %d\n", ((int*)buffer)[i]);
   wprintf(buffer);

   return 0;
}

