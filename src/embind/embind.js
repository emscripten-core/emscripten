/*global Module*/
/*global _malloc, _free, _memcpy*/
/*global FUNCTION_TABLE, HEAP32*/
/*global Pointer_stringify, writeStringToMemory*/
/*global __emval_register, _emval_handle_array, __emval_decref*/

function exposePublicSymbol(name, value) {
    if (Module.hasOwnProperty(name)) {
        throw new BindingError("Cannot register public name '" + name + "' twice");
    }
    Module[name] = value;
}

function createNamedFunction(name, body) {
    /*jshint evil:true*/
    return new Function(
        "body",
        "return function " + name + "() {\n" +
        "    return body.apply(this, arguments);\n" +
        "};\n"
    )(body);
}

function _embind_repr(v) {
    var t = typeof v;
    if (t === 'object' || t === 'array' || t === 'function') {
        return v.toString();
    } else {
        return '' + v;
    }
}

var typeRegistry = {};

function registerType(type, name, info) {
    if (!type) {
        throw new BindingError('type "' + name + '" must have a positive integer typeid pointer');
    }
    if (typeRegistry.hasOwnProperty(type)) {
        throw new BindingError("Cannot register type '" + name + "' twice");
    }
    typeRegistry[type] = info;
}

function __embind_register_void(voidType, name) {
    name = Pointer_stringify(name);
    registerType(voidType, name, {
        name: name,
        fromWireType: function() {
            return undefined;
        }
    });
}

function __embind_register_bool(boolType, name, trueValue, falseValue) {
    name = Pointer_stringify(name);
    registerType(boolType, name, {
        name: name,
        toWireType: function(destructors, o) {
            return o ? trueValue : falseValue;
        },
        fromWireType: function(wt) {
            // ambiguous emscripten ABI: sometimes return values are
            // true or false, and sometimes integers (0 or 1)
            return !!wt;
        },
    });
}

function __embind_register_integer(primitiveType, name) {
    name = Pointer_stringify(name);
    registerType(primitiveType, name, {
        name: name,
        toWireType: function(destructors, value) {
            if (typeof value !== "number") {
                throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + name);
            }
            return value | 0;
        },
        fromWireType: function(value) {
            return value;
        }
    });
}

function __embind_register_float(primitiveType, name) {
    name = Pointer_stringify(name);
    registerType(primitiveType, name, {
        name: name,
        toWireType: function(destructors, value) {
            if (typeof value !== "number") {
                throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + name);
            }
            return value;
        },
        fromWireType: function(value) {
            return value;
        }
    });
}

function __embind_register_cstring(stringType, name) {
    name = Pointer_stringify(name);
    registerType(stringType, name, {
        name: name,
        toWireType: function(destructors, value) {
            var ptr = _malloc(value.length + 1);
            writeStringToMemory(value, ptr);
            destructors.push(_free);
            destructors.push(ptr);
            return ptr;
        },
        fromWireType: function(value) {
            var rv = Pointer_stringify(value);
            _free(value);
            return rv;
        }
    });
}

function __embind_register_emval(emvalType, name) {
    name = Pointer_stringify(name);
    registerType(emvalType, name, {
        name: name,
        toWireType: function(destructors, value) {
            return __emval_register(value);
        },
        fromWireType: function(handle) {
            var rv = _emval_handle_array[handle].value;
            __emval_decref(handle);
            return rv;
        }
    });
}

var BindingError = Error;
/** @expose */
Module.BindingError = BindingError;

function typeName(typeID) {
    // could use our carnal knowledge of RTTI but for now just return the pointer...
    return typeID;
}

function requireRegisteredType(type, humanName) {
    var impl = typeRegistry[type];
    if (undefined === impl) {
        throw new BindingError(humanName + " has unknown type: " + typeName(type));
    }
    return impl;
}

function requireArgumentTypes(argCount, argTypes, name) {
    var argTypeImpls = new Array(argCount);
    for (var i = 0; i < argCount; ++i) {
        var argType = HEAP32[(argTypes >> 2) + i];
        if (i === 0) {
            argTypeImpls[i] = requireRegisteredType(argType, name + " return value");
        } else {
            argTypeImpls[i] = requireRegisteredType(argType, name + " parameter " + i);
        }
    }
    return argTypeImpls;
}

function runDestructors(destructors) {
    while (destructors.length) {
        var ptr = destructors.pop();
        var del = destructors.pop();
        del(ptr);
    }
}

