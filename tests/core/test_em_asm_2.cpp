// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main()
{
  printf("EM_ASM: Simple expression without trailing semicolon\n");
  EM_ASM(console.log('1. expression without trailing semicolon'));
  EM_ASM("console.log('2. expression without trailing semicolon')");
  EM_ASM({"console.log('3. expression without trailing semicolon')"});
  EM_ASM({console.log('4. expression without trailing semicolon')});
  EM_ASM("{console.log('5. expression without trailing semicolon')}");

  printf("\nEM_ASM: Double quotes\n");
  EM_ASM(console.log("1. string in double quotes"));
  EM_ASM("console.log(\"2. string in double quotes\")");
  EM_ASM({"console.log(\"3. string in double quotes\")"});
  EM_ASM({console.log("4. string in double quotes")});
  EM_ASM("{console.log(\"5. string in double quotes\")}");

  printf("\nEM_ASM: Double quotes inside a string\n");
  EM_ASM(console.log('1. this is \"double\" \"quotes\"'));
  EM_ASM(console.log('2. this is "double" "quotes" without escaping'));
  EM_ASM("console.log('3. this is \"double\" \"quotes\"')");
  EM_ASM({"console.log('4. this is \"double\" \"quotes\"')"});
  EM_ASM({console.log('5. this is \"double\" \"quotes\"')});
  EM_ASM({console.log('6. this is "double" "quotes" without esacping')});
  EM_ASM("{console.log('7. this is \"double\" \"quotes\"')}");

  printf("\nEM_ASM: Pass a string\n");
  EM_ASM(console.log('1. hello ' + UTF8ToString($0)), "world!");
  EM_ASM("console.log('2. hello ' + UTF8ToString($0))", "world!");
  EM_ASM({"console.log('3. hello ' + UTF8ToString($0))"}, "world!");
  EM_ASM({console.log('4. hello ' + UTF8ToString($0))}, "world!");
  EM_ASM("{console.log('5. hello ' + UTF8ToString($0))}", "world!");

  printf("\nEM_ASM: Simple expression without trailing semicolon, wrap code block in extra parentheses\n");
  EM_ASM((console.log('1. expression without trailing semicolon, in parentheses')));
  EM_ASM(("console.log('2. expression without trailing semicolon, in parentheses')"));
  EM_ASM(({"console.log('3. expression without trailing semicolon, in parentheses')"}));
  EM_ASM(({console.log('4. expression without trailing semicolon, in parentheses')}));
  EM_ASM(("{console.log('5. expression without trailing semicolon, in parentheses')}"));

  printf("\nEM_ASM: Two statements, separated with a semicolon\n");
  EM_ASM(console.log('1. two'); console.log('1. statements'););
  EM_ASM("console.log('2. two'); console.log('2. statements 2');");
  EM_ASM({"console.log('3. two'); console.log('3. statements 3');"});
  EM_ASM({console.log('4. two'); console.log('4. statements 4');});
  EM_ASM("{console.log('5. two'); console.log('5. statements 5');}");

  printf("\nEM_ASM: Pass an integer\n");
  EM_ASM(console.log('1. int ' + $0), 42);
  EM_ASM("console.log('2. int ' + $0)", 43);
  EM_ASM({"console.log('3. int ' + $0)"}, 44);
  EM_ASM({console.log('4. int ' + $0)}, 45);
  EM_ASM("{console.log('5. int ' + $0)}", 46);

  printf("\nEM_ASM: Evaluate an anonymous function\n");
  EM_ASM((function() {console.log('1. evaluating anonymous function ' + $0)})(), 42);
  EM_ASM("(function() {console.log('2. evaluating anonymous function ' + $0)})()", 42);
  EM_ASM({"(function() {console.log('3. evaluating anonymous function ' + $0)})()"}, 42);
  EM_ASM({(function() {console.log('4. evaluating anonymous function ' + $0)})()}, 42);
  EM_ASM("{(function() {console.log('5. evaluating anonymous function ' + $0)})()}", 42);

  printf("\nEM_ASM: Pass an integer and a double\n");
  EM_ASM(console.log('1. int and double ' + $0 + ' ' + $1), 42, 43.5);
  EM_ASM("console.log('2. int and double ' + $0 + ' ' + $1)", 42, 43.5);
  EM_ASM({"console.log('3. int and double ' + $0 + ' ' + $1)"}, 42, 43.5);
  EM_ASM({console.log('4. int and double ' + $0 + ' ' + $1)}, 42, 43.5);
  EM_ASM("{console.log('5. int and double ' + $0 + ' ' + $1)}", 42, 43.5);

  int i;

  printf("\nEM_ASM_INT: Pass an integer, return an integer back\n");
  i = EM_ASM_INT(console.log('1. got int ' + $0); return 3.5;, 42); printf("1. returned int %d\n", i);
  i = EM_ASM_INT("console.log('2. got int ' + $0); return 4.5;", 42); printf("2. returned int %d\n", i);
  i = EM_ASM_INT({"console.log('3. got int ' + $0); return 5.5;"}, 42); printf("3. returned int %d\n", i);
  i = EM_ASM_INT({console.log('4. got int ' + $0); return 6.5;}, 42); printf("4. returned int %d\n", i);
  i = EM_ASM_INT("{console.log('5. got int ' + $0); return 7.5;}", 42); printf("5. returned int %d\n", i);

  printf("\nEM_ASM_INT: Pass an integer, return an integer back, wrap code block in extra parentheses\n");
  i = EM_ASM_INT((console.log('1. got int, extra parentheses ' + $0); return 3.5;), 42); printf("1. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(("console.log('2. got int, extra parentheses ' + $0); return 4.5;"), 42); printf("2. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(({"console.log('3. got int, extra parentheses ' + $0); return 5.5;"}), 42); printf("3. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(({console.log('4. got int, extra parentheses ' + $0); return 6.5;}), 42); printf("4. returned int, extra parentheses %d\n", i);
  i = EM_ASM_INT(("{console.log('5. got int, extra parentheses ' + $0); return 7.5;}"), 42); printf("5. returned int, extra parentheses %d\n", i);

  printf("\nEM_ASM_INT: More imaginable ways for user to wrap in extra parentheses\n");
  i = EM_ASM_INT({("console.log('1. got int, extra extra parentheses ' + $0); return 5.5;")}, 42); printf("1. returned int, extra extra parentheses %d\n", i);
  i = EM_ASM_INT({(console.log('2. got int, extra extra parentheses ' + $0); return 6.5;)}, 42); printf("2. returned int, extra extra parentheses %d\n", i);
  i = EM_ASM_INT(((((((((((console.log('3. got int, extra extra extra parentheses ' + $0); return 6.5;)))))))))), 42); printf("3. returned int, extra extra extra parentheses %d\n", i);

  printf("\nEM_ASM_INT: Return an integer back.\n");
  i = EM_ASM_INT(console.log('1. got int ' + $0); return 3.5;, 42); printf("1. returned int %d\n", i);
  i = EM_ASM_INT("console.log('2. got int ' + $0); return 4.5;", 42); printf("2. returned int %d\n", i);
  i = EM_ASM_INT({"console.log('3. got int ' + $0); return 5.5;"}, 42); printf("3. returned int %d\n", i);
  i = EM_ASM_INT({console.log('4. got int ' + $0); return 6.5;}, 42); printf("4. returned int %d\n", i);
  i = EM_ASM_INT("{console.log('5. got int ' + $0); return 7.5;}", 42); printf("5. returned int %d\n", i);

  printf("\nEM_ASM_INT: Return an integer in a single brief statement.\n");
  i = EM_ASM_INT(return HEAP8.length); printf("1. returned statement %d\n", i);
  i = EM_ASM_INT("return HEAP8.length+1"); printf("2. returned statement %d\n", i);
  i = EM_ASM_INT({"return HEAP8.length+2"}); printf("3. returned statement %d\n", i);
  i = EM_ASM_INT({return HEAP8.length+3}); printf("4. returned statement %d\n", i);
  i = EM_ASM_INT("return HEAP8.length+4"); printf("5. returned statement %d\n", i);

  // Note that expressions do not evaluate to return values, but the "return" keyword is needed. That is, the following line would return undefined and store i <- 0.
  // i = EM_ASM_INT(HEAP8.length); printf("returned statement %d\n", i);

  double d;

  printf("\nEM_ASM_DOUBLE: Pass no parameters, return a double.\n");
  d = EM_ASM_DOUBLE(console.log('1. returning double'); return 3.5;); printf("1. got double %f\n", d);
  d = EM_ASM_DOUBLE("console.log('2. returning double'); return 4.5;"); printf("2. got double %f\n", d);
  d = EM_ASM_DOUBLE({"console.log('3. returning double'); return 5.5;"}); printf("3. got double %f\n", d);
  d = EM_ASM_DOUBLE({console.log('4. returning double'); return 6.5;}); printf("4. got double %f\n", d);
  d = EM_ASM_DOUBLE("{console.log('5. returning double'); return 7.5;}"); printf("5. got double %f\n", d);

  printf("\nEM_ASM_DOUBLE: Pass an integer, return a double.\n");
  d = EM_ASM_DOUBLE(console.log('1. got int ' + $0); return 3.5;, 42); printf("1. returned double %f\n", d);
  d = EM_ASM_DOUBLE("console.log('2. got int ' + $0); return 4.5;", 42); printf("2. returned double %f\n", d);
  d = EM_ASM_DOUBLE({"console.log('3. got int ' + $0); return 5.5;"}, 42); printf("3. returned double %f\n", d);
  d = EM_ASM_DOUBLE({console.log('4. got int ' + $0); return 6.5;}, 42); printf("4. returned double %f\n", d);
  d = EM_ASM_DOUBLE("{console.log('5. got int ' + $0); return 7.5;}", 42); printf("5. returned double %f\n", d);

  printf("\nEM_ASM_DOUBLE: Pass a double and an integer, return a double.\n");
  d = EM_ASM_DOUBLE(console.log('1. got double and int ' + $0 + ' ' + $1); return 3.5;, 5.5, 42); printf("1. returned double %f\n", d);
  d = EM_ASM_DOUBLE("console.log('2. got double and int ' + $0 + ' ' + $1); return 4.5;", 5.5, 42); printf("2. returned double %f\n", d);
  d = EM_ASM_DOUBLE({"console.log('3. got double and int ' + $0 + ' ' + $1); return 5.5;"}, 5.5, 42); printf("3. returned double %f\n", d);
  d = EM_ASM_DOUBLE({console.log('4. got double and int ' + $0 + ' ' + $1); return 6.5;}, 5.5, 42); printf("4. returned double %f\n", d);
  d = EM_ASM_DOUBLE("{console.log('5. got double and int ' + $0 + ' ' + $1); return 7.5;}", 5.5, 42); printf("5. returned double %f\n", d);

  printf("\nEM_ASM_INT: A comma character (,) inside the code block may need extra parentheses\n");
// i = EM_ASM_INT(console.log('1. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b, 10); printf("1. returned %d\n", i); // This would not compile: use of undeclared identifier 'b'
  i = EM_ASM_INT((console.log('1. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b), 10); printf("1. returned %d\n", i); // However by wrapping the code block inside parentheses, it will be ok
  i = EM_ASM_INT("console.log('2. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b", 10); printf("2. returned %d\n", i);
  i = EM_ASM_INT({"console.log('3. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b"}, 10); printf("3. returned %d\n", i);
// i = EM_ASM_INT({console.log('4. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b}, 10); printf("4. returned %d\n", i); // This would also not compile: use of undeclared identifier 'b'
  i = EM_ASM_INT(({console.log('4. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b}), 10); printf("4. returned %d\n", i); // Again by wrapping the code block inside parentheses, it will work
  i = EM_ASM_INT("{console.log('5. comma in em_asm'); var foo = { a: 5, b: $0 }; return foo.a + foo.b}", 10); printf("5. returned %d\n", i);

  printf("\nEM_ASM: Expression contains a tab character\n");
  EM_ASM(console.log('1. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS'));
  EM_ASM("console.log('2. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')");
  EM_ASM({"console.log('3. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')"});
  EM_ASM({console.log('4. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')});
  EM_ASM("{console.log('5. the following word is delimited by tab characters: H\tE\tL\tL\tO\tT\tA\tB\tS')}");
}
