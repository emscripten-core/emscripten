// js -m -n -e "load('lzma-full.js')" test-full.js

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
  assertEq(a.length, b.length);
  for (var i = 0; i < a.length; i++) {
    assertEq(a[i]&255, b[i]&255);
  }
}

function testSimple() {
  print('testing simple..');
  var data = [100, 200, 200, 200, 200, 200, 200, 100, 100, 200, 200, 200, 200, 0, 1];
  var compressed = LZMA.compress(data);
  var decompressed = LZMA.decompress(compressed);

  byteCompare(data, decompressed);
  assertNeq(data.length, compressed.length);
}

function testBig() {
  print('testing big..');
  var seed1 = 100;
  var seed2 = 200;
  var last = 255;
  function fakeRandom() {
    // numbers from http://triptico.com/docs/sp_random.html
    seed1 = ((seed1 * 58321) + 11113) | 0;
    var ret = (seed1 >> 16) & 255;
    seed2 = ((seed2 * 58321) + 11113) | 0;
    if (seed2 % 5) {
      return last;
    }
    last = ret;
    return last;
  }
  print('           ..generating data..');
  var size = 1*1024*1024;
  var data = new Array(size);
  for (var i = 0; i < size; i++) {
    data[i] = fakeRandom();
  }

  print('           ..compressing ' + data.length + ' bytes..');
  var t = Date.now();
  var compressed = LZMA.compress(data);
  print('           ..took ' + ((Date.now() - t)/1000).toFixed(2) + ' secs');
  print('           ..decompressing ' + compressed.length + ' bytes..');
  t = Date.now();
  var decompressed = LZMA.decompress(compressed);
  print('           ..took ' + ((Date.now() - t)/1000).toFixed(2) + ' secs');
  print('           ..got ' + decompressed.length + ' bytes..');

  byteCompare(data, decompressed);
  assertNeq(data.length, compressed.length);
  print('           ..decompressed == original');
}

testSimple();
testBig();

print('ok.');

