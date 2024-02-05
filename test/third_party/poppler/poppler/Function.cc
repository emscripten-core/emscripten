//========================================================================
//
// Function.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2008-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "goo/gmem.h"
#include "goo/gstrtod.h"
#include "Object.h"
#include "Dict.h"
#include "Stream.h"
#include "Error.h"
#include "Function.h"
#include "PopplerCache.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
// Function
//------------------------------------------------------------------------

Function::Function() {
}

Function::~Function() {
}

Function *Function::parse(Object *funcObj) {
  std::set<int> usedParents;
  return parse(funcObj, &usedParents);
}

Function *Function::parse(Object *funcObj, std::set<int> *usedParents) {
  Function *func;
  Dict *dict;
  int funcType;
  Object obj1;

  if (funcObj->isStream()) {
    dict = funcObj->streamGetDict();
  } else if (funcObj->isDict()) {
    dict = funcObj->getDict();
  } else if (funcObj->isName("Identity")) {
    return new IdentityFunction();
  } else {
    error(-1, "Expected function dictionary or stream");
    return NULL;
  }

  if (!dict->lookup("FunctionType", &obj1)->isInt()) {
    error(-1, "Function type is missing or wrong type");
    obj1.free();
    return NULL;
  }
  funcType = obj1.getInt();
  obj1.free();

  if (funcType == 0) {
    func = new SampledFunction(funcObj, dict);
  } else if (funcType == 2) {
    func = new ExponentialFunction(funcObj, dict);
  } else if (funcType == 3) {
    func = new StitchingFunction(funcObj, dict, usedParents);
  } else if (funcType == 4) {
    func = new PostScriptFunction(funcObj, dict);
  } else {
    error(-1, "Unimplemented function type (%d)", funcType);
    return NULL;
  }
  if (!func->isOk()) {
    delete func;
    return NULL;
  }

  return func;
}

GBool Function::init(Dict *dict) {
  Object obj1, obj2;
  int i;

  //----- Domain
  if (!dict->lookup("Domain", &obj1)->isArray()) {
    error(-1, "Function is missing domain");
    goto err2;
  }
  m = obj1.arrayGetLength() / 2;
  if (m > funcMaxInputs) {
    error(-1, "Functions with more than %d inputs are unsupported",
	  funcMaxInputs);
    goto err2;
  }
  for (i = 0; i < m; ++i) {
    obj1.arrayGet(2*i, &obj2);
    if (!obj2.isNum()) {
      error(-1, "Illegal value in function domain array");
      goto err1;
    }
    domain[i][0] = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2*i+1, &obj2);
    if (!obj2.isNum()) {
      error(-1, "Illegal value in function domain array");
      goto err1;
    }
    domain[i][1] = obj2.getNum();
    obj2.free();
  }
  obj1.free();

  //----- Range
  hasRange = gFalse;
  n = 0;
  if (dict->lookup("Range", &obj1)->isArray()) {
    hasRange = gTrue;
    n = obj1.arrayGetLength() / 2;
    if (n > funcMaxOutputs) {
      error(-1, "Functions with more than %d outputs are unsupported",
	    funcMaxOutputs);
      goto err2;
    }
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(2*i, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function range array");
	goto err1;
      }
      range[i][0] = obj2.getNum();
      obj2.free();
      obj1.arrayGet(2*i+1, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function range array");
	goto err1;
      }
      range[i][1] = obj2.getNum();
      obj2.free();
    }
  }
  obj1.free();

  return gTrue;

 err1:
  obj2.free();
 err2:
  obj1.free();
  return gFalse;
}

//------------------------------------------------------------------------
// IdentityFunction
//------------------------------------------------------------------------

IdentityFunction::IdentityFunction() {
  int i;

  // fill these in with arbitrary values just in case they get used
  // somewhere
  m = funcMaxInputs;
  n = funcMaxOutputs;
  for (i = 0; i < funcMaxInputs; ++i) {
    domain[i][0] = 0;
    domain[i][1] = 1;
  }
  hasRange = gFalse;
}

IdentityFunction::~IdentityFunction() {
}

void IdentityFunction::transform(double *in, double *out) {
  int i;

  for (i = 0; i < funcMaxOutputs; ++i) {
    out[i] = in[i];
  }
}

//------------------------------------------------------------------------
// SampledFunction
//------------------------------------------------------------------------

