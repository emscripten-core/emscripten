// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main()
{
  printf("EM_ASM: Simple expression without trailing semicolon\n");
  EM_ASM(out('1. expression without trailing semicolon'));
  EM_ASM("out('2. expression without trailing semicolon')");
  EM_ASM({"out('3. expression without trailing semicolon')"});
  EM_ASM({out('4. expression without trailing semicolon')});
  EM_ASM("{out('5. expression without trailing semicolon')}");

  printf("\nEM_ASM: Double quotes\n");
  EM_ASM(out("1. string in double quotes"));
  EM_ASM("out(\"2. string in double quotes\")");
  EM_ASM({"out(\"3. string in double quotes\")"});
  EM_ASM({out("4. string in double quotes")});
  EM_ASM("{out(\"5. string in double quotes\")}");

  printf("\nEM_ASM: Double quotes inside a string\n");
  EM_ASM(out('1. this is \"double\" \"quotes\"'));
  EM_ASM(out('2. this is "double" "quotes" without escaping'));
  EM_ASM("out('3. this is \"double\" \"quotes\"')");
  EM_ASM({"out('4. this is \"double\" \"quotes\"')"});
  EM_ASM({out('5. this is \"double\" \"quotes\"')});
  EM_ASM({out('6. this is "double" "quotes" without esacping')});
  EM_ASM("{out('7. this is \"double\" \"quotes\"')}");

  printf("\nEM_ASM: Pass a string\n");
  EM_ASM(out('1. hello ' + UTF8ToString($0)), "world!");
  EM_ASM("out('2. hello ' + UTF8ToString($0))", "world!");
  EM_ASM({"out('3. hello ' + UTF8ToString($0))"}, "world!");
  EM_ASM({out('4. hello ' + UTF8ToString($0))}, "world!");
  EM_ASM("{out('5. hello ' + UTF8ToString($0))}", "world!");

  printf("\nEM_ASM: Simple expression without trailing semicolon, wrap code block in extra parentheses\n");
  EM_ASM((out('1. expression without trailing semicolon, in parentheses')));
  EM_ASM(("out('2. expression without trailing semicolon, in parentheses')"));
  EM_ASM(({"out('3. expression without trailing semicolon, in parentheses')"}));
  EM_ASM(({out('4. expression without trailing semicolon, in parentheses')}));
  EM_ASM(("{out('5. expression without trailing semicolon, in parentheses')}"));

  printf("\nEM_ASM: Two statements, separated with a semicolon\n");
  EM_ASM(out('1. two'); out('1. statements'););
  EM_ASM("out('2. two'); out('2. statements 2');");
  EM_ASM({"out('3. two'); out('3. statements 3');"});
  EM_ASM({out('4. two'); out('4. statements 4');});
  EM_ASM("{out('5. two'); out('5. statements 5');}");

  printf("\nEM_ASM: Pass an integer\n");
  EM_ASM(out('1. int ' + $0), 42);
  EM_ASM("out('2. int ' + $0)", 43);
  EM_ASM({"out('3. int ' + $0)"}, 44);
  EM_ASM({out('4. int ' + $0)}, 45);
  EM_ASM("{out('5. int ' + $0)}", 46);

  printf("\nEM_ASM: Evaluate an anonymous function\n");
  EM_ASM((function() {out('1. evaluating anonymous function ' + $0)})(), 42);
  EM_ASM("(function() {out('2. evaluating anonymous function ' + $0)})()", 42);
  EM_ASM({"(function() {out('3. evaluating anonymous function ' + $0)})()"}, 42);
  EM_ASM({(function() {out('4. evaluating anonymous function ' + $0)})()}, 42);
  EM_ASM("{(function() {out('5. evaluating anonymous function ' + $0)})()}", 42);

  printf("\nEM_ASM: Pass an integer and a double\n");
  EM_ASM(out('1. int and double ' + $0 + ' ' + $1), 42, 43.5);
  EM_ASM("out('2. int and double ' + $0 + ' ' + $1)", 42, 43.5);
  EM_ASM({"out('3. int and double ' + $0 + ' ' + $1)"}, 42, 43.5);
  EM_ASM({out('4. int and double ' + $0 + ' ' + $1)}, 42, 43.5);
  EM_ASM("{out('5. int and double ' + $0 + ' ' + $1)}", 42, 43.5);

  int i;

  printf("\nEM_ASM_INT: Pass an integer, return an integer back\n");
  i = EM_ASM_INT(out('1. got int ' + $0); return 3.5;, 42); printf("1. returned int %d\n", i);
  i = EM_ASM_INT("out('2. got int ' + $0); return 4.5;", 42); printf("2. returned int %d\n", i);
  i = EM_ASM_INT({"out('3. got int ' + $0); return 5.5;"}, 42); printf("3. returned int %d\n", i);
  i = EM_ASM_INT({out('4. got int ' + $0); return 6.5;}, 42); printf("4. returned int %d\n", i);
  i = EM_ASM_INT("{out('5. got int ' + $0); return 7.5;}", 42); printf("5. returned int %d\n", i);

  printf("\nEM_ASM_INT: Pass an integer, return an integer back, wrap code block in extra parentheses\n");
  i = EM_ASM_INT((out('1. got int, extra parentheses ' + $0); return 3.5;), 42); printf("1. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(("out('2. got int, extra parentheses ' + $0); return 4.5;"), 42); printf("2. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(({"out('3. got int, extra parentheses ' + $0); return 5.5;"}), 42); printf("3. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(({out('4. got int, extra parentheses ' + $0); return 6.5;}), 42); printf("4. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(("{out('5. got int, extra parentheses ' + $0); return 7.5;}"), 42); printf("5. returned int, extra parentheses %d\n", i);

  printf("\nEM_ASM_INT: More imaginable ways for user to wrap in extra parentheses\n");
  i = EM_ASM_INT({("out('1. got int, extra extra parentheses ' + $0); return 5.5;")}, 42); printf("1. returned int, extra extra parentheses %d\n", i);
  i = EM_ASM_INT({(out('2. got int, extra extra parentheses ' + $0); return 6.5;)}, 42); printf("2. returned int, extra extra parentheses %d\n", i);
  i = EM_ASM_INT(((((((((((out('3. got int, extra extra extra parentheses ' + $0); return 6.5;)))))))))), 42); printf("3. returned int, extra extra extra parentheses %d\n", i);

  printf("\nEM_ASM_INT: Return an integer back.\n");
  i = EM_ASM_INT(out('1. got int ' + $0); return 3.5;, 42); printf("1. returned int %d\n", i);
  i = EM_ASM_INT("out('2. got int ' + $0); return 4.5;", 42); printf("2. returned int %d\n", i);
  i = EM_ASM_INT({"out('3. got int ' + $0); return 5.5;"}, 42); printf("3. returned int %d\n", i);
  i = EM_ASM_INT({out('4. got int ' + $0); return 6.5;}, 42); printf("4. returned int %d\n", i);
  i = EM_ASM_INT("{out('5. got int ' + $0); return 7.5;}", 42); printf("5. returned int %d\n", i);

  printf("\nEM_ASM_INT: Return an integer in a single brief statement.\n");
  i = EM_ASM_INT(return 42); printf("1. returned statement %d\n", i);
  i = EM_ASM_INT("return 42+1"); printf("2. returned statement %d\n", i);
  i = EM_ASM_INT({"return 42+2"}); printf("3. returned statement %d\n", i);
  i = EM_ASM_INT({return 42+3}); printf("4. returned statement %d\n", i);
  i = EM_ASM_INT("return 42+4"); printf("5. returned statement %d\n", i);

  // Note that expressions do not evaluate to return values, but the "return" keyword is needed. That is, the following line would return undefined and store i <- 0.
  // i = EM_ASM_INT(HEAP8.length); printf("returned statement %d\n", i);

  void* p;

  printf("\nEM_ASM_PTR: Return a pointer back.\n");
  p = EM_ASM_PTR(out('1. got arg ' + $0); return 3;, 42); printf("1. returned ptr %p\n", p);
  p = EM_ASM_PTR("out('2. got arg ' + $0); return 4;", 42); printf("2. returned ptr %p\n", p);
  p = EM_ASM_PTR({"out('3. got arg ' + $0); return 5;"}, 42); printf("3. returned ptr %p\n", p);
  p = EM_ASM_PTR({out('4. got arg ' + $0); return 6;}, 42); printf("4. returned ptr %p\n", p);
  p = EM_ASM_PTR("{out('5. got arg ' + $0); return 7;}", 42); printf("5. returned ptr %p\n", p);

  double d;

  printf("\nEM_ASM_DOUBLE: Pass no parameters, return a double.\n");
  d = EM_ASM_DOUBLE(out('1. returning double'); return 3.5;); printf("1. got double %f\n", d);
  d = EM_ASM_DOUBLE("out('2. returning double'); return 4.5;"); printf("2. got double %f\n", d);
  d = EM_ASM_DOUBLE({"out('3. returning double'); return 5.5;"}); printf("3. got double %f\n", d);
  d = EM_ASM_DOUBLE({out('4. returning double'); return 6.5;}); printf("4. got double %f\n", d);
  d = EM_ASM_DOUBLE("{out('5. returning double'); return 7.5;}"); printf("5. got double %f\n", d);

  printf("\nEM_ASM_DOUBLE: Pass an integer, return a double.\n");
  d = EM_ASM_DOUBLE(out('1. got int ' + $0); return 3.5;, 42); printf("1. returned double %f\n", d);
  d = EM_ASM_DOUBLE("out('2. got int ' + $0); return 4.5;", 42); printf("2. returned double %f\n", d);
  d = EM_ASM_DOUBLE({"out('3. got int ' + $0); return 5.5;"}, 42); printf("3. returned double %f\n", d);
  d = EM_ASM_DOUBLE({out('4. got int ' + $0); return 6.5;}, 42); printf("4. returned double %f\n", d);
  d = EM_ASM_DOUBLE("{out('5. got int ' + $0); return 7.5;}", 42); printf("5. returned double %f\n", d);

  printf("\nEM_ASM_DOUBLE: Pass a double and an integer, return a double.\n");
  d = EM_ASM_DOUBLE(out('1. got double and int ' + $0 + ' ' + $1); return 3.5;, 5.5, 42); printf("1. returned double %f\n", d);
  d = EM_ASM_DOUBLE("out('2. got double and int ' + $0 + ' ' + $1); return 4.5;", 5.5, 42); printf("2. returned double %f\n", d);
  d = EM_ASM_DOUBLE({"out('3. got double and int ' + $0 + ' ' + $1); return 5.5;"}, 5.5, 42); printf("3. returned double %f\n", d);
  d = EM_ASM_DOUBLE({out('4. got double and int ' + $0 + ' ' + $1); return 6.5;}, 5.5, 42); printf("4. returned double %f\n", d);
  d = EM_ASM_DOUBLE("{out('5. got double and int ' + $0 + ' ' + $1); return 7.5;}", 5.5, 42); printf("5. returned double %f\n", d);

  printf("\nEM_ASM_INT: A comma character (,) inside the code block may need extra parentheses\n");
// i = EM_ASM_INT(out('1. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b, 10); printf("1. returned %d\n", i); // This would not compile: use of undeclared identifier 'b'
  i = EM_ASM_INT((out('1. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b), 10); printf("1. returned %d\n", i); // However by wrapping the code block inside parentheses, it will be ok
  i = EM_ASM_INT("out('2. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b", 10); printf("2. returned %d\n", i);
  i = EM_ASM_INT({"out('3. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b"}, 10); printf("3. returned %d\n", i);
// i = EM_ASM_INT({out('4. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b}, 10); printf("4. returned %d\n", i); // This would also not compile: use of undeclared identifier 'b'
  i = EM_ASM_INT(({out('4. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b}), 10); printf("4. returned %d\n", i); // Again by wrapping the code block inside parentheses, it will work
  i = EM_ASM_INT("{out('5. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b}", 10); printf("5. returned %d\n", i);

  printf("\nEM_ASM: Expression contains a tab character\n");
  EM_ASM(out('1. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS'));
  EM_ASM("out('2. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')");
  EM_ASM({"out('3. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')"});
  EM_ASM({out('4. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')});
  EM_ASM("{out('5. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')}");
}
