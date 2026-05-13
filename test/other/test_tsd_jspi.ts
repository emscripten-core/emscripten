import moduleFactory from './test_emit_tsd_jspi.js'

async function go() {
  const module = await moduleFactory();
  await module._fooVoid();
  let result = await module._fooInt(7, 8);
  let mainResult = await module._main(0, 0);
  module.UTF8ArrayToString([], 99);
}
