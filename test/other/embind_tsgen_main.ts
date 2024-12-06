// Example TS program that consumes the emscripten-generated module to to
// illustrate how the type definitions are used and test they are workings as
// expected.

// The imported file will either be an ES module or a CommonJS module depending
// on the test.
import moduleFactory from './embind_tsgen.js';

// Async IIFE is required for TSC with commonjs modules. This is not needed for
// ESM output since top level await can be used.
(async function() {

  const module = await moduleFactory();

  // Test a few variations of passing value_objects with strings.
  module.setValObj({
    bar: module.Bar.valueOne,
    string: "ABCD",
    callback: () => {}
  });

  module.setValObj({
    bar: module.Bar.valueOne,
    string: new Int8Array([65, 66, 67, 68]),
    callback: () => {}
  });

  const valObj = module.getValObj();
  // TODO: remove the cast below when better definitions are generated for value
  // objects.
  const valString : string = valObj.string as string;

  // Ensure nonnull pointers do no need a cast or nullptr check to use.
  const obj = module.getNonnullPointer();
  obj.delete();

  console.log('ts ran');

})();
