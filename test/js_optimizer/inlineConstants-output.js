const NUM = 42;

const FLOAT = 3.14;

const BOOL_T = true;

const BOOL_F = false;

const NEG = -1;

const BIG = 100n;

const NEG_BIG = -1n;

const ALIAS = 42;

const MULTI_A = 1, MULTI_B = 2;

const UNUSED = 999;

export const EXPORTED_CONST = 77;

function testExpressions(cmd) {
  if (cmd === 42) return true;
  if (cmd < 3.14) return false;
  if (cmd === -1) return -1n;
  switch (cmd) {
   case 1:
    return 10;

   case 2:
    return 20;
  }
  return 42 + 100n;
}

const obj = {
  NUM: 42,
  prop: 3.14,
  [-1]: "computed",
  NUM: "uncomputed key should not be changed"
};

function testMember(x) {
  var a = x[42];
  var b = x.NUM;
  return a + b;
}

function testParamShadow(NUM) {
  return NUM;
}

function testLocalConstShadow() {
  const NUM = 100;
  return 100;
}

function testLocalVarShadow() {
  if (true) {
    var NUM = 200;
  }
  return NUM;
}

function testCatchShadow() {
  try {
    throw 1;
  } catch (NUM) {
    return NUM;
  }
}

const arrow = x => x + 42;

export { NUM };