SampledFunction::SampledFunction(Object *funcObj, Dict *dict) {
  Stream *str;
  int sampleBits;
  double sampleMul;
  Object obj1, obj2;
  Guint buf, bitMask;
  int bits;
  Guint s;
  int i;

  samples = NULL;
  sBuf = NULL;
  ok = gFalse;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (!hasRange) {
    error(-1, "Type 0 function is missing range");
    goto err1;
  }
  if (m > sampledFuncMaxInputs) {
    error(-1, "Sampled functions with more than %d inputs are unsupported",
	  sampledFuncMaxInputs);
    goto err1;
  }

  //----- buffer
  sBuf = (double *)gmallocn(1 << m, sizeof(double));

  //----- get the stream
  if (!funcObj->isStream()) {
    error(-1, "Type 0 function isn't a stream");
    goto err1;
  }
  str = funcObj->getStream();

  //----- Size
  if (!dict->lookup("Size", &obj1)->isArray() ||
      obj1.arrayGetLength() != m) {
    error(-1, "Function has missing or invalid size array");
    goto err2;
  }
  for (i = 0; i < m; ++i) {
    obj1.arrayGet(i, &obj2);
    if (!obj2.isInt()) {
      error(-1, "Illegal value in function size array");
      goto err3;
    }
    sampleSize[i] = obj2.getInt();
    obj2.free();
  }
  obj1.free();
  idxMul[0] = n;
  for (i = 1; i < m; ++i) {
    idxMul[i] = idxMul[i-1] * sampleSize[i-1];
  }

  //----- BitsPerSample
  if (!dict->lookup("BitsPerSample", &obj1)->isInt()) {
    error(-1, "Function has missing or invalid BitsPerSample");
    goto err2;
  }
  sampleBits = obj1.getInt();
  sampleMul = 1.0 / (pow(2.0, (double)sampleBits) - 1);
  obj1.free();

  //----- Encode
  if (dict->lookup("Encode", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2*m) {
    for (i = 0; i < m; ++i) {
      obj1.arrayGet(2*i, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function encode array");
	goto err3;
      }
      encode[i][0] = obj2.getNum();
      obj2.free();
      obj1.arrayGet(2*i+1, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function encode array");
	goto err3;
      }
      encode[i][1] = obj2.getNum();
      obj2.free();
    }
  } else {
    for (i = 0; i < m; ++i) {
      encode[i][0] = 0;
      encode[i][1] = sampleSize[i] - 1;
    }
  }
  obj1.free();
  for (i = 0; i < m; ++i) {
    inputMul[i] = (encode[i][1] - encode[i][0]) /
                  (domain[i][1] - domain[i][0]);
  }

  //----- Decode
  if (dict->lookup("Decode", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2*n) {
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(2*i, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function decode array");
	goto err3;
      }
      decode[i][0] = obj2.getNum();
      obj2.free();
      obj1.arrayGet(2*i+1, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function decode array");
	goto err3;
      }
      decode[i][1] = obj2.getNum();
      obj2.free();
    }
  } else {
    for (i = 0; i < n; ++i) {
      decode[i][0] = range[i][0];
      decode[i][1] = range[i][1];
    }
  }
  obj1.free();

  //----- samples
  nSamples = n;
  for (i = 0; i < m; ++i)
    nSamples *= sampleSize[i];
  samples = (double *)gmallocn(nSamples, sizeof(double));
  buf = 0;
  bits = 0;
  bitMask = (1 << sampleBits) - 1;
  str->reset();
  for (i = 0; i < nSamples; ++i) {
    if (sampleBits == 8) {
      s = str->getChar();
    } else if (sampleBits == 16) {
      s = str->getChar();
      s = (s << 8) + str->getChar();
    } else if (sampleBits == 32) {
      s = str->getChar();
      s = (s << 8) + str->getChar();
      s = (s << 8) + str->getChar();
      s = (s << 8) + str->getChar();
    } else {
      while (bits < sampleBits) {
	buf = (buf << 8) | (str->getChar() & 0xff);
	bits += 8;
      }
      s = (buf >> (bits - sampleBits)) & bitMask;
      bits -= sampleBits;
    }
    samples[i] = (double)s * sampleMul;
  }
  str->close();

  ok = gTrue;
  return;

 err3:
  obj2.free();
 err2:
  obj1.free();
 err1:
  return;
}

SampledFunction::~SampledFunction() {
  if (samples) {
    gfree(samples);
  }
  if (sBuf) {
    gfree(sBuf);
  }
}

SampledFunction::SampledFunction(SampledFunction *func) {
  memcpy(this, func, sizeof(SampledFunction));
  samples = (double *)gmallocn(nSamples, sizeof(double));
  memcpy(samples, func->samples, nSamples * sizeof(double));
  sBuf = (double *)gmallocn(1 << m, sizeof(double));
}

void SampledFunction::transform(double *in, double *out) {
  double x;
  int e[funcMaxInputs][2];
  double efrac0[funcMaxInputs];
  double efrac1[funcMaxInputs];
  int i, j, k, idx, t;

  // map input values into sample array
  for (i = 0; i < m; ++i) {
    x = (in[i] - domain[i][0]) * inputMul[i] + encode[i][0];
    if (x < 0) {
      x = 0;
    } else if (x > sampleSize[i] - 1) {
      x = sampleSize[i] - 1;
    }
    e[i][0] = (int)x;
    if ((e[i][1] = e[i][0] + 1) >= sampleSize[i]) {
      // this happens if in[i] = domain[i][1]
      e[i][1] = e[i][0];
    }
    efrac1[i] = x - e[i][0];
    efrac0[i] = 1 - efrac1[i];
  }

  // for each output, do m-linear interpolation
  for (i = 0; i < n; ++i) {

    // pull 2^m values out of the sample array
    for (j = 0; j < (1<<m); ++j) {
      idx = i;
      for (k = 0, t = j; k < m; ++k, t >>= 1) {
	idx += idxMul[k] * (e[k][t & 1]);
      }
      if (likely(idx >= 0 && idx < nSamples)) {
        sBuf[j] = samples[idx];
      } else {
        sBuf[j] = 0; // TODO Investigate if this is what Adobe does
      }
    }

    // do m sets of interpolations
    for (j = 0, t = (1<<m); j < m; ++j, t >>= 1) {
      for (k = 0; k < t; k += 2) {
	sBuf[k >> 1] = efrac0[j] * sBuf[k] + efrac1[j] * sBuf[k+1];
      }
    }

    // map output value to range
    out[i] = sBuf[0] * (decode[i][1] - decode[i][0]) + decode[i][0];
    if (out[i] < range[i][0]) {
      out[i] = range[i][0];
    } else if (out[i] > range[i][1]) {
      out[i] = range[i][1];
    }
  }
}

//------------------------------------------------------------------------
// ExponentialFunction
//------------------------------------------------------------------------

ExponentialFunction::ExponentialFunction(Object *funcObj, Dict *dict) {
  Object obj1, obj2;
  int i;

  ok = gFalse;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (m != 1) {
    error(-1, "Exponential function with more than one input");
    goto err1;
  }

  //----- C0
  if (dict->lookup("C0", &obj1)->isArray()) {
    if (hasRange && obj1.arrayGetLength() != n) {
      error(-1, "Function's C0 array is wrong length");
      goto err2;
    }
    n = obj1.arrayGetLength();
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function C0 array");
	goto err3;
      }
      c0[i] = obj2.getNum();
      obj2.free();
    }
  } else {
    if (hasRange && n != 1) {
      error(-1, "Function's C0 array is wrong length");
      goto err2;
    }
    n = 1;
    c0[0] = 0;
  }
  obj1.free();

  //----- C1
  if (dict->lookup("C1", &obj1)->isArray()) {
    if (obj1.arrayGetLength() != n) {
      error(-1, "Function's C1 array is wrong length");
      goto err2;
    }
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!obj2.isNum()) {
	error(-1, "Illegal value in function C1 array");
	goto err3;
      }
      c1[i] = obj2.getNum();
      obj2.free();
    }
  } else {
    if (n != 1) {
      error(-1, "Function's C1 array is wrong length");
      goto err2;
    }
    c1[0] = 1;
  }
  obj1.free();

  //----- N (exponent)
  if (!dict->lookup("N", &obj1)->isNum()) {
    error(-1, "Function has missing or invalid N");
    goto err2;
  }
  e = obj1.getNum();
  obj1.free();

  isLinear = fabs(e-1.) < 1e-10;
  ok = gTrue;
  return;

 err3:
  obj2.free();
 err2:
  obj1.free();
 err1:
  return;
}

