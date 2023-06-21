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
/*global getStringOrSymbol, emval_handles, Emval, __emval_unregister, count_emval_handles, emval_symbols, __emval_decref, emval_newers*/
/*global craftEmvalAllocator, emval_addMethodCaller, emval_methodCallers, LibraryManager, mergeInto, emval_allocateDestructors, global, emval_lookupTypes, makeLegalFunctionName*/
/*global emval_get_global*/

var LibraryEmVal = {
  $emval_handles__deps: ['$HandleAllocator'],
  $emval_handles: "new HandleAllocator();",
  $emval_symbols: {}, // address -> string

  $init_emval__deps: ['$count_emval_handles', '$emval_handles'],
  $init_emval__postset: 'init_emval();',
  $init_emval: function() {
    // reserve some special values. These never get de-allocated.
    // The HandleAllocator takes care of reserving zero.
    emval_handles.allocated.push(
      {value: undefined},
      {value: null},
      {value: true},
      {value: false},
    );
    emval_handles.reserved = emval_handles.allocated.length
    Module['count_emval_handles'] = count_emval_handles;
  },

  $count_emval_handles__deps: ['$emval_handles'],
  $count_emval_handles: function() {
    var count = 0;
    for (var i = emval_handles.reserved; i < emval_handles.allocated.length; ++i) {
      if (emval_handles.allocated[i] !== undefined) {
        ++count;
      }
    }
    return count;
  },

  _emval_register_symbol__deps: ['$emval_symbols', '$readLatin1String'],
  _emval_register_symbol: function(address) {
    emval_symbols[address] = readLatin1String(address);
  },

  $getStringOrSymbol__deps: ['$emval_symbols', '$readLatin1String'],
  $getStringOrSymbol: function(address) {
    var symbol = emval_symbols[address];
    if (symbol === undefined) {
      return readLatin1String(address);
    }
    return symbol;
  },

  $Emval__deps: ['$emval_handles', '$throwBindingError', '$init_emval'],
  $Emval: {
    toValue: (handle) => {
      if (!handle) {
          throwBindingError('Cannot use deleted val. handle = ' + handle);
      }
      return emval_handles.get(handle).value;
    },

    toHandle: (value) => {
      switch (value) {
        case undefined: return 1;
        case null: return 2;
        case true: return 3;
        case false: return 4;
        default:{
          return emval_handles.allocate({refcount: 1, value: value});
        }
      }
    }
  },

  _emval_incref__deps: ['$emval_handles'],
  _emval_incref: function(handle) {
    if (handle > 4) {
      emval_handles.get(handle).refcount += 1;
    }
  },

  _emval_decref__deps: ['$emval_handles'],
  _emval_decref: function(handle) {
    if (handle >= emval_handles.reserved && 0 === --emval_handles.get(handle).refcount) {
      emval_handles.free(handle);
    }
  },

  _emval_run_destructors__deps: ['_emval_decref', '$Emval', '$runDestructors'],
  _emval_run_destructors: function(handle) {
    var destructors = Emval.toValue(handle);
    runDestructors(destructors);
    __emval_decref(handle);
  },

  _emval_new_array__deps: ['$Emval'],
  _emval_new_array: function() {
    return Emval.toHandle([]);
  },

  _emval_new_array_from_memory_view__deps: ['$Emval'],
  _emval_new_array_from_memory_view: function(view) {
    view = Emval.toValue(view);
    // using for..loop is faster than Array.from
    var a = new Array(view.length);
    for (var i = 0; i < view.length; i++) a[i] = view[i];
    return Emval.toHandle(a);
  },

  _emval_new_object__deps: ['$Emval'],
  _emval_new_object: function() {
    return Emval.toHandle({});
  },

  _emval_new_cstring__deps: ['$getStringOrSymbol', '$Emval'],
  _emval_new_cstring: function(v) {
    return Emval.toHandle(getStringOrSymbol(v));
  },

  _emval_new_u8string__deps: ['$Emval'],
  _emval_new_u8string: function(v) {
    return Emval.toHandle(UTF8ToString(v));
  },

  _emval_new_u16string__deps: ['$Emval'],
  _emval_new_u16string: function(v) {
    return Emval.toHandle(UTF16ToString(v));
  },

  _emval_take_value__deps: ['$Emval', '$requireRegisteredType'],
  _emval_take_value: function(type, arg) {
    type = requireRegisteredType(type, '_emval_take_value');
    var v = type['readValueFromPointer'](arg);
    return Emval.toHandle(v);
  },

  $emval_newers: {}, // arity -> function
  $craftEmvalAllocator__deps: ['$Emval', '$requireRegisteredType'],
  $craftEmvalAllocator: function(argCount) {
    /*This function returns a new function that looks like this:
    function emval_allocator_3(constructor, argTypes, args) {
        var argType0 = requireRegisteredType(HEAP32[(argTypes >> 2)], "parameter 0");
        var arg0 = argType0['readValueFromPointer'](args);
        var argType1 = requireRegisteredType(HEAP32[(argTypes >> 2) + 1], "parameter 1");
        var arg1 = argType1['readValueFromPointer'](args + 8);
        var argType2 = requireRegisteredType(HEAP32[(argTypes >> 2) + 2], "parameter 2");
        var arg2 = argType2['readValueFromPointer'](args + 16);
        var obj = new constructor(arg0, arg1, arg2);
        return Emval.toHandle(obj);
    } */
#if !DYNAMIC_EXECUTION
    var argsList = new Array(argCount + 1);
    return function(constructor, argTypes, args) {
      argsList[0] = constructor;
      for (var i = 0; i < argCount; ++i) {
        var argType = requireRegisteredType({{{ makeGetValue('argTypes', 'i * ' + POINTER_SIZE, '*') }}}, 'parameter ' + i);
        argsList[i + 1] = argType['readValueFromPointer'](args);
        args += argType['argPackAdvance'];
      }
      var obj = new (constructor.bind.apply(constructor, argsList));
      return Emval.toHandle(obj);
    };
#else
    var argsList = "";
    for (var i = 0; i < argCount; ++i) {
      argsList += (i!==0?", ":"")+"arg"+i; // 'arg0, arg1, ..., argn'
    }

    // The body of the generated function does not have access to enclosing
    // scope where HEAPU64/HEAPU32/etc are defined, and we cannot pass them
    // directly as arguments (like we do the Module object) since memory
    // growth can cause them to be re-bound.
    var getMemory = () => {{{ MEMORY64 ? "HEAPU64" : "HEAPU32" }}};

    var functionBody =
        "return function emval_allocator_"+argCount+"(constructor, argTypes, args) {\n" +
        "  var {{{ MEMORY64 ? 'HEAPU64' : 'HEAPU32' }}} = getMemory();\n";

    for (var i = 0; i < argCount; ++i) {
        functionBody +=
            "var argType"+i+" = requireRegisteredType({{{ makeGetValue('argTypes', '0', '*') }}}, 'parameter "+i+"');\n" +
            "var arg"+i+" = argType"+i+".readValueFromPointer(args);\n" +
            "args += argType"+i+"['argPackAdvance'];\n" +
            "argTypes += {{{ POINTER_SIZE }}};\n";
    }
    functionBody +=
        "var obj = new constructor("+argsList+");\n" +
        "return valueToHandle(obj);\n" +
        "}\n";

    /*jshint evil:true*/
    return (new Function("requireRegisteredType", "Module", "valueToHandle", "getMemory" , functionBody))(
        requireRegisteredType, Module, Emval.toHandle, getMemory);
#endif
  },

  _emval_new__deps: ['$craftEmvalAllocator', '$emval_newers', '$Emval'],
  _emval_new: function(handle, argCount, argTypes, args) {
    handle = Emval.toValue(handle);

    var newer = emval_newers[argCount];
    if (!newer) {
      newer = craftEmvalAllocator(argCount);
      emval_newers[argCount] = newer;
    }

    return newer(handle, argTypes, args);
  },

#if !DYNAMIC_EXECUTION
  $emval_get_global: function() {
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
  $emval_get_global: function() {
    if (typeof globalThis == 'object') {
      return globalThis;
    }
    return (function(){
      return Function;
    })()('return this')();
  },
#endif
  _emval_get_global__deps: ['$Emval', '$getStringOrSymbol', '$emval_get_global'],
  _emval_get_global: function(name) {
    if (name===0) {
      return Emval.toHandle(emval_get_global());
    } else {
      name = getStringOrSymbol(name);
      return Emval.toHandle(emval_get_global()[name]);
    }
  },

  _emval_get_module_property__deps: ['$getStringOrSymbol', '$Emval'],
  _emval_get_module_property: function(name) {
    name = getStringOrSymbol(name);
    return Emval.toHandle(Module[name]);
  },

  _emval_get_property__deps: ['$Emval'],
  _emval_get_property: function(handle, key) {
    handle = Emval.toValue(handle);
    key = Emval.toValue(key);
    return Emval.toHandle(handle[key]);
  },

  _emval_set_property__deps: ['$Emval'],
  _emval_set_property: function(handle, key, value) {
    handle = Emval.toValue(handle);
    key = Emval.toValue(key);
    value = Emval.toValue(value);
    handle[key] = value;
  },

  _emval_as__deps: ['$Emval', '$requireRegisteredType'],
  _emval_as: function(handle, returnType, destructorsRef) {
    handle = Emval.toValue(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    var destructors = [];
    var rd = Emval.toHandle(destructors);
    {{{ makeSetValue('destructorsRef', '0', 'rd', '*') }}};
    return returnType['toWireType'](destructors, handle);
  },

  _emval_as_int64__deps: ['$Emval', '$requireRegisteredType'],
  _emval_as_int64: function(handle, returnType) {
    handle = Emval.toValue(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    return returnType['toWireType'](null, handle);
  },

  _emval_as_uint64__deps: ['$Emval', '$requireRegisteredType'],
  _emval_as_uint64: function(handle, returnType) {
    handle = Emval.toValue(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    return returnType['toWireType'](null, handle);
  },

  _emval_equals__deps: ['$Emval'],
  _emval_equals: function(first, second) {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first == second;
  },

  _emval_strictly_equals__deps: ['$Emval'],
  _emval_strictly_equals: function(first, second) {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first === second;
  },

  _emval_greater_than__deps: ['$Emval'],
  _emval_greater_than: function(first, second) {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first > second;
  },

  _emval_less_than__deps: ['$Emval'],
  _emval_less_than: function(first, second) {
    first = Emval.toValue(first);
    second = Emval.toValue(second);
    return first < second;
  },

  _emval_not__deps: ['$Emval'],
  _emval_not: function(object) {
    object = Emval.toValue(object);
    return !object;
  },

  _emval_call__deps: ['$emval_lookupTypes', '$Emval'],
  _emval_call: function(handle, argCount, argTypes, argv) {
    handle = Emval.toValue(handle);
    var types = emval_lookupTypes(argCount, argTypes);

    var args = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
      var type = types[i];
      args[i] = type['readValueFromPointer'](argv);
      argv += type['argPackAdvance'];
    }

    var rv = handle.apply(undefined, args);
    return Emval.toHandle(rv);
  },

  $emval_lookupTypes__deps: ['$requireRegisteredType'],
  $emval_lookupTypes: function(argCount, argTypes) {
    var a = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
      a[i] = requireRegisteredType({{{ makeGetValue('argTypes', 'i * ' + POINTER_SIZE, '*') }}},
                                   "parameter " + i);
    }
    return a;
  },

  $emval_allocateDestructors__deps: ['$Emval'],
  $emval_allocateDestructors: function(destructorsRef) {
    var destructors = [];
    {{{ makeSetValue('destructorsRef', '0', 'Emval.toHandle(destructors)', '*') }}};
    return destructors;
  },

  // Leave id 0 undefined.  It's not a big deal, but might be confusing
  // to have null be a valid method caller.
  $emval_methodCallers: [undefined],

  $emval_addMethodCaller__deps: ['$emval_methodCallers'],
  $emval_addMethodCaller: function(caller) {
    var id = emval_methodCallers.length;
    emval_methodCallers.push(caller);
    return id;
  },

  $emval_registeredMethods: [],
  _emval_get_method_caller__deps: [
    '$emval_addMethodCaller', '$emval_lookupTypes',,
    '$makeLegalFunctionName', '$emval_registeredMethods',
#if DYNAMIC_EXECUTION
    '$newFunc',
#endif
  ],
  _emval_get_method_caller: function(argCount, argTypes) {
    var types = emval_lookupTypes(argCount, argTypes);
    var retType = types[0];
    var signatureName = retType.name + "_$" + types.slice(1).map(function (t) { return t.name; }).join("_") + "$";
    var returnId = emval_registeredMethods[signatureName];
    if (returnId !== undefined) {
      return returnId;
    }

#if !DYNAMIC_EXECUTION
    var argN = new Array(argCount - 1);
    var invokerFunction = (handle, name, destructors, args) => {
      var offset = 0;
      for (var i = 0; i < argCount - 1; ++i) {
        argN[i] = types[i + 1]['readValueFromPointer'](args + offset);
        offset += types[i + 1]['argPackAdvance'];
      }
      var rv = handle[name].apply(handle, argN);
      for (var i = 0; i < argCount - 1; ++i) {
        if (types[i + 1].deleteObject) {
          types[i + 1].deleteObject(argN[i]);
        }
      }
      if (!retType.isVoid) {
        return retType['toWireType'](destructors, rv);
      }
    };
#else
    var params = ["retType"];
    var args = [retType];

    var argsList = ""; // 'arg0, arg1, arg2, ... , argN'
    for (var i = 0; i < argCount - 1; ++i) {
      argsList += (i !== 0 ? ", " : "") + "arg" + i;
      params.push("argType" + i);
      args.push(types[1 + i]);
    }

    var functionName = makeLegalFunctionName("methodCaller_" + signatureName);
    var functionBody =
        "return function " + functionName + "(handle, name, destructors, args) {\n";

    var offset = 0;
    for (var i = 0; i < argCount - 1; ++i) {
        functionBody +=
        "    var arg" + i + " = argType" + i + ".readValueFromPointer(args" + (offset ? ("+"+offset) : "") + ");\n";
        offset += types[i + 1]['argPackAdvance'];
    }
    functionBody +=
        "    var rv = handle[name](" + argsList + ");\n";
    for (var i = 0; i < argCount - 1; ++i) {
        if (types[i + 1]['deleteObject']) {
            functionBody +=
            "    argType" + i + ".deleteObject(arg" + i + ");\n";
        }
    }
    if (!retType.isVoid) {
        functionBody +=
        "    return retType.toWireType(destructors, rv);\n";
    }
    functionBody +=
        "};\n";

    params.push(functionBody);
    var invokerFunction = newFunc(Function, params).apply(null, args);
#endif
    returnId = emval_addMethodCaller(invokerFunction);
    emval_registeredMethods[signatureName] = returnId;
    return returnId;
  },

  _emval_call_method__deps: ['$emval_allocateDestructors', '$getStringOrSymbol', '$emval_methodCallers', '$Emval'],
  _emval_call_method: function(caller, handle, methodName, destructorsRef, args) {
    caller = emval_methodCallers[caller];
    handle = Emval.toValue(handle);
    methodName = getStringOrSymbol(methodName);
    return caller(handle, methodName, emval_allocateDestructors(destructorsRef), args);
  },

  _emval_call_void_method__deps: ['$emval_allocateDestructors', '$getStringOrSymbol', '$emval_methodCallers', '$Emval'],
  _emval_call_void_method: function(caller, handle, methodName, args) {
    caller = emval_methodCallers[caller];
    handle = Emval.toValue(handle);
    methodName = getStringOrSymbol(methodName);
    caller(handle, methodName, null, args);
  },

  _emval_typeof__deps: ['$Emval'],
  _emval_typeof: function(handle) {
    handle = Emval.toValue(handle);
    return Emval.toHandle(typeof handle);
  },

  _emval_instanceof__deps: ['$Emval'],
  _emval_instanceof: function(object, constructor) {
    object = Emval.toValue(object);
    constructor = Emval.toValue(constructor);
    return object instanceof constructor;
  },

  _emval_is_number__deps: ['$Emval'],
  _emval_is_number: function(handle) {
    handle = Emval.toValue(handle);
    return typeof handle == 'number';
  },

  _emval_is_string__deps: ['$Emval'],
  _emval_is_string: function(handle) {
    handle = Emval.toValue(handle);
    return typeof handle == 'string';
  },

  _emval_in__deps: ['$Emval'],
  _emval_in: function(item, object) {
    item = Emval.toValue(item);
    object = Emval.toValue(object);
    return item in object;
  },

  _emval_delete__deps: ['$Emval'],
  _emval_delete: function(object, property) {
    object = Emval.toValue(object);
    property = Emval.toValue(property);
    return delete object[property];
  },

  _emval_throw__deps: ['$Emval'],
  _emval_throw: function(object) {
    object = Emval.toValue(object);
    throw object;
  },

#if ASYNCIFY
  _emval_await__deps: ['$Emval', '$Asyncify'],
  _emval_await__async: true,
  _emval_await: function(promise) {
    return Asyncify.handleAsync(() => {
      promise = Emval.toValue(promise);
      return promise.then(Emval.toHandle);
    });
  },
#endif
};

mergeInto(LibraryManager.library, LibraryEmVal);