function makeInvoker(name, argCount, argTypes, invoker, fn) {
    return function() {
        if (arguments.length !== argCount - 1) {
            throw new BindingError('function ' + name + ' called with ' + arguments.length + ' arguments, expected ' + argCount - 1);
        }
        var destructors = [];
        var args = new Array(argCount);
        args[0] = fn;
        for (var i = 1; i < argCount; ++i) {
            args[i] = argTypes[i].toWireType(destructors, arguments[i-1]);
        }
        var rv = argTypes[0].fromWireType(invoker.apply(null, args));
        runDestructors(destructors);
        return rv;
    };
}

function __embind_register_function(name, argCount, argTypes, invoker, fn) {
    name = Pointer_stringify(name);
    invoker = FUNCTION_TABLE[invoker];
    argTypes = requireArgumentTypes(argCount, argTypes, name);
    exposePublicSymbol(name, makeInvoker(name, argCount, argTypes, invoker, fn));
}

function __embind_register_tuple(tupleType, name, constructor, destructor) {
    name = Pointer_stringify(name);
    constructor = FUNCTION_TABLE[constructor];
    destructor = FUNCTION_TABLE[destructor];

    var elements = [];

    registerType(tupleType, name, {
        name: name,
        elements: elements,
        fromWireType: function(ptr) {
            var len = elements.length;
            var rv = new Array(len);
            for (var i = 0; i < len; ++i) {
                rv[i] = elements[i].read(ptr);
            }
            destructor(ptr);
            return rv;
        },
        toWireType: function(destructors, o) {
            var len = elements.length;
            if (len !== o.length) {
                throw new TypeError("Incorrect number of tuple elements");
            }
            var ptr = constructor();
            for (var i = 0; i < len; ++i) {
                elements[i].write(ptr, o[i]);
            }
            destructors.push(destructor);
            destructors.push(ptr);
            return ptr;
        }
    });
}

function copyMemberPointer(memberPointer, memberPointerSize) {
    var copy = _malloc(memberPointerSize);
    if (!copy) {
        throw new Error('Failed to allocate member pointer copy');
    }
    _memcpy(copy, memberPointer, memberPointerSize);
    return copy;
}

function __embind_register_tuple_element(
    tupleType,
    elementType,
    getter,
    setter,
    memberPointerSize,
    memberPointer
) {
    tupleType = requireRegisteredType(tupleType, 'tuple');
    elementType = requireRegisteredType(elementType, "element " + tupleType.name + "[" + tupleType.elements.length + "]");
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);

    tupleType.elements.push({
        read: function(ptr) {
            return elementType.fromWireType(getter(ptr, memberPointer));
        },
        write: function(ptr, o) {
            var destructors = [];
            setter(ptr, memberPointer, elementType.toWireType(destructors, o));
            runDestructors(destructors);
        }
    });
}

function __embind_register_tuple_element_accessor(
    tupleType,
    elementType,
    staticGetter,
    getterSize,
    getter,
    staticSetter,
    setterSize,
    setter
) {
    tupleType = requireRegisteredType(tupleType, 'tuple');
    elementType = requireRegisteredType(elementType, "element " + tupleType.name + "[" + tupleType.elements.length + "]");
    staticGetter = FUNCTION_TABLE[staticGetter];
    getter = copyMemberPointer(getter, getterSize);
    staticSetter = FUNCTION_TABLE[staticSetter];
    setter = copyMemberPointer(setter, setterSize);

    tupleType.elements.push({
        read: function(ptr) {
            return elementType.fromWireType(staticGetter(ptr, HEAP32[getter >> 2]));
        },
        write: function(ptr, o) {
            var destructors = [];
            staticSetter(
                ptr,
                HEAP32[setter >> 2],
                elementType.toWireType(destructors, o));
            runDestructors(destructors);
        }
    });
}

function __embind_register_struct(
    structType,
    name,
    constructor,
    destructor
) {
    name = Pointer_stringify(name);
    constructor = FUNCTION_TABLE[constructor];
    destructor = FUNCTION_TABLE[destructor];

    registerType(structType, name, {
        fields: {},
        fromWireType: function(ptr) {
            var fields = this.fields;
            var rv = {};
            for (var i in fields) {
                rv[i] = fields[i].read(ptr);
            }
            destructor(ptr);
            return rv;
        },
        toWireType: function(destructors, o) {
            var fields = this.fields;
            for (var fieldName in fields) {
                if (!(fieldName in o)) {
                    throw new TypeError('Missing field');
                }
            }
            var ptr = constructor();
            for (var fieldName in fields) {
                fields[fieldName].write(ptr, o[fieldName]);
            }
            destructors.push(destructor);
            destructors.push(ptr);
            return ptr;
        }
    });
}

