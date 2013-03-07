/*global Module*/
/*global _malloc, _free, _memcpy*/
/*global FUNCTION_TABLE, HEAP32, HEAPU8*/
/*global Pointer_stringify*/
/*global __emval_register, _emval_handle_array, __emval_decref*/
/*global ___getDynamicPointerType: false*/
/*global ___typeName:false*/
/*global ___staticPointerCast: false*/

var BindingError = Module.BindingError = extendError(Error, 'BindingError');
var CastError = Module.CastError = extendError(Error, 'CastError');

function throwBindingError(value) {
    throw new BindingError(value);
}

function exposePublicSymbol(name, value) {
    if (Module.hasOwnProperty(name)) {
        throwBindingError("Cannot register public name '" + name + "' twice");
    }
    Module[name] = value;
}

// from https://github.com/imvu/imvujs/blob/master/src/error.js
function extendError(baseErrorType, errorName) {
    var errorClass = createNamedFunction(errorName, function(message) {
        this.name = errorName;
        this.message = message;

        var stack = (new Error(message)).stack;
        if (stack !== undefined) {
            this.stack = this.toString() + '\n' +
                stack.replace(/^Error(:[^\n]*)?\n/, '');
        }
    });
    errorClass.prototype = Object.create(baseErrorType.prototype);
    errorClass.prototype.constructor = errorClass;
    errorClass.prototype.toString = function() {
        if (this.message === undefined) {
            return this.name;
        } else {
            return this.name + ': ' + this.message;
        }
    };

    return errorClass;
}


// from https://github.com/imvu/imvujs/blob/master/src/function.js
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

var baseClasses = {}; // rawType -> rawBaseType

// typeID -> { toWireType: ..., fromWireType: ... }
var registeredTypes = {};

// typeID -> [callback]
var awaitingDependencies = {};

function registerType(rawType, registeredInstance) {
    var name = registeredInstance.name;
    if (!rawType) {
        throwBindingError('type "' + name + '" must have a positive integer typeid pointer');
    }
    if (registeredTypes.hasOwnProperty(rawType)) {
        throwBindingError("Cannot register type '" + name + "' twice");
    }

    registeredTypes[rawType] = registeredInstance;

    if (awaitingDependencies.hasOwnProperty(rawType)) {
        var callbacks = awaitingDependencies[rawType];
        delete awaitingDependencies[rawType];
        callbacks.forEach(function(cb) {
            cb();
        });
    }
}

function whenDependentTypesAreResolved(dependentTypes, onComplete) {
    var typeConverters = new Array(dependentTypes.length);
    var unregisteredTypes = [];
    var registered = 0;
    dependentTypes.forEach(function(dt, i) {
        if (registeredTypes.hasOwnProperty(dt)) {
            typeConverters[i] = registeredTypes[dt];
        } else {
            unregisteredTypes.push(dt);
            if (!awaitingDependencies.hasOwnProperty(dt)) {
                awaitingDependencies[dt] = [];
            }
            awaitingDependencies[dt].push(function() {
                typeConverters[i] = registeredTypes[dt];
                ++registered;
                if (registered === unregisteredTypes.length) {
                    onComplete(typeConverters);
                }
            });
        }
    });
    if (0 === unregisteredTypes.length) {
        onComplete(typeConverters);
    }
}

function typeName(rawType) {
    var bt = ___typeName(rawType);
    var rv = Pointer_stringify(bt);
    _free(bt);
    return rv;
}

function heap32VectorToArray(count, firstElement) {
    var array = [];
    for (var i = 0; i < count; i++) {
        array.push(HEAP32[(firstElement >> 2) + i]);
    }
    return array;
}

function requireRegisteredType(rawType, humanName) {
    var impl = registeredTypes[rawType];
    if (undefined === impl) {
        throwBindingError(humanName + " has unknown type " + typeName(rawType));
    }
    return impl;
}

