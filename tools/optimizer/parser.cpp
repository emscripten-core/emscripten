
#include "parser.h"

namespace cashew {

// common strings

IString TOPLEVEL("toplevel"),
        DEFUN("defun"),
        BLOCK("block"),
        STAT("stat"),
        ASSIGN("assign"),
        NAME("name"),
        VAR("var"),
        CONDITIONAL("conditional"),
        BINARY("binary"),
        RETURN("return"),
        IF("if"),
        ELSE("else"),
        WHILE("while"),
        DO("do"),
        FOR("for"),
        SEQ("seq"),
        SUB("sub"),
        CALL("call"),
        NUM("num"),
        LABEL("label"),
        BREAK("break"),
        CONTINUE("continue"),
        SWITCH("switch"),
        STRING("string"),
        INF("inf"),
        NaN("nan"),
        TEMP_RET0("tempRet0"),
        UNARY_PREFIX("unary-prefix"),
        UNARY_POSTFIX("unary-postfix"),
        MATH_FROUND("Math_fround"),
        SIMD_FLOAT32X4("SIMD_float32x4"),
        SIMD_INT32X4("SIMD_int32x4"),
        PLUS("+"),
        MINUS("-"),
        OR("|"),
        AND("&"),
        XOR("^"),
        L_NOT("!"),
        B_NOT("~"),
        LT("<"),
        GE(">="),
        LE("<="),
        GT(">"),
        EQ("=="),
        NE("!="),
        DIV("/"),
        MOD("%"),
        RSHIFT(">>"),
        LSHIFT("<<"),
        TRSHIFT(">>>"),
        TEMP_DOUBLE_PTR("tempDoublePtr"),
        HEAP8("HEAP8"),
        HEAP16("HEAP16"),
        HEAP32("HEAP32"),
        HEAPF32("HEAPF32"),
        HEAPU8("HEAPU8"),
        HEAPU16("HEAPU16"),
        HEAPU32("HEAPU32"),
        HEAPF64("HEAPF64"),
        F0("f0"),
        EMPTY(""),
        FUNCTION("function"),
        OPEN_PAREN("("),
        OPEN_BRACE("["),
        COMMA(","),
        QUESTION("?"),
        COLON(":"),
        CASE("case"),
        DEFAULT("default"),
        SET("=");

IStringSet keywords("var function if else do while for break continue return switch case default throw try catch finally true false null"),
           allOperators(". ! ~ - + * / % + - << >> >>> < <= > >= == != & ^ | ? : = ,");

const char *OPERATOR_INITS = "+-*/%<>&^|~=!,?:",
           *SEPARATORS = "([;{";

int MAX_OPERATOR_SIZE = 3;

std::vector<OperatorClass> operatorClasses;

struct Init {
  Init() {
    // operators, rtl, type
    operatorClasses.push_back(OperatorClass("! ~ + -",   true,  OperatorClass::Prefix));
    operatorClasses.push_back(OperatorClass("* / %",     false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("+ -",       false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("<< >> >>>", false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("< <= > >=", false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("== !=",     false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("&",         false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("^",         false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("|",         false, OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass("? :",       true,  OperatorClass::Tertiary));
    operatorClasses.push_back(OperatorClass("=",         true,  OperatorClass::Binary));
    operatorClasses.push_back(OperatorClass(",",         false, OperatorClass::Binary));
  }
};

Init init;

} // namespace cashew

