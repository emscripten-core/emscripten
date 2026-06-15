async function run() {
  console.log('START');
  let bufferCollected = false;
  const registry = new FinalizationRegistry(() => {
    bufferCollected = true;
  });

  await (async () => {
    const instance = await Module();
    console.log('GOT INSTANCE');
    // Once this scope exits we expect the instance and its memory to be
    // collected.
    registry.register(instance.wasmMemory.buffer, "wasmMemory");
  })();

  // Force GC
  const assert = require('node:assert/strict');
  assert(global.gc);
  for (let i = 0; i < 20; i++) {
    global.gc();
    await new Promise(r => setTimeout(r, 50));
    if (bufferCollected) break;
  }

  if (bufferCollected) {
    console.log('SUCCESS: No leak detected');
    process.exit(0);
  } else {
    console.log('FAILURE: Leak detected');
    process.exit(1);
  }
}

if (!isPthread)
  run();
