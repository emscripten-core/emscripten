// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

/*global Module:true, Runtime*/
/*global HEAP32*/
/*global newFunc*/
/*global createNamedFunction*/
/*global readLatin1String, stringToUTF8*/
/*global requireRegisteredType, throwBindingError, runDestructors*/
/*jslint sub:true*/ /* The symbols 'fromWireType' and 'toWireType' must be accessed via array notation to be closure-safe since craftInvokerFunction crafts functions as strings that can't be closured. */

// -- jshint doesn't understand library syntax, so we need to mark the symbols exposed here
/*global getStringOrSymbol, emval_freelist, emval_handles, Emval, __emval_unregister, count_emval_handles, emval_symbols, __emval_decref*/
/*global emval_addMethodCaller, emval_methodCallers, addToLibrary, global, emval_lookupTypes, makeLegalFunctionName*/
/*global emval_get_global*/

// Number of handles reserved for non-use (0) or common values w/o refcount.
{{{ 
  globalThis.EMVAL_RESERVED_HANDLES = 5;
  globalThis.EMVAL_LAST_RESERVED_HANDLE = globalThis.EMVAL_RESERVED_HANDLES * 2 - 1;
  null;
}}}
var LibraryEmVal = {
  // Stack of handles available for reuse.
  $emval_freelist: [],
  // Array of alternating pairs (value, refcount).
  $emval_handles: [],
  $emval_symbols: {}, // address -> string

  $init_emval__deps: ['$count_emval_handles', '$emval_handles'],
  $init_emval__postset: 'init_emval();',
  $init_emval: () => {
    // reserve 0 and some special values. These never get de-allocated.
    emval_handles.push(
      0, 1,
      undefined, 1,
      null, 1,
      true, 1,
      false, 1,
    );
  #if ASSERTIONS
    assert(emval_handles.length === {{{ EMVAL_RESERVED_HANDLES }}} * 2);
  #endif
    Module['count_emval_handles'] = count_emval_handles;
  },

  $count_emval_handles__deps: ['$emval_freelist', '$emval_handles'],
  $count_emval_handles: () => {
    return emval_handles.length / 2 - {{{ EMVAL_RESERVED_HANDLES }}} - emval_freelist.length;
  },

  _emval_register_symbol__deps: ['$emval_symbols', '$readLatin1String'],
  _emval_register_symbol: (address) => {
    emval_symbols[address] = readLatin1String(address);
  },

  $getStringOrSymbol__deps: ['$emval_symbols', '$readLatin1String'],
  $getStringOrSymbol: (address) => {
    var symbol = emval_symbols[address];
    if (symbol === undefined) {
      return readLatin1String(address);
    }
    return symbol;
  },

  $Emval__deps: ['$emval_freelist', '$emval_handles', '$throwBindingError', '$init_emval'],
  $Emval: {
    toValue: (handle) => {
      if (!handle) {
          throwBindingError('Cannot use deleted val. handle = ' + handle);
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

  _emval_new_array_from_memory_view__deps: ['$Emval'],
  _emval_new_array_from_memory_view: (view) => {
    view = Emval.toValue(view);
    // using for..loop is faster than Array.from
    var a = new Array(view.length);
    for (var i = 0; i < view.length; i++) a[i] = view[i];
    return Emval.toHandle(a);
  },

  _emval_new_object__deps: ['$Emval'],
  _emval_new_object: () => Emval.toHandle({}),

  _emval_new_cstring__deps: ['$getStringOrSymbol', '$Emval'],
  _emval_new_cstring: (v) => Emval.toHandle(getStringOrSymbol(v)),

  _emval_new_u8string__deps: ['$Emval'],
  _emval_new_u8string: (v) => Emval.toHandle(UTF8ToString(v)),

  _emval_new_u16string__deps: ['$Emval'],
  _emval_new_u16string: (v) => Emval.toHandle(UTF16ToString(v)),

  _emval_take_value__deps: ['$Emval', '$requireRegisteredType'],
  _emval_take_value: (type, arg) => {
    type = requireRegisteredType(type, '_emval_take_value');
    var v = type['readValueFromPointer'](arg);
    return Emval.toHandle(v);
  },

#if !DYNAMIC_EXECUTION
  $emval_get_global: () => {
    if (typeof globalThis == 'object') {
      return globalThis;
    }
    function testGlobal(obj) {
      obj['$$$embind_global$$$'] = obj;
      var success = typeof $$$embind_global$$$ == 'object' && obj['$$$embind_global$$$'] == obj;
      if (!success) {
        delete obj['$$$embind_global$$$'];
      }
      return success;
    }
    if (typeof $$$embind_global$$$ == 'object') {
      return $$$embind_global$$$;
    }
    if (typeof global == 'object' && testGlobal(global)) {
      $$$embind_global$$$ = global;
    } else if (typeof self == 'object' && testGlobal(self)) {
      $$$embind_global$$$ = self; // This works for both "window" and "self" (Web Workers) global objects
    }
    if (typeof $$$embind_global$$$ == 'object') {
      return $$$embind_global$$$;
    }
    throw Error('unable to get global object.');
  },
#else
  // appease jshint (technically this code uses eval)
  $emval_get_global: () => {
    if (typeof globalThis == 'object') {
      return globalThis;
    }
    return (function(){
      return Function;
    })()('return this')();
  },
#endif
  _emval_get_global__deps: ['$Emval', '$getStringOrSymbol', '$emval_get_global'],
  _emval_get_global: (name) => {
    if (name===0) {
      return Emval.toHandle(emval_get_global());
    } else {
      name = getStringOrSymbol(name);
      return Emval.toHandle(emval_get_global()[name]);
    }
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
  $emval_returnValue: (returnType, destructorsRef, handle) => {
    var destructors = [];
    var result = returnType['toWireType'](destructors, handle);
    if (destructors.length) {
      // void, primitives and any other types w/o destructors don't need to allocate a handle
      {{{ makeSetValue('destructorsRef', '0', 'Emval.toHandle(destructors)', '*') }}};
    }
    return result;
  },

  _emval_as__deps: ['$Emval', '$requireRegisteredType', '$emval_returnValue'],
  _emval_as: (handle, returnType, destructorsRef) => {
    handle = Emval.toValue(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    return emval_returnValue(returnType, destructorsRef, handle);
  },

  _emval_as_int64__deps: ['$Emval', '$requireRegisteredType'],
  _emval_as_int64: (handle, returnType) => {
    handle = Emval.toValue(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    return returnType['toWireType'](null, handle);
  },

  _emval_as_uint64__deps: ['$Emval', '$requireRegisteredType'],
  _emval_as_uint64: (handle, returnType) => {
    handle = Emval.toValue(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    return returnType['toWireType'](null, handle);
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

  _emval_call__deps: ['$emval_methodCallers', '$Emval'],
  _emval_call: (caller, handle, destructorsRef, args) => {
    caller = emval_methodCallers[caller];
    handle = Emval.toValue(handle);
    return caller(null, handle, destructorsRef, args);
  },

  $emval_lookupTypes__deps: ['$requireRegisteredType'],
  $emval_lookupTypes: (argCount, argTypes) => {
    var a = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
      a[i] = requireRegisteredType({{{ makeGetValue('argTypes', 'i * ' + POINTER_SIZE, '*') }}},
                                   "parameter " + i);
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

#if MIN_CHROME_VERSION < 49 || MIN_FIREFOX_VERSION < 42 || MIN_SAFARI_VERSION < 100101
  $reflectConstruct: null,
  $reflectConstruct__postset: `
    if (typeof Reflect != 'undefined') {
      reflectConstruct = Reflect.construct;
    } else {
      reflectConstruct = function(target, args) {
        // limited polyfill for Reflect.construct that handles variadic args and native objects, but not new.target
        return new (target.bind.apply(target, [null].concat(args)))();
      };
    }
  `,
#else
  $reflectConstruct: 'Reflect.construct',
#endif

  _emval_get_method_caller__deps: [
    '$emval_addMethodCaller', '$emval_lookupTypes',
    '$createNamedFunction',
    '$reflectConstruct', '$emval_returnValue',
#if DYNAMIC_EXECUTION
    '$newFunc',
#endif
  ],
  _emval_get_method_caller: (argCount, argTypes, kind) => {
    var types = emval_lookupTypes(argCount, argTypes);
    var retType = types.shift();
    argCount--; // remove the shifted off return type

#if !DYNAMIC_EXECUTION
    var argN = new Array(argCount);
    var invokerFunction = (obj, func, destructorsRef, args) => {
      var offset = 0;
      for (var i = 0; i < argCount; ++i) {
        argN[i] = types[i]['readValueFromPointer'](args + offset);
        offset += types[i].argPackAdvance;
      }
      var rv = kind === /* CONSTRUCTOR */ 1 ? reflectConstruct(func, argN) : func.apply(obj, argN);
      return emval_returnValue(retType, destructorsRef, rv);
    };
#else
    var functionBody =
      `return function (obj, func, destructorsRef, args) {\n`;

    var offset = 0;
    var argsList = []; // 'obj?, arg0, arg1, arg2, ... , argN'
    if (kind === /* FUNCTION */ 0) {
      argsList.push("obj");
    }
    var params = ["retType"];
    var args = [retType];
    for (var i = 0; i < argCount; ++i) {
      argsList.push("arg" + i);
      params.push("argType" + i);
      args.push(types[i]);
      functionBody +=
        `  var arg${i} = argType${i}.readValueFromPointer(args${offset ? "+" + offset : ""});\n`;
      offset += types[i].argPackAdvance;
    }
    var invoker = kind === /* CONSTRUCTOR */ 1 ? 'new func' : 'func.call';
    functionBody +=
      `  var rv = ${invoker}(${argsList.join(", ")});\n`;
    if (!retType.isVoid) {
      params.push("emval_returnValue");
      args.push(emval_returnValue);
      functionBody +=
        "  return emval_returnValue(retType, destructorsRef, rv);\n";
    }
    functionBody +=
      "};\n";

    params.push(functionBody);
    var invokerFunction = newFunc(Function, params)(...args);
#endif
    var functionName = `methodCaller<(${types.map(t => t.name).join(', ')}) => ${retType.name}>`;
    return emval_addMethodCaller(createNamedFunction(functionName, invokerFunction));
  },

  _emval_call_method__deps: ['$getStringOrSymbol', '$emval_methodCallers', '$Emval'],
  _emval_call_method: (caller, objHandle, methodName, destructorsRef, args) => {
    caller = emval_methodCallers[caller];
    objHandle = Emval.toValue(objHandle);
    methodName = getStringOrSymbol(methodName);
    return caller(objHandle, objHandle[methodName], destructorsRef, args);
  },

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

  _emval_throw__deps: ['$Emval'],
  _emval_throw: (object) => {
    object = Emval.toValue(object);
    throw object;
  },

#if ASYNCIFY
  _emval_await__deps: ['$Emval', '$Asyncify'],
  _emval_await__async: true,
  _emval_await: (promise) => {
    return Asyncify.handleAsync(() => {
      promise = Emval.toValue(promise);
      return promise.then(Emval.toHandle);
    });
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

  _emval_coro_suspend__deps: ['$Emval', '_emval_coro_resume'],
  _emval_coro_suspend: (promiseHandle, awaiterPtr) => {
    Emval.toValue(promiseHandle).then(result => {
      __emval_coro_resume(awaiterPtr, Emval.toHandle(result));
    });
  },

  _emval_coro_make_promise__deps: ['$Emval', '__cxa_rethrow'],
  _emval_coro_make_promise: (resolveHandlePtr, rejectHandlePtr) => {
    return Emval.toHandle(new Promise((resolve, reject) => {
      const rejectWithCurrentException = () => {
        try {
          // Use __cxa_rethrow which already has mechanism for generating
          // user-friendly error message and stacktrace from C++ exception
          // if EXCEPTION_STACK_TRACES is enabled and numeric exception
          // with metadata optimised out otherwise.
          ___cxa_rethrow();
        } catch (e) {
          // But catch it so that it rejects the promise instead of throwing
          // in an unpredictable place during async execution.
          reject(e);
        }
      };

      {{{ makeSetValue('resolveHandlePtr', '0', 'Emval.toHandle(resolve)', '*') }}};
      {{{ makeSetValue('rejectHandlePtr', '0', 'Emval.toHandle(rejectWithCurrentException)', '*') }}};
    }));
  },
};

addToLibrary(LibraryEmVal);