ExponentialFunction::~ExponentialFunction() {
}

ExponentialFunction::ExponentialFunction(ExponentialFunction *func) {
  memcpy(this, func, sizeof(ExponentialFunction));
}

void ExponentialFunction::transform(double *in, double *out) {
  double x;
  int i;

  if (in[0] < domain[0][0]) {
    x = domain[0][0];
  } else if (in[0] > domain[0][1]) {
    x = domain[0][1];
  } else {
    x = in[0];
  }
  for (i = 0; i < n; ++i) {
    out[i] = c0[i] + (isLinear ? x : pow(x, e)) * (c1[i] - c0[i]);
    if (hasRange) {
      if (out[i] < range[i][0]) {
	out[i] = range[i][0];
      } else if (out[i] > range[i][1]) {
	out[i] = range[i][1];
      }
    }
  }
  return;
}

//------------------------------------------------------------------------
// StitchingFunction
//------------------------------------------------------------------------

StitchingFunction::StitchingFunction(Object *funcObj, Dict *dict, std::set<int> *usedParents) {
  Object obj1, obj2;
  int i;

  ok = gFalse;
  funcs = NULL;
  bounds = NULL;
  encode = NULL;
  scale = NULL;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (m != 1) {
    error(-1, "Stitching function with more than one input");
    goto err1;
  }

  //----- Functions
  if (!dict->lookup("Functions", &obj1)->isArray()) {
    error(-1, "Missing 'Functions' entry in stitching function");
    goto err1;
  }
  k = obj1.arrayGetLength();
  funcs = (Function **)gmallocn(k, sizeof(Function *));
  bounds = (double *)gmallocn(k + 1, sizeof(double));
  encode = (double *)gmallocn(2 * k, sizeof(double));
  scale = (double *)gmallocn(k, sizeof(double));
  for (i = 0; i < k; ++i) {
    funcs[i] = NULL;
  }
  for (i = 0; i < k; ++i) {
    std::set<int> usedParentsAux = *usedParents;
    obj1.arrayGetNF(i, &obj2);
    if (obj2.isRef()) {
      const Ref ref = obj2.getRef();
      if (usedParentsAux.find(ref.num) == usedParentsAux.end()) {
        usedParentsAux.insert(ref.num);
        obj2.free();
        obj1.arrayGet(i, &obj2);
      } else {
        goto err2;
      }
    }
    if (!(funcs[i] = Function::parse(&obj2, &usedParentsAux))) {
      goto err2;
    }
    if (i > 0 && (funcs[i]->getInputSize() != 1 ||
		  funcs[i]->getOutputSize() != funcs[0]->getOutputSize())) {
      error(-1, "Incompatible subfunctions in stitching function");
      goto err2;
    }
    obj2.free();
  }
  obj1.free();

  //----- Bounds
  if (!dict->lookup("Bounds", &obj1)->isArray() ||
      obj1.arrayGetLength() != k - 1) {
    error(-1, "Missing or invalid 'Bounds' entry in stitching function");
    goto err1;
  }
  bounds[0] = domain[0][0];
  for (i = 1; i < k; ++i) {
    if (!obj1.arrayGet(i - 1, &obj2)->isNum()) {
      error(-1, "Invalid type in 'Bounds' array in stitching function");
      goto err2;
    }
    bounds[i] = obj2.getNum();
    obj2.free();
  }
  bounds[k] = domain[0][1];
  obj1.free();

  //----- Encode
  if (!dict->lookup("Encode", &obj1)->isArray() ||
      obj1.arrayGetLength() != 2 * k) {
    error(-1, "Missing or invalid 'Encode' entry in stitching function");
    goto err1;
  }
  for (i = 0; i < 2 * k; ++i) {
    if (!obj1.arrayGet(i, &obj2)->isNum()) {
      error(-1, "Invalid type in 'Encode' array in stitching function");
      goto err2;
    }
    encode[i] = obj2.getNum();
    obj2.free();
  }
  obj1.free();

  //----- pre-compute the scale factors
  for (i = 0; i < k; ++i) {
    if (bounds[i] == bounds[i+1]) {
      // avoid a divide-by-zero -- in this situation, function i will
      // never be used anyway
      scale[i] = 0;
    } else {
      scale[i] = (encode[2*i+1] - encode[2*i]) / (bounds[i+1] - bounds[i]);
    }
  }

  ok = gTrue;
  return;

 err2:
  obj2.free();
 err1:
  obj1.free();
}

