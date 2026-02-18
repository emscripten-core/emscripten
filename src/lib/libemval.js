// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Number of handles reserved for non-use (0) or common values w/o refcount.
{{{
  const EMVAL_RESERVED_HANDLES = 5;
  const EMVAL_LAST_RESERVED_HANDLE = EMVAL_RESERVED_HANDLES * 2 - 1;
}}}
var LibraryEmVal = {
  // Stack of handles available for reuse.
  $emval_freelist: [],
  // Array of alternating pairs (value, refcount).
  // reserve 0 and some special values. These never get de-allocated.
  $emval_handles: [
    0, 1,
    undefined, 1,
    null, 1,
    true, 1,
    false, 1,
  ],
#if ASSERTIONS
  $emval_handles__postset: 'assert(emval_handles.length === {{{ EMVAL_RESERVED_HANDLES }}} * 2)',
#endif
  $emval_symbols: {}, // address -> string

  $count_emval_handles__deps: ['$emval_freelist', '$emval_handles'],
  $count_emval_handles: () => {
    return emval_handles.length / 2 - {{{ EMVAL_RESERVED_HANDLES }}} - emval_freelist.length;
  },

  _emval_register_symbol__deps: ['$emval_symbols', '$AsciiToString'],
  _emval_register_symbol: (address) => {
    emval_symbols[address] = AsciiToString(address);
  },

  $getStringOrSymbol__deps: ['$emval_symbols', '$AsciiToString'],
  $getStringOrSymbol: (address) => {
    var symbol = emval_symbols[address];
    if (symbol === undefined) {
      return AsciiToString(address);
    }
    return symbol;
  },

  $Emval__deps: ['$emval_freelist', '$emval_handles', '$throwBindingError'],
  $Emval: {
    toValue: (handle) => {
      if (!handle) {
          throwBindingError(`Cannot use deleted val. handle = ${handle}`);
      }
  #if ASSERTIONS
      // handle 2 is supposed to be `undefined`.
      assert(handle === 2 || emval_handles[handle] !== undefined && handle % 2 === 0, `invalid handle: ${handle}`);
  #endif
      return emval_handles[handle];
    },

    toHandle: (value) => {
      switch (value) {
        case undefined: return 2;
        case null: return 4;
        case true: return 6;
        case false: return 8;
        default:{
          const handle = emval_freelist.pop() || emval_handles.length;
          emval_handles[handle] = value;
          emval_handles[handle + 1] = 1;
          return handle;
        }
      }
    }
  },

  _emval_incref__deps: ['$emval_handles'],
  _emval_incref: (handle) => {
    if (handle > {{{ EMVAL_LAST_RESERVED_HANDLE }}}) {
      emval_handles[handle + 1] += 1;
    }
  },

  _emval_decref__deps: ['$emval_freelist', '$emval_handles'],
  _emval_decref: (handle) => {
    if (handle > {{{ EMVAL_LAST_RESERVED_HANDLE }}} && 0 === --emval_handles[handle + 1]) {
  #if ASSERTIONS
      assert(emval_handles[handle] !== undefined, `Decref for unallocated handle.`);
  #endif
      emval_handles[handle] = undefined;
      emval_freelist.push(handle);
    }
  },

  _emval_run_destructors__deps: ['_emval_decref', '$Emval', '$runDestructors'],
  _emval_run_destructors: (handle) => {
    var destructors = Emval.toValue(handle);
    runDestructors(destructors);
    __emval_decref(handle);
  },

  _emval_new_array__deps: ['$Emval'],
  _emval_new_array: () => Emval.toHandle([]),

#if !SUPPORT_BIG_ENDIAN
  _emval_new_array_from_memory_view__deps: ['$Emval'],
  _emval_new_array_from_memory_view: (view) => {
    view = Emval.toValue(view);
    // using for..loop is faster than Array.from
    var a = new Array(view.length);
    for (var i = 0; i < view.length; i++) a[i] = view[i];
    return Emval.toHandle(a);
  },
  _emval_array_to_memory_view__deps: ['$Emval'],
  _emval_array_to_memory_view: (dst, src) => {
    dst = Emval.toValue(dst);
    src = Emval.toValue(src);
    dst.set(src);
  },
#else
  _emval_new_array_from_memory_view__deps: ['$Emval'],
  _emval_new_array_from_memory_view: (view) => {
    view = Emval.toValue(view);
    const dv = new DataView(view.buffer, view.byteOffset);
    const reader = {
      Int8Array: dv.getInt8,
      Uint8Array: dv.getUint8,
      Int16Array: dv.getInt16,
      Uint16Array: dv.getUint16,
      Int32Array: dv.getInt32,
      Uint32Array: dv.getUint32,
      BigInt64Array: dv.getBigInt64,
      BigUint64Array: dv.getBigUint64,
      Float32Array: dv.getFloat32,
      Float64Array: dv.getFloat64,
    }[view[Symbol.toStringTag]];
    var a = new Array(view.length);
    for (var i = 0; i < view.length; i++) a[i] = reader.call(dv, i * view.BYTES_PER_ELEMENT, true);
    return Emval.toHandle(a);
  },
  _emval_array_to_memory_view__deps: ['$Emval'],
  _emval_array_to_memory_view: (dst, src) => {
    dst = Emval.toValue(dst);
    src = Emval.toValue(src);
    const dv = new DataView(dst.buffer, dst.byteOffset);
    const writer = {
      Int8Array: dv.setInt8,
      Uint8Array: dv.setUint8,
      Int16Array: dv.setInt16,
      Uint16Array: dv.setUint16,
      Int32Array: dv.setInt32,
      Uint32Array: dv.setUint32,
      BigInt64Array: dv.setBigInt64,
      BigUint64Array: dv.setBigUint64,
      Float32Array: dv.setFloat32,
      Float64Array: dv.setFloat64,
    }[dst[Symbol.toStringTag]];
    for (var i = 0; i < src.length; i++) writer.call(dv, i * dst.BYTES_PER_ELEMENT, src[i], true);
  },
#endif

  _emval_new_object__deps: ['$Emval'],
  _emval_new_object: () => Emval.toHandle({}),

  _emval_new_cstring__deps: ['$getStringOrSymbol', '$Emval'],
  _emval_new_cstring: (v) => Emval.toHandle(getStringOrSymbol(v)),

  _emval_new_u8string__deps: ['$Emval'],
  _emval_new_u8string: (v) => Emval.toHandle(UTF8ToString(v)),

  _emval_new_u16string__deps: ['$Emval'],
  _emval_new_u16string: (v) => Emval.toHandle(UTF16ToString(v)),

  _emval_get_global__deps: ['$Emval', '$getStringOrSymbol'],
  _emval_get_global: (name) => {
    if (!name) {
      return Emval.toHandle(globalThis);
    }
    name = getStringOrSymbol(name);
    return Emval.toHandle(globalThis[name]);
  },

  _emval_get_module_property__deps: ['$getStringOrSymbol', '$Emval'],
  _emval_get_module_property: (name) => {
    name = getStringOrSymbol(name);
    return Emval.toHandle(Module[name]);
  },

  _emval_get_property__deps: ['$Emval'],
  _emval_get_property: (handle, key) => {
    handle = Emval.toValue(handle);
    key = Emval.toValue(key);
    return Emval.toHandle(handle[key]);
  },

  _emval_set_property__deps: ['$Emval'],
  _emval_set_property: (handle, key, value) => {
    handle = Emval.toValue(handle);
    key = Emval.toValue(key);
    value = Emval.toValue(value);
    handle[key] = value;
  },

  $emval_returnValue__deps: ['$Emval'],
  $emval_returnValue: (toReturnWire, destructorsRef, handle) => {
    var destructors = [];
    var result = toReturnWire(destructors, handle);
    if (destructors.length) {
      // void, primitives and any other types w/o destructors don't need to allocate a handle
      {{{ makeSetValue('destructorsRef', '0', 'Emval.toHandle(destructors)', '*') }}};
    }
    return result;
  },

  _emval_equals__deps: ['$Emval'],
  _emval_equals: (first, second) => {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first == second;
  },

  _emval_strictly_equals__deps: ['$Emval'],
  _emval_strictly_equals: (first, second) => {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first === second;
  },

  _emval_greater_than__deps: ['$Emval'],
  _emval_greater_than: (first, second) => {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first > second;
  },

  _emval_less_than__deps: ['$Emval'],
  _emval_less_than: (first, second) => {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first < second;
  },

  _emval_not__deps: ['$Emval'],
  _emval_not: (object) => {
    object = Emval.toValue(object);
    return !object;
  },

  $emval_lookupTypes__deps: ['$requireRegisteredType'],
  $emval_lookupTypes: (argCount, argTypes) => {
    var a = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
      a[i] = requireRegisteredType({{{ makeGetValue('argTypes', `i*${POINTER_SIZE}`, '*') }}},
                                   `parameter ${i}`);
    }
    return a;
  },

  // Leave id 0 undefined.  It's not a big deal, but might be confusing
  // to have null be a valid method caller.
  $emval_methodCallers: [undefined],

  $emval_addMethodCaller__deps: ['$emval_methodCallers'],
  $emval_addMethodCaller: (caller) => {
    var id = emval_methodCallers.length;
    emval_methodCallers.push(caller);
    return id;
  },

  _emval_create_invoker__deps: [
    '$emval_addMethodCaller', '$emval_lookupTypes',
    '$createNamedFunction', '$emval_returnValue',
    '$Emval', '$getStringOrSymbol',
  ],
  _emval_create_invoker: (argCount, argTypesPtr, kind) => {
    var GenericWireTypeSize = {{{ 2 * POINTER_SIZE }}};

    var [retType, ...argTypes] = emval_lookupTypes(argCount, argTypesPtr);
    var toReturnWire = retType.toWireType.bind(retType);
    var argFromPtr = argTypes.map(type => type.readValueFromPointer.bind(type));
    argCount--; // remove the extracted return type

#if DYNAMIC_EXECUTION
    var captures = {'toValue': Emval.toValue};
    var args = argFromPtr.map((argFromPtr, i) => {
      var captureName = `argFromPtr${i}`;
      captures[captureName] = argFromPtr;
      return `${captureName}(args${i ? '+' + i * GenericWireTypeSize : ''})`;
    });
    var functionBody;
    switch (kind){
      case {{{ cDefs['internal::EM_INVOKER_KIND::FUNCTION'] }}}:
        functionBody = 'toValue(handle)';
        break;
      case {{{ cDefs['internal::EM_INVOKER_KIND::CONSTRUCTOR'] }}}:
        functionBody = 'new (toValue(handle))';
        break;
      case {{{ cDefs['internal::EM_INVOKER_KIND::CAST'] }}}:
        functionBody = '';
        break;
      case {{{ cDefs['internal::EM_INVOKER_KIND::METHOD'] }}}:
        captures['getStringOrSymbol'] = getStringOrSymbol;
        functionBody = 'toValue(handle)[getStringOrSymbol(methodName)]';
        break;
    }
    functionBody += `(${args})`;
    if (!retType.isVoid) {
      captures['toReturnWire'] = toReturnWire;
      captures['emval_returnValue'] = emval_returnValue;
      functionBody = `return emval_returnValue(toReturnWire, destructorsRef, ${functionBody})`;
    }
    functionBody = `return function (handle, methodName, destructorsRef, args) {
${functionBody}
}`;

    var invokerFunction = new Function(Object.keys(captures), functionBody)(...Object.values(captures));
#else
    var argN = new Array(argCount);
    var invokerFunction = (handle, methodName, destructorsRef, args) => {
      var offset = 0;
      for (var i = 0; i < argCount; ++i) {
        argN[i] = argFromPtr[i](args + offset);
        offset += GenericWireTypeSize;
      }
      var rv;
      switch (kind) {
        case {{{ cDefs['internal::EM_INVOKER_KIND::FUNCTION'] }}}:
          rv = Emval.toValue(handle).apply(null, argN);
          break;
        case {{{ cDefs['internal::EM_INVOKER_KIND::CONSTRUCTOR'] }}}:
          rv = Reflect.construct(Emval.toValue(handle), argN);
          break;
        case {{{ cDefs['internal::EM_INVOKER_KIND::CAST'] }}}:
          // no-op, just return the argument
          rv = argN[0];
          break;
        case {{{ cDefs['internal::EM_INVOKER_KIND::METHOD'] }}}:
          rv = Emval.toValue(handle)[getStringOrSymbol(methodName)](...argN);
          break;
      }
      return emval_returnValue(toReturnWire, destructorsRef, rv);
    };
#endif
    var functionName = `methodCaller<(${argTypes.map(t => t.name)}) => ${retType.name}>`;
    return emval_addMethodCaller(createNamedFunction(functionName, invokerFunction));
  },

  _emval_invoke__deps: ['$getStringOrSymbol', '$emval_methodCallers', '$Emval'],
  _emval_invoke: (caller, handle, methodName, destructorsRef, args) => {
    return emval_methodCallers[caller](handle, methodName, destructorsRef, args);
  },

  // Same as `_emval_invoke`, just imported into Wasm under a different return type.
  // TODO: remove this if/when https://github.com/emscripten-core/emscripten/issues/20478 is fixed.
  _emval_invoke_i64: '_emval_invoke',

  _emval_typeof__deps: ['$Emval'],
  _emval_typeof: (handle) => {
    handle = Emval.toValue(handle);
    return Emval.toHandle(typeof handle);
  },

  _emval_instanceof__deps: ['$Emval'],
  _emval_instanceof: (object, constructor) => {
    object = Emval.toValue(object);
    constructor = Emval.toValue(constructor);
    return object instanceof constructor;
  },

  _emval_is_number__deps: ['$Emval'],
  _emval_is_number: (handle) => {
    handle = Emval.toValue(handle);
    return typeof handle == 'number';
  },

  _emval_is_string__deps: ['$Emval'],
  _emval_is_string: (handle) => {
    handle = Emval.toValue(handle);
    return typeof handle == 'string';
  },

  _emval_in__deps: ['$Emval'],
  _emval_in: (item, object) => {
    item = Emval.toValue(item);
    object = Emval.toValue(object);
    return item in object;
  },

  _emval_delete__deps: ['$Emval'],
  _emval_delete: (object, property) => {
    object = Emval.toValue(object);
    property = Emval.toValue(property);
    return delete object[property];
  },

  _emval_throw__deps: ['$Emval',
#if !WASM_EXCEPTIONS
    '$exceptionLast',
#endif
  ],
  _emval_throw: (object) => {
    object = Emval.toValue(object);
#if !WASM_EXCEPTIONS
    // If we are throwing Emcripten C++ exception, set exceptionLast, as we do
    // in __cxa_throw. When EXCEPTION_STACK_TRACES is set, a C++ exception will
    // be an instance of EmscriptenEH, and when EXCEPTION_STACK_TRACES is not
    // set, it will be a pointer (number).
    //
    // This is different from __cxa_throw() in libexception.js because
    // __cxa_throw() is called from the user C++ code when the 'throw' keyword
    // is used, and the value thrown is a C++ pointer. When
    // EXCEPTION_STACK_TRACES is true, we wrap it with CppException. But this
    // _emval_throw is called when we throw whatever is contained in 'object',
    // which can be anything including a CppException object, or a number, or
    // other JS object. So we don't use storeException() wrapper here and we
    // throw it as is.
#if EXCEPTION_STACK_TRACES
    if (object instanceof CppException) {
      exceptionLast = object;
    }
#else
    if (object === object+0) { // Check if it is a number
      exceptionLast = object;
    }
#endif
#endif
    throw object;
  },