function __embind_register_struct_field(
    structType,
    fieldName,
    fieldType,
    getter,
    setter,
    memberPointerSize,
    memberPointer
) {
    structType = requireRegisteredType(structType, 'struct');
    fieldName = Pointer_stringify(fieldName);
    fieldType = requireRegisteredType(fieldType, 'field "' + structType.name + '.' + fieldName + '"');
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);

    structType.fields[fieldName] = {
        read: function(ptr) {
            return fieldType.fromWireType(getter(ptr, memberPointer));
        },
        write: function(ptr, o) {
            var destructors = [];
            setter(ptr, memberPointer, fieldType.toWireType(destructors, o));
            runDestructors(destructors);
        }
    };
}

function __embind_register_smart_ptr(
    pointerType,
    pointeeType,
    name,
    destructor,
    getPointee
) {
    name = Pointer_stringify(name);
    pointeeType = requireRegisteredType(pointeeType, 'class');
    destructor = FUNCTION_TABLE[destructor];
    getPointee = FUNCTION_TABLE[getPointee];
    
    var Handle = createNamedFunction(name, function(ptr) {
        this.count = {value: 1};
        this.smartPointer = ptr; // std::shared_ptr<T>*
        this.ptr = getPointee(ptr); // T*
    });

    // TODO: test for SmartPtr.prototype.constructor property?
    // We likely want it distinct from pointeeType.prototype.constructor
    Handle.prototype = Object.create(pointeeType.Handle.prototype);
    
    Handle.prototype.clone = function() {
        if (!this.ptr) {
            throw new BindingError(pointeeType.name + ' instance already deleted');
        }

        var clone = Object.create(Handle.prototype);
        clone.count = this.count;
        clone.smartPointer = this.smartPointer;
        clone.ptr = this.ptr;
        
        clone.count.value += 1;
        return clone;
    };
    
    Handle.prototype['delete'] = function() {
        if (!this.ptr) {
            throw new BindingError(pointeeType.name + ' instance already deleted');
        }
        
        this.count.value -= 1;
        if (0 === this.count.value) {
            destructor(this.smartPointer);
        }
        this.smartPointer = undefined;
        this.ptr = undefined;
    };

    registerType(pointerType, name, {
        name: name,
        Handle: Handle,
        fromWireType: function(ptr) {
            if (!getPointee(ptr)) {
                destructor(ptr);
                return null;
            }
            return new Handle(ptr);
        },
        toWireType: function(destructors, o) {
            if (null === o) {
                return 0;
            } else {
                return o.smartPointer;
            }
        }
    });
}

function __embind_register_vector(
    vectorType,
    elementType,
    name,
    constructor,
    destructor,
    length,
    getter,
    setter
) {
    name = Pointer_stringify(name);
    elementType = requireRegisteredType(elementType, 'vector ' + name);
    
    constructor = FUNCTION_TABLE[constructor];
    destructor = FUNCTION_TABLE[destructor];
    length = FUNCTION_TABLE[length];
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];

    registerType(vectorType, name, {
        name: name,
        fromWireType: function(ptr) {
            var arr = [];
            var n = length(ptr);

            for (var i = 0; i < n; i++) {
                var v = elementType.fromWireType(getter(ptr, i));
                arr.push(v);
            }

            destructor(ptr);
            return arr;
        },
        toWireType: function(destructors, o) {
            var vec = constructor();
            for (var val in o) {
                setter(vec, elementType.toWireType(destructors, o[val]));
            }
            destructors.push(destructor);
            destructors.push(vec);
            return vec;
        }
    });
}

