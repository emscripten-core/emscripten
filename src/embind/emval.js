/*global Module*/
/*global HEAP32*/
/*global Pointer_stringify, writeStringToMemory*/
/*global requireRegisteredType*/

var _emval_handle_array = [];
var _emval_free_list = [];

// Public JS API

/** @expose */
Module.count_emval_handles = function() {
    var count = 0;
    for (var i = 0; i < _emval_handle_array.length; ++i) {
        if (_emval_handle_array[i] !== undefined) {
            ++count;
        }
    }
    return count;
};

/** @expose */
Module.get_first_emval = function() {
    for (var i = 0; i < _emval_handle_array.length; ++i) {
        if (_emval_handle_array[i] !== undefined) {
            return _emval_handle_array[i];
        }
    }
    return null;
};

// Private C++ API

function __emval_register(value) {
    var handle = _emval_free_list.length ?
        _emval_free_list.pop() :
        _emval_handle_array.length;

    _emval_handle_array[handle] = {refcount: 1, value: value};
    return handle;
}

function __emval_incref(handle) {
    _emval_handle_array[handle].refcount += 1;
}

function __emval_decref(handle) {
    if (0 === --_emval_handle_array[handle].refcount) {
        delete _emval_handle_array[handle];
        _emval_free_list.push(handle);

        var actual_length = _emval_handle_array.length;
        while (actual_length > 0 && _emval_handle_array[actual_length - 1] === undefined) {
            --actual_length;
        }
        _emval_handle_array.length = actual_length;
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
    return __emval_register(Pointer_stringify(v));
}

function __emval_take_value(type, v) {
    type = requireRegisteredType(type, '_emval_take_value');
    v = type.fromWireType(v);
    return __emval_register(v);
}

var __newers = {}; // arity -> function

function __emval_new(handle, argCount, argTypes) {
    var args = parseParameters(
        argCount,
        argTypes,
        Array.prototype.slice.call(arguments, 3));

    // Alas, we are forced to use operator new until WebKit enables
    // constructing typed arrays without new.
    // In WebKit, Uint8Array(10) throws an error.
    // In every other browser, it's identical to new Uint8Array(10).

    var newer = __newers[argCount];
    if (!newer) {
        var parameters = new Array(argCount);
        for (var i = 0; i < argCount; ++i) {
            parameters[i] = 'a' + i;
        }
        /*jshint evil:true*/
        newer = __newers[argCount] = new Function(
            ['c'].concat(parameters),
            "return new c(" + parameters.join(',') + ");");
    }
    
    var constructor = _emval_handle_array[handle].value;
    var obj = newer.apply(undefined, [constructor].concat(args));
/*
    // implement what amounts to operator new
    function dummy(){}
    dummy.prototype = constructor.prototype;
    var obj = new constructor;
    var rv = constructor.apply(obj, args);
    if (typeof rv === 'object') {
        obj = rv;
    }
*/
    return __emval_register(obj);
}

// appease jshint (technically this code uses eval)
var global = (function(){return Function;})()('return this')();

function __emval_get_global(name) {
    name = Pointer_stringify(name);
    return __emval_register(global[name]);
}

function __emval_get_module_property(name) {
    name = Pointer_stringify(name);
    return __emval_register(Module[name]);
}

function __emval_get_property(handle, key) {
    return __emval_register(_emval_handle_array[handle].value[_emval_handle_array[key].value]);
}

function __emval_set_property(handle, key, value) {
    _emval_handle_array[handle].value[_emval_handle_array[key].value] = _emval_handle_array[value].value;
}

function __emval_as(handle, returnType) {
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
    var fn = _emval_handle_array[handle].value;
    var args = parseParameters(
        argCount,
        argTypes,
        Array.prototype.slice.call(arguments, 3));
    var rv = fn.apply(undefined, args);
    return __emval_register(rv);
}

function __emval_call_method(handle, name, argCount, argTypes) {
    name = Pointer_stringify(name);

    var args = parseParameters(
        argCount,
        argTypes,
        Array.prototype.slice.call(arguments, 4));
    var obj = _emval_handle_array[handle].value;
    var rv = obj[name].apply(obj, args);
    return __emval_register(rv);
}

function __emval_call_void_method(handle, name, argCount, argTypes) {
    name = Pointer_stringify(name);

    var args = parseParameters(
        argCount,
        argTypes,
        Array.prototype.slice.call(arguments, 4));
    var obj = _emval_handle_array[handle].value;
    obj[name].apply(obj, args);
}

function __emval_has_function(handle, name) {
    name = Pointer_stringify(name);
    return _emval_handle_array[handle].value[name] instanceof Function;
}