#if ASYNCIFY
  _emval_await__deps: ['$Emval', '$Asyncify'],
  _emval_await__async: 'auto',
  _emval_await: async (promise) => {
    var value = await Emval.toValue(promise);
    return Emval.toHandle(value);
  },
#endif

  _emval_iter_begin__deps: ['$Emval'],
  _emval_iter_begin: (iterable) => {
    iterable = Emval.toValue(iterable);
    return Emval.toHandle(iterable[Symbol.iterator]());
  },

  _emval_iter_next__deps: ['$Emval'],
  _emval_iter_next: (iterator) => {
    iterator = Emval.toValue(iterator);
    var result = iterator.next();
    return result.done ? 0 : Emval.toHandle(result.value);
  },

  _emval_coro_suspend__deps: ['$Emval', '_emval_coro_resume',  '_emval_coro_reject'],
  _emval_coro_suspend: (promiseHandle, awaiterPtr) => {
    Emval.toValue(promiseHandle)
      .then((result) => __emval_coro_resume(awaiterPtr, Emval.toHandle(result)),
            (error) => __emval_coro_reject(awaiterPtr, Emval.toHandle(error)));
  },

  _emval_coro_make_promise__deps: ['$Emval'],
  _emval_coro_make_promise: (resolveHandlePtr, rejectHandlePtr) => {
    return Emval.toHandle(new Promise((resolve, reject) => {
      {{{ makeSetValue('resolveHandlePtr', '0', 'Emval.toHandle(resolve)', '*') }}};
      {{{ makeSetValue('rejectHandlePtr', '0', 'Emval.toHandle(reject)', '*') }}};
    }));
  },

  _emval_from_current_cxa_exception__deps: ['$Emval', '__cxa_rethrow'],
  _emval_from_current_cxa_exception: () => {
    try {
      // Use __cxa_rethrow which already has mechanism for generating
      // user-friendly error message and stacktrace from C++ exception
      // if EXCEPTION_STACK_TRACES is enabled and numeric exception
      // with metadata optimised out otherwise.
      ___cxa_rethrow();
    } catch (e) {
      return Emval.toHandle(e);
    }
  },
};

addToLibrary(LibraryEmVal);
