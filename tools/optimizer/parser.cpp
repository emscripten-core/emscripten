// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
        CONST("const"),
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
        MATH_FROUND("Math_fround"),
        SIMD_FLOAT32X4("SIMD_Float32x4"),
        SIMD_FLOAT64X2("SIMD_Float64x2"),
        SIMD_INT8X16("SIMD_Int8x16"),
        SIMD_INT16X8("SIMD_Int16x8"),
        SIMD_INT32X4("SIMD_Int32x4"),
        SIMD_BOOL8X16("SIMD_Bool8x16"),
        SIMD_BOOL16X8("SIMD_Bool16x8"),
        SIMD_BOOL32X4("SIMD_Bool32x4"),
        SIMD_BOOL64X2("SIMD_Bool64x2"),
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
        MUL("*"),
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
        OPEN_CURLY("{"),
        CLOSE_CURLY("}"),
        COMMA(","),
        QUESTION("?"),
        COLON(":"),
        CASE("case"),
        DEFAULT("default"),
        DOT("dot"),
        PERIOD("."),
        NEW("new"),
        ARRAY("array"),
        OBJECT("object"),
        THROW("throw"),
        SET("=");

IStringSet keywords("var const function if else do while for break continue return switch case default throw try catch finally true false null new");

const char *OPERATOR_INITS = "+-*/%<>&^|~=!,?:.",
           *SEPARATORS = "([;{}";

int MAX_OPERATOR_SIZE = 3;

std::vector<OperatorClass> operatorClasses;

static std::vector<std::unordered_map<IString, int>> precedences; // op, type => prec

struct Init {
  Init() {
    // operators, rtl, type
    operatorClasses.push_back(OperatorClass(".",         false, OperatorClass::Binary));
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
    operatorClasses.push_back(OperatorClass(",",         true,  OperatorClass::Binary));

    precedences.resize(OperatorClass::Tertiary + 1);

    for (size_t prec = 0; prec < operatorClasses.size(); prec++) {
      for (auto curr : operatorClasses[prec].ops) {
        precedences[operatorClasses[prec].type][curr] = prec;
      }
    }
  }
};

Init init;

int OperatorClass::getPrecedence(Type type, IString op) {
  return precedences[type][op];
}

bool OperatorClass::getRtl(int prec) {
  return operatorClasses[prec].rtl;
}

bool isIdentInit(char x) { return (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || x == '_' || x == '$'; }
bool isIdentPart(char x) { return isIdentInit(x) || (x >= '0' && x <= '9'); }

} // namespace cashew

