/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
   FILE *fp;
   int res;
   long len;

   fp = fopen("testappend", "wb+");
   res = fwrite("1234567890", 10, 1, fp);
   fclose(fp);

   fp = fopen("testappend", "ab+");
   res = fwrite("1234567890", 10, 1, fp);

   fseek(fp, -7, SEEK_END);
   len = ftell(fp);
   assert(len == 13);
   fclose(fp);

   puts("success");
   return 0;
}
