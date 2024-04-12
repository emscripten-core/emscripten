addOnPostRun(() => {
  // check >4gb alloc
  const bigChunk = _malloc(4 * 1024 * 1024 * 1024 + 100);
  assert(bigChunk > 0);

  const littleChunk = _malloc(100);
  HEAP8[littleChunk] = 2;
  assert(HEAP8[littleChunk] === 2);

  // .subarray
  const subarray = HEAP8.subarray(littleChunk, littleChunk + 100);
  assert(subarray[0] === 2);

  // check .fill
  HEAP8.fill(3, littleChunk, littleChunk + 99);
  assert(subarray[0] === 3);
  assert(subarray[98] === 3);
  assert(subarray[99] === 0);
  assert(HEAP8[littleChunk] === 3);
  assert(HEAP8[littleChunk + 98] === 3);
  assert(HEAP8[littleChunk + 99] === 0);

  // check .set
  const filler = new Uint8Array(10);
  filler[0] = 4;
  filler[9] = 4;
  HEAP8.set(filler, littleChunk, 10);
  assert(subarray[0] === 4);
  assert(subarray[9] === 4);
  assert(HEAP8[littleChunk] === 4);

  // .copyWithin
  HEAP8.copyWithin(bigChunk, littleChunk, littleChunk + 100);
  assert(HEAP8[bigChunk] === 4);

  // .slice
  const slice = HEAP8.slice(bigChunk, bigChunk + 100);
  slice[0] = 5;
  assert(HEAP8[bigChunk] === 4);
});
