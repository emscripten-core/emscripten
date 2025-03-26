import moduleFactory from './test_emit_tsd.js'

async function go() {
  const module = await moduleFactory();
  module._fooVoid();
  let result = module._fooInt(7, 8);
  module.UTF8ArrayToString([], 99);
}
