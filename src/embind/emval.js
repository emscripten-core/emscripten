/*global Module:true, Runtime*/
/*global HEAP32*/
/*global new_*/
/*global createNamedFunction*/
/*global readLatin1String, writeStringToMemory*/
/*global requireRegisteredType, throwBindingError, runDestructors*/
/*jslint sub:true*/ /* The symbols 'fromWireType' and 'toWireType' must be accessed via array notation to be closure-safe since craftInvokerFunction crafts functions as strings that can't be closured. */

var Module = Module || {};

var _emval_handle_array = [{}]; // reserve zero
var _emval_free_list = [];

// Public JS API

/** @expose */
Module.count_emval_handles = function() {
    var count = 0;
    for (var i = 1; i < _emval_handle_array.length; ++i) {
        if (_emval_handle_array[i] !== undefined) {
            ++count;
        }
    }
    return count;
};

/** @expose */
Module.get_first_emval = function() {
    for (var i = 1; i < _emval_handle_array.length; ++i) {
        if (_emval_handle_array[i] !== undefined) {
            return _emval_handle_array[i];
        }
    }
    return null;
};

// Private C++ API

var _emval_symbols = {}; // address -> string

function __emval_register_symbol(address) {
    _emval_symbols[address] = readLatin1String(address);
}

function getStringOrSymbol(address) {
    var symbol = _emval_symbols[address];
    if (symbol === undefined) {
        return readLatin1String(address);
    } else {
        return symbol;
    }
}

function requireHandle(handle) {
    if (!handle) {
        throwBindingError('Cannot use deleted val. handle = ' + handle);
    }
    return _emval_handle_array[handle].value;
}

function __emval_register(value) {
    var handle = _emval_free_list.length ?
        _emval_free_list.pop() :
        _emval_handle_array.length;

    _emval_handle_array[handle] = {refcount: 1, value: value};
    return handle;
}

function __emval_incref(handle) {
    if (handle) {
        _emval_handle_array[handle].refcount += 1;
    }
}

function __emval_decref(handle) {
    if (handle && 0 === --_emval_handle_array[handle].refcount) {
        _emval_handle_array[handle] = undefined;
        _emval_free_list.push(handle);
    }
}

function __emval_run_destructors(handle) {
    var destructors = _emval_handle_array[handle].value;
    runDestructors(destructors);
    __emval_decref(handle);
}

function __emval_new_array() {
    return __emval_register([]);
}

function __emval_new_object() {
    return __emval_register({});
}

function __emval_undefined() {
    return __emval_register(undefined);
}

function __emval_null() {
    return __emval_register(null);
}

function __emval_new_cstring(v) {
    return __emval_register(getStringOrSymbol(v));
}

function __emval_take_value(type, argv) {
    type = requireRegisteredType(type, '_emval_take_value');
    var v = type['readValueFromPointer'](argv);
    return __emval_register(v);
}

var __newers = {}; // arity -> function


function craftEmvalAllocator(argCount) {
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
            "args += argType"+i+".argPackAdvance;\n";
    }
    functionBody +=
        "var obj = new constructor("+argsList+");\n" +
        "return __emval_register(obj);\n" +
        "}\n";

    /*jshint evil:true*/
    return (new Function("requireRegisteredType", "HEAP32", "__emval_register", functionBody))(
        requireRegisteredType, HEAP32, __emval_register);
}

function __emval_new(handle, argCount, argTypes, args) {
    handle = requireHandle(handle);

    var newer = __newers[argCount];
    if (!newer) {
        newer = craftEmvalAllocator(argCount);
        __newers[argCount] = newer;
    }

    return newer(handle, argTypes, args);
}

// appease jshint (technically this code uses eval)
var global = (function(){return Function;})()('return this')();

function __emval_get_global(name) {
    name = getStringOrSymbol(name);
    return __emval_register(global[name]);
}

function __emval_get_module_property(name) {
    name = getStringOrSymbol(name);
    return __emval_register(Module[name]);
}

function __emval_get_property(handle, key) {
    handle = requireHandle(handle);
    key = requireHandle(key);
    return __emval_register(handle[key]);
}

function __emval_set_property(handle, key, value) {
    handle = requireHandle(handle);
    key = requireHandle(key);
    value = requireHandle(value);
    handle[key] = value;
}

function __emval_as(handle, returnType, destructorsRef) {
    handle = requireHandle(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    var destructors = [];
    var rd = __emval_register(destructors);
    HEAP32[destructorsRef >> 2] = rd;
    return returnType['toWireType'](destructors, handle);
}

function __emval_call(handle, argCount, argTypes, argv) {
    handle = requireHandle(handle);
    var types = lookupTypes(argCount, argTypes);

    var args = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        var type = types[i];
        args[i] = type['readValueFromPointer'](argv);
        argv += type.argPackAdvance;
    }

    var rv = handle.apply(undefined, args);
    return __emval_register(rv);
}

function lookupTypes(argCount, argTypes, argWireTypes) {
    var a = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        a[i] = requireRegisteredType(
            HEAP32[(argTypes >> 2) + i],
            "parameter " + i);
    }
    return a;
}

function allocateDestructors(destructorsRef) {
    var destructors = [];
    HEAP32[destructorsRef >> 2] = __emval_register(destructors);
    return destructors;
}

// Leave id 0 undefined.  It's not a big deal, but might be confusing
// to have null be a valid method caller.
var methodCallers = [undefined];

function addMethodCaller(caller) {
    var id = methodCallers.length;
    methodCallers.push(caller);
    return id;
}

function __emval_get_method_caller(argCount, argTypes) {
    var types = lookupTypes(argCount, argTypes);

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

    var functionBody =
        "return function (handle, name, destructors, args) {\n";

    for (var i = 0; i < argCount - 1; ++i) {
        functionBody +=
        "    var arg" + i + " = argType" + i + ".readValueFromPointer(args);\n" +
        "    args += argType" + i + ".argPackAdvance;\n";
    }
    functionBody +=
        "    var rv = handle[name](" + argsList + ");\n" +
        "    return retType.toWireType(destructors, rv);\n" +
        "};\n";

    params.push(functionBody);
    var invokerFunction = new_(Function, params).apply(null, args);
    return addMethodCaller(createNamedFunction(signatureName, invokerFunction));
}

function __emval_call_method(caller, handle, methodName, destructorsRef, args) {
    caller = methodCallers[caller];
    handle = requireHandle(handle);
    methodName = getStringOrSymbol(methodName);
    return caller(handle, methodName, allocateDestructors(destructorsRef), args);
}

function __emval_has_function(handle, name) {
    handle = requireHandle(handle);
    name = getStringOrSymbol(name);
    return handle[name] instanceof Function;
}
