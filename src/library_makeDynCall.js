/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $createDyncallWrapper__deps: ['$generateFuncType', '$uleb128Encode', 'setTempRet0', '$wasmTable'],
  $createDyncallWrapper: (sig) => {
    var sections = [];
    var prelude = [
      0x00, 0x61, 0x73, 0x6d, // magic ("\0asm")
      0x01, 0x00, 0x00, 0x00, // version: 1
    ];
    sections.push(prelude);
    var wrappersig = [
      // if return type is j, we will put the upper 32 bits into tempRet0.
      sig[0].replace("j", "i"),
      "i", // The first argument is the function pointer to call
      // in the rest of the argument list, one 64 bit integer is legalized into
      // two 32 bit integers.
      sig.slice(1).replace(/j/g, "ii")
    ].join("");

    var typeSectionBody = [
      0x03, // number of types = 3
    ];
    generateFuncType(wrappersig, typeSectionBody); // The signature of the wrapper we are generating
    generateFuncType(sig, typeSectionBody); // the signature of the function pointer we will call
    generateFuncType("vi", typeSectionBody); // the signature of setTempRet0

    var typeSection = [0x01 /* Type section code */];
    uleb128Encode(typeSectionBody.length, typeSection); // length of section in bytes
    typeSection.push(...typeSectionBody);
    sections.push(typeSection);

    var importSection = [
      0x02, // import section code
      0x0F, // length of section in bytes
      0x02, // number of imports = 2
      // Import the wasmTable, which we will call "t"
      0x01, 0x65, // name "e"
      0x01, 0x74, // name "t"
      0x01, 0x70, // importing a table
      0x00, // with no max # of elements
      0x00, // and min of 0 elements
      // Import the setTempRet0 function, which we will call "r"
      0x01, 0x65, // name "e"
      0x01, 0x72, // name "r"
      0x00, // importing a function
      0x02, // type 2
    ];
    sections.push(importSection);

    var functionSection = [
      0x03, // function section code
      0x02, // length of section in bytes
      0x01, // number of functions = 1
      0x00, // type 0 = wrappersig
    ];
    sections.push(functionSection);

    var exportSection = [
      0x07, // export section code
      0x05, // length of section in bytes
      0x01, // One export
      0x01, 0x66, // name "f"
      0x00, // type: function
      0x01, // function index 1 = the wrapper function (index 0 is setTempRet0)
    ];
    sections.push(exportSection);

    var convert_code = [];
    if (sig[0] === "j") {
      // Add a single extra i64 local. In order to legalize the return value we
      // need a local to store it in. Local variables are run length encoded.
      convert_code = [
        0x01, // One run
        0x01, // of length 1
        0x7e, // of i64
      ];
    } else {
      convert_code.push(0x00); // no local variables (except the arguments)
    }

    function localGet(j) {
      convert_code.push(0x20); // local.get
      uleb128Encode(j, convert_code);
    }

    var j = 1;
    for (var i = 1; i < sig.length; i++) {
      if (sig[i] == "j") {
        localGet(j + 1);
        convert_code.push(
          0xad, // i64.extend_i32_unsigned
          0x42, 0x20, // i64.const 32
          0x86, // i64.shl,
        )
        localGet(j);
        convert_code.push(
          0xac, // i64.extend_i32_signed
          0x84, // i64.or
        );
        j+=2;
      } else {
        localGet(j);
        j++;
      }
    }

    convert_code.push(
      0x20, 0x00, // local.get 0 (put function pointer on stack)
      0x11, 0x01, 0x00, // call_indirect type 1 = wrapped_sig, table 0 = only table
    );
if (sig[0] === "j") {
    // tee into j (after the argument handling loop, j is one past the
    // argument list so it points to the i64 local we added)
      convert_code.push(0x22);
      uleb128Encode(j, convert_code);
      convert_code.push(
        0x42, 0x20, // i64.const 32
        0x88, // i64.shr_u
        0xa7, // i32.wrap_i64
        0x10, 0x00, // Call function 0
      );
      localGet(j);
      convert_code.push(
        0xa7, // i32.wrap_i64
      );
    }
    convert_code.push(0x0b); // end

    var codeBody = [0x01]; // one code
    uleb128Encode(convert_code.length, codeBody);
    codeBody.push(...convert_code);
    var codeSection = [0x0A /* Code section code */];
    uleb128Encode(codeBody.length, codeSection);
    codeSection.push(...codeBody);
    sections.push(codeSection);

    var bytes = new Uint8Array([].concat.apply([], sections));
    // We can compile this wasm module synchronously because it is small.
    var module = new WebAssembly.Module(bytes);
    var instance = new WebAssembly.Instance(module, {
    'e': {
      't': wasmTable,
      'r': setTempRet0,
    }
    });
    var wrappedFunc = instance.exports['f'];
    return wrappedFunc;
  },
});