function staticPointerCast(from, fromType, toType) {
    if (!from) {
        return from;
    }
    var to = ___staticPointerCast(from, fromType, toType);
    if (to <= 0) {
        throw new CastError("Pointer conversion from " + typeName(fromType) + " to " + typeName(toType) + " is not available");
    }
    return to;
}

function __embind_register_void(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, {
        name: name,
        fromWireType: function() {
            return undefined;
        },
    });
}

function __embind_register_bool(rawType, name, trueValue, falseValue) {
    name = Pointer_stringify(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(wt) {
            // ambiguous emscripten ABI: sometimes return values are
            // true or false, and sometimes integers (0 or 1)
            return !!wt;
        },
        toWireType: function(destructors, o) {
            return o ? trueValue : falseValue;
        },
    });
}

function __embind_register_integer(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(value) {
            return value;
        },
        toWireType: function(destructors, value) {
            if (typeof value !== "number") {
                throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + this.name);
            }
            return value | 0;
        },
    });
}

function __embind_register_float(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(value) {
            return value;
        },
        toWireType: function(destructors, value) {
            if (typeof value !== "number") {
                throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' +this.name);
            }
            return value;
        },
    });
}

function __embind_register_cstring(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(value) {
            var length = HEAP32[value >> 2];
            var a = new Array(length);
            for (var i = 0; i < length; ++i) {
                a[i] = String.fromCharCode(HEAPU8[value + 4 + i]);
            }
            _free(value);
            return a.join('');
        },
        toWireType: function(destructors, value) {
            // assumes 4-byte alignment
            var length = value.length;
            var ptr = _malloc(4 + length);
            HEAP32[ptr >> 2] = length;
            for (var i = 0; i < length; ++i) {
                HEAPU8[ptr + 4 + i] = value.charCodeAt(i);
            }
            destructors.push(_free);
            destructors.push(ptr);
            return ptr;
        },
    });
}

function __embind_register_emval(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(handle) {
            var rv = _emval_handle_array[handle].value;
            __emval_decref(handle);
            return rv;
        },
        toWireType: function(destructors, value) {
            return __emval_register(value);
        },
    });
}

function runDestructors(destructors) {
    while (destructors.length) {
        var ptr = destructors.pop();
        var del = destructors.pop();
        del(ptr);
    }
}

function makeInvoker(name, argCount, argTypes, invoker, fn) {
    if (!FUNCTION_TABLE[fn]) {
        throwBindingError('function '+name+' is not defined');
    }
    return function() {
        if (arguments.length !== argCount - 1) {
            throwBindingError('function ' + name + ' called with ' + arguments.length + ' arguments, expected ' + (argCount - 1));
        }
        var destructors = [];
        var args = new Array(argCount);
        args[0] = fn;
        for (var i = 1; i < argCount; ++i) {
            args[i] = argTypes[i].toWireType(destructors, arguments[i - 1]);
        }
        var rv = invoker.apply(null, args);
        if (argTypes[0].fromWireTypeAutoDowncast) {
            rv = argTypes[0].fromWireTypeAutoDowncast(rv);
        } else {
            rv = argTypes[0].fromWireType(rv);
        }
        runDestructors(destructors);
        return rv;
    };
}

function __embind_register_function(name, argCount, rawArgTypesAddr, rawInvoker, fn) {
    var argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = Pointer_stringify(name);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    whenDependentTypesAreResolved(argTypes, function(argTypes) {
        exposePublicSymbol(name, makeInvoker(name, argCount, argTypes, rawInvoker, fn));
    });
}

