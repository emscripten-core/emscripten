// js -m -n -e "load('lzma-decoder.js')" test-decoder.js

function assertEq(a, b) {
  if (a !== b) {
    throw 'Should have been equal: ' + a + ' : ' + b;
  }
  return false;
}

function assertNeq(a, b) {
  try {
    assertEq(a, b);
  } catch(e) {
    return;
  }
  throw 'Should have not been equal: ' + a + ' : ' + b;
}

function byteCompare(a, b) {
  assertEq(JSON.stringify(new Uint8Array(a)), JSON.stringify(new Uint8Array(b)));
}

function testSimple() {
  print('testing simple..');
  var data = [100, 200, 200, 200, 200, 200, 200, 100, 100, 200, 200, 200, 200, 0, 1];
  var compressed = [76,90,73,80,1,12,0,50,50,28,-1,-1,-1,-1,-16,0,0,0,-34,-10,-9,-43,2,0,0,0,0,0,0,0,38,0,0,0,0,0,
    0,0,76,90,73,80,1,12,0,100,105,61,-1,-1,-1,-1,-32,0,0,0,-46,66,-98,-91,5,0,0,0,0,0,0,0,38,
    0,0,0,0,0,0,0,76,90,73,80,1,12,0,50,100,27,-1,-1,-1,-2,0,0,0,29,25,-105,10,2,0,0,0,0,0,0,0,
    37,0,0,0,0,0,0,0,76,90,73,80,1,12,0,100,104,-67,-1,-1,-1,-1,-32,0,0,0,-55,-55,-99,-101,4,0,0,0,
    0,0,0,0,38,0,0,0,0,0,0,0,76,90,73,80,1,12,0,0,0,121,-128,-83,-1,-1,-20,-57,0,0,105,34,-34,54,2,
    0,0,0,0,0,0,0,38,0,0,0,0,0,0,0];
  var decompressed = LZMA.decompress(compressed);
  byteCompare(data, decompressed);
}

testSimple();

print('ok.');

