
#include "optimizer.h"

using namespace cashew;

IString ASM_FLOAT_ZERO;

IString SIMD_INT8X16_CHECK("SIMD_Int8x16_check"),
        SIMD_INT16X8_CHECK("SIMD_Int16x8_check"),
        SIMD_INT32X4_CHECK("SIMD_Int32x4_check"),
        SIMD_FLOAT32X4_CHECK("SIMD_Float32x4_check"),
        SIMD_FLOAT64X2_CHECK("SIMD_Float64x2_check"),
        SIMD_BOOL8X16_CHECK("SIMD_Bool8x16_check"),
        SIMD_BOOL16X8_CHECK("SIMD_Bool16x8_check"),
        SIMD_BOOL32X4_CHECK("SIMD_Bool32x4_check"),
        SIMD_BOOL64X2_CHECK("SIMD_Bool64x2_check");

bool isInteger(double x) {
  return fmod(x, 1) == 0;
}

bool isInteger32(double x) {
  return isInteger(x) && (x == (int32_t)x || x == (uint32_t)x);
}

int parseInt(const char *str) {
  int ret = *str - '0';
  while (*(++str)) {
    ret *= 10;
    ret += *str - '0';
  }
  return ret;
}

HeapInfo parseHeap(const char *name) {
  HeapInfo ret;
  if (name[0] != 'H' || name[1] != 'E' || name[2] != 'A' || name[3] != 'P') {
    ret.valid = false;
    return ret;
  }
  ret.valid = true;
  ret.unsign = name[4] == 'U';
  ret.floaty = name[4] == 'F';
  ret.bits = parseInt(name + (ret.unsign || ret.floaty ? 5 : 4));
  ret.type = !ret.floaty ? ASM_INT : (ret.bits == 64 ? ASM_DOUBLE : ASM_FLOAT);
  return ret;
}

AsmType detectType(Ref node, AsmData *asmData, bool inVarDef) {
  switch (node[0]->getCString()[0]) {
    case 'n': {
      if (node[0] == NUM) {
        if (!isInteger(node[1]->getNumber())) return ASM_DOUBLE;
        return ASM_INT;
      } else if (node[0] == NAME) {
        if (asmData) {
          AsmType ret = asmData->getType(node[1]->getCString());
          if (ret != ASM_NONE) return ret;
        }
        if (!inVarDef) {
          if (node[1] == INF || node[1] == NaN) return ASM_DOUBLE;
          if (node[1] == TEMP_RET0) return ASM_INT;
          return ASM_NONE;
        }
        // We are in a variable definition, where Math_fround(0) optimized into a global constant becomes f0 = Math_fround(0)
        if (ASM_FLOAT_ZERO.isNull()) ASM_FLOAT_ZERO = node[1]->getIString();
        else assert(node[1] == ASM_FLOAT_ZERO);
        return ASM_FLOAT;
      }
      break;
    }
    case 'u': {
      if (node[0] == UNARY_PREFIX) {
        switch (node[1]->getCString()[0]) {
          case '+': return ASM_DOUBLE;
          case '-': return detectType(node[2], asmData, inVarDef);
          case '!': case '~': return ASM_INT;
        }
        break;
      }
      break;
    }
    case 'c': {
      if (node[0] == CALL) {
        if (node[1][0] == NAME) {
          IString name = node[1][1]->getIString();
          if (name == MATH_FROUND) return ASM_FLOAT;
          else if (name == SIMD_FLOAT32X4 || name == SIMD_FLOAT32X4_CHECK) return ASM_FLOAT32X4;
          else if (name == SIMD_FLOAT64X2 || name == SIMD_FLOAT64X2_CHECK) return ASM_FLOAT64X2;
          else if (name == SIMD_INT8X16   || name == SIMD_INT8X16_CHECK) return ASM_INT8X16;
          else if (name == SIMD_INT16X8   || name == SIMD_INT16X8_CHECK) return ASM_INT16X8;
          else if (name == SIMD_INT32X4   || name == SIMD_INT32X4_CHECK) return ASM_INT32X4;
          else if (name == SIMD_BOOL8X16  || name == SIMD_BOOL8X16_CHECK) return ASM_BOOL8X16;
          else if (name == SIMD_BOOL16X8  || name == SIMD_BOOL16X8_CHECK) return ASM_BOOL16X8;
          else if (name == SIMD_BOOL32X4  || name == SIMD_BOOL32X4_CHECK) return ASM_BOOL32X4;
          else if (name == SIMD_BOOL64X2  || name == SIMD_BOOL64X2_CHECK) return ASM_BOOL64X2;
        }
        return ASM_NONE;
      } else if (node[0] == CONDITIONAL) {
        return detectType(node[2], asmData, inVarDef);
      }
      break;
    }
    case 'b': {
      if (node[0] == BINARY) {
        switch (node[1]->getCString()[0]) {
          case '+': case '-':
          case '*': case '/': case '%': return detectType(node[2], asmData, inVarDef);
          case '|': case '&': case '^': case '<': case '>': // handles <<, >>, >>=, <=, >=
          case '=': case '!': { // handles ==, !=
            return ASM_INT;
          }
        }
      }
      break;
    }
    case 's': {
      if (node[0] == SEQ) {
        return detectType(node[2], asmData, inVarDef);
      } else if (node[0] == SUB) {
        assert(node[1][0] == NAME);
        HeapInfo info = parseHeap(node[1][1]->getCString());
        if (info.valid) return ASM_NONE;
        return info.floaty ? ASM_DOUBLE : ASM_INT; // XXX ASM_FLOAT?
      }
      break;
    }
  }
  //dump("horrible", node);
  //assert(0);
  return ASM_NONE;
}

AsmSign detectSign(Ref node) {
  IString type = node[0]->getIString();
  if (type == BINARY) {
    IString op = node[1]->getIString();
    switch (op.str[0]) {
      case '>': {
        if (op == TRSHIFT) return ASM_UNSIGNED;
        // fallthrough
      }
      case '|': case '&': case '^': case '<': case '=': case '!': return ASM_SIGNED;
      case '+': case '-': return ASM_FLEXIBLE;
      case '*': case '/': return ASM_NONSIGNED; // without a coercion, these are double
      default: abort();
    }
  } else if (type == UNARY_PREFIX) {
    IString op = node[1]->getIString();
    switch (op.str[0]) {
      case '-': return ASM_FLEXIBLE;
      case '+': return ASM_NONSIGNED; // XXX double
      case '~': return ASM_SIGNED;
      default: abort();
    }
  } else if (type == NUM) {
    double value = node[1]->getNumber();
    if (value < 0) return ASM_SIGNED;
    if (value > uint32_t(-1) || fmod(value, 1) != 0) return ASM_NONSIGNED;
    if (value == int32_t(value)) return ASM_FLEXIBLE;
    return ASM_UNSIGNED;
  } else if (type == NAME) {
    return ASM_FLEXIBLE;
  } else if (type == CONDITIONAL) {
    return detectSign(node[2]);
  } else if (type == CALL) {
    if (node[1][0] == NAME && node[1][1] == MATH_FROUND) return ASM_NONSIGNED;
  }
  abort();
}