function __embind_register_tuple(rawType, name, rawConstructor, rawDestructor) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    registerType(rawType, {
        name: name,
        rawConstructor: rawConstructor,
        rawDestructor: rawDestructor,
        elements: [],
        fromWireType: function(ptr) {
            var len = this.elements.length;
            var rv = new Array(len);
            for (var i = 0; i < len; ++i) {
                rv[i] = this.elements[i].read(ptr);
            }
            this.rawDestructor(ptr);
            return rv;
        },
        toWireType: function(destructors, o) {
            var len = this.elements.length;
            if (len !== o.length) {
                throw new TypeError("Incorrect number of tuple elements");
            }
            var ptr = this.rawConstructor();
            for (var i = 0; i < len; ++i) {
                this.elements[i].write(ptr, o[i]);
            }
            destructors.push(rawDestructor);
            destructors.push(ptr);
            return ptr;
        },
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
    rawTupleType,
    rawType,
    getter,
    setter,
    memberPointerSize,
    memberPointer
) {
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);
    var tupleType = requireRegisteredType(rawTupleType, 'tuple');

    // TODO: this could register elements out of order
    whenDependentTypesAreResolved([rawType], function(type) {
        type = type[0];
        tupleType.elements.push({
            read: function(ptr) {
                return type.fromWireType(getter(ptr, memberPointer));
            },
            write: function(ptr, o) {
                var destructors = [];
                setter(ptr, memberPointer, type.toWireType(destructors, o));
                runDestructors(destructors);
            }
        });
    });
}

function __embind_register_tuple_element_accessor(
    rawTupleType,
    rawElementType,
    rawStaticGetter,
    getterSize,
    getter,
    rawStaticSetter,
    setterSize,
    setter
) {
    var tupleType = requireRegisteredType(rawTupleType, 'tuple');
    rawStaticGetter = FUNCTION_TABLE[rawStaticGetter];
    getter = copyMemberPointer(getter, getterSize);
    rawStaticSetter = FUNCTION_TABLE[rawStaticSetter];
    setter = copyMemberPointer(setter, setterSize);

    whenDependentTypesAreResolved([rawElementType], function(elementType) {
        elementType = elementType[0];
        tupleType.elements.push({
            read: function(ptr) {
                return elementType.fromWireType(rawStaticGetter(ptr, HEAP32[getter >> 2]));
            },
            write: function(ptr, o) {
                var destructors = [];
                rawStaticSetter(
                    ptr,
                    HEAP32[setter >> 2],
                    elementType.toWireType(destructors, o));
                runDestructors(destructors);
            }
        });
    });
}

function __embind_register_struct(
    rawType,
    name,
    rawConstructor,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    registerType(rawType, {
        name: name,
        rawConstructor: rawConstructor,
        rawDestructor: rawDestructor,
        fields: {},
        fromWireType: function(ptr) {
            var fields = this.fields;
            var rv = {};
            for (var i in fields) {
                rv[i] = fields[i].read(ptr);
            }
            this.rawDestructor(ptr);
            return rv;
        },
        toWireType: function(destructors, o) {
            var fields = this.fields;
            for (var fieldName in fields) {
                if (!(fieldName in o)) {
                    throw new TypeError('Missing field');
                }
            }
            var ptr = this.rawConstructor();
            for (fieldName in fields) {
                fields[fieldName].write(ptr, o[fieldName]);
            }
            destructors.push(rawDestructor);
            destructors.push(ptr);
            return ptr;
        },
    });
}

function __embind_register_struct_field(
    rawStructType,
    fieldName,
    rawFieldType,
    rawGetter,
    rawSetter,
    memberPointerSize,
    memberPointer
) {
    var structType = requireRegisteredType(rawStructType, 'struct');
    fieldName = Pointer_stringify(fieldName);
    rawGetter = FUNCTION_TABLE[rawGetter];
    rawSetter = FUNCTION_TABLE[rawSetter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);
    // TODO: this could register elements out of order
    whenDependentTypesAreResolved([rawFieldType], function(fieldType) {
        fieldType = fieldType[0];
        structType.fields[fieldName] = {
            read: function(ptr) {
                return fieldType.fromWireType(rawGetter(ptr, memberPointer));
            },
            write: function(ptr, o) {
                var destructors = [];
                rawSetter(ptr, memberPointer, fieldType.toWireType(destructors, o));
                runDestructors(destructors);
            }
        };
    });
}