StitchingFunction::StitchingFunction(StitchingFunction *func) {
  int i;

  memcpy(this, func, sizeof(StitchingFunction));

  k = func->k;
  funcs = (Function **)gmallocn(k, sizeof(Function *));
  for (i = 0; i < k; ++i) {
    funcs[i] = func->funcs[i]->copy();
  }
  bounds = (double *)gmallocn(k + 1, sizeof(double));
  memcpy(bounds, func->bounds, (k + 1) * sizeof(double));
  encode = (double *)gmallocn(2 * k, sizeof(double));
  memcpy(encode, func->encode, 2 * k * sizeof(double));
  scale = (double *)gmallocn(k, sizeof(double));
  memcpy(scale, func->scale, k * sizeof(double));
  ok = gTrue;
}

StitchingFunction::~StitchingFunction() {
  int i;

  if (funcs) {
    for (i = 0; i < k; ++i) {
      if (funcs[i]) {
	delete funcs[i];
      }
    }
  }
  gfree(funcs);
  gfree(bounds);
  gfree(encode);
  gfree(scale);
}

void StitchingFunction::transform(double *in, double *out) {
  double x;
  int i;

  if (in[0] < domain[0][0]) {
    x = domain[0][0];
  } else if (in[0] > domain[0][1]) {
    x = domain[0][1];
  } else {
    x = in[0];
  }
  for (i = 0; i < k - 1; ++i) {
    if (x < bounds[i+1]) {
      break;
    }
  }
  x = encode[2*i] + (x - bounds[i]) * scale[i];
  funcs[i]->transform(&x, out);
}

//------------------------------------------------------------------------
// PostScriptFunction
//------------------------------------------------------------------------

enum PSOp {
  psOpAbs,
  psOpAdd,
  psOpAnd,
  psOpAtan,
  psOpBitshift,
  psOpCeiling,
  psOpCopy,
  psOpCos,
  psOpCvi,
  psOpCvr,
  psOpDiv,
  psOpDup,
  psOpEq,
  psOpExch,
  psOpExp,
  psOpFalse,
  psOpFloor,
  psOpGe,
  psOpGt,
  psOpIdiv,
  psOpIndex,
  psOpLe,
  psOpLn,
  psOpLog,
  psOpLt,
  psOpMod,
  psOpMul,
  psOpNe,
  psOpNeg,
  psOpNot,
  psOpOr,
  psOpPop,
  psOpRoll,
  psOpRound,
  psOpSin,
  psOpSqrt,
  psOpSub,
  psOpTrue,
  psOpTruncate,
  psOpXor,
  psOpIf,
  psOpIfelse,
  psOpReturn
};

// Note: 'if' and 'ifelse' are parsed separately.
// The rest are listed here in alphabetical order.
// The index in this table is equivalent to the entry in PSOp.
static const char *psOpNames[] = {
  "abs",
  "add",
  "and",
  "atan",
  "bitshift",
  "ceiling",
  "copy",
  "cos",
  "cvi",
  "cvr",
  "div",
  "dup",
  "eq",
  "exch",
  "exp",
  "false",
  "floor",
  "ge",
  "gt",
  "idiv",
  "index",
  "le",
  "ln",
  "log",
  "lt",
  "mod",
  "mul",
  "ne",
  "neg",
  "not",
  "or",
  "pop",
  "roll",
  "round",
  "sin",
  "sqrt",
  "sub",
  "true",
  "truncate",
  "xor"
};

#define nPSOps (sizeof(psOpNames) / sizeof(char *))

enum PSObjectType {
  psBool,
  psInt,
  psReal,
  psOperator,
  psBlock
};

// In the code array, 'if'/'ifelse' operators take up three slots
// plus space for the code in the subclause(s).
//
//         +---------------------------------+
//         | psOperator: psOpIf / psOpIfelse |
//         +---------------------------------+
//         | psBlock: ptr=<A>                |
//         +---------------------------------+
//         | psBlock: ptr=<B>                |
//         +---------------------------------+
//         | if clause                       |
//         | ...                             |
//         | psOperator: psOpReturn          |
//         +---------------------------------+
//     <A> | else clause                     |
//         | ...                             |
//         | psOperator: psOpReturn          |
//         +---------------------------------+
//     <B> | ...                             |
//
// For 'if', pointer <A> is present in the code stream but unused.

