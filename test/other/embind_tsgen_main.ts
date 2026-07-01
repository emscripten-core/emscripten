// Example TS program that consumes the emscripten-generated module to
// illustrate how the type definitions are used and test they are working as
// expected.

// The imported file will either be an ES module or a CommonJS module depending
// on the test.
import moduleFactory from './embind_tsgen.js';

// Async IIFE is required for TSC with commonjs modules. This is not needed for
// ESM output since top level await can be used.
(async function() {

  const module = await moduleFactory();

  module.setValObj({
    firstEnum: module.FirstEnum.kValueOne,
    secondEnum: module.SecondEnum.kValueA,
    thirdEnum: module.ThirdEnum.kValueAlpha,
    string: "ABCD",
    callback: () => {}
  });

  // Test a few variations of passing enum value
  module.setValObj({
    firstEnum: module.FirstEnum.kValueOne,
    secondEnum: 0,
    thirdEnum: "kValueAlpha",
    string: new Int8Array([65, 66, 67, 68]),
    callback: () => {}
  });

  // Test optional field
  module.setValObj({
    firstEnum: module.FirstEnum.kValueOne,
    secondEnum: module.SecondEnum.kValueA,
    thirdEnum: module.ThirdEnum.kValueAlpha,
    string: "ABCD",
    callback: () => {},
    optionalInt: 99
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
