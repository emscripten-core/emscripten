#include <emscripten.h>

EM_JS(void, test_remove_cpp_comments_in_shaders, (void), {
  var numFailed = 0;
  function test(input, expected) {
    var obtained = remove_cpp_comments_in_shaders(input);
    function remove_nl(t) { return t.replace(/\\n/g, "\\\\\\n")}

    if (obtained == expected) {
      out(`OK: '${remove_nl(input)}' -> '${remove_nl(expected)}'`);
    } else {
      err(`\\nFailed! \\nInput: '${remove_nl(input)}'\\nObtained: '${remove_nl(obtained)}'\\nExpected: '${remove_nl(expected)}'\\n`);
      ++numFailed;
    }
  }
  test('foo(); // test // test2 // test3', 'foo(); '); // Test that C++ comments '//' are removed
  test('foo(); / /// test',                'foo(); / '); // Test that no confusion with slightly similar looking inputs
  test('foo(); // /*',                     'foo(); '); // Test that // takes away /*
  test('foo(); //\nbar();',                'foo(); \nbar();'); // Test that // ends at the next newline
  test('foo(); //*\nbar(); */',            'foo(); \nbar(); */'); // Test that //* is interpreted as //
  test('foo(); /* aaa',                    'foo(); '); // Test that on malformed /* without ending */, we remove all content up to end of string
  test('foo(); /*\neee\n*/bar();',         'foo(); bar();'); // Test that /* */ works over multiple lines
  test('foo(); /* // */bar();',            'foo(); bar();'); // Test that // comments inside /* */ comments are not processed
  test('foo(); /*eee*//bar();',            'foo(); /bar();'); // Test that // comment as part of /* */ is not processed
  test('foo(); /* a /* b */ c */',         'foo();  c */'); // Test that /* */ should not nest

  // Verify known gotchas/quirks due to partial support: these are broken by design, as shaders do not support strings.
  test('" // comment inside a string "',   '" '); // Test that (unfortunately) comments will be removed even if they exist inside "" strings
  test("' // comment inside a string '",   "' "); // Test that (unfortunately) comments will be removed even if they exist inside '' strings
  test('" // comment inside a multi\nline string "', '" \nline string "'); // Test that (unfortunately) newline inside string breaks a comment
  test('" /* C comment inside a string */ "',   '"  "'); // Test that (unfortunately) C comments will also be removed inside strings

  if (numFailed) throw numFailed + ' tests failed!';
});

EM_JS_DEPS(main, "$preprocess_c_code,$remove_cpp_comments_in_shaders");