function RegisteredPointer(name, rawType, registeredClass, pointeeType, Handle, isSmartPointer, rawGetPointee, rawConstructor, rawDestructor) {
    this.name = name;
    this.rawType = rawType;
    this.registeredClass = registeredClass;
    this.pointeeType = pointeeType;
    this.Handle = Handle; // <-- I think I can kill this
    this.isSmartPointer = isSmartPointer;
    this.rawGetPointee = rawGetPointee;
    this.rawConstructor = rawConstructor;
    this.rawDestructor = rawDestructor;
}

RegisteredPointer.prototype.isPolymorphic = function() {
    return this.registeredClass.isPolymorphic;
};

RegisteredPointer.prototype.toWireType = function(destructors, handle) {
    var fromRawType;
    if (handle === null) {
        return 0; // todo: maybe this should return a zero-initialized smart pointer object
    }
    if (!(handle instanceof ClassHandle)) {
        throwBindingError('Expected pointer or null, got ' + IMVU.repr(handle));
    }
    if (this.isSmartPointer && undefined === handle.$$.smartPtr) {
        throwBindingError('Passing raw pointer to smart pointer is illegal');
    }
    var pointeeType = handle.$$.pointeeType;
    if (pointeeType.isPolymorphic()) {
        fromRawType = pointeeType.getDynamicRawPointerType(handle.$$.ptr);
    } else {
        fromRawType = pointeeType.rawType;
    }
    if (fromRawType === this.pointeeType.rawType) {
        return this.isSmartPointer ? handle.$$.smartPtr : handle.$$.ptr;
    }
    var ptr = staticPointerCast(handle.$$.ptr, fromRawType, this.pointeeType.rawType);
    if (this.isSmartPointer) {
        // If this is for smart ptr type conversion, I think it
        // assumes that smart_ptr<T> has an identical binary layout to
        // smart_ptr<U>.  I wonder if that's untrue for any common
        // smart pointer. - chad
        ptr = this.rawConstructor(
            ptr,
            handle.$$.smartPtr);
        destructors.push(this.rawDestructor);
        destructors.push(ptr);
    }
    return ptr;
};

RegisteredPointer.prototype.getPointee = function(ptr) {
    if (this.rawGetPointee) {
        ptr = this.rawGetPointee(ptr);
    }
    return ptr;
};

RegisteredPointer.prototype.destructor = function(ptr) {
    if (this.rawDestructor) {
        this.rawDestructor(ptr);
    }
};

RegisteredPointer.prototype.fromWireType = function(ptr) {
    if (!this.getPointee(ptr)) {
        this.destructor(ptr);
        return null;
    }
    return new this.Handle(ptr);
};

// todo: could this return the actual type if not polymorphic?
RegisteredPointer.prototype.getDynamicRawPointerType = function(ptr) {
    var type = null;
    if (this.isPolymorphic()) {
        if (this.rawGetPointee) {
            type = ___getDynamicPointerType(this.rawGetPointee(ptr));
        } else {
            type = ___getDynamicPointerType(ptr);
        }
    }
    return type;
};

RegisteredPointer.prototype.getDynamicDowncastType = function(ptr) {
    var downcastType =  null;
    var type = this.getDynamicRawPointerType(ptr);
    if (type && type !== this.pointeeType.rawType) {
        var derivation = Module.__getDerivationPath(type, this.pointeeType.rawType);
        for (var i = 0; i < derivation.size(); i++) {
            downcastType = registeredTypes[derivation.get(i)];
            if (downcastType && (!this.isSmartPointer || downcastType.smartPointerType)) {
                break;
            }
        }
        derivation.delete();
    }
    return downcastType;
};

