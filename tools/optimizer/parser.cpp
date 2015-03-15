
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