struct PSObject {
  PSObjectType type;
  union {
    GBool booln;		// boolean (stack only)
    int intg;			// integer (stack and code)
    double real;		// real (stack and code)
    PSOp op;			// operator (code only)
    int blk;			// if/ifelse block pointer (code only)
  };
};

#define psStackSize 100

class PSStack {
public:

  PSStack() {sp = psStackSize; }
  void clear() { sp = psStackSize; }
  void pushBool(GBool booln)
  {
    if (checkOverflow()) {
      stack[--sp].type = psBool;
      stack[sp].booln = booln;
    }
  }
  void pushInt(int intg)
  {
    if (checkOverflow()) {
      stack[--sp].type = psInt;
      stack[sp].intg = intg;
    }
  }
  void pushReal(double real)
  {
    if (checkOverflow()) {
      stack[--sp].type = psReal;
      stack[sp].real = real;
    }
  }
  GBool popBool()
  {
    if (checkUnderflow() && checkType(psBool, psBool)) {
      return stack[sp++].booln;
    }
    return gFalse;
  }
  int popInt()
  {
    if (checkUnderflow() && checkType(psInt, psInt)) {
      return stack[sp++].intg;
    }
    return 0;
  }
  double popNum()
  {
    double ret;

    if (checkUnderflow() && checkType(psInt, psReal)) {
      ret = (stack[sp].type == psInt) ? (double)stack[sp].intg : stack[sp].real;
      ++sp;
      return ret;
    }
    return 0;
  }
  GBool empty() { return sp == psStackSize; }
  GBool topIsInt() { return sp < psStackSize && stack[sp].type == psInt; }
  GBool topTwoAreInts()
    { return sp < psStackSize - 1 &&
	     stack[sp].type == psInt &&
             stack[sp+1].type == psInt; }
  GBool topIsReal() { return sp < psStackSize && stack[sp].type == psReal; }
  GBool topTwoAreNums()
    { return sp < psStackSize - 1 &&
	     (stack[sp].type == psInt || stack[sp].type == psReal) &&
	     (stack[sp+1].type == psInt || stack[sp+1].type == psReal); }
  void copy(int n);
  void roll(int n, int j);
  void index(int i)
  {
    if (!checkOverflow()) {
      return;
    }
    --sp;
    stack[sp] = stack[sp + 1 + i];
  }
  void pop()
  {
    if (!checkUnderflow()) {
      return;
    }
    ++sp;
  }

private:

  GBool checkOverflow(int n = 1)
  {
    if (sp - n < 0) {
      error(-1, "Stack overflow in PostScript function");
      return gFalse;
    }
    return gTrue;
  }
  GBool checkUnderflow()
  {
    if (sp == psStackSize) {
      error(-1, "Stack underflow in PostScript function");
      return gFalse;
    }
    return gTrue;
  }
  GBool checkType(PSObjectType t1, PSObjectType t2)
  {
    if (stack[sp].type != t1 && stack[sp].type != t2) {
      error(-1, "Type mismatch in PostScript function");
      return gFalse;
    }
    return gTrue;
  }
  PSObject stack[psStackSize];
  int sp;
};


void PSStack::copy(int n) {
  int i;

  if (sp + n > psStackSize) {
    error(-1, "Stack underflow in PostScript function");
    return;
  }
  if (!checkOverflow(n)) {
    return;
  }
  for (i = sp + n - 1; i >= sp; --i) {
    stack[i - n] = stack[i];
  }
  sp -= n;
}

void PSStack::roll(int n, int j) {
  PSObject obj;
  int i, k;

  if (unlikely(n == 0)) {
    return;
  }
  if (j >= 0) {
    j %= n;
  } else {
    j = -j % n;
    if (j != 0) {
      j = n - j;
    }
  }
  if (n <= 0 || j == 0) {
    return;
  }
  if (j <= n / 2) {
    for (i = 0; i < j; ++i) {
      obj = stack[sp];
      for (k = sp; k < sp + n - 1; ++k) {
        stack[k] = stack[k+1];
      }
      stack[sp + n - 1] = obj;
    }
  } else {
    j = n - j;
    for (i = 0; i < j; ++i) {
      obj = stack[sp + n - 1];
      for (k = sp + n - 1; k > sp; --k) {
        stack[k] = stack[k-1];
      }
      stack[sp] = obj;
    }
  }
}

class PostScriptFunctionKey : public PopplerCacheKey
{
  public:
    PostScriptFunctionKey(int sizeA, double *inA, bool copyA)
    {
      init(sizeA, inA, copyA);
    }
    
    PostScriptFunctionKey(const PostScriptFunctionKey &key)
    {
      init(key.size, key.in, key.copied);
    }
    
    void init(int sizeA, double *inA, bool copyA)
    {
      copied = copyA;
      size = sizeA;
      if (copied) {
        in = new double[size];
        for (int i = 0; i < size; ++i) in[i] = inA[i];
      } else {
        in = inA;
      }
    }
    
    ~PostScriptFunctionKey()
    {
      if (copied) delete[] in;
    }
       