RegisteredPointer.prototype.fromWireTypeAutoDowncast = function(ptr) { // ptr is a raw pointer (or a raw smartpointer)
    var handle;
    if (!this.getPointee(ptr)) {
        this.destructor(ptr);
        return null;
    }
    var toType = this.getDynamicDowncastType(ptr);
    if (toType) {
        var fromType = this.pointeeType;
        if (this.isSmartPointer) {
            handle = toType.smartPointerType.fromWireType(ptr);
        } else {
            handle = toType.fromWireType(ptr);
        }
        handle.$$.ptr = staticPointerCast(handle.$$.ptr, fromType.rawType, toType.rawType);
    } else {
        handle = this.fromWireType(ptr);
    }
    return handle;
};

function __embind_register_smart_ptr(
    rawType,
    rawPointeeType,
    name,
    rawConstructor,
    rawDestructor,
    rawGetPointee
) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    rawGetPointee = FUNCTION_TABLE[rawGetPointee];

    whenDependentTypesAreResolved([rawPointeeType], function(pointeeType) {
        pointeeType = pointeeType[0];

        var Handle = createNamedFunction(name, function(ptr) {
            Object.defineProperty(this, '$$', {
                value: {
                    count: {value: 1},
                    smartPtr: ptr,
                    ptr: rawGetPointee(ptr),
                    pointeeType: pointeeType,
                },
            });
        });

        // TODO: test for SmartPtr.prototype.constructor property?
        // We likely want it distinct from pointeeType.prototype.constructor
        Handle.prototype = Object.create(pointeeType.Handle.prototype);

        Handle.prototype.clone = function() {
            if (!this.$$.ptr) {
                throwBindingError(pointeeType.name + ' instance already deleted');
            }

            var clone = Object.create(Handle.prototype);
            Object.defineProperty(clone, '$$', {
                value: {
                    count: this.$$.count,
                    smartPtr: this.$$.smartPtr,
                    ptr: this.$$.ptr,
                },
            });

            clone.$$.count.value += 1;
            return clone;
        };

        Handle.prototype['delete'] = function() {
            if (!this.$$.ptr) {
                throwBindingError(pointeeType.name + ' instance already deleted');
            }

            this.$$.count.value -= 1;
            if (0 === this.$$.count.value) {
                rawDestructor(this.$$.smartPtr);
            }
            this.$$.smartPtr = undefined;
            this.$$.ptr = undefined;
        };
        
        var registeredPointer = new RegisteredPointer(
            name,
            rawType,
            pointeeType.registeredClass,
            pointeeType,
            Handle,
            true,
            rawGetPointee,
            rawConstructor,
            rawDestructor);
        registerType(rawType, registeredPointer);
        pointeeType.smartPointerType = registeredPointer;
    });
}

function ClassHandle() {
}

function RegisteredClass(
    name,
    isPolymorphic,
    baseClassRawType,
    baseClass,
    upcast,
    downcast
) {
    this.name = name;
    this.isPolymorphic = isPolymorphic;
    this.baseClassRawType = baseClassRawType;
    this.baseClass = baseClass;
    this.upcast = upcast;
    this.downcast = downcast;
}

