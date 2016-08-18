/*global Module:true, Runtime*/
/*global HEAP32*/
/*global new_*/
/*global createNamedFunction*/
/*global readLatin1String, stringToUTF8*/
/*global requireRegisteredType, throwBindingError, runDestructors*/
/*jslint sub:true*/ /* The symbols 'fromWireType' and 'toWireType' must be accessed via array notation to be closure-safe since craftInvokerFunction crafts functions as strings that can't be closured. */

// -- jshint doesn't understand library syntax, so we need to mark the symbols exposed here
/*global getStringOrSymbol, emval_handle_array, __emval_register, __emval_unregister, requireHandle, count_emval_handles, emval_symbols, emval_free_list, get_first_emval, __emval_decref, emval_newers*/
/*global craftEmvalAllocator, __emval_addMethodCaller, emval_methodCallers, LibraryManager, mergeInto, __emval_allocateDestructors, global, __emval_lookupTypes, makeLegalFunctionName*/
/*global emval_get_global*/

var LibraryEmVal = {
  $emval_handle_array: [{},
    {value: undefined},{value: null},
    {value: true},{value: false}], // reserve zero and special values
  $emval_free_list: [],
  $emval_symbols: {}, // address -> string

  $init_emval__deps: ['$count_emval_handles', '$get_first_emval'],
  $init_emval__postset: 'init_emval();',
  $init_emval: function() {
    Module['count_emval_handles'] = count_emval_handles;
    Module['get_first_emval'] = get_first_emval;
  },

  $count_emval_handles__deps: ['$emval_handle_array'],
  $count_emval_handles: function() {
    var count = 0;
    for (var i = 5; i < emval_handle_array.length; ++i) {
        if (emval_handle_array[i] !== undefined) {
            ++count;
        }
    }
    return count;
  },

  $get_first_emval__deps: ['$emval_handle_array'],
  $get_first_emval: function() {
    for (var i = 5; i < emval_handle_array.length; ++i) {
        if (emval_handle_array[i] !== undefined) {
            return emval_handle_array[i];
        }
    }
    return null;
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
    } else {
        return symbol;
    }
  },

  $requireHandle__deps: ['$emval_handle_array', '$throwBindingError'],
  $requireHandle: function(handle) {
    if (!handle) {
        throwBindingError('Cannot use deleted val. handle = ' + handle);
    }
    return emval_handle_array[handle].value;
  },

  _emval_register__deps: ['$emval_free_list', '$emval_handle_array', '$init_emval'],
  _emval_register: function(value) {

    switch(value){
      case undefined :{ return 1; }
      case null :{ return 2; }
      case true :{ return 3; }
      case false :{ return 4; }
      default:{
        var handle = emval_free_list.length ?
            emval_free_list.pop() :
            emval_handle_array.length;

        emval_handle_array[handle] = {refcount: 1, value: value};
        return handle;
        }
      }
  },

  _emval_incref__deps: ['$emval_handle_array'],
  _emval_incref: function(handle) {
    if (handle > 4) {
        emval_handle_array[handle].refcount += 1;
    }
  },

  _emval_decref__deps: ['$emval_free_list', '$emval_handle_array'],
  _emval_decref: function(handle) {
    if (handle > 4 && 0 === --emval_handle_array[handle].refcount) {
        emval_handle_array[handle] = undefined;
        emval_free_list.push(handle);
    }
  },

  _emval_run_destructors__deps: ['_emval_decref', '$emval_handle_array', '$runDestructors'],
  _emval_run_destructors: function(handle) {
    var destructors = emval_handle_array[handle].value;
    runDestructors(destructors);
    __emval_decref(handle);
  },

  _emval_new_array__deps: ['_emval_register'],
  _emval_new_array: function() {
    return __emval_register([]);
  },

  _emval_new_object__deps: ['_emval_register'],
  _emval_new_object: function() {
    return __emval_register({});
  },

  _emval_new_cstring__deps: ['$getStringOrSymbol', '_emval_register'],
  _emval_new_cstring: function(v) {
    return __emval_register(getStringOrSymbol(v));
  },

  _emval_take_value__deps: ['_emval_register', '$requireRegisteredType'],
  _emval_take_value: function(type, argv) {
    type = requireRegisteredType(type, '_emval_take_value');
    var v = type['readValueFromPointer'](argv);
    return __emval_register(v);
  },

  $emval_newers: {}, // arity -> function
  $craftEmvalAllocator__deps: ['_emval_register', '$requireRegisteredType'],
  $craftEmvalAllocator: function(argCount) {
    /*This function returns a new function that looks like this:
    function emval_allocator_3(constructor, argTypes, args) {
        var argType0 = requireRegisteredType(HEAP32[(argTypes >> 2)], "parameter 0");
        var arg0 = argType0.readValueFromPointer(args);
        var argType1 = requireRegisteredType(HEAP32[(argTypes >> 2) + 1], "parameter 1");
        var arg1 = argType1.readValueFromPointer(args + 8);
        var argType2 = requireRegisteredType(HEAP32[(argTypes >> 2) + 2], "parameter 2");
        var arg2 = argType2.readValueFromPointer(args + 16);
        var obj = new constructor(arg0, arg1, arg2);
        return __emval_register(obj);
    } */

    var argsList = "";
    for(var i = 0; i < argCount; ++i) {
        argsList += (i!==0?", ":"")+"arg"+i; // 'arg0, arg1, ..., argn'
    }

    var functionBody =
        "return function emval_allocator_"+argCount+"(constructor, argTypes, args) {\n";

    for(var i = 0; i < argCount; ++i) {
        functionBody +=
            "var argType"+i+" = requireRegisteredType(HEAP32[(argTypes >> 2) + "+i+"], \"parameter "+i+"\");\n" +
            "var arg"+i+" = argType"+i+".readValueFromPointer(args);\n" +
            "args += argType"+i+"['argPackAdvance'];\n";
    }
    functionBody +=
        "var obj = new constructor("+argsList+");\n" +
        "return __emval_register(obj);\n" +
        "}\n";

    /*jshint evil:true*/
    return (new Function("requireRegisteredType", "HEAP32", "__emval_register", functionBody))(
        requireRegisteredType, HEAP32, __emval_register);
  },

  _emval_new__deps: ['$craftEmvalAllocator', '$emval_newers', '$requireHandle'],
  _emval_new: function(handle, argCount, argTypes, args) {
    handle = requireHandle(handle);

    var newer = emval_newers[argCount];
    if (!newer) {
        newer = craftEmvalAllocator(argCount);
        emval_newers[argCount] = newer;
    }

    return newer(handle, argTypes, args);
  },

  // appease jshint (technically this code uses eval)
  $emval_get_global: function() { return (function(){return Function;})()('return this')(); },
  _emval_get_global__deps: ['_emval_register', '$getStringOrSymbol', '$emval_get_global'],
  _emval_get_global: function(name) {
    if(name===0){
      return __emval_register(emval_get_global());
    } else {
      name = getStringOrSymbol(name);
      return __emval_register(emval_get_global()[name]);
    }
  },

  _emval_get_module_property__deps: ['$getStringOrSymbol', '_emval_register'],
  _emval_get_module_property: function(name) {
    name = getStringOrSymbol(name);
    return __emval_register(Module[name]);
  },

  _emval_get_property__deps: ['_emval_register', '$requireHandle'],
  _emval_get_property: function(handle, key) {
    handle = requireHandle(handle);
    key = requireHandle(key);
    return __emval_register(handle[key]);
  },

  _emval_set_property__deps: ['$requireHandle'],
  _emval_set_property: function(handle, key, value) {
    handle = requireHandle(handle);
    key = requireHandle(key);
    value = requireHandle(value);
    handle[key] = value;
  },

  _emval_as__deps: ['_emval_register', '$requireHandle', '$requireRegisteredType'],
  _emval_as: function(handle, returnType, destructorsRef) {
    handle = requireHandle(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    var destructors = [];
    var rd = __emval_register(destructors);
    HEAP32[destructorsRef >> 2] = rd;
    return returnType['toWireType'](destructors, handle);
  },

  _emval_equals__deps: ['_emval_register', '$requireHandle'],
  _emval_equals: function(first, second ){
    first = requireHandle(first);
    second = requireHandle(second);
    return first==second;
  },

  _emval_strictly_equals__deps: ['_emval_register', '$requireHandle'],
  _emval_strictly_equals: function(first, second ){
    first = requireHandle(first);
    second = requireHandle(second);
    return first===second;
  },

  _emval_call__deps: ['_emval_lookupTypes', '_emval_register', '$requireHandle'],
  _emval_call: function(handle, argCount, argTypes, argv) {
    handle = requireHandle(handle);
    var types = __emval_lookupTypes(argCount, argTypes);

    var args = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        var type = types[i];
        args[i] = type['readValueFromPointer'](argv);
        argv += type['argPackAdvance'];
    }

    var rv = handle.apply(undefined, args);
    return __emval_register(rv);
  },

  _emval_lookupTypes__deps: ['$requireRegisteredType'],
  _emval_lookupTypes: function(argCount, argTypes, argWireTypes) {
    var a = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        a[i] = requireRegisteredType(
            HEAP32[(argTypes >> 2) + i],
            "parameter " + i);
    }
    return a;
  },

  _emval_allocateDestructors__deps: ['_emval_register'],
  _emval_allocateDestructors: function(destructorsRef) {
    var destructors = [];
    HEAP32[destructorsRef >> 2] = __emval_register(destructors);
    return destructors;
  },

  // Leave id 0 undefined.  It's not a big deal, but might be confusing
  // to have null be a valid method caller.
  $emval_methodCallers: [undefined],

  _emval_addMethodCaller__deps: ['$emval_methodCallers'],
  _emval_addMethodCaller: function(caller) {
    var id = emval_methodCallers.length;
    emval_methodCallers.push(caller);
    return id;
  },

  _emval_get_method_caller__deps: ['_emval_addMethodCaller', '_emval_lookupTypes', '$new_', '$makeLegalFunctionName'],
  _emval_get_method_caller: function(argCount, argTypes) {
    var types = __emval_lookupTypes(argCount, argTypes);

    var retType = types[0];
    var signatureName = retType.name + "_$" + types.slice(1).map(function (t) { return t.name; }).join("_") + "$";

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
    var invokerFunction = new_(Function, params).apply(null, args);
    return __emval_addMethodCaller(invokerFunction);
  },

  _emval_call_method__deps: ['_emval_allocateDestructors', '$getStringOrSymbol', '$emval_methodCallers', '$requireHandle'],
  _emval_call_method: function(caller, handle, methodName, destructorsRef, args) {
    caller = emval_methodCallers[caller];
    handle = requireHandle(handle);
    methodName = getStringOrSymbol(methodName);
    return caller(handle, methodName, __emval_allocateDestructors(destructorsRef), args);
  },

  _emval_call_void_method__deps: ['_emval_allocateDestructors', '$getStringOrSymbol', '$emval_methodCallers', '$requireHandle'],
  _emval_call_void_method: function(caller, handle, methodName, args) {
    caller = emval_methodCallers[caller];
    handle = requireHandle(handle);
    methodName = getStringOrSymbol(methodName);
    caller(handle, methodName, null, args);
  },

  _emval_typeof__deps: ['_emval_register', '$requireHandle'],
  _emval_typeof: function(handle) {
    handle = requireHandle(handle);
    return __emval_register(typeof handle);
  },
};

mergeInto(LibraryManager.library, LibraryEmVal);
