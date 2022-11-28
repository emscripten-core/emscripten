#include <emscripten.h>
#include <stdio.h>

int test_side() {
  EM_ASM(out("no args works"));

  EM_ASM({
    console.log("int types:");
    out("         char : " + $0);
    out("  signed char : " + $1);
    out("unsigned char : " + $2);
    out("         short: " + $3);
    out("  signed short: " + $4);
    out("unsigned short: " + $5);
    out("         int  : " + $6);
    out("  signed int  : " + $7);
    out("unsigned int  : " + $8);
    out("         long : " + $9);
    out("  signed long : " + $10);
    out("unsigned long : " + $11);
    out("    terminator: " + $12);
  }, (char)1, (signed char)2, (unsigned char)3,
     (short)4, (signed short)5, (unsigned short)6,
     (int)7, (signed int)8, (unsigned int)9,
     (long)10, (signed long)11, (unsigned long)12, 42);

  return 0;
}
