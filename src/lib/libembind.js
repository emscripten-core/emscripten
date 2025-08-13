// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "libembind_shared.js"

var LibraryEmbind = {
  $UnboundTypeError: class extends Error {},
  $PureVirtualError: class extends Error {},
#if EMBIND_AOT
  $InvokerFunctions: '<<< EMBIND_AOT_INVOKERS >>>',
#endif
  // If register_type is used, emval will be registered multiple times for
  // different type id's, but only a single type object is needed on the JS side
  // for all of them. Store the type for reuse.
  $EmValType__deps: ['_emval_decref', '$Emval', '$readPointer'],
  $EmValType: `{
    name: 'emscripten::val',
    fromWireType: (handle) => {
      var rv = Emval.toValue(handle);
      __emval_decref(handle);
      return rv;
    },
    toWireType: (destructors, value) => Emval.toHandle(value),
    readValueFromPointer: readPointer,
    destructorFunction: null, // This type does not need a destructor

    // TODO: do we need a deleteObject here?  write a test where
    // emval is passed into JS via an interface
  }`,
  $EmValOptionalType__deps: ['$EmValType'],
  $EmValOptionalType: '=Object.assign({optional: true}, EmValType);',

  $throwUnboundTypeError__deps: ['$registeredTypes', '$typeDependencies', '$UnboundTypeError', '$getTypeName'],
  $throwUnboundTypeError: (message, types) => {
    var unboundTypes = [];
    var seen = {};
    function visit(type) {
      if (seen[type]) {
        return;
      }
      if (registeredTypes[type]) {
        return;
      }
      if (typeDependencies[type]) {
        typeDependencies[type].forEach(visit);
        return;
      }
      unboundTypes.push(type);
      seen[type] = true;
    }
    types.forEach(visit);

    throw new UnboundTypeError(`${message}: ` + unboundTypes.map(getTypeName).join([', ']));
  },

  // Creates a function overload resolution table to the given method 'methodName' in the given prototype,
  // if the overload table doesn't yet exist.
  $ensureOverloadTable__deps: ['$throwBindingError'],
  $ensureOverloadTable: (proto, methodName, humanName) => {
    if (undefined === proto[methodName].overloadTable) {
      var prevFunc = proto[methodName];
      // Inject an overload resolver function that routes to the appropriate overload based on the number of arguments.
      proto[methodName] = function(...args) {
        // TODO This check can be removed in -O3 level "unsafe" optimizations.
        if (!proto[methodName].overloadTable.hasOwnProperty(args.length)) {
          throwBindingError(`Function '${humanName}' called with an invalid number of arguments (${args.length}) - expects one of (${proto[methodName].overloadTable})!`);
        }
        return proto[methodName].overloadTable[args.length].apply(this, args);
      };
      // Move the previous function into the overload table.
      proto[methodName].overloadTable = [];
      proto[methodName].overloadTable[prevFunc.argCount] = prevFunc;
    }
  },

  /*
   Registers a symbol (function, class, enum, ...) as part of the Module JS object so that
   hand-written code is able to access that symbol via 'Module.name'.
   name: The name of the symbol that's being exposed.
   value: The object itself to expose (function, class, ...)
   numArguments: For functions, specifies the number of arguments the function takes in. For other types, unused and undefined.

   To implement support for multiple overloads of a function, an 'overload selector' function is used. That selector function chooses
   the appropriate overload to call from an function overload table. This selector function is only used if multiple overloads are
   actually registered, since it carries a slight performance penalty. */
  $exposePublicSymbol__deps: ['$ensureOverloadTable', '$throwBindingError'],
  $exposePublicSymbol__docs: '/** @param {number=} numArguments */',
  $exposePublicSymbol: (name, value, numArguments) => {
    if (Module.hasOwnProperty(name)) {
      if (undefined === numArguments || (undefined !== Module[name].overloadTable && undefined !== Module[name].overloadTable[numArguments])) {
        throwBindingError(`Cannot register public name '${name}' twice`);
      }

      // We are exposing a function with the same name as an existing function. Create an overload table and a function selector
      // that routes between the two.
      ensureOverloadTable(Module, name, name);
      if (Module[name].overloadTable.hasOwnProperty(numArguments)) {
        throwBindingError(`Cannot register multiple overloads of a function with the same number of arguments (${numArguments})!`);
      }
      // Add the new function into the overload table.
      Module[name].overloadTable[numArguments] = value;
    } else {
      Module[name] = value;
      Module[name].argCount = numArguments;
    }
  },

  $replacePublicSymbol__deps: ['$throwInternalError'],
  $replacePublicSymbol__docs: '/** @param {number=} numArguments */',
  $replacePublicSymbol: (name, value, numArguments) => {
    if (!Module.hasOwnProperty(name)) {
      throwInternalError('Replacing nonexistent public symbol');
    }
    // If there's an overload table for this symbol, replace the symbol in the overload table instead.
    if (undefined !== Module[name].overloadTable && undefined !== numArguments) {
      Module[name].overloadTable[numArguments] = value;
    } else {
      Module[name] = value;
      Module[name].argCount = numArguments;
    }
  },

  $createNamedFunction: (name, func) => Object.defineProperty(func, 'name', { value: name }),

  $embindRepr: (v) => {
    if (v === null) {
        return 'null';
    }
    var t = typeof v;
    if (t === 'object' || t === 'array' || t === 'function') {
        return v.toString();
    } else {
        return '' + v;
    }
  },

  // raw pointer -> instance
  $registeredInstances: {},

  $getBasestPointer__deps: ['$throwBindingError'],
  $getBasestPointer: (class_, ptr) => {
    if (ptr === undefined) {
        throwBindingError('ptr should not be undefined');
    }
    while (class_.baseClass) {
        ptr = class_.upcast(ptr);
        class_ = class_.baseClass;
    }
    return ptr;
  },

  $registerInheritedInstance__deps: ['$registeredInstances', '$getBasestPointer', '$throwBindingError'],
  $registerInheritedInstance: (class_, ptr, instance) => {
    ptr = getBasestPointer(class_, ptr);
    if (registeredInstances.hasOwnProperty(ptr)) {
        throwBindingError(`Tried to register registered instance: ${ptr}`);
    } else {
        registeredInstances[ptr] = instance;
    }
  },

  $unregisterInheritedInstance__deps: ['$registeredInstances', '$getBasestPointer', '$throwBindingError'],
  $unregisterInheritedInstance: (class_, ptr) => {
    ptr = getBasestPointer(class_, ptr);
    if (registeredInstances.hasOwnProperty(ptr)) {
        delete registeredInstances[ptr];
    } else {
        throwBindingError(`Tried to unregister unregistered instance: ${ptr}`);
    }
  },

  $getInheritedInstance__deps: ['$registeredInstances', '$getBasestPointer'],
  $getInheritedInstance: (class_, ptr) => {
    ptr = getBasestPointer(class_, ptr);
    return registeredInstances[ptr];
  },

  $getInheritedInstanceCount__deps: ['$registeredInstances'],
  $getInheritedInstanceCount: () => Object.keys(registeredInstances).length,

  $getLiveInheritedInstances__deps: ['$registeredInstances'],
  $getLiveInheritedInstances: () => {
    var rv = [];
    for (var k in registeredInstances) {
      if (registeredInstances.hasOwnProperty(k)) {
        rv.push(registeredInstances[k]);
      }
    }
    return rv;
  },

  // class typeID -> {pointerType: ..., constPointerType: ...}
  $registeredPointers: {},

  $registerType__deps: ['$sharedRegisterType'],
  $registerType__docs: '/** @param {Object=} options */',
  $registerType: function(rawType, registeredInstance, options = {}) {
    return sharedRegisterType(rawType, registeredInstance, options);
  },

  _embind_register_void__deps: ['$AsciiToString', '$registerType'],
  _embind_register_void: (rawType, name) => {
    name = AsciiToString(name);
    registerType(rawType, {
      isVoid: true, // void return values can be optimized out sometimes
      name,
      fromWireType: () => undefined,
      // TODO: assert if anything else is given?
      toWireType: (destructors, o) => undefined,
    });
  },

  _embind_register_bool__docs: '/** @suppress {globalThis} */',
  _embind_register_bool__deps: ['$AsciiToString', '$registerType'],
  _embind_register_bool: (rawType, name, trueValue, falseValue) => {
    name = AsciiToString(name);
    registerType(rawType, {
      name,
      fromWireType: function(wt) {
        // ambiguous emscripten ABI: sometimes return values are
        // true or false, and sometimes integers (0 or 1)
        return !!wt;
      },
      toWireType: function(destructors, o) {
        return o ? trueValue : falseValue;
      },
      readValueFromPointer: function(pointer) {
        return this.fromWireType(HEAPU8[pointer]);
      },
      destructorFunction: null, // This type does not need a destructor
    });
  },

  $integerReadValueFromPointer__deps: [],
  $integerReadValueFromPointer: (name, width, signed) => {
    // integers are quite common, so generate very specialized functions
    switch (width) {
      case 1: return signed ?
        (pointer) => {{{ makeGetValue('pointer', 0, 'i8') }}} :
        (pointer) => {{{ makeGetValue('pointer', 0, 'u8') }}};
      case 2: return signed ?
        (pointer) => {{{ makeGetValue('pointer', 0, 'i16') }}} :
        (pointer) => {{{ makeGetValue('pointer', 0, 'u16') }}}
      case 4: return signed ?
        (pointer) => {{{ makeGetValue('pointer', 0, 'i32') }}} :
        (pointer) => {{{ makeGetValue('pointer', 0, 'u32') }}}
#if WASM_BIGINT
      case 8: return signed ?
        (pointer) => {{{ makeGetValue('pointer', 0, 'i64') }}} :
        (pointer) => {{{ makeGetValue('pointer', 0, 'u64') }}}
#endif
      default:
        throw new TypeError(`invalid integer width (${width}): ${name}`);
    }
  },

  $enumReadValueFromPointer__deps: [],
  $enumReadValueFromPointer: (name, width, signed) => {
    switch (width) {
      case 1: return signed ?
        function(pointer) { return this.fromWireType({{{ makeGetValue('pointer', 0, 'i8') }}}) } :
        function(pointer) { return this.fromWireType({{{ makeGetValue('pointer', 0, 'u8') }}}) };
      case 2: return signed ?
        function(pointer) { return this.fromWireType({{{ makeGetValue('pointer', 0, 'i16') }}}) } :
        function(pointer) { return this.fromWireType({{{ makeGetValue('pointer', 0, 'u16') }}}) };
      case 4: return signed ?
        function(pointer) { return this.fromWireType({{{ makeGetValue('pointer', 0, 'i32') }}}) } :
        function(pointer) { return this.fromWireType({{{ makeGetValue('pointer', 0, 'u32') }}}) };
      default:
        throw new TypeError(`invalid integer width (${width}): ${name}`);
    }
  },

  $floatReadValueFromPointer__deps: [],
  $floatReadValueFromPointer: (name, width) => {
    switch (width) {
      case 4: return function(pointer) {
        return this.fromWireType({{{ makeGetValue('pointer', 0, 'float') }}});
      };
      case 8: return function(pointer) {
        return this.fromWireType({{{ makeGetValue('pointer', 0, 'double') }}});
      };
      default:
        throw new TypeError(`invalid float width (${width}): ${name}`);
    }
  },

#if ASSERTIONS
  $assertIntegerRange__deps: ['$embindRepr'],
  $assertIntegerRange: (typeName, value, minRange, maxRange) => {
    if (value < minRange || value > maxRange) {
      throw new TypeError(`Passing a number "${embindRepr(value)}" from JS side to C/C++ side to an argument of type "${typeName}", which is outside the valid range [${minRange}, ${maxRange}]!`);
    }
  },
#endif

  _embind_register_integer__docs: '/** @suppress {globalThis} */',
  // When converting a number from JS to C++ side, the valid range of the number is
  // [minRange, maxRange], inclusive.
  _embind_register_integer__deps: [
    '$integerReadValueFromPointer', '$AsciiToString', '$registerType',
#if ASSERTIONS
    '$embindRepr',
    '$assertIntegerRange',
#endif
  ],
  _embind_register_integer: (primitiveType, name, size, minRange, maxRange) => {
    name = AsciiToString(name);

    const isUnsignedType = minRange === 0;

    let fromWireType = (value) => value;
    if (isUnsignedType) {
      var bitshift = 32 - 8*size;
      fromWireType = (value) => (value << bitshift) >>> bitshift;
      maxRange = fromWireType(maxRange);
    }

    registerType(primitiveType, {
      name,
      fromWireType: fromWireType,
      toWireType: (destructors, value) => {
#if ASSERTIONS
        if (typeof value != "number" && typeof value != "boolean") {
          throw new TypeError(`Cannot convert "${embindRepr(value)}" to ${name}`);
        }
        assertIntegerRange(name, value, minRange, maxRange);
  #endif
        // The VM will perform JS to Wasm value conversion, according to the spec:
        // https://www.w3.org/TR/wasm-js-api-1/#towebassemblyvalue
        return value;
      },
      readValueFromPointer: integerReadValueFromPointer(name, size, minRange !== 0),
      destructorFunction: null, // This type does not need a destructor
    });
  },

#if WASM_BIGINT
  _embind_register_bigint__docs: '/** @suppress {globalThis} */',
  _embind_register_bigint__deps: [
    '$AsciiToString', '$registerType', '$integerReadValueFromPointer',
#if ASSERTIONS
    '$embindRepr',
    '$assertIntegerRange',
#endif
  ],
  _embind_register_bigint: (primitiveType, name, size, minRange, maxRange) => {
    name = AsciiToString(name);

    const isUnsignedType = minRange === 0n;

    let fromWireType = (value) => value;
    if (isUnsignedType) {
      // uint64 get converted to int64 in ABI, fix them up like we do for 32-bit integers.
      const bitSize = size * 8;
      fromWireType = (value) => {
#if MEMORY64
        // FIXME(https://github.com/emscripten-core/emscripten/issues/16975)
        // `size_t` ends up here, but it's transferred in the ABI as a plain number instead of a bigint.
        if (typeof value == 'number') {
          return value >>> 0;
        }
#endif
        return BigInt.asUintN(bitSize, value);
      }
      maxRange = fromWireType(maxRange);
    }

    registerType(primitiveType, {
      name,
      fromWireType: fromWireType,
      toWireType: (destructors, value) => {
        if (typeof value == "number") {
          value = BigInt(value);
        }
#if ASSERTIONS
        else if (typeof value != "bigint") {
          throw new TypeError(`Cannot convert "${embindRepr(value)}" to ${this.name}`);
        }
        assertIntegerRange(name, value, minRange, maxRange);
#endif
        return value;
      },
      readValueFromPointer: integerReadValueFromPointer(name, size, !isUnsignedType),
      destructorFunction: null, // This type does not need a destructor
    });
  },
#else
  _embind_register_bigint__deps: [],
  _embind_register_bigint: (primitiveType, name, size, minRange, maxRange) => {},
#endif

  _embind_register_float__deps: [
    '$floatReadValueFromPointer', '$AsciiToString', '$registerType',
#if ASSERTIONS
    '$embindRepr',
#endif
  ],
  _embind_register_float: (rawType, name, size) => {
    name = AsciiToString(name);
    registerType(rawType, {
      name,
      fromWireType: (value) => value,
      toWireType: (destructors, value) => {
#if ASSERTIONS
        if (typeof value != "number" && typeof value != "boolean") {
          throw new TypeError(`Cannot convert ${embindRepr(value)} to ${this.name}`);
        }
#endif
        // The VM will perform JS to Wasm value conversion, according to the spec:
        // https://www.w3.org/TR/wasm-js-api-1/#towebassemblyvalue
        return value;
      },
      readValueFromPointer: floatReadValueFromPointer(name, size),
      destructorFunction: null, // This type does not need a destructor
    });
  },

  $readPointer__docs: '/** @suppress {globalThis} */',
  $readPointer: function(pointer) {
    return this.fromWireType({{{ makeGetValue('pointer', '0', '*') }}});
  },

  _embind_register_std_string__deps: [
    '$AsciiToString', '$registerType',
    '$readPointer', '$throwBindingError',
    '$stringToUTF8', '$lengthBytesUTF8', 'malloc', 'free'],
  _embind_register_std_string: (rawType, name) => {
    name = AsciiToString(name);
    var stdStringIsUTF8 = {{{ EMBIND_STD_STRING_IS_UTF8 }}};

    registerType(rawType, {
      name,
      // For some method names we use string keys here since they are part of
      // the public/external API and/or used by the runtime-generated code.
      fromWireType(value) {
        var length = {{{ makeGetValue('value', '0', '*') }}};
        var payload = value + {{{ POINTER_SIZE }}};

        var str;
        if (stdStringIsUTF8) {
          str = UTF8ToString(payload, length, true);
        } else {
          str = '';
          for (var i = 0; i < length; ++i) {
            str += String.fromCharCode(HEAPU8[payload + i]);
          }
        }

        _free(value);

        return str;
      },
      toWireType(destructors, value) {
        if (value instanceof ArrayBuffer) {
          value = new Uint8Array(value);
        }

        var length;
        var valueIsOfTypeString = (typeof value == 'string');

        // We accept `string` or array views with single byte elements
        if (!(valueIsOfTypeString || (ArrayBuffer.isView(value) && value.BYTES_PER_ELEMENT == 1))) {
          throwBindingError('Cannot pass non-string to std::string');
        }
        if (stdStringIsUTF8 && valueIsOfTypeString) {
          length = lengthBytesUTF8(value);
        } else {
          length = value.length;
        }

        // assumes POINTER_SIZE alignment
        var base = _malloc({{{ POINTER_SIZE }}} + length + 1);
        var ptr = base + {{{ POINTER_SIZE }}};
        {{{ makeSetValue('base', '0', 'length', SIZE_TYPE) }}};
        if (valueIsOfTypeString) {
          if (stdStringIsUTF8) {
            stringToUTF8(value, ptr, length + 1);
          } else {
            for (var i = 0; i < length; ++i) {
              var charCode = value.charCodeAt(i);
              if (charCode > 255) {
                _free(base);
                throwBindingError('String has UTF-16 code units that do not fit in 8 bits');
              }
              HEAPU8[ptr + i] = charCode;
            }
          }
        } else {
          HEAPU8.set(value, ptr);
        }

        if (destructors !== null) {
          destructors.push(_free, base);
        }
        return base;
      },
      readValueFromPointer: readPointer,
      destructorFunction(ptr) {
        _free(ptr);
      },
    });
  },

  _embind_register_std_wstring__deps: [
    '$AsciiToString', '$registerType', '$readPointer',
    '$UTF16ToString', '$stringToUTF16', '$lengthBytesUTF16',
    '$UTF32ToString', '$stringToUTF32', '$lengthBytesUTF32',
    ],
  _embind_register_std_wstring: (rawType, charSize, name) => {
    name = AsciiToString(name);
    var decodeString, encodeString, lengthBytesUTF;
    if (charSize === 2) {
      decodeString = UTF16ToString;
      encodeString = stringToUTF16;
      lengthBytesUTF = lengthBytesUTF16;
    } else {
#if ASSERTIONS
      assert(charSize === 4, 'only 2-byte and 4-byte strings are currently supported');
#endif
      decodeString = UTF32ToString;
      encodeString = stringToUTF32;
      lengthBytesUTF = lengthBytesUTF32;
    }
    registerType(rawType, {
      name,
      fromWireType: (value) => {
        // Code mostly taken from _embind_register_std_string fromWireType
        var length = {{{ makeGetValue('value', 0, '*') }}};
        var str = decodeString(value + {{{ POINTER_SIZE }}}, length * charSize, true);

        _free(value);

        return str;
      },
      toWireType: (destructors, value) => {
        if (!(typeof value == 'string')) {
          throwBindingError(`Cannot pass non-string to C++ string type ${name}`);
        }

        // assumes POINTER_SIZE alignment
        var length = lengthBytesUTF(value);
        var ptr = _malloc({{{ POINTER_SIZE }}} + length + charSize);
        {{{ makeSetValue('ptr', '0', 'length / charSize', SIZE_TYPE) }}};

        encodeString(value, ptr + {{{ POINTER_SIZE }}}, length + charSize);

        if (destructors !== null) {
          destructors.push(_free, ptr);
        }
        return ptr;
      },
      readValueFromPointer: readPointer,
      destructorFunction(ptr) {
        _free(ptr);
      }
    });
  },

  _embind_register_emval__deps: [
    '$registerType',  '$EmValType'],
  _embind_register_emval: (rawType) => registerType(rawType, EmValType),

  _embind_register_user_type__deps: ['_embind_register_emval'],
  _embind_register_user_type: (rawType, name) => {
    __embind_register_emval(rawType);
  },

  _embind_register_optional__deps: ['$registerType', '$EmValOptionalType'],
  _embind_register_optional: (rawOptionalType, rawType) => {
    registerType(rawOptionalType, EmValOptionalType);
  },

  _embind_register_memory_view__deps: ['$AsciiToString', '$registerType'],
  _embind_register_memory_view: (rawType, dataTypeIndex, name) => {
    var typeMapping = [
      Int8Array,
      Uint8Array,
      Int16Array,
      Uint16Array,
      Int32Array,
      Uint32Array,
      Float32Array,
      Float64Array,
#if WASM_BIGINT
      BigInt64Array,
      BigUint64Array,
#endif
    ];

    var TA = typeMapping[dataTypeIndex];

    function decodeMemoryView(handle) {
      var size = {{{ makeGetValue('handle', 0, '*') }}};
      var data = {{{ makeGetValue('handle', POINTER_SIZE, '*') }}};
      return new TA(HEAP8.buffer, data, size);
    }

    name = AsciiToString(name);
    registerType(rawType, {
      name,
      fromWireType: decodeMemoryView,
      readValueFromPointer: decodeMemoryView,
    }, {
      ignoreDuplicateRegistrations: true,
    });
  },

  $runDestructors: (destructors) => {
    while (destructors.length) {
      var ptr = destructors.pop();
      var del = destructors.pop();
      del(ptr);
    }
  },

  // The path to interop from JS code to C++ code:
  // (hand-written JS code) -> (autogenerated JS invoker) -> (template-generated C++ invoker) -> (target C++ function)
  // craftInvokerFunction generates the JS invoker function for each function exposed to JS through embind.
  $craftInvokerFunction__deps: [
    '$createNamedFunction', '$runDestructors', '$throwBindingError', '$usesDestructorStack',
#if DYNAMIC_EXECUTION && !EMBIND_AOT
    '$createJsInvoker',
#endif
#if EMBIND_AOT
    '$InvokerFunctions',
    '$createJsInvokerSignature',
#endif
#if ASYNCIFY == 1
    '$Asyncify',
#endif
#if ASSERTIONS
    '$getRequiredArgCount',
    '$checkArgCount',
#endif
  ],
  $craftInvokerFunction: function(humanName, argTypes, classType, cppInvokerFunc, cppTargetFunc, /** boolean= */ isAsync) {
    // humanName: a human-readable string name for the function to be generated.
    // argTypes: An array that contains the embind type objects for all types in the function signature.
    //    argTypes[0] is the type object for the function return value.
    //    argTypes[1] is the type object for function this object/class type, or null if not crafting an invoker for a class method.
    //    argTypes[2...] are the actual function parameters.
    // classType: The embind type object for the class to be bound, or null if this is not a method of a class.
    // cppInvokerFunc: JS Function object to the C++-side function that interops into C++ code.
    // cppTargetFunc: Function pointer (an integer to FUNCTION_TABLE) to the target C++ function the cppInvokerFunc will end up calling.
    // isAsync: Optional. If true, returns an async function. Async bindings are only supported with JSPI.
    var argCount = argTypes.length;

    if (argCount < 2) {
      throwBindingError("argTypes array size mismatch! Must at least get return value and 'this' types!");
    }

#if ASSERTIONS && ASYNCIFY != 2
    assert(!isAsync, 'Async bindings are only supported with JSPI.');
#endif
    var isClassMethodFunc = (argTypes[1] !== null && classType !== null);

    // Free functions with signature "void function()" do not need an invoker that marshalls between wire types.
    // TODO: This omits argument count check - enable only at -O3 or similar.
    //    if (ENABLE_UNSAFE_OPTS && argCount == 2 && argTypes[0].name == "void" && !isClassMethodFunc) {
    //       return FUNCTION_TABLE[fn];
    //    }


    // Determine if we need to use a dynamic stack to store the destructors for the function parameters.
    // TODO: Remove this completely once all function invokers are being dynamically generated.
    var needsDestructorStack = usesDestructorStack(argTypes);

    var returns = !argTypes[0].isVoid;

    var expectedArgCount = argCount - 2;
#if ASSERTIONS
    var minArgs = getRequiredArgCount(argTypes);
#endif
#if DYNAMIC_EXECUTION == 0 && !EMBIND_AOT
    var argsWired = new Array(expectedArgCount);
    var invokerFuncArgs = [];
    var destructors = [];
    var invokerFn = function(...args) {
#if ASSERTIONS
      checkArgCount(args.length, minArgs, expectedArgCount, humanName, throwBindingError);
#endif
#if EMSCRIPTEN_TRACING
      Module.emscripten_trace_enter_context(`embind::${humanName}`);
#endif
      destructors.length = 0;
      var thisWired;
      invokerFuncArgs.length = isClassMethodFunc ? 2 : 1;
      invokerFuncArgs[0] = cppTargetFunc;
      if (isClassMethodFunc) {
        thisWired = argTypes[1].toWireType(destructors, this);
        invokerFuncArgs[1] = thisWired;
      }
      for (var i = 0; i < expectedArgCount; ++i) {
        argsWired[i] = argTypes[i + 2].toWireType(destructors, args[i]);
        invokerFuncArgs.push(argsWired[i]);
      }

      var rv = cppInvokerFunc(...invokerFuncArgs);

      function onDone(rv) {
        if (needsDestructorStack) {
          runDestructors(destructors);
        } else {
          for (var i = isClassMethodFunc ? 1 : 2; i < argTypes.length; i++) {
            var param = i === 1 ? thisWired : argsWired[i - 2];
            if (argTypes[i].destructorFunction !== null) {
              argTypes[i].destructorFunction(param);
            }
          }
        }

  #if EMSCRIPTEN_TRACING
        Module.emscripten_trace_exit_context();
  #endif

        if (returns) {
          return argTypes[0].fromWireType(rv);
        }
      }

#if ASYNCIFY == 1
      if (Asyncify.currData) {
        return Asyncify.whenDone().then(onDone);
      }
#elif ASYNCIFY == 2
      if (isAsync) {
        return rv.then(onDone);
      }
#endif

      return onDone(rv);
    };
#else
    // Builld the arguments that will be passed into the closure around the invoker
    // function.
    var retType = argTypes[0];
    var instType = argTypes[1];
    var closureArgs = [humanName, throwBindingError, cppInvokerFunc, cppTargetFunc, runDestructors, retType.fromWireType.bind(retType), instType?.toWireType.bind(instType)];
#if EMSCRIPTEN_TRACING
    closureArgs.push(Module);
#endif
    for (var i = 2; i < argCount; ++i) {
      var argType = argTypes[i];
      closureArgs.push(argType.toWireType.bind(argType));
    }
#if ASYNCIFY == 1
    closureArgs.push(Asyncify);
#endif
    if (!needsDestructorStack) {
      // Skip return value at index 0 - it's not deleted here. Also skip class type if not a method.
      for (var i = isClassMethodFunc?1:2; i < argTypes.length; ++i) {
        if (argTypes[i].destructorFunction !== null) {
          closureArgs.push(argTypes[i].destructorFunction);
        }
      }
    }
#if ASSERTIONS
    closureArgs.push(checkArgCount, minArgs, expectedArgCount);
#endif

#if EMBIND_AOT
    var signature = createJsInvokerSignature(argTypes, isClassMethodFunc, returns, isAsync);
    var invokerFn = InvokerFunctions[signature](...closureArgs);
#else

    let invokerFactory = createJsInvoker(argTypes, isClassMethodFunc, returns, isAsync);
    var invokerFn = invokerFactory(...closureArgs);
#endif
#endif
    return createNamedFunction(humanName, invokerFn);
  },

  $embind__requireFunction__deps: ['$AsciiToString', '$throwBindingError'
#if DYNCALLS || !WASM_BIGINT || MEMORY64 || CAN_ADDRESS_2GB
    , '$getDynCaller'
#endif
  ],
  $embind__requireFunction: (signature, rawFunction, isAsync = false) => {
#if ASSERTIONS && ASYNCIFY != 2
    assert(!isAsync, 'Async bindings are only supported with JSPI.');
#endif

    signature = AsciiToString(signature);

    function makeDynCaller() {
#if DYNCALLS
      return getDynCaller(signature, rawFunction);
#else
#if !WASM_BIGINT
      if (signature.includes('j')) {
        return getDynCaller(signature, rawFunction);
      }
#endif
#if MEMORY64 || CAN_ADDRESS_2GB
      if (signature.includes('p')) {
        return getDynCaller(signature, rawFunction, isAsync);
      }
#endif
      var rtn = getWasmTableEntry(rawFunction);
#if JSPI
      if (isAsync) {
        rtn = WebAssembly.promising(rtn);
      }
#endif
      return rtn;
#endif
    }

    var fp = makeDynCaller();
    if (typeof fp != 'function') {
        throwBindingError(`unknown function pointer with signature ${signature}: ${rawFunction}`);
    }
    return fp;
  },

  _embind_register_function__deps: [
    '$craftInvokerFunction', '$exposePublicSymbol', '$heap32VectorToArray',
    '$AsciiToString', '$replacePublicSymbol', '$embind__requireFunction',
    '$throwUnboundTypeError', '$whenDependentTypesAreResolved', '$getFunctionName'],
  _embind_register_function: (name, argCount, rawArgTypesAddr, signature, rawInvoker, fn, isAsync, isNonnullReturn) => {
    var argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = AsciiToString(name);
    name = getFunctionName(name);

    rawInvoker = embind__requireFunction(signature, rawInvoker, isAsync);

    exposePublicSymbol(name, function() {
      throwUnboundTypeError(`Cannot call ${name} due to unbound types`, argTypes);
    }, argCount - 1);

    whenDependentTypesAreResolved([], argTypes, (argTypes) => {
      var invokerArgsArray = [argTypes[0] /* return value */, null /* no class 'this'*/].concat(argTypes.slice(1) /* actual params */);
      replacePublicSymbol(name, craftInvokerFunction(name, invokerArgsArray, null /* no class 'this'*/, rawInvoker, fn, isAsync), argCount - 1);
      return [];
    });
  },

  _embind_register_value_array__deps: [
    '$tupleRegistrations', '$AsciiToString', '$embind__requireFunction'],
  _embind_register_value_array: (
    rawType,
    name,
    constructorSignature,
    rawConstructor,
    destructorSignature,
    rawDestructor
  ) => {
    tupleRegistrations[rawType] = {
      name: AsciiToString(name),
      rawConstructor: embind__requireFunction(constructorSignature, rawConstructor),
      rawDestructor: embind__requireFunction(destructorSignature, rawDestructor),
      elements: [],
    };
  },

  _embind_register_value_array_element__deps: [
    '$tupleRegistrations', '$embind__requireFunction'],
  _embind_register_value_array_element: (
    rawTupleType,
    getterReturnType,
    getterSignature,
    getter,
    getterContext,
    setterArgumentType,
    setterSignature,
    setter,
    setterContext
  ) => {
    tupleRegistrations[rawTupleType].elements.push({
      getterReturnType,
      getter: embind__requireFunction(getterSignature, getter),
      getterContext,
      setterArgumentType,
      setter: embind__requireFunction(setterSignature, setter),
      setterContext,
    });
  },

  _embind_finalize_value_array__deps: [
    '$tupleRegistrations', '$runDestructors',
    '$readPointer', '$whenDependentTypesAreResolved'],
  _embind_finalize_value_array: (rawTupleType) => {
    var reg = tupleRegistrations[rawTupleType];
    delete tupleRegistrations[rawTupleType];
    var elements = reg.elements;
    var elementsLength = elements.length;
    var elementTypes = elements.map((elt) => elt.getterReturnType).
                concat(elements.map((elt) => elt.setterArgumentType));

    var rawConstructor = reg.rawConstructor;
    var rawDestructor = reg.rawDestructor;

    whenDependentTypesAreResolved([rawTupleType], elementTypes, (elementTypes) => {
      elements.forEach((elt, i) => {
        var getterReturnType = elementTypes[i];
        var getter = elt.getter;
        var getterContext = elt.getterContext;
        var setterArgumentType = elementTypes[i + elementsLength];
        var setter = elt.setter;
        var setterContext = elt.setterContext;
        elt.read = (ptr) => getterReturnType.fromWireType(getter(getterContext, ptr));
        elt.write = (ptr, o) => {
          var destructors = [];
          setter(setterContext, ptr, setterArgumentType.toWireType(destructors, o));
          runDestructors(destructors);
        };
      });

      return [{
        name: reg.name,
        fromWireType: (ptr) => {
          var rv = new Array(elementsLength);
          for (var i = 0; i < elementsLength; ++i) {
            rv[i] = elements[i].read(ptr);
          }
          rawDestructor(ptr);
          return rv;
        },
        toWireType: (destructors, o) => {
          if (elementsLength !== o.length) {
            throw new TypeError(`Incorrect number of tuple elements for ${reg.name}: expected=${elementsLength}, actual=${o.length}`);
          }
          var ptr = rawConstructor();
          for (var i = 0; i < elementsLength; ++i) {
            elements[i].write(ptr, o[i]);
          }
          if (destructors !== null) {
            destructors.push(rawDestructor, ptr);
          }
          return ptr;
        },
        readValueFromPointer: readPointer,
        destructorFunction: rawDestructor,
      }];
    });
  },

  _embind_register_value_object__deps: [
    '$structRegistrations', '$AsciiToString', '$embind__requireFunction'],
  _embind_register_value_object: (
    rawType,
    name,
    constructorSignature,
    rawConstructor,
    destructorSignature,
    rawDestructor
  ) => {
    structRegistrations[rawType] = {
      name: AsciiToString(name),
      rawConstructor: embind__requireFunction(constructorSignature, rawConstructor),
      rawDestructor: embind__requireFunction(destructorSignature, rawDestructor),
      fields: [],
    };
  },

  _embind_register_value_object_field__deps: [
    '$structRegistrations', '$AsciiToString', '$embind__requireFunction'],
  _embind_register_value_object_field: (
    structType,
    fieldName,
    getterReturnType,
    getterSignature,
    getter,
    getterContext,
    setterArgumentType,
    setterSignature,
    setter,
    setterContext
  ) => {
    structRegistrations[structType].fields.push({
      fieldName: AsciiToString(fieldName),
      getterReturnType,
      getter: embind__requireFunction(getterSignature, getter),
      getterContext,
      setterArgumentType,
      setter: embind__requireFunction(setterSignature, setter),
      setterContext,
    });
  },

  _embind_finalize_value_object__deps: [
    '$structRegistrations', '$runDestructors',
    '$readPointer', '$whenDependentTypesAreResolved'],
  _embind_finalize_value_object: (structType) => {
    var reg = structRegistrations[structType];
    delete structRegistrations[structType];

    var rawConstructor = reg.rawConstructor;
    var rawDestructor = reg.rawDestructor;
    var fieldRecords = reg.fields;
    var fieldTypes = fieldRecords.map((field) => field.getterReturnType).
              concat(fieldRecords.map((field) => field.setterArgumentType));
    whenDependentTypesAreResolved([structType], fieldTypes, (fieldTypes) => {
      var fields = {};
      fieldRecords.forEach((field, i) => {
        var fieldName = field.fieldName;
        var getterReturnType = fieldTypes[i];
        var optional = fieldTypes[i].optional;
        var getter = field.getter;
        var getterContext = field.getterContext;
        var setterArgumentType = fieldTypes[i + fieldRecords.length];
        var setter = field.setter;
        var setterContext = field.setterContext;
        fields[fieldName] = {
          read: (ptr) => getterReturnType.fromWireType(getter(getterContext, ptr)),
          write: (ptr, o) => {
            var destructors = [];
            setter(setterContext, ptr, setterArgumentType.toWireType(destructors, o));
            runDestructors(destructors);
          },
          optional,
        };
      });

      return [{
        name: reg.name,
        fromWireType: (ptr) => {
          var rv = {};
          for (var i in fields) {
            rv[i] = fields[i].read(ptr);
          }
          rawDestructor(ptr);
          return rv;
        },
        toWireType: (destructors, o) => {
          // todo: Here we have an opportunity for -O3 level "unsafe" optimizations:
          // assume all fields are present without checking.
          for (var fieldName in fields) {
            if (!(fieldName in o) && !fields[fieldName].optional) {
              throw new TypeError(`Missing field: "${fieldName}"`);
            }
          }
          var ptr = rawConstructor();
          for (fieldName in fields) {
            fields[fieldName].write(ptr, o[fieldName]);
          }
          if (destructors !== null) {
            destructors.push(rawDestructor, ptr);
          }
          return ptr;
        },
        readValueFromPointer: readPointer,
        destructorFunction: rawDestructor,
      }];
    });
  },

  $genericPointerToWireType__docs: '/** @suppress {globalThis} */',
  $genericPointerToWireType__deps: ['$throwBindingError', '$upcastPointer'],
  $genericPointerToWireType: function(destructors, handle) {
    var ptr;
    if (handle === null) {
      if (this.isReference) {
        throwBindingError(`null is not a valid ${this.name}`);
      }

      if (this.isSmartPointer) {
        ptr = this.rawConstructor();
        if (destructors !== null) {
          destructors.push(this.rawDestructor, ptr);
        }
        return ptr;
      } else {
        return 0;
      }
    }

    if (!handle || !handle.$$) {
      throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`);
    }
    if (!handle.$$.ptr) {
      throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`);
    }
    if (!this.isConst && handle.$$.ptrType.isConst) {
      throwBindingError(`Cannot convert argument of type ${(handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name)} to parameter type ${this.name}`);
    }
    var handleClass = handle.$$.ptrType.registeredClass;
    ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);

    if (this.isSmartPointer) {
      // TODO: this is not strictly true
      // We could support BY_EMVAL conversions from raw pointers to smart pointers
      // because the smart pointer can hold a reference to the handle
      if (undefined === handle.$$.smartPtr) {
        throwBindingError('Passing raw pointer to smart pointer is illegal');
      }

      switch (this.sharingPolicy) {
        case 0: // NONE
          // no upcasting
          if (handle.$$.smartPtrType === this) {
            ptr = handle.$$.smartPtr;
          } else {
            throwBindingError(`Cannot convert argument of type ${(handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name)} to parameter type ${this.name}`);
          }
          break;

        case 1: // INTRUSIVE
          ptr = handle.$$.smartPtr;
          break;

        case 2: // BY_EMVAL
          if (handle.$$.smartPtrType === this) {
            ptr = handle.$$.smartPtr;
          } else {
            var clonedHandle = handle['clone']();
            ptr = this.rawShare(
              ptr,
              Emval.toHandle(() => clonedHandle['delete']())
            );
            if (destructors !== null) {
              destructors.push(this.rawDestructor, ptr);
            }
          }
          break;

        default:
          throwBindingError('Unsupporting sharing policy');
      }
    }
    return ptr;
  },

  $constNoSmartPtrRawPointerToWireType__docs: '/** @suppress {globalThis} */',
  // If we know a pointer type is not going to have SmartPtr logic in it, we can
  // special-case optimize it a bit (compare to genericPointerToWireType)
  $constNoSmartPtrRawPointerToWireType__deps: ['$throwBindingError', '$upcastPointer', '$embindRepr'],
  $constNoSmartPtrRawPointerToWireType: function(destructors, handle) {
    if (handle === null) {
      if (this.isReference) {
        throwBindingError(`null is not a valid ${this.name}`);
      }
      return 0;
    }

    if (!handle.$$) {
      throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`);
    }
    if (!handle.$$.ptr) {
      throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`);
    }
    var handleClass = handle.$$.ptrType.registeredClass;
    var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
    return ptr;
  },

  $nonConstNoSmartPtrRawPointerToWireType__docs: '/** @suppress {globalThis} */',
  // An optimized version for non-const method accesses - there we must additionally restrict that
  // the pointer is not a const-pointer.
  $nonConstNoSmartPtrRawPointerToWireType__deps: ['$throwBindingError', '$upcastPointer', '$embindRepr'],
  $nonConstNoSmartPtrRawPointerToWireType: function(destructors, handle) {
    if (handle === null) {
      if (this.isReference) {
        throwBindingError(`null is not a valid ${this.name}`);
      }
      return 0;
    }

    if (!handle.$$) {
      throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`);
    }
    if (!handle.$$.ptr) {
      throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`);
    }
    if (handle.$$.ptrType.isConst) {
      throwBindingError(`Cannot convert argument of type ${handle.$$.ptrType.name} to parameter type ${this.name}`);
    }
    var handleClass = handle.$$.ptrType.registeredClass;
    var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
    return ptr;
  },

  $init_RegisteredPointer__deps: [
    '$RegisteredPointer',
    '$readPointer',
    '$RegisteredPointer_fromWireType',
  ],
  $init_RegisteredPointer: () => {
    Object.assign(RegisteredPointer.prototype, {
      getPointee(ptr) {
        if (this.rawGetPointee) {
          ptr = this.rawGetPointee(ptr);
        }
        return ptr;
      },
      destructor(ptr) {
        this.rawDestructor?.(ptr);
      },
      readValueFromPointer: readPointer,
      fromWireType: RegisteredPointer_fromWireType,
    });
  },

  $RegisteredPointer__docs: `/** @constructor
    @param {*=} pointeeType,
    @param {*=} sharingPolicy,
    @param {*=} rawGetPointee,
    @param {*=} rawConstructor,
    @param {*=} rawShare,
    @param {*=} rawDestructor,
     */`,
  $RegisteredPointer__deps: [
    '$constNoSmartPtrRawPointerToWireType', '$genericPointerToWireType',
    '$nonConstNoSmartPtrRawPointerToWireType', '$init_RegisteredPointer'],
  $RegisteredPointer__postset: 'init_RegisteredPointer()',
  $RegisteredPointer: function(
    name,
    registeredClass,
    isReference,
    isConst,

    // smart pointer properties
    isSmartPointer,
    pointeeType,
    sharingPolicy,
    rawGetPointee,
    rawConstructor,
    rawShare,
    rawDestructor
  ) {
    this.name = name;
    this.registeredClass = registeredClass;
    this.isReference = isReference;
    this.isConst = isConst;

    // smart pointer properties
    this.isSmartPointer = isSmartPointer;
    this.pointeeType = pointeeType;
    this.sharingPolicy = sharingPolicy;
    this.rawGetPointee = rawGetPointee;
    this.rawConstructor = rawConstructor;
    this.rawShare = rawShare;
    this.rawDestructor = rawDestructor;

    if (!isSmartPointer && registeredClass.baseClass === undefined) {
      if (isConst) {
        this.toWireType = constNoSmartPtrRawPointerToWireType;
        this.destructorFunction = null;
      } else {
        this.toWireType = nonConstNoSmartPtrRawPointerToWireType;
        this.destructorFunction = null;
      }
    } else {
      this.toWireType = genericPointerToWireType;
      // Here we must leave this.destructorFunction undefined, since whether genericPointerToWireType returns
      // a pointer that needs to be freed up is runtime-dependent, and cannot be evaluated at registration time.
      // TODO: Create an alternative mechanism that allows removing the use of var destructors = []; array in
      //       craftInvokerFunction altogether.
    }
  },

  $RegisteredPointer_fromWireType__docs: '/** @suppress {globalThis} */',
  $RegisteredPointer_fromWireType__deps: [
    '$downcastPointer', '$registeredPointers',
    '$getInheritedInstance', '$makeClassHandle',
#if MEMORY64
    '$bigintToI53Checked'
#endif
  ],
  $RegisteredPointer_fromWireType: function(ptr) {
    // ptr is a raw pointer (or a raw smartpointer)
#if MEMORY64
    ptr = bigintToI53Checked(ptr);
#if ASSERTIONS
    assert(Number.isSafeInteger(ptr));
#endif
#endif

    // rawPointer is a maybe-null raw pointer
    var rawPointer = this.getPointee(ptr);
    if (!rawPointer) {
      this.destructor(ptr);
      return null;
    }

    var registeredInstance = getInheritedInstance(this.registeredClass, rawPointer);
    if (undefined !== registeredInstance) {
      // JS object has been neutered, time to repopulate it
      if (0 === registeredInstance.$$.count.value) {
        registeredInstance.$$.ptr = rawPointer;
        registeredInstance.$$.smartPtr = ptr;
        return registeredInstance['clone']();
      } else {
        // else, just increment reference count on existing object
        // it already has a reference to the smart pointer
        var rv = registeredInstance['clone']();
        this.destructor(ptr);
        return rv;
      }
    }

    function makeDefaultHandle() {
      if (this.isSmartPointer) {
        return makeClassHandle(this.registeredClass.instancePrototype, {
          ptrType: this.pointeeType,
          ptr: rawPointer,
          smartPtrType: this,
          smartPtr: ptr,
        });
      } else {
        return makeClassHandle(this.registeredClass.instancePrototype, {
          ptrType: this,
          ptr,
        });
      }
    }

    var actualType = this.registeredClass.getActualType(rawPointer);
    var registeredPointerRecord = registeredPointers[actualType];
    if (!registeredPointerRecord) {
      return makeDefaultHandle.call(this);
    }

    var toType;
    if (this.isConst) {
      toType = registeredPointerRecord.constPointerType;
    } else {
      toType = registeredPointerRecord.pointerType;
    }
    var dp = downcastPointer(
        rawPointer,
        this.registeredClass,
        toType.registeredClass);
    if (dp === null) {
      return makeDefaultHandle.call(this);
    }
    if (this.isSmartPointer) {
      return makeClassHandle(toType.registeredClass.instancePrototype, {
        ptrType: toType,
        ptr: dp,
        smartPtrType: this,
        smartPtr: ptr,
      });
    } else {
      return makeClassHandle(toType.registeredClass.instancePrototype, {
        ptrType: toType,
        ptr: dp,
      });
    }
  },

  $runDestructor: ($$) => {
    if ($$.smartPtr) {
      $$.smartPtrType.rawDestructor($$.smartPtr);
    } else {
      $$.ptrType.registeredClass.rawDestructor($$.ptr);
    }
  },

  $releaseClassHandle__deps: ['$runDestructor'],
  $releaseClassHandle: ($$) => {
    $$.count.value -= 1;
    var toDelete = 0 === $$.count.value;
    if (toDelete) {
      runDestructor($$);
    }
  },

  $finalizationRegistry: false,

  $detachFinalizer_deps: ['$finalizationRegistry'],
  $detachFinalizer: (handle) => {},

  $attachFinalizer__deps: [
    '$finalizationRegistry', '$detachFinalizer', '$releaseClassHandle',
#if ASSERTIONS
    '$RegisteredPointer_fromWireType'
#endif
  ],
  $attachFinalizer: (handle) => {
    if ('undefined' === typeof FinalizationRegistry) {
      attachFinalizer = (handle) => handle;
      return handle;
    }
    // If the running environment has a FinalizationRegistry (see
    // https://github.com/tc39/proposal-weakrefs), then attach finalizers
    // for class handles.  We check for the presence of FinalizationRegistry
    // at run-time, not build-time.
    finalizationRegistry = new FinalizationRegistry((info) => {
#if ASSERTIONS
      console.warn(info.leakWarning);
#endif
      releaseClassHandle(info.$$);
    });
    attachFinalizer = (handle) => {
      var $$ = handle.$$;
      var hasSmartPtr = !!$$.smartPtr;
      if (hasSmartPtr) {
        // We should not call the destructor on raw pointers in case other code expects the pointee to live
        var info = { $$: $$ };
#if ASSERTIONS
        // Create a warning as an Error instance in advance so that we can store
        // the current stacktrace and point to it when / if a leak is detected.
        // This is more useful than the empty stacktrace of `FinalizationRegistry`
        // callback.
        var cls = $$.ptrType.registeredClass;
        var err = new Error(`Embind found a leaked C++ instance ${cls.name} <${ptrToString($$.ptr)}>.\n` +
        "We'll free it automatically in this case, but this functionality is not reliable across various environments.\n" +
        "Make sure to invoke .delete() manually once you're done with the instance instead.\n" +
        "Originally allocated"); // `.stack` will add "at ..." after this sentence
        if ('captureStackTrace' in Error) {
          Error.captureStackTrace(err, RegisteredPointer_fromWireType);
        }
        info.leakWarning = err.stack.replace(/^Error: /, '');
#endif
        finalizationRegistry.register(handle, info, handle);
      }
      return handle;
    };
    detachFinalizer = (handle) => finalizationRegistry.unregister(handle);
    return attachFinalizer(handle);
  },

  $makeClassHandle__deps: ['$throwInternalError', '$attachFinalizer'],
  $makeClassHandle: (prototype, record) => {
    if (!record.ptrType || !record.ptr) {
      throwInternalError('makeClassHandle requires ptr and ptrType');
    }
    var hasSmartPtrType = !!record.smartPtrType;
    var hasSmartPtr = !!record.smartPtr;
    if (hasSmartPtrType !== hasSmartPtr) {
      throwInternalError('Both smartPtrType and smartPtr must be specified');
    }
    record.count = { value: 1 };
    return attachFinalizer(Object.create(prototype, {
      $$: {
        value: record,
        writable: true,
      },
    }));
  },

  $init_ClassHandle__deps: [
    '$ClassHandle',
    '$shallowCopyInternalPointer',
    '$throwInstanceAlreadyDeleted',
    '$attachFinalizer',
    '$releaseClassHandle',
    '$throwBindingError',
    '$detachFinalizer',
    '$flushPendingDeletes',
    '$delayFunction',
  ],
  $init_ClassHandle: () => {
    let proto = ClassHandle.prototype;

    Object.assign(proto, {
      "isAliasOf"(other) {
        if (!(this instanceof ClassHandle)) {
          return false;
        }
        if (!(other instanceof ClassHandle)) {
          return false;
        }

        var leftClass = this.$$.ptrType.registeredClass;
        var left = this.$$.ptr;
        other.$$ = /** @type {Object} */ (other.$$);
        var rightClass = other.$$.ptrType.registeredClass;
        var right = other.$$.ptr;

        while (leftClass.baseClass) {
          left = leftClass.upcast(left);
          leftClass = leftClass.baseClass;
        }

        while (rightClass.baseClass) {
          right = rightClass.upcast(right);
          rightClass = rightClass.baseClass;
        }

        return leftClass === rightClass && left === right;
      },

      "clone"() {
        if (!this.$$.ptr) {
          throwInstanceAlreadyDeleted(this);
        }

        if (this.$$.preservePointerOnDelete) {
          this.$$.count.value += 1;
          return this;
        } else {
          var clone = attachFinalizer(Object.create(Object.getPrototypeOf(this), {
            $$: {
              value: shallowCopyInternalPointer(this.$$),
            }
          }));

          clone.$$.count.value += 1;
          clone.$$.deleteScheduled = false;
          return clone;
        }
      },

      "delete"() {
        if (!this.$$.ptr) {
          throwInstanceAlreadyDeleted(this);
        }

        if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
          throwBindingError('Object already scheduled for deletion');
        }

        detachFinalizer(this);
        releaseClassHandle(this.$$);

        if (!this.$$.preservePointerOnDelete) {
          this.$$.smartPtr = undefined;
          this.$$.ptr = undefined;
        }
      },

      "isDeleted"() {
        return !this.$$.ptr;
      },

      "deleteLater"() {
        if (!this.$$.ptr) {
          throwInstanceAlreadyDeleted(this);
        }
        if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
          throwBindingError('Object already scheduled for deletion');
        }
        deletionQueue.push(this);
        if (deletionQueue.length === 1 && delayFunction) {
          delayFunction(flushPendingDeletes);
        }
        this.$$.deleteScheduled = true;
        return this;
      },
    });

    // Support `using ...` from https://github.com/tc39/proposal-explicit-resource-management.
    const symbolDispose = Symbol.dispose;
    if (symbolDispose) {
      proto[symbolDispose] = proto['delete'];
    }
  },

  $ClassHandle__docs: '/** @constructor */',
  $ClassHandle__deps: ['$init_ClassHandle'],
  $ClassHandle__postset: 'init_ClassHandle()',
  // root of all pointer and smart pointer handles in embind
  $ClassHandle: function() {
  },

  $throwInstanceAlreadyDeleted__deps: ['$throwBindingError'],
  $throwInstanceAlreadyDeleted: (obj) => {
    function getInstanceTypeName(handle) {
      return handle.$$.ptrType.registeredClass.name;
    }
    throwBindingError(getInstanceTypeName(obj) + ' instance already deleted');
  },

  $deletionQueue: [],

  $flushPendingDeletes__deps: ['$deletionQueue'],
  $flushPendingDeletes: () => {
    while (deletionQueue.length) {
      var obj = deletionQueue.pop();
      obj.$$.deleteScheduled = false;
      obj['delete']();
    }
  },

  $delayFunction: undefined,

  $setDelayFunction__deps: ['$delayFunction', '$deletionQueue', '$flushPendingDeletes'],
  $setDelayFunction: (fn) => {
    delayFunction = fn;
    if (deletionQueue.length && delayFunction) {
      delayFunction(flushPendingDeletes);
    }
  },

  $RegisteredClass__docs: '/** @constructor */',
  $RegisteredClass: function(name,
                             constructor,
                             instancePrototype,
                             rawDestructor,
                             baseClass,
                             getActualType,
                             upcast,
                             downcast) {
    this.name = name;
    this.constructor = constructor;
    this.instancePrototype = instancePrototype;
    this.rawDestructor = rawDestructor;
    this.baseClass = baseClass;
    this.getActualType = getActualType;
    this.upcast = upcast;
    this.downcast = downcast;
    this.pureVirtualFunctions = [];
  },

  $shallowCopyInternalPointer: (o) => {
    return {
      count: o.count,
      deleteScheduled: o.deleteScheduled,
      preservePointerOnDelete: o.preservePointerOnDelete,
      ptr: o.ptr,
      ptrType: o.ptrType,
      smartPtr: o.smartPtr,
      smartPtrType: o.smartPtrType,
    };
  },

  _embind_register_class__deps: [
    '$BindingError', '$ClassHandle', '$createNamedFunction',
    '$registeredPointers', '$exposePublicSymbol',
    '$makeLegalFunctionName', '$AsciiToString',
    '$RegisteredClass', '$RegisteredPointer', '$replacePublicSymbol',
    '$embind__requireFunction', '$throwUnboundTypeError',
    '$whenDependentTypesAreResolved'],
  _embind_register_class: (rawType,
                           rawPointerType,
                           rawConstPointerType,
                           baseClassRawType,
                           getActualTypeSignature,
                           getActualType,
                           upcastSignature,
                           upcast,
                           downcastSignature,
                           downcast,
                           name,
                           destructorSignature,
                           rawDestructor) => {
    name = AsciiToString(name);
    getActualType = embind__requireFunction(getActualTypeSignature, getActualType);
    upcast &&= embind__requireFunction(upcastSignature, upcast);
    downcast &&= embind__requireFunction(downcastSignature, downcast);
    rawDestructor = embind__requireFunction(destructorSignature, rawDestructor);
    var legalFunctionName = makeLegalFunctionName(name);

    exposePublicSymbol(legalFunctionName, function() {
      // this code cannot run if baseClassRawType is zero
      throwUnboundTypeError(`Cannot construct ${name} due to unbound types`, [baseClassRawType]);
    });

    whenDependentTypesAreResolved(
      [rawType, rawPointerType, rawConstPointerType],
      baseClassRawType ? [baseClassRawType] : [],
      (base) => {
        base = base[0];

        var baseClass;
        var basePrototype;
        if (baseClassRawType) {
          baseClass = base.registeredClass;
          basePrototype = baseClass.instancePrototype;
        } else {
          basePrototype = ClassHandle.prototype;
        }

        var constructor = createNamedFunction(name, function(...args) {
          if (Object.getPrototypeOf(this) !== instancePrototype) {
            throw new BindingError(`Use 'new' to construct ${name}`);
          }
          if (undefined === registeredClass.constructor_body) {
            throw new BindingError(`${name} has no accessible constructor`);
          }
          var body = registeredClass.constructor_body[args.length];
          if (undefined === body) {
            throw new BindingError(`Tried to invoke ctor of ${name} with invalid number of parameters (${args.length}) - expected (${Object.keys(registeredClass.constructor_body).toString()}) parameters instead!`);
          }
          return body.apply(this, args);
        });

        var instancePrototype = Object.create(basePrototype, {
          constructor: { value: constructor },
        });

        constructor.prototype = instancePrototype;

        var registeredClass = new RegisteredClass(name,
                                                  constructor,
                                                  instancePrototype,
                                                  rawDestructor,
                                                  baseClass,
                                                  getActualType,
                                                  upcast,
                                                  downcast);

        if (registeredClass.baseClass) {
          // Keep track of class hierarchy. Used to allow sub-classes to inherit class functions.
          registeredClass.baseClass.__derivedClasses ??= [];

          registeredClass.baseClass.__derivedClasses.push(registeredClass);
        }

        var referenceConverter = new RegisteredPointer(name,
                                                       registeredClass,
                                                       true,
                                                       false,
                                                       false);

        var pointerConverter = new RegisteredPointer(name + '*',
                                                     registeredClass,
                                                     false,
                                                     false,
                                                     false);

        var constPointerConverter = new RegisteredPointer(name + ' const*',
                                                          registeredClass,
                                                          false,
                                                          true,
                                                          false);

        registeredPointers[rawType] = {
          pointerType: pointerConverter,
          constPointerType: constPointerConverter
        };

        replacePublicSymbol(legalFunctionName, constructor);

        return [referenceConverter, pointerConverter, constPointerConverter];
      }
    );
  },

  _embind_register_class_constructor__deps: [
    '$heap32VectorToArray', '$embind__requireFunction',
    '$whenDependentTypesAreResolved',
    '$craftInvokerFunction'],
  _embind_register_class_constructor: (
    rawClassType,
    argCount,
    rawArgTypesAddr,
    invokerSignature,
    invoker,
    rawConstructor
  ) => {
#if ASSERTIONS
    assert(argCount > 0);
#endif
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    invoker = embind__requireFunction(invokerSignature, invoker);
    var args = [rawConstructor];
    var destructors = [];

    whenDependentTypesAreResolved([], [rawClassType], (classType) => {
      classType = classType[0];
      var humanName = `constructor ${classType.name}`;

      if (undefined === classType.registeredClass.constructor_body) {
        classType.registeredClass.constructor_body = [];
      }
      if (undefined !== classType.registeredClass.constructor_body[argCount - 1]) {
        throw new BindingError(`Cannot register multiple constructors with identical number of parameters (${argCount-1}) for class '${classType.name}'! Overload resolution is currently only performed using the parameter count, not actual type info!`);
      }
      classType.registeredClass.constructor_body[argCount - 1] = () => {
        throwUnboundTypeError(`Cannot construct ${classType.name} due to unbound types`, rawArgTypes);
      };

      whenDependentTypesAreResolved([], rawArgTypes, (argTypes) => {
        // Insert empty slot for context type (argTypes[1]).
        argTypes.splice(1, 0, null);
        classType.registeredClass.constructor_body[argCount - 1] = craftInvokerFunction(humanName, argTypes, null, invoker, rawConstructor);
        return [];
      });
      return [];
    });
  },

  $downcastPointer: (ptr, ptrClass, desiredClass) => {
    if (ptrClass === desiredClass) {
      return ptr;
    }
    if (undefined === desiredClass.baseClass) {
      return null; // no conversion
    }

    var rv = downcastPointer(ptr, ptrClass, desiredClass.baseClass);
    if (rv === null) {
      return null;
    }
    return desiredClass.downcast(rv);
  },

  $upcastPointer__deps: ['$throwBindingError'],
  $upcastPointer: (ptr, ptrClass, desiredClass) => {
    while (ptrClass !== desiredClass) {
      if (!ptrClass.upcast) {
        throwBindingError(`Expected null or instance of ${desiredClass.name}, got an instance of ${ptrClass.name}`);
      }
      ptr = ptrClass.upcast(ptr);
      ptrClass = ptrClass.baseClass;
    }
    return ptr;
  },

  $validateThis__deps: ['$throwBindingError', '$upcastPointer'],
  $validateThis: (this_, classType, humanName) => {
    if (!(this_ instanceof Object)) {
      throwBindingError(`${humanName} with invalid "this": ${this_}`);
    }
    if (!(this_ instanceof classType.registeredClass.constructor)) {
      throwBindingError(`${humanName} incompatible with "this" of type ${this_.constructor.name}`);
    }
    if (!this_.$$.ptr) {
      throwBindingError(`cannot call emscripten binding method ${humanName} on deleted object`);
    }

    // todo: kill this
    return upcastPointer(this_.$$.ptr,
                         this_.$$.ptrType.registeredClass,
                         classType.registeredClass);
  },

  _embind_register_class_function__deps: [
    '$craftInvokerFunction', '$heap32VectorToArray', '$AsciiToString',
    '$embind__requireFunction', '$throwUnboundTypeError',
    '$whenDependentTypesAreResolved', '$getFunctionName'],
  _embind_register_class_function: (rawClassType,
                                    methodName,
                                    argCount,
                                    rawArgTypesAddr, // [ReturnType, ThisType, Args...]
                                    invokerSignature,
                                    rawInvoker,
                                    context,
                                    isPureVirtual,
                                    isAsync,
                                    isNonnullReturn) => {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = AsciiToString(methodName);
    methodName = getFunctionName(methodName);
    rawInvoker = embind__requireFunction(invokerSignature, rawInvoker, isAsync);

    whenDependentTypesAreResolved([], [rawClassType], (classType) => {
      classType = classType[0];
      var humanName = `${classType.name}.${methodName}`;

      if (methodName.startsWith("@@")) {
        methodName = Symbol[methodName.substring(2)];
      }

      if (isPureVirtual) {
        classType.registeredClass.pureVirtualFunctions.push(methodName);
      }

      function unboundTypesHandler() {
        throwUnboundTypeError(`Cannot call ${humanName} due to unbound types`, rawArgTypes);
      }

      var proto = classType.registeredClass.instancePrototype;
      var method = proto[methodName];
      if (undefined === method || (undefined === method.overloadTable && method.className !== classType.name && method.argCount === argCount - 2)) {
        // This is the first overload to be registered, OR we are replacing a
        // function in the base class with a function in the derived class.
        unboundTypesHandler.argCount = argCount - 2;
        unboundTypesHandler.className = classType.name;
        proto[methodName] = unboundTypesHandler;
      } else {
        // There was an existing function with the same name registered. Set up
        // a function overload routing table.
        ensureOverloadTable(proto, methodName, humanName);
        proto[methodName].overloadTable[argCount - 2] = unboundTypesHandler;
      }

      whenDependentTypesAreResolved([], rawArgTypes, (argTypes) => {
        var memberFunction = craftInvokerFunction(humanName, argTypes, classType, rawInvoker, context, isAsync);

        // Replace the initial unbound-handler-stub function with the
        // appropriate member function, now that all types are resolved. If
        // multiple overloads are registered for this function, the function
        // goes into an overload table.
        if (undefined === proto[methodName].overloadTable) {
          // Set argCount in case an overload is registered later
          memberFunction.argCount = argCount - 2;
          proto[methodName] = memberFunction;
        } else {
          proto[methodName].overloadTable[argCount - 2] = memberFunction;
        }

        return [];
      });
      return [];
    });
  },

  _embind_register_class_property__deps: [
    '$AsciiToString', '$embind__requireFunction', '$runDestructors',
    '$throwBindingError', '$throwUnboundTypeError',
    '$whenDependentTypesAreResolved', '$validateThis'],
  _embind_register_class_property: (classType,
                                    fieldName,
                                    getterReturnType,
                                    getterSignature,
                                    getter,
                                    getterContext,
                                    setterArgumentType,
                                    setterSignature,
                                    setter,
                                    setterContext) => {
    fieldName = AsciiToString(fieldName);
    getter = embind__requireFunction(getterSignature, getter);

    whenDependentTypesAreResolved([], [classType], (classType) => {
      classType = classType[0];
      var humanName = `${classType.name}.${fieldName}`;
      var desc = {
        get() {
          throwUnboundTypeError(`Cannot access ${humanName} due to unbound types`, [getterReturnType, setterArgumentType]);
        },
        enumerable: true,
        configurable: true
      };
      if (setter) {
        desc.set = () => throwUnboundTypeError(`Cannot access ${humanName} due to unbound types`, [getterReturnType, setterArgumentType]);
      } else {
        desc.set = (v) => throwBindingError(humanName + ' is a read-only property');
      }

      Object.defineProperty(classType.registeredClass.instancePrototype, fieldName, desc);

      whenDependentTypesAreResolved(
        [],
        (setter ? [getterReturnType, setterArgumentType] : [getterReturnType]),
      (types) => {
        var getterReturnType = types[0];
        var desc = {
          get() {
            var ptr = validateThis(this, classType, humanName + ' getter');
            return getterReturnType.fromWireType(getter(getterContext, ptr));
          },
          enumerable: true
        };

        if (setter) {
          setter = embind__requireFunction(setterSignature, setter);
          var setterArgumentType = types[1];
          desc.set = function(v) {
            var ptr = validateThis(this, classType, humanName + ' setter');
            var destructors = [];
            setter(setterContext, ptr, setterArgumentType.toWireType(destructors, v));
            runDestructors(destructors);
          };
        }

        Object.defineProperty(classType.registeredClass.instancePrototype, fieldName, desc);
        return [];
      });

      return [];
    });
  },

  _embind_register_class_class_function__deps: [
    '$craftInvokerFunction', '$ensureOverloadTable', '$heap32VectorToArray',
    '$AsciiToString', '$embind__requireFunction', '$throwUnboundTypeError',
    '$whenDependentTypesAreResolved', '$getFunctionName'],
  _embind_register_class_class_function: (rawClassType,
                                          methodName,
                                          argCount,
                                          rawArgTypesAddr,
                                          invokerSignature,
                                          rawInvoker,
                                          fn,
                                          isAsync,
                                          isNonnullReturn) => {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = AsciiToString(methodName);
    methodName = getFunctionName(methodName);
    rawInvoker = embind__requireFunction(invokerSignature, rawInvoker, isAsync);
    whenDependentTypesAreResolved([], [rawClassType], (classType) => {
      classType = classType[0];
      var humanName = `${classType.name}.${methodName}`;

      function unboundTypesHandler() {
        throwUnboundTypeError(`Cannot call ${humanName} due to unbound types`, rawArgTypes);
      }

      if (methodName.startsWith('@@')) {
        methodName = Symbol[methodName.substring(2)];
      }

      var proto = classType.registeredClass.constructor;
      if (undefined === proto[methodName]) {
        // This is the first function to be registered with this name.
        unboundTypesHandler.argCount = argCount-1;
        proto[methodName] = unboundTypesHandler;
      } else {
        // There was an existing function with the same name registered. Set up
        // a function overload routing table.
        ensureOverloadTable(proto, methodName, humanName);
        proto[methodName].overloadTable[argCount-1] = unboundTypesHandler;
      }

      whenDependentTypesAreResolved([], rawArgTypes, (argTypes) => {
        // Replace the initial unbound-types-handler stub with the proper
        // function. If multiple overloads are registered, the function handlers
        // go into an overload table.
        var invokerArgsArray = [argTypes[0] /* return value */, null /* no class 'this'*/].concat(argTypes.slice(1) /* actual params */);
        var func = craftInvokerFunction(humanName, invokerArgsArray, null /* no class 'this'*/, rawInvoker, fn, isAsync);
        if (undefined === proto[methodName].overloadTable) {
          func.argCount = argCount-1;
          proto[methodName] = func;
        } else {
          proto[methodName].overloadTable[argCount-1] = func;
        }

        if (classType.registeredClass.__derivedClasses) {
          for (const derivedClass of classType.registeredClass.__derivedClasses) {
            if (!derivedClass.constructor.hasOwnProperty(methodName)) {
              // TODO: Add support for overloads
              derivedClass.constructor[methodName] = func;
            }
          }
        }

        return [];
      });
      return [];
    });
  },

  _embind_register_class_class_property__deps: [
    '$AsciiToString', '$embind__requireFunction', '$runDestructors',
    '$throwBindingError', '$throwUnboundTypeError',
    '$whenDependentTypesAreResolved'],
  _embind_register_class_class_property: (rawClassType,
                                          fieldName,
                                          rawFieldType,
                                          rawFieldPtr,
                                          getterSignature,
                                          getter,
                                          setterSignature,
                                          setter) => {
    fieldName = AsciiToString(fieldName);
    getter = embind__requireFunction(getterSignature, getter);

    whenDependentTypesAreResolved([], [rawClassType], (classType) => {
      classType = classType[0];
      var humanName = `${classType.name}.${fieldName}`;
      var desc = {
        get() {
          throwUnboundTypeError(`Cannot access ${humanName} due to unbound types`, [rawFieldType]);
        },
        enumerable: true,
        configurable: true
      };
      if (setter) {
        desc.set = () => {
          throwUnboundTypeError(`Cannot access ${humanName} due to unbound types`, [rawFieldType]);
        };
      } else {
        desc.set = (v) => {
          throwBindingError(`${humanName} is a read-only property`);
        };
      }

      Object.defineProperty(classType.registeredClass.constructor, fieldName, desc);

      whenDependentTypesAreResolved([], [rawFieldType], (fieldType) => {
        fieldType = fieldType[0];
        var desc = {
          get() {
            return fieldType.fromWireType(getter(rawFieldPtr));
          },
          enumerable: true
        };

        if (setter) {
          setter = embind__requireFunction(setterSignature, setter);
          desc.set = (v) => {
            var destructors = [];
            setter(rawFieldPtr, fieldType.toWireType(destructors, v));
            runDestructors(destructors);
          };
        }

        Object.defineProperty(classType.registeredClass.constructor, fieldName, desc);
        return [];
      });

      return [];
    });
  },

  _embind_create_inheriting_constructor__deps: [
    '$createNamedFunction', '$Emval',
    '$PureVirtualError', '$AsciiToString',
    '$registerInheritedInstance',
    '$requireRegisteredType', '$throwBindingError',
    '$unregisterInheritedInstance', '$detachFinalizer', '$attachFinalizer'],
  _embind_create_inheriting_constructor: (constructorName, wrapperType, properties) => {
    constructorName = AsciiToString(constructorName);
    wrapperType = requireRegisteredType(wrapperType, 'wrapper');
    properties = Emval.toValue(properties);

    var registeredClass = wrapperType.registeredClass;
    var wrapperPrototype = registeredClass.instancePrototype;
    var baseClass = registeredClass.baseClass;
    var baseClassPrototype = baseClass.instancePrototype;
    var baseConstructor = registeredClass.baseClass.constructor;
    var ctor = createNamedFunction(constructorName, function(...args) {
      registeredClass.baseClass.pureVirtualFunctions.forEach(function(name) {
        if (this[name] === baseClassPrototype[name]) {
          throw new PureVirtualError(`Pure virtual function ${name} must be implemented in JavaScript`);
        }
      }.bind(this));

      Object.defineProperty(this, '__parent', {
        value: wrapperPrototype
      });
      this['__construct'](...args);
    });

    // It's a little nasty that we're modifying the wrapper prototype here.

    wrapperPrototype['__construct'] = function __construct(...args) {
      if (this === wrapperPrototype) {
        throwBindingError("Pass correct 'this' to __construct");
      }

      var inner = baseConstructor['implement'](this, ...args);
      detachFinalizer(inner);
      var $$ = inner.$$;
      inner['notifyOnDestruction']();
      $$.preservePointerOnDelete = true;
      Object.defineProperties(this, { $$: {
          value: $$
      }});
      attachFinalizer(this);
      registerInheritedInstance(registeredClass, $$.ptr, this);
    };

    wrapperPrototype['__destruct'] = function __destruct() {
      if (this === wrapperPrototype) {
        throwBindingError("Pass correct 'this' to __destruct");
      }

      detachFinalizer(this);
      unregisterInheritedInstance(registeredClass, this.$$.ptr);
    };

    ctor.prototype = Object.create(wrapperPrototype);
    Object.assign(ctor.prototype, properties);
    return Emval.toHandle(ctor);
  },

  $char_0: '0'.charCodeAt(0),
  $char_9: '9'.charCodeAt(0),
  $makeLegalFunctionName__deps: ['$char_0', '$char_9'],
  $makeLegalFunctionName: (name) => {
#if ASSERTIONS
    assert(typeof name === 'string');
#endif
    name = name.replace(/[^a-zA-Z0-9_]/g, '$');
    var f = name.charCodeAt(0);
    if (f >= char_0 && f <= char_9) {
      return `_${name}`;
    }
    return name;
  },

  _embind_register_smart_ptr__deps: ['$RegisteredPointer', '$embind__requireFunction', '$whenDependentTypesAreResolved'],
  _embind_register_smart_ptr: (rawType,
                               rawPointeeType,
                               name,
                               sharingPolicy,
                               getPointeeSignature,
                               rawGetPointee,
                               constructorSignature,
                               rawConstructor,
                               shareSignature,
                               rawShare,
                               destructorSignature,
                               rawDestructor) => {
    name = AsciiToString(name);
    rawGetPointee = embind__requireFunction(getPointeeSignature, rawGetPointee);
    rawConstructor = embind__requireFunction(constructorSignature, rawConstructor);
    rawShare = embind__requireFunction(shareSignature, rawShare);
    rawDestructor = embind__requireFunction(destructorSignature, rawDestructor);

    whenDependentTypesAreResolved([rawType], [rawPointeeType], (pointeeType) => {
      pointeeType = pointeeType[0];

      var registeredPointer = new RegisteredPointer(name,
                                                    pointeeType.registeredClass,
                                                    false,
                                                    false,
                                                    // smart pointer properties
                                                    true,
                                                    pointeeType,
                                                    sharingPolicy,
                                                    rawGetPointee,
                                                    rawConstructor,
                                                    rawShare,
                                                    rawDestructor);
      return [registeredPointer];
    });
  },

  _embind_register_enum__docs: '/** @suppress {globalThis} */',
  _embind_register_enum__deps: ['$exposePublicSymbol', '$enumReadValueFromPointer',
    '$AsciiToString', '$registerType'],
  _embind_register_enum: (rawType, name, size, isSigned) => {
    name = AsciiToString(name);

    function ctor() {}
    ctor.values = {};

    registerType(rawType, {
      name,
      constructor: ctor,
      fromWireType: function(c) {
        return this.constructor.values[c];
      },
      toWireType: (destructors, c) => c.value,
      readValueFromPointer: enumReadValueFromPointer(name, size, isSigned),
      destructorFunction: null,
    });
    exposePublicSymbol(name, ctor);
  },

  _embind_register_enum_value__deps: ['$createNamedFunction', '$AsciiToString', '$requireRegisteredType'],
  _embind_register_enum_value: (rawEnumType, name, enumValue) => {
    var enumType = requireRegisteredType(rawEnumType, 'enum');
    name = AsciiToString(name);

    var Enum = enumType.constructor;

    var Value = Object.create(enumType.constructor.prototype, {
      value: {value: enumValue},
      constructor: {value: createNamedFunction(`${enumType.name}_${name}`, function() {})},
    });
    Enum.values[enumValue] = Value;
    Enum[name] = Value;
  },

  _embind_register_constant__deps: ['$AsciiToString', '$whenDependentTypesAreResolved'],
  _embind_register_constant: (name, type, value) => {
    name = AsciiToString(name);
    whenDependentTypesAreResolved([], [type], (type) => {
      type = type[0];
      Module[name] = type.fromWireType(value);
      return [];
    });
  },
};

addToLibrary(LibraryEmbind);
