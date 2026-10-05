// Constants of various primitive types
const NUM = 42;
const FLOAT = 3.14;
const BOOL_T = true;
const BOOL_F = false;
const NEG = -1;
const BIG = 100n;
const NEG_BIG = -1n;
const ALIAS = NUM;
const MULTI_A = 1, MULTI_B = 2;
const UNUSED = 999;
export const EXPORTED_CONST = 77;

// Uses in expressions
function testExpressions(cmd) {
  if (cmd === NUM) return BOOL_T;
  if (cmd < FLOAT) return BOOL_F;
  if (cmd === NEG) return NEG_BIG;
  switch (cmd) {
    case MULTI_A: return 10;
    case MULTI_B: return 20;
  }
  return ALIAS + BIG;
}

// Uses in objects
const obj = {
  NUM,
  prop: FLOAT,
  [NEG]: "computed",
  NUM: "uncomputed key should not be changed"
};

// Uses in member expressions
function testMember(x) {
  var a = x[NUM];
  var b = x.NUM;
  return a + b;
}

// Shadowing: parameter shadows outer const
function testParamShadow(NUM) {
  return NUM;
}

// Shadowing: local const shadows outer const
function testLocalConstShadow() {
  const NUM = 100;
  return NUM;
}

// Shadowing: local var shadows outer const
function testLocalVarShadow() {
  if (true) {
    var NUM = 200;
  }
  return NUM;
}

// Shadowing: catch parameter shadows outer const
function testCatchShadow() {
  try {
    throw 1;
  } catch (NUM) {
    return NUM;
  }
}

// Arrow function with concise body
const arrow = (x) => x + NUM;

// Export specifier
export { NUM };