function __embind_register_class(
    classType,
    pointerType,
    constPointerType,
    name,
    destructor
) {
    name = Pointer_stringify(name);
    destructor = FUNCTION_TABLE[destructor];
    
    var Handle = createNamedFunction(name, function(ptr) {
        var h = function() {
            if(h.operator_call !== undefined) {
                return h.operator_call.apply(h, arguments);
            } else {
                throw new BindingError(name + ' does not define call operator');
            }
        };
        
        h.count = {value: 1};
        h.ptr = ptr;
        
        for(var prop in Handle.prototype) {
            var dp = Object.getOwnPropertyDescriptor(Handle.prototype, prop);
            Object.defineProperty(h, prop, dp);
        }
        
        return h; 
    });

    Handle.prototype.clone = function() {
        if (!this.ptr) {
            throw new BindingError(classType.name + ' instance already deleted');
        }

        var clone = Object.create(Handle.prototype);
        clone.count = this.count;
        clone.ptr = this.ptr;
        
        clone.count.value += 1;
        return clone;
    };

    Handle.prototype.move = function() {
        var rv = this.clone();
        this.delete();
        return rv;
    };

    Handle.prototype['delete'] = function() {
        if (!this.ptr) {
            throw new BindingError(classType.name + ' instance already deleted');
        }

        this.count.value -= 1;
        if (0 === this.count.value) {
            destructor(this.ptr);
        }
        this.ptr = undefined;
    };

    var constructor = createNamedFunction(name, function() {
        var body = constructor.body;
        return body.apply(this, arguments);
    });
    constructor.prototype = Handle.prototype;

    registerType(classType, name, {
        name: name,
        constructor: constructor,
        Handle: Handle,
        fromWireType: function(ptr) {
            return new Handle(ptr);
        },
        toWireType: function(destructors, o) {
            return o.ptr;
        }
    });

    var pointerName = name + '*';
    registerType(pointerType, pointerName, {
        name: pointerName,
        fromWireType: function(ptr) {
            return new Handle(ptr);
        },
        toWireType: function(destructors, o) {
            return o.ptr;
        }
    });

    var constPointerName = name + ' const*';
    registerType(constPointerType, constPointerName, {
        name: constPointerName,
        toWireType: function(destructors, o) {
            return o.ptr;
        }
    });

    exposePublicSymbol(name, constructor);
}

