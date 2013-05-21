/*global Module:true, Runtime*/
/*global HEAP32*/
/*global new_*/
/*global createNamedFunction*/
/*global readLatin1String, writeStringToMemory*/
/*global requireRegisteredType, throwBindingError*/

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

function __emval_take_value(type, v) {
    type = requireRegisteredType(type, '_emval_take_value');
    v = type.fromWireType(v);
    return __emval_register(v);
}

var __newers = {}; // arity -> function


function craftEmvalAllocator(argCount) {
    /*This function returns a new function that looks like this:
    function emval_allocator_3(handle, argTypes, arg0Wired, arg1Wired, arg2Wired) {
        var argType0 = requireRegisteredType(HEAP32[(argTypes >> 2)], "parameter 0");
        var arg0 = argType0.fromWireType(arg0Wired);
        var argType1 = requireRegisteredType(HEAP32[(argTypes >> 2) + 1], "parameter 1");
        var arg1 = argType1.fromWireType(arg1Wired);
        var argType2 = requireRegisteredType(HEAP32[(argTypes >> 2) + 2], "parameter 2");
        var arg2 = argType2.fromWireType(arg2Wired);
        var constructor = _emval_handle_array[handle].value;
        var emval = new constructor(arg0, arg1, arg2);
        return emval;
    } */

    var args1 = ["requireRegisteredType", "HEAP32", "_emval_handle_array", "__emval_register"];
    var args2 = [requireRegisteredType, HEAP32, _emval_handle_array, __emval_register];

    var argsList = "";
    var argsListWired = "";
    for(var i = 0; i < argCount; ++i) {
        argsList += (i!==0?", ":"")+"arg"+i; // 'arg0, arg1, ..., argn'
        argsListWired += ", arg"+i+"Wired"; // ', arg0Wired, arg1Wired, ..., argnWired'
    }

    var invokerFnBody =
        "return function emval_allocator_"+argCount+"(handle, argTypes " + argsListWired + ") {\n";

    for(var i = 0; i < argCount; ++i) {
        invokerFnBody += 
            "var argType"+i+" = requireRegisteredType(HEAP32[(argTypes >> 2) + "+i+"], \"parameter "+i+"\");\n" +
            "var arg"+i+" = argType"+i+".fromWireType(arg"+i+"Wired);\n";
    }
    invokerFnBody +=
        "var constructor = _emval_handle_array[handle].value;\n" +
        "var obj = new constructor("+argsList+");\n" +
        "return __emval_register(obj);\n" +
        "}\n";

    args1.push(invokerFnBody);
    var invokerFunction = new_(Function, args1).apply(null, args2);
    return invokerFunction;
}

function __emval_new(handle, argCount, argTypes) {
    requireHandle(handle);
    
    var newer = __newers[argCount];
    if (!newer) {
        newer = craftEmvalAllocator(argCount);
        __newers[argCount] = newer;
    }

    if (argCount === 0) {
        return newer(handle, argTypes);
    } else if (argCount === 1) {
        return newer(handle, argTypes, arguments[3]);
    } else if (argCount === 2) {
        return newer(handle, argTypes, arguments[3], arguments[4]);
    } else if (argCount === 3) {
        return newer(handle, argTypes, arguments[3], arguments[4], arguments[5]);
    } else if (argCount === 4) {
        return newer(handle, argTypes, arguments[3], arguments[4], arguments[5], arguments[6]);
    } else {
        // This is a slow path! (.apply and .splice are slow), so a few specializations are present above.
        return newer.apply(null, arguments.splice(1));
    }
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
    requireHandle(handle);
    return __emval_register(_emval_handle_array[handle].value[_emval_handle_array[key].value]);
}

function __emval_set_property(handle, key, value) {
    requireHandle(handle);
    _emval_handle_array[handle].value[_emval_handle_array[key].value] = _emval_handle_array[value].value;
}

function __emval_as(handle, returnType) {
    requireHandle(handle);
    returnType = requireRegisteredType(returnType, 'emval::as');
    var destructors = [];
    // caller owns destructing
    return returnType.toWireType(destructors, _emval_handle_array[handle].value);
}

function parseParameters(argCount, argTypes, argWireTypes) {
    var a = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        var argType = requireRegisteredType(
            HEAP32[(argTypes >> 2) + i],
            "parameter " + i);
        a[i] = argType.fromWireType(argWireTypes[i]);
    }
    return a;
}

function __emval_call(handle, argCount, argTypes) {
    requireHandle(handle);
    var types = lookupTypes(argCount, argTypes);

    var args = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        args[i] = types[i].fromWireType(arguments[3 + i]);
    }

    var fn = _emval_handle_array[handle].value;
    var rv = fn.apply(undefined, args);
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

function __emval_get_method_caller(argCount, argTypes) {
    var types = lookupTypes(argCount, argTypes);

    var retType = types[0];
    var signatureName = retType.name + "_$" + types.slice(1).map(function (t) { return t.name; }).join("_") + "$";

    var args1 = ["Runtime", "createNamedFunction", "requireHandle", "getStringOrSymbol", "_emval_handle_array", "retType"];
    var args2 = [Runtime, createNamedFunction, requireHandle, getStringOrSymbol, _emval_handle_array, retType];

    var argsList = ""; // 'arg0, arg1, arg2, ... , argN'
    var argsListWired = ""; // 'arg0Wired, ..., argNWired'
    for (var i = 0; i < argCount - 1; ++i) {
        argsList += (i !== 0 ? ", " : "") + "arg" + i;
        argsListWired += ", arg" + i + "Wired";
        args1.push("argType" + i);
        args2.push(types[1 + i]);
    }

    var invokerFnBody =
        "return Runtime.addFunction(createNamedFunction('" + signatureName + "', function (handle, name" + argsListWired + ") {\n" +
        "requireHandle(handle);\n" +
        "name = getStringOrSymbol(name);\n";

    for (var i = 0; i < argCount - 1; ++i) {
        invokerFnBody += "var arg" + i + " = argType" + i + ".fromWireType(arg" + i + "Wired);\n";
    }
    invokerFnBody +=
        "var obj = _emval_handle_array[handle].value;\n" +
        "return retType.toWireType(null, obj[name](" + argsList + "));\n" + 
        "}));\n";

    args1.push(invokerFnBody);
    var invokerFunction = new_(Function, args1).apply(null, args2);
    return invokerFunction;
}

function __emval_has_function(handle, name) {
    name = getStringOrSymbol(name);
    return _emval_handle_array[handle].value[name] instanceof Function;
}