function __embind_register_class(
    rawType,
    rawPointerType,
    rawConstPointerType,
    baseClassRawType,
    upcast,
    downcast,
    isPolymorphic,
    name,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    upcast = FUNCTION_TABLE[upcast];
    downcast = FUNCTION_TABLE[downcast];

    whenDependentTypesAreResolved(baseClassRawType ? [baseClassRawType] : [], function(base) {
        base = base[0];

        var baseClass;
        var basePrototype;
        var depth;
        if (baseClassRawType) {
            baseClasses[rawType] = baseClassRawType;

            baseClass = base.registeredClass;
            basePrototype = base.Handle.prototype;
        } else {
            basePrototype = ClassHandle.prototype;
        }

        var registeredClass = new RegisteredClass(
            name,
            isPolymorphic,
            baseClassRawType,
            baseClass,
            upcast,
            downcast);

        var Handle = createNamedFunction(name, function(ptr) {
            Object.defineProperty(this, '$$', {
                value: {
                    count: {value: 1, ptr: ptr},
                    ptr: ptr,
                    pointeeType: type,
                }
            });
        });
        
        Handle.prototype = Object.create(basePrototype, {
            constructor: { value: Handle },
        });
        Handle.prototype.clone = function() {
            if (!this.$$.ptr) {
                throwBindingError(type.name + ' instance already deleted');
            }

            var clone = Object.create(Handle.prototype);
            Object.defineProperty(clone, '$$', {
                value: {
                    count: this.$$.count,
                    ptr: this.$$.ptr,
                },
            });

            clone.$$.count.value += 1;
            return clone;
        };

        // todo: test delete with upcast and downcast multiply derived pointers
        // todo: then replace this.$$.count.ptr below with this.$$.ptr and make sure it fails
        Handle.prototype['delete'] = function() {
            if (!this.$$.ptr) {
                throwBindingError(type.name + ' instance already deleted'); // todo: but 'type' hasn't been resolved!?!
            }
            
            this.$$.count.value -= 1;
            if (0 === this.$$.count.value) {
                rawDestructor(this.$$.count.ptr);
            }
            this.$$.ptr = undefined;
        };

        // todo: clean this up!
        var type = new RegisteredPointer(
            name,
            rawType,
            registeredClass,
            undefined,
            Handle,
            false);
        type.pointeeType = type; // :(
        registerType(rawType, type);

        registerType(rawPointerType, new RegisteredPointer(
            name + '*',
            rawPointerType,
            registeredClass,
            type,
            Handle,
            false));

        // todo: implement const pointers (no modification Javascript side)
        registerType(rawConstPointerType, new RegisteredPointer(
            name + ' const*',
            rawConstPointerType,
            registeredClass,
            type,
            Handle,
            false));

        type.constructor = createNamedFunction(name, function() {
            if (Object.getPrototypeOf(this) !== Handle.prototype) {
                throw new BindingError("Use 'new' to construct " + name);
            }
            var body = type.constructor_body;
            if (undefined === body) {
                throw new BindingError(name + " has no accessible constructor");
            }
            return body.apply(this, arguments);
        });
        type.constructor.prototype = type.Handle.prototype;
        type.constructor.type = type;

        exposePublicSymbol(name, type.constructor);
    });
}