function __embind_register_class_constructor(
    classType,
    argCount,
    argTypes,
    constructor
) {
    classType = requireRegisteredType(classType, 'class');
    var humanName = 'constructor ' + classType.name;
    argTypes = requireArgumentTypes(argCount, argTypes, humanName);
    constructor = FUNCTION_TABLE[constructor];

    classType.constructor.body = function() {
        if (arguments.length !== argCount - 1) {
            throw new BindingError('emscripten binding ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
        }
        var destructors = [];
        var args = new Array(argCount-1);
        for (var i = 1; i < argCount; ++i) {
            args[i-1] = argTypes[i].toWireType(destructors, arguments[i-1]);
        }

        var ptr = constructor.apply(null, args);
        runDestructors(destructors);
        
        return classType.Handle.call(this, ptr);
    };
}

function __embind_register_class_method(
    classType,
    methodName,
    argCount,
    argTypes,
    invoker,
    memberFunctionSize,
    memberFunction
) {
    classType = requireRegisteredType(classType, 'class');
    methodName = Pointer_stringify(methodName);
    var humanName = classType.name + '.' + methodName;

    argTypes = requireArgumentTypes(argCount, argTypes, 'method ' + humanName);
    invoker = FUNCTION_TABLE[invoker];
    memberFunction = copyMemberPointer(memberFunction, memberFunctionSize);

    classType.Handle.prototype[methodName] = function() {
        
        if (!this.ptr) {
            throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
        }
        if (arguments.length !== argCount - 1) {
            throw new BindingError('emscripten binding method ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
        }
        
        var destructors = [];
        var args = new Array(argCount + 1);
        args[0] = this.ptr;
        args[1] = memberFunction;
        for (var i = 1; i < argCount; ++i) {
            args[i + 1] = argTypes[i].toWireType(destructors, arguments[i-1]);
        }

        var rv = argTypes[0].fromWireType(invoker.apply(null, args));
        runDestructors(destructors);
        return rv;
    };
}

function __embind_register_cast_method(
    classType,
    methodName,
    returnType,
    invoker
) {
    classType = requireRegisteredType(classType, 'class');
    methodName = Pointer_stringify(methodName);
    var humanName = classType.name + '.' + methodName;

    returnType = requireRegisteredType(returnType, 'method ' + humanName + ' return value');
    invoker = FUNCTION_TABLE[invoker];

    classType.Handle.prototype[methodName] = function() {
        if (!this.ptr) {
            throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
        }
        if (arguments.length !== 0) {
            throw new BindingError('emscripten binding method ' + humanName + ' called with arguments, none expected');
        }

        var args = new Array(1);
        args[0] = this.ptr;
        var rv = returnType.fromWireType(invoker.apply(null, args)); // in case ptr needs to be adjusted for multiple inheritance
        return rv;
    };
}

function __embind_register_pointer_cast_method(
    classType,
    methodName,
    returnType,
    invoker
) {
    classType = requireRegisteredType(classType, 'class');
    methodName = Pointer_stringify(methodName);
    var humanName = classType.name + '.' + methodName;

    returnType = requireRegisteredType(returnType, 'method ' + humanName + ' return value');
    invoker = FUNCTION_TABLE[invoker];

    classType.Handle.prototype[methodName] = function() {
        if (!this.ptr) {
            throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
        }
        if (arguments.length !== 0) {
            throw new BindingError('emscripten binding method ' + humanName + ' called with arguments, none expected');
        }
        var args = new Array(2);
        var newPtr = _malloc(8);
        args[0] = newPtr;
        args[1] = this.smartPointer;
        invoker.apply(null,args);
        var rv = returnType.fromWireType(newPtr); // in case ptr needs to be adjusted for multiple inheritance
        return rv;
    };
}

function __embind_register_class_classmethod(
    classType,
    methodName,
    argCount,
    argTypes,
    invoker,
    fn
) {
    classType = requireRegisteredType(classType, 'class');
    methodName = Pointer_stringify(methodName);
    var humanName = classType.name + '.' + methodName;
    argTypes = requireArgumentTypes(argCount, argTypes, 'classmethod ' + humanName);
    invoker = FUNCTION_TABLE[invoker];
    classType.constructor[methodName] = makeInvoker(humanName, argCount, argTypes, invoker, fn);
}

function __embind_register_class_operator_call(
    classType,
    argCount,
    argTypes,
    invoker
) {
    classType = requireRegisteredType(classType, 'class');
    argTypes = requireArgumentTypes(argCount, argTypes, 'method ' + humanName);
    invoker = FUNCTION_TABLE[invoker];
    var humanName = classType.name + '.' + 'operator_call';
    
    classType.Handle.prototype.operator_call = function() {
        if (!this.ptr) {
            throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
        }
        if (arguments.length !== argCount - 1) {
            throw new BindingError('emscripten binding method ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
        }
        
        var destructors = [];
        var args = new Array(argCount);
        args[0] = this.ptr;
        for (var i = 1; i < argCount; ++i) {
            args[i] = argTypes[i].toWireType(destructors, arguments[i-1]);
        }

        var rv = argTypes[0].fromWireType(invoker.apply(null, args));
        runDestructors(destructors);
        return rv;
    };
}

function __embind_register_class_field(
    classType,
    fieldName,
    fieldType,
    getter,
    setter,
    memberPointerSize,
    memberPointer
) {
    classType = requireRegisteredType(classType, 'class');
    fieldName = Pointer_stringify(fieldName);
    var humanName = classType.name + '.' + fieldName;
    fieldType = requireRegisteredType(fieldType, 'field ' + humanName);
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);

    Object.defineProperty(classType.Handle.prototype, fieldName, {
        get: function() {
            if (!this.ptr) {
                throw new BindingError('cannot access emscripten binding field ' + humanName + ' on deleted object');
            }
            return fieldType.fromWireType(getter(this.ptr, memberPointer));
        },
        set: function(v) {
            if (!this.ptr) {
                throw new BindingError('cannot modify emscripten binding field ' + humanName + ' on deleted object');
            }
            var destructors = [];
            setter(this.ptr, memberPointer, fieldType.toWireType(destructors, v));
            runDestructors(destructors);
        },
        enumerable: true
    });
}

function __embind_register_enum(
    enumType,
    name
) {
    name = Pointer_stringify(name);

    function Enum() {
    }
    Enum.values = {};

    registerType(enumType, name, {
        name: name,
        constructor: Enum,
        toWireType: function(destructors, c) {
            return c.value;
        },
        fromWireType: function(c) {
            return Enum.values[c];
        },
    });
    
    exposePublicSymbol(name, Enum);
}

function __embind_register_enum_value(
    enumType,
    name,
    enumValue
) {
    enumType = requireRegisteredType(enumType, 'enum');
    name = Pointer_stringify(name);

    var Enum = enumType.constructor;

    var Value = Object.create(enumType.constructor.prototype, {
        value: {value: enumValue},
        constructor: {value: createNamedFunction(enumType.name + '_' + name, function() {})},
    });
    Enum.values[enumValue] = Value;
    Enum[name] = Value;
}

function __embind_register_interface(
    interfaceType,
    name,
    constructor,
    destructor
) {
    name = Pointer_stringify(name);
    constructor = FUNCTION_TABLE[constructor];
    destructor = FUNCTION_TABLE[destructor];

    registerType(interfaceType, name, {
        name: name,
        toWireType: function(destructors, o) {
            var handle = __emval_register(o);
            var ptr = constructor(handle);
            destructors.push(destructor);
            destructors.push(ptr);
            return ptr;
        },
    });
}