    bool operator==(const PopplerCacheKey &key) const
    {
      const PostScriptFunctionKey *k = static_cast<const PostScriptFunctionKey*>(&key);
      if (size == k->size) {
        bool equal = true;
        for (int i = 0; equal && i < size; ++i) {
          equal = in[i] == k->in[i];
        }
        return equal;
      } else {
        return false;
      }
    }
  
    bool copied;
    int size;
    double *in;
};

class PostScriptFunctionItem : public PopplerCacheItem
{
  public:
    PostScriptFunctionItem(int sizeA, double *outA)
    {
      init(sizeA, outA);
    }
    
    PostScriptFunctionItem(const PostScriptFunctionItem &item)
    {
      init(item.size, item.out);
    }
    
    void init(int sizeA, double *outA)
    {
      size = sizeA;
      out = new double[size];
      for (int i = 0; i < size; ++i) out[i] = outA[i];
    }
    
    ~PostScriptFunctionItem()
    {
      delete[] out;
    }
    
    int size;
    double *out;
};

PostScriptFunction::PostScriptFunction(Object *funcObj, Dict *dict) {
  Stream *str;
  int codePtr;
  GooString *tok;

  code = NULL;
  codeString = NULL;
  codeSize = 0;
  stack = NULL;
  ok = gFalse;
  cache = new PopplerCache(5);

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (!hasRange) {
    error(-1, "Type 4 function is missing range");
    goto err1;
  }

  //----- get the stream
  if (!funcObj->isStream()) {
    error(-1, "Type 4 function isn't a stream");
    goto err1;
  }
  str = funcObj->getStream();

  //----- parse the function
  codeString = new GooString();
  str->reset();
  if (!(tok = getToken(str)) || tok->cmp("{")) {
    error(-1, "Expected '{' at start of PostScript function");
    if (tok) {
      delete tok;
    }
    goto err1;
  }
  delete tok;
  codePtr = 0;
  if (!parseCode(str, &codePtr)) {
    goto err2;
  }
  str->close();

  ok = gTrue;
  
  stack = new PSStack();

 err2:
  str->close();
 err1:
  return;
}

PostScriptFunction::PostScriptFunction(PostScriptFunction *func) {
  memcpy(this, func, sizeof(PostScriptFunction));
  code = (PSObject *)gmallocn(codeSize, sizeof(PSObject));
  memcpy(code, func->code, codeSize * sizeof(PSObject));
  codeString = func->codeString->copy();
  stack = new PSStack();
  memcpy(stack, func->stack, sizeof(PSStack));
  
  cache = new PopplerCache(func->cache->size());
  for (int i = 0; i < func->cache->numberOfItems(); ++i)
  {
    PostScriptFunctionKey *key = new PostScriptFunctionKey(*(PostScriptFunctionKey*)func->cache->key(i));
    PostScriptFunctionItem *item = new PostScriptFunctionItem(*(PostScriptFunctionItem*)func->cache->item(i));
    cache->put(key, item);
  }
}

PostScriptFunction::~PostScriptFunction() {
  gfree(code);
  delete codeString;
  delete stack;
  delete cache;
}

void PostScriptFunction::transform(double *in, double *out) {
  int i;
  
  PostScriptFunctionKey key(m, in, false);
  PopplerCacheItem *item = cache->lookup(key);
  if (item) {
    PostScriptFunctionItem *it = static_cast<PostScriptFunctionItem *>(item);
    for (int i = 0; i < n; ++i) {
      out[i] = it->out[i];
    }
    return;
  }

  stack->clear();
  for (i = 0; i < m; ++i) {
    //~ may need to check for integers here
    stack->pushReal(in[i]);
  }
  exec(stack, 0);
  for (i = n - 1; i >= 0; --i) {
    out[i] = stack->popNum();
    if (out[i] < range[i][0]) {
      out[i] = range[i][0];
    } else if (out[i] > range[i][1]) {
      out[i] = range[i][1];
    }
  }

  PostScriptFunctionKey *newKey = new PostScriptFunctionKey(m, in, true);
  PostScriptFunctionItem *newItem = new PostScriptFunctionItem(n, out);
  cache->put(newKey, newItem);
  
  // if (!stack->empty()) {
  //   error(-1, "Extra values on stack at end of PostScript function");
  // }
}