EM_JS(void, test_c_preprocessor, (void), {
  var numFailed = 0;
  function test(input, expected) {
    var obtained = preprocess_c_code(input);
    function remove_nl(t) { return t.replace(/\\n/g, String.fromCharCode(92) + 'n'); }

    if (obtained == expected) {
      out(`OK: '${remove_nl(input)}' -> '${remove_nl(expected)}'`);
    } else {
      err(`\\nFailed! \\nInput: '${remove_nl(input)}'\\nObtained: '${remove_nl(obtained)}'\\nExpected: '${remove_nl(expected)}'\\n`);
      ++numFailed;
    }
  }

  test('A\n #ifdef FOO\n B\n #endif', 'A\n'); // Test #ifdef not taken (also test whitespace at the beginning of a line)

  test(' #define FOO 1\nA\n#ifdef FOO\n B\n #endif', 'A\n B\n'); // Test #ifdef taken

  test(' #define FOO 0\nA\n#ifdef FOO\n B\n #endif', 'A\n B\n'); // Test #ifdef should be taken when var #defined to 0.

  test('A\n#ifndef FOO\nB\n#endif', 'A\nB\n'); // Test #ifndef taken
  test('#define FOO 1\nA\n#ifndef FOO\nB\n#endif', 'A\n'); // Test #ifndef not taken
  test('#define FOO 0\nA\n#ifndef FOO\nB\n#endif', 'A\n'); // Test #ifndef not taken when #defined to 0.

  test('#define FOO 1\nA\n#ifdef FOO\nB\n#endif\n#undef FOO\n#ifdef FOO\nC\n#endif\n', 'A\nB\n'); // Test #undef
  test('#define FOO 1\nA\n#ifdef FOO\nB\n#endif\n#undef FOO\n#ifndef FOO\nC\n#endif\n', 'A\nB\nC\n'); // Test #undef

  test('A\n#ifdef FOO\nB\n#else\nC\n#endif', 'A\nC\n'); // Test #ifdef-#else-#endif not taken
  test('A\n#define FOO 0\n#ifdef FOO\nB\n#else\nC\n#endif', 'A\nB\n'); // Test #ifdef-#else-#endif taken

  test(' #define FOO  0\nA\n#if FOO\n B\n#else \n C\n #endif', 'A\n C\n'); // Test #if-#else-#endif not taken
  test(' #define FOO 10\nA\n#if FOO\n B\n#else \n C\n #endif', 'A\n B\n');  // Test #if-#else-#endif taken

  test('#define FOO 1\n#define BAR 0\n#if FOO && BAR\nB\n#endif', ""); // Test && operator yielding negative
  test('#define FOO 1\n#define BAR 1\n#if FOO && BAR\nB\n#endif', 'B\n'); // Test && operator yielding positive
  test('#define FOO\t1\n#define BAR\t \t1\n#if FOO  \t  \t  &&BAR\t\nB\n#endif', 'B\n'); // Test more complex whitespace around &&

  test('#define FOO 1\n#define BAR 1\n#if (FOO && BAR)\nB\n#endif', 'B\n'); // Test parentheses around &&
  test('#define FOO 1\n#define BAR 1\n#if (((FOO && BAR)))\nB\n#endif', 'B\n'); // Test nested parenthesess around &&
  test('#define FOO 0\n#define BAR 1\n#define BAZ 1\n#if FOO&&BAR||BAZ\nB\n#endif', 'B\n'); // Test precedence of && and ||
  test('#define FOO 0\n#define BAR 1\n#define BAZ 1\n#if (FOO&&BAR)||BAZ\nB\n#endif', 'B\n'); // Test parentheses with && and ||
  test('#define FOO 0\n#define BAR 1\n#define BAZ 1\n#if FOO&&(BAR||BAZ)\nB\n#endif', "");    // Test parentheses with && and ||
  test('#define FOO 1\n#define BAR 1\n#define BAZ 0\n#if FOO||BAR&&BAZ\nB\n#endif', 'B\n');   // Test parentheses with && and ||
  test('#define FOO 1\n#define BAR 1\n#define BAZ 0\n#if (FOO||BAR)&&BAZ\nB\n#endif', "");    // Test parentheses with && and ||
  test('#define FOO 1\n#define BAR 1\n#define BAZ 0\n#if FOO||(BAR&&BAZ)\nB\n#endif', 'B\n'); // Test parentheses with && and ||

  test('#define FOO 1\n#define BAR 1\n#define BAZ 0\n#if FOO&&BAR&&BAZ\nB\n#endif', ""); // Test two &&s
  test('#define FOO 0\n#define BAR 1\n#define BAZ 1\n#if FOO&&BAR&&BAZ\nB\n#endif', ""); // Test two &&s
  test('#define FOO 1\n#define BAR 0\n#define BAZ 1\n#if FOO&&BAR&&BAZ\nB\n#endif', ""); // Test two &&s
  test('#define FOO 1\n#define BAR 1\n#define BAZ 1\n#if FOO&&BAR&&BAZ\nB\n#endif', 'B\n'); // Test two &&s

  test('#define FOO 1\n#define BAR 0\n#define BAZ 0\n#if FOO||BAR||BAZ\nB\n#endif', 'B\n'); // Test two ||s
  test('#define FOO 0\n#define BAR 1\n#define BAZ 0\n#if FOO||BAR||BAZ\nB\n#endif', 'B\n'); // Test two ||s
  test('#define FOO 0\n#define BAR 0\n#define BAZ 1\n#if FOO||BAR||BAZ\nB\n#endif', 'B\n'); // Test two ||s
  test('#define FOO 0\n#define BAR 0\n#define BAZ 0\n#if FOO||BAR||BAZ\nB\n#endif', ""); // Test two ||s

  test('#define FOO 1\n#if FOO < 3\nB\n#endif', 'B\n');  // Test <
  test('#define FOO 1\n#if FOO < 1\nB\n#endif', "");     // Test <
  test('#define FOO 1\n#if FOO > 0\nB\n#endif', 'B\n');  // Test >
  test('#define FOO 1\n#if FOO > 1\nB\n#endif', "");     // Test >
  test('#define FOO 1\n#if FOO <= 1\nB\n#endif', 'B\n'); // Test <=
  test('#define FOO 1\n#if FOO <= 0\nB\n#endif', "");    // Test <=
  test('#define FOO 1\n#if FOO >= 1\nB\n#endif', 'B\n'); // Test >=
  test('#define FOO 1\n#if FOO >= 2\nB\n#endif', "");    // Test >=

  test('#define FOO 0\n#define BAR 0\n#if FOO\n#if BAR\n1\n#else\n2\n#endif\n#else\n#if BAR\n3\n#else\n4\n#endif\n#endif', '4\n'); // Test nested #if-#elses
  test('#define FOO 0\n#define BAR 1\n#if FOO\n#if BAR\n1\n#else\n2\n#endif\n#else\n#if BAR\n3\n#else\n4\n#endif\n#endif', '3\n'); // Test nested #if-#elses
  test('#define FOO 1\n#define BAR 0\n#if FOO\n#if BAR\n1\n#else\n2\n#endif\n#else\n#if BAR\n3\n#else\n4\n#endif\n#endif', '2\n'); // Test nested #if-#elses
  test('#define FOO 1\n#define BAR 1\n#if FOO\n#if BAR\n1\n#else\n2\n#endif\n#else\n#if BAR\n3\n#else\n4\n#endif\n#endif', '1\n'); // Test nested #if-#elses

  test('#define FOO 1\n#define BAR 2\n#if FOO < 3 && BAR < 4\nB\n#endif', 'B\n'); // Test evaluation of && and <
  test('#define FOO 1\n#define BAR 2\n#if FOO < 3 && BAR < 2\nB\n#endif', "");    // Test evaluation of && and <

  test('#define FOO 1\n#define BAR 2\n#if FOO == 1 && BAR == 2\nB\n#endif', 'B\n'); // Test evaluation of && and ==
  test('#define FOO 1\n#define BAR 2\n#if FOO == 1 && BAR != 2\nB\n#endif', "");    // Test evaluation of && and !=

  test('#define FOO 1\n#define BAR 2\n#if FOO < 3 || BAR < 0\nB\n#endif', 'B\n'); // Test evaluation of || and <
  test('#define FOO 1\n#define BAR 2\n#if FOO < 1 || BAR < 2\nB\n#endif', "");    // Test evaluation of || and <

  test('#define FOO 1\n#define BAR 2\n#if FOO == 1 || BAR == 3\nB\n#endif', 'B\n'); // Test evaluation of || and ==
  test('#define FOO 1\n#define BAR 2\n#if FOO != 1 || BAR != 2\nB\n#endif', "");    // Test evaluation of || and !=

  test('#define FOO 1\n#define BAR 2\n#if FOO < 2 && BAR > 1 && FOO <= 1 && BAR >= 2 && FOO == 1 && BAR != 1\nB\n#endif', 'B\n'); // Test complex comparisons
  test('#define FOO 1\n#define BAR 2\n#if FOO < 2 && BAR > 1 && FOO <= 1 && BAR >= 2 && FOO == 1 && BAR != 2\nB\n#endif', "");    // Test complex comparisons

  test('#define FOO 1\n#define BAR 1\n#if FOO == BAR\nB\n#endif', 'B\n'); // Test comparison of two vars against each other
  test('#define FOO 1\n#define BAR 1\n#if FOO != BAR\nB\n#endif', "");    // Test comparison of two vars against each other
  test('#define FOO 1\n#define BAR 2\n#if FOO == BAR\nB\n#endif', "");    // Test comparison of two vars against each other
  test('#define FOO 1\n#define BAR 2\n#if FOO != BAR\nB\n#endif', 'B\n'); // Test comparison of two vars against each other

  test('#define FOO 0\n#if !FOO\nB\n#endif', 'B\n'); // Test unary !
  test('#define FOO 1\n#if !FOO\nB\n#endif', "");    // Test unary !

  test('#define FOO 1\n#define BAR 0\n#if !FOO && BAR\nB\n#endif', "");      // Test unary ! and && precedence
  test('#define FOO 1\n#define BAR 0\n#if FOO && !BAR\nB\n#endif', 'B\n');   // Test unary ! and && precedence
  test('#define FOO 0\n#define BAR 1\n#if !(FOO && BAR)\nB\n#endif', 'B\n'); // Test unary ! and && precedence with parentheses
  test('#define FOO 1\n#define BAR 1\n#if !(FOO && BAR)\nB\n#endif', "");    // Test unary ! and && precedence with parentheses

  test('#define FOO 1\n#if !!FOO\nB\n#endif', 'B\n'); // Test double !!
  test('#define FOO 1\n#if !!(FOO)\nB\n#endif', 'B\n'); // Test double !!
  test('#define FOO 1\n#if !(!FOO)\nB\n#endif', 'B\n'); // Test double !!
  test('#define FOO 1\n#if ((!((!((!!!FOO))))))\nB\n#endif', ""); // Test complex ! and ()

  test('#define M12 1\n#define M22 0\n#if M12 == 1 && M22 == 0\nB\n#endif', 'B\n'); // Test including a number in a variable name
  test('#define M12 1\n#define M22 0\n#if M22 == 1 && M12 == 0\nB\n#endif', "");    // Test including a number in a variable name

  test('#define FOO 42\nFOO\n', '42\n'); // Test expanding a preprocessor symbol

  test('#define FOO 42\n#define BAR FOO\nBAR\n', '42\n'); // Test chained expanding a preprocessor symbol
  test('#define MACRO(x) x\nMACRO(42)\n', '42\n'); // Test one-parameter preprocessor macro
  test('#define MACRO(x,y) x\nMACRO(42, 53)\n', '42\n'); // Test a two-parameter preprocessor macro
  test('#define MACRO(  \t  x   ,   y   )    \t x    \t\nMACRO(42, 53)\n', '42\n'); // Test a macro with odd whitescape in it

  test('#define MACRO(x,y,z) x+y<=z\nMACRO(42,15,30)\n', '42+15<=30\n'); // Test three-arg macro

  test('#define FOO 1\n#define BAR 2\n#define BAZ 1\n#define MACRO(x,y,z) x<y&&z\n#if MACRO(FOO,BAR,BAZ)\nA\n#endif', 'A\n'); // Test three-arg macro in an #if comparison
  test('#define FOO 1\n#define BAR 2\n#define BAZ 0\n#define MACRO(x,y,z) x<y&&z\n#if MACRO(FOO,BAR,BAZ)\nA\n#endif', ""); // Test three-arg macro in an #if comparison

  test('#if 1>2&&0\nA\n#endif', "");    // Test precedence between > and &&
  test('#if 0&&1>2\nA\n#endif', "");    // Test precedence between > and &&
  test('#if 2<3&&1\nA\n#endif', "A\n"); // Test precedence between > and &&
  test('#if 1&&2<3\nA\n#endif', "A\n"); // Test precedence between > and &&

  test('#if 1+2<=3\nA\n#endif', 'A\n'); // Test arithmetic + and comparison op precedence
  test('#if 1+2 <3\nA\n#endif', "");    // Test arithmetic + and comparison op precedence

  test('#if 1+2*0\nA\n#endif', "A\n");  // Test arithmetic + and * precedence
  test('#if 1+1*2==4\nA\n#endif', "");  // Test arithmetic + and * precedence

  test('#if 5/2==2\nA\n#endif', "A\n");  // Test arithmetic integer divide /

  test('#if defined(FOO)\nA\n#endif', "");  // Test defined() macro
  test('#define FOO 0\n#if defined(FOO)\nA\n#endif', "A\n");  // Test defined() macro
  test('#define FOO 0\n#if !defined(FOO)\nA\n#endif', "");  // Test that !defined() works
  test('#define FOO 0\n#if defined FOO \nA\n#endif', "A\n");  // Test defined without using parens
  test('#define FOO 0\n#if ! defined FOO \nA\n#endif', "");  // Test that ! defined works, with odd whitespace
  test('#define FOO 0\n#if defined   FOO  \nA\n#endif', "A\n");  // Test defined without using parens, with odd whitespace
  test('#define FOO 0\n#if defined (FOO) \nA\n#endif', "A\n");  // Test defined with parens and whitespace
  test('#define FOO 0\n#if defined ( FOO ) \nA\n#endif', "A\n");  // Test defined with parens and whitespace
  test('#define FOO 0\n#undef FOO\n#if defined(FOO)\nA\n#endif', "");  // Test defined() macro after #undef
  test('#define FOO 0\n#if defined FOO && BAR \nA\n#endif', "");  // Test that defined operator binds tighter than &&

  test('#define SAMPLE_TEXTURE_2D texture \nvec4 c = SAMPLE_TEXTURE_2D(tex, texCoord);\n', 'vec4 c = texture(tex, texCoord);\n'); // Test expanding a non-macro to a macro-like call site

  test('#extension GL_EXT_shader_texture_lod : enable\n', '#extension GL_EXT_shader_texture_lod : enable\n'); // Test that GLSL preprocessor macros are preserved
  test('#version 300 es\n', '#version 300 es\n'); // Test that GLSL preprocessor macros are preserved
  test('#pragma foo\n', '#pragma foo\n'); // Test that GLSL preprocessor macros are preserved

  test('#define FOO() bar\nFOO()\n', 'bar\n'); // Test preprocessor macros that do not take in any parameters

  test('#define FOO 1\n#if FOO\n#define BAR this_is_right\n#else\n#define BAR this_is_wrong\n#endif\nBAR', 'this_is_right\n'); // Test nested #defines in both sides of an #if-#else block.

  test('\n#define FOO 1\nFOO\n', '\n1\n'); // Test that preprocessor is not confused by an input that starts with a \n

  test('#line 162 "foo.glsl"\n', '#line 162 "foo.glsl"\n'); // Test that #line directives are retained in the output

  if (numFailed) throw numFailed + ' tests failed!';
});

int main()
{
  test_remove_cpp_comments_in_shaders();

  test_c_preprocessor();
}