function __embind_register_class_constructor(
    rawClassType,
    argCount,
    rawArgTypesAddr,
    invoker,
    rawConstructor
) {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    invoker = FUNCTION_TABLE[invoker];

    whenDependentTypesAreResolved([rawClassType].concat(rawArgTypes), function(argTypes) {
        var classType = argTypes[0];
        argTypes = argTypes.slice(1);
        var humanName = 'constructor ' + classType.name;
        classType.constructor_body = function() {
            if (arguments.length !== argCount - 1) {
                throwBindingError(humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
            }
            var destructors = [];
            var args = new Array(argCount);
            args[0] = rawConstructor;
            for (var i = 1; i < argCount; ++i) {
                args[i] = argTypes[i].toWireType(destructors, arguments[i - 1]);
            }

            var ptr = invoker.apply(null, args);
            runDestructors(destructors);

            return argTypes[0].fromWireType(ptr);
        };
    });
}

function upcastPointer(ptr, ptrClass, desiredClass) {
    while (ptrClass !== desiredClass) {
        ptr = ptrClass.upcast(ptr);
        ptrClass = ptrClass.baseClass;
    }
    return ptr;
}

function __embind_register_class_method(
    rawClassType,
    methodName,
    argCount,
    rawArgTypesAddr,
    rawInvoker,
    memberFunctionSize,
    memberFunction
) {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = Pointer_stringify(methodName);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    memberFunction = copyMemberPointer(memberFunction, memberFunctionSize);

    whenDependentTypesAreResolved([rawClassType].concat(rawArgTypes), function(argTypes) {
        var classType = argTypes[0];
        argTypes = argTypes.slice(1);
        var humanName = classType.name + '.' + methodName;
        classType.Handle.prototype[methodName] = function() {
            if (!this.$$.ptr) {
                throwBindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
            }
            if (arguments.length !== argCount - 1) {
                throwBindingError('emscripten binding method ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
            }

            // TODO: error if pointer type doesn't match signature
            var ptr = upcastPointer(
                this.$$.ptr,
                this.$$.pointeeType.registeredClass,
                classType.registeredClass);
            
            var destructors = [];
            var args = new Array(argCount + 1);
            args[0] = ptr;
            args[1] = memberFunction;
            for (var i = 1; i < argCount; ++i) {
                args[i + 1] = argTypes[i].toWireType(destructors, arguments[i - 1]);
            }
            var rv = rawInvoker.apply(null, args);
            if (argTypes[0].fromWireTypeAutoDowncast) {
                rv = argTypes[0].fromWireTypeAutoDowncast(rv);
            } else {
                rv = argTypes[0].fromWireType(rv);
            }
            runDestructors(destructors);
            return rv;
        };
    });
}

function __embind_register_class_classmethod(
    rawClassType,
    methodName,
    argCount,
    rawArgTypesAddr,
    rawInvoker,
    fn
) {
    // todo: whenDependentTypesAreResolved
    var classType = requireRegisteredType(rawClassType, 'class');
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = Pointer_stringify(methodName);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    whenDependentTypesAreResolved(rawArgTypes, function(argTypes) {
        var humanName = classType.name + '.' + methodName;
        classType.constructor[methodName] = makeInvoker(humanName, argCount, argTypes, rawInvoker, fn);
    });
}

function __embind_register_class_field(
    rawClassType,
    fieldName,
    rawFieldType,
    getter,
    setter,
    memberPointerSize,
    memberPointer
) {
    // todo: whenDependentTypesAreResolved
    fieldName = Pointer_stringify(fieldName);
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);
    whenDependentTypesAreResolved([rawClassType, rawFieldType], function(converters) {
        var classType = converters[0];
        var fieldType = converters[1];
        var humanName = classType.name + '.' + fieldName;
        Object.defineProperty(classType.Handle.prototype, fieldName, {
            get: function() {
                if (!this.$$.ptr) {
                    throwBindingError('cannot access emscripten binding field ' + humanName + ' on deleted object');
                }

                // TODO: error if pointer type doesn't match signature
                var ptr = upcastPointer(
                    this.$$.ptr,
                    this.$$.pointeeType.registeredClass,
                    classType.registeredClass);

                return fieldType.fromWireType(getter(ptr, memberPointer));
            },
            set: function(v) {
                if (!this.$$.ptr) {
                    throwBindingError('cannot modify emscripten binding field ' + humanName + ' on deleted object');
                }

                // TODO: error if pointer type doesn't match signature
                var ptr = upcastPointer(
                    this.$$.ptr,
                    this.$$.pointeeType.registeredClass,
                    classType.registeredClass);

                var destructors = [];
                setter(ptr, memberPointer, fieldType.toWireType(destructors, v));
                runDestructors(destructors);
            },
            enumerable: true
        });
    });
}

function __embind_register_enum(
    rawType,
    name
) {
    name = Pointer_stringify(name);

    function constructor() {
    }
    constructor.values = {};

    registerType(rawType, {
        name: name,
        constructor: constructor,
        fromWireType: function(c) {
            return this.constructor.values[c];
        },
        toWireType: function(destructors, c) {
            return c.value;
        },
    });
    exposePublicSymbol(name, constructor);
}

function __embind_register_enum_value(
    rawEnumType,
    name,
    enumValue
) {
    var enumType = requireRegisteredType(rawEnumType, 'enum');
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
    rawType,
    name,
    rawConstructor,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    registerType(rawType, {
        name: name,
        rawConstructor: rawConstructor,
        rawDestructor: rawDestructor,
        toWireType: function(destructors, o) {
            var handle = __emval_register(o);
            var ptr = this.rawConstructor(handle);
            destructors.push(this.rawDestructor);
            destructors.push(ptr);
            return ptr;
        },
    });
}