GBool PostScriptFunction::parseCode(Stream *str, int *codePtr) {
  GooString *tok;
  char *p;
  GBool isReal;
  int opPtr, elsePtr;
  int a, b, mid, cmp;

  while (1) {
    if (!(tok = getToken(str))) {
      error(-1, "Unexpected end of PostScript function stream");
      return gFalse;
    }
    p = tok->getCString();
    if (isdigit(*p) || *p == '.' || *p == '-') {
      isReal = gFalse;
      for (++p; *p; ++p) {
	if (*p == '.') {
	  isReal = gTrue;
	  break;
	}
      }
      resizeCode(*codePtr);
      if (isReal) {
	code[*codePtr].type = psReal;
          code[*codePtr].real = gatof(tok->getCString());
      } else {
	code[*codePtr].type = psInt;
	code[*codePtr].intg = atoi(tok->getCString());
      }
      ++*codePtr;
      delete tok;
    } else if (!tok->cmp("{")) {
      delete tok;
      opPtr = *codePtr;
      *codePtr += 3;
      resizeCode(opPtr + 2);
      if (!parseCode(str, codePtr)) {
	return gFalse;
      }
      if (!(tok = getToken(str))) {
	error(-1, "Unexpected end of PostScript function stream");
	return gFalse;
      }
      if (!tok->cmp("{")) {
	elsePtr = *codePtr;
	if (!parseCode(str, codePtr)) {
	  return gFalse;
	}
	delete tok;
	if (!(tok = getToken(str))) {
	  error(-1, "Unexpected end of PostScript function stream");
	  return gFalse;
	}
      } else {
	elsePtr = -1;
      }
      if (!tok->cmp("if")) {
	if (elsePtr >= 0) {
	  error(-1, "Got 'if' operator with two blocks in PostScript function");
	  return gFalse;
	}
	code[opPtr].type = psOperator;
	code[opPtr].op = psOpIf;
	code[opPtr+2].type = psBlock;
	code[opPtr+2].blk = *codePtr;
      } else if (!tok->cmp("ifelse")) {
	if (elsePtr < 0) {
	  error(-1, "Got 'ifelse' operator with one blocks in PostScript function");
	  return gFalse;
	}
	code[opPtr].type = psOperator;
	code[opPtr].op = psOpIfelse;
	code[opPtr+1].type = psBlock;
	code[opPtr+1].blk = elsePtr;
	code[opPtr+2].type = psBlock;
	code[opPtr+2].blk = *codePtr;
      } else {
	error(-1, "Expected if/ifelse operator in PostScript function");
	delete tok;
	return gFalse;
      }
      delete tok;
    } else if (!tok->cmp("}")) {
      delete tok;
      resizeCode(*codePtr);
      code[*codePtr].type = psOperator;
      code[*codePtr].op = psOpReturn;
      ++*codePtr;
      break;
    } else {
      a = -1;
      b = nPSOps;
      // invariant: psOpNames[a] < tok < psOpNames[b]
      while (b - a > 1) {
	mid = (a + b) / 2;
	cmp = tok->cmp(psOpNames[mid]);
	if (cmp > 0) {
	  a = mid;
	} else if (cmp < 0) {
	  b = mid;
	} else {
	  a = b = mid;
	}
      }
      if (cmp != 0) {
	error(-1, "Unknown operator '%s' in PostScript function",
	      tok->getCString());
	delete tok;
	return gFalse;
      }
      delete tok;
      resizeCode(*codePtr);
      code[*codePtr].type = psOperator;
      code[*codePtr].op = (PSOp)a;
      ++*codePtr;
    }
  }
  return gTrue;
}

GooString *PostScriptFunction::getToken(Stream *str) {
  GooString *s;
  int c;
  GBool comment;

  s = new GooString();
  comment = gFalse;
  while (1) {
    if ((c = str->getChar()) == EOF) {
      break;
    }
    codeString->append(c);
    if (comment) {
      if (c == '\x0a' || c == '\x0d') {
	comment = gFalse;
      }
    } else if (c == '%') {
      comment = gTrue;
    } else if (!isspace(c)) {
      break;
    }
  }
  if (c == '{' || c == '}') {
    s->append((char)c);
  } else if (isdigit(c) || c == '.' || c == '-') {
    while (1) {
      s->append((char)c);
      c = str->lookChar();
      if (c == EOF || !(isdigit(c) || c == '.' || c == '-')) {
	break;
      }
      str->getChar();
      codeString->append(c);
    }
  } else {
    while (1) {
      s->append((char)c);
      c = str->lookChar();
      if (c == EOF || !isalnum(c)) {
	break;
      }
      str->getChar();
      codeString->append(c);
    }
  }
  return s;
}

void PostScriptFunction::resizeCode(int newSize) {
  if (newSize >= codeSize) {
    codeSize += 64;
    code = (PSObject *)greallocn(code, codeSize, sizeof(PSObject));
  }
}

void PostScriptFunction::exec(PSStack *stack, int codePtr) {
  int i1, i2;
  double r1, r2, result;
  GBool b1, b2;

  while (1) {
    switch (code[codePtr].type) {
    case psInt:
      stack->pushInt(code[codePtr++].intg);
      break;
    case psReal:
      stack->pushReal(code[codePtr++].real);
      break;
    case psOperator:
      switch (code[codePtr++].op) {
      case psOpAbs:
	if (stack->topIsInt()) {
	  stack->pushInt(abs(stack->popInt()));
	} else {
	  stack->pushReal(fabs(stack->popNum()));
	}
	break;
      case psOpAdd:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushInt(i1 + i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushReal(r1 + r2);
	}
	break;
      case psOpAnd:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushInt(i1 & i2);
	} else {
	  b2 = stack->popBool();
	  b1 = stack->popBool();
	  stack->pushBool(b1 && b2);
	}
	break;
      case psOpAtan:
	r2 = stack->popNum();
	r1 = stack->popNum();
	result = atan2(r1, r2) * 180.0 / M_PI;
	if (result < 0) result += 360.0;
	stack->pushReal(result);
	break;
      case psOpBitshift:
	i2 = stack->popInt();
	i1 = stack->popInt();
	if (i2 > 0) {
	  stack->pushInt(i1 << i2);
	} else if (i2 < 0) {
	  stack->pushInt((int)((Guint)i1 >> i2));
	} else {
	  stack->pushInt(i1);
	}
	break;
      case psOpCeiling:
	if (!stack->topIsInt()) {
	  stack->pushReal(ceil(stack->popNum()));
	}
	break;
      case psOpCopy:
	stack->copy(stack->popInt());
	break;
      case psOpCos:
	stack->pushReal(cos(stack->popNum() * M_PI / 180.0));
	break;
      case psOpCvi:
	if (!stack->topIsInt()) {
	  stack->pushInt((int)stack->popNum());
	}
	break;
      case psOpCvr:
	if (!stack->topIsReal()) {
	  stack->pushReal(stack->popNum());
	}
	break;
      case psOpDiv:
	r2 = stack->popNum();
	r1 = stack->popNum();
	stack->pushReal(r1 / r2);
	break;
      case psOpDup:
	stack->copy(1);
	break;
      case psOpEq:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushBool(i1 == i2);
	} else if (stack->topTwoAreNums()) {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushBool(r1 == r2);
	} else {
	  b2 = stack->popBool();
	  b1 = stack->popBool();
	  stack->pushBool(b1 == b2);
	}
	break;
      case psOpExch:
	stack->roll(2, 1);
	break;
      case psOpExp:
	r2 = stack->popNum();
	r1 = stack->popNum();
	stack->pushReal(pow(r1, r2));
	break;
      case psOpFalse:
	stack->pushBool(gFalse);
	break;
      case psOpFloor:
	if (!stack->topIsInt()) {
	  stack->pushReal(floor(stack->popNum()));
	}
	break;
      case psOpGe:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushBool(i1 >= i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushBool(r1 >= r2);
	}
	break;
      case psOpGt:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushBool(i1 > i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushBool(r1 > r2);
	}
	break;
      case psOpIdiv:
	i2 = stack->popInt();
	i1 = stack->popInt();
	stack->pushInt(i1 / i2);
	break;
      case psOpIndex:
	stack->index(stack->popInt());
	break;
      case psOpLe:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushBool(i1 <= i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushBool(r1 <= r2);
	}
	break;
      case psOpLn:
	stack->pushReal(log(stack->popNum()));
	break;
      case psOpLog:
	stack->pushReal(log10(stack->popNum()));
	break;
      case psOpLt:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushBool(i1 < i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushBool(r1 < r2);
	}
	break;
      case psOpMod:
	i2 = stack->popInt();
	i1 = stack->popInt();
	stack->pushInt(i1 % i2);
	break;
      case psOpMul:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  //~ should check for out-of-range, and push a real instead
	  stack->pushInt(i1 * i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushReal(r1 * r2);
	}
	break;
      case psOpNe:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushBool(i1 != i2);
	} else if (stack->topTwoAreNums()) {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushBool(r1 != r2);
	} else {
	  b2 = stack->popBool();
	  b1 = stack->popBool();
	  stack->pushBool(b1 != b2);
	}
	break;
      case psOpNeg:
	if (stack->topIsInt()) {
	  stack->pushInt(-stack->popInt());
	} else {
	  stack->pushReal(-stack->popNum());
	}
	break;
      case psOpNot:
	if (stack->topIsInt()) {
	  stack->pushInt(~stack->popInt());
	} else {
	  stack->pushBool(!stack->popBool());
	}
	break;
      case psOpOr:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushInt(i1 | i2);
	} else {
	  b2 = stack->popBool();
	  b1 = stack->popBool();
	  stack->pushBool(b1 || b2);
	}
	break;
      case psOpPop:
	stack->pop();
	break;
      case psOpRoll:
	i2 = stack->popInt();
	i1 = stack->popInt();
	stack->roll(i1, i2);
	break;
      case psOpRound:
	if (!stack->topIsInt()) {
	  r1 = stack->popNum();
	  stack->pushReal((r1 >= 0) ? floor(r1 + 0.5) : ceil(r1 - 0.5));
	}
	break;
      case psOpSin:
	stack->pushReal(sin(stack->popNum() * M_PI / 180.0));
	break;
      case psOpSqrt:
	stack->pushReal(sqrt(stack->popNum()));
	break;
      case psOpSub:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushInt(i1 - i2);
	} else {
	  r2 = stack->popNum();
	  r1 = stack->popNum();
	  stack->pushReal(r1 - r2);
	}
	break;
      case psOpTrue:
	stack->pushBool(gTrue);
	break;
      case psOpTruncate:
	if (!stack->topIsInt()) {
	  r1 = stack->popNum();
	  stack->pushReal((r1 >= 0) ? floor(r1) : ceil(r1));
	}
	break;
      case psOpXor:
	if (stack->topTwoAreInts()) {
	  i2 = stack->popInt();
	  i1 = stack->popInt();
	  stack->pushInt(i1 ^ i2);
	} else {
	  b2 = stack->popBool();
	  b1 = stack->popBool();
	  stack->pushBool(b1 ^ b2);
	}
	break;
      case psOpIf:
	b1 = stack->popBool();
	if (b1) {
	  exec(stack, codePtr + 2);
	}
	codePtr = code[codePtr + 1].blk;
	break;
      case psOpIfelse:
	b1 = stack->popBool();
	if (b1) {
	  exec(stack, codePtr + 2);
	} else {
	  exec(stack, code[codePtr].blk);
	}
	codePtr = code[codePtr + 1].blk;
	break;
      case psOpReturn:
	return;
      }
      break;
    default:
      error(-1, "Internal: bad object in PostScript function code");
      break;
    }
  }
}
