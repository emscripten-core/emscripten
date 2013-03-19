/*global Module*/
/*global _malloc, _free, _memcpy*/
/*global FUNCTION_TABLE, HEAP32, HEAPU8*/
/*global Pointer_stringify*/
/*global __emval_register, _emval_handle_array, __emval_decref*/
/*global ___typeName:false*/

var InternalError = Module.InternalError = extendError(Error, 'InternalError');
var BindingError = Module.BindingError = extendError(Error, 'BindingError');

function throwInternalError(value) {
    throw new InternalError(value);
}

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

// class typeID -> {pointerType: ..., constPointerType: ...}
var registeredPointers = {};

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
            destructors.push(_free, ptr);
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
        rv = argTypes[0].fromWireType(rv);
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
            destructors.push(rawDestructor, ptr);
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
            destructors.push(rawDestructor, ptr);
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

// todo: I guarantee there is a way to simplify the following data structure.
function RegisteredPointer(
    name,
    rawType,
    registeredClass,
    Handle,
    isReference,
    isConst,
    isSmartPointer,
    pointeeType,
    sharingPolicy,
    rawGetPointee,
    rawConstructor,
    rawShare,
    rawDestructor
) {
    this.name = name;
    this.rawType = rawType;
    this.registeredClass = registeredClass;
    this.Handle = Handle; // <-- I think I can kill this
    this.isReference = isReference;
    this.isConst = isConst;
    this.isSmartPointer = isSmartPointer;
    this.pointeeType = pointeeType;
    this.sharingPolicy = sharingPolicy;
    this.rawGetPointee = rawGetPointee;
    this.rawConstructor = rawConstructor;
    this.rawShare = rawShare;
    this.rawDestructor = rawDestructor;
}

RegisteredPointer.prototype.toWireType = function(destructors, handle) {
    var self = this;
    function throwCannotConvert() {
        var name;
        if (handle.$$.smartPtrType) {
            name = handle.$$.smartPtrType.name;
        } else {
            name = handle.$$.ptrType.name;
        }
        throwBindingError('Cannot convert argument of type ' + name + ' to parameter type ' + self.name);
    }

    if (handle === null) {
        if (this.isReference) {
            throwBindingError('null is not a valid ' + this.name);
        }

        if (this.isSmartPointer) {
            var ptr = this.rawConstructor();
            destructors.push(this.rawDestructor, ptr);
            return ptr;
        } else {
            return 0;
        }
    }
    if (!(handle instanceof this.registeredClass.constructor)) {
        throwBindingError('Expected null or instance of ' + this.name + ', got ' + IMVU.repr(handle));
    }
    // TODO: this is not strictly true
    // It seems legal to support BY_EMVAL and INTRUSIVE conversions from raw pointers to smart pointers
    if (this.isSmartPointer && undefined === handle.$$.smartPtr) {
        throwBindingError('Passing raw pointer to smart pointer is illegal');
    }
    if (!this.isConst && handle.$$.ptrType.isConst) {
        throwCannotConvert();
    }
    var handleClass = handle.$$.ptrType.registeredClass;
    var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
    if (this.isSmartPointer) {
        switch (this.sharingPolicy) {
            case 0: // NONE
                // no upcasting
                if (handle.$$.smartPtrType === this) {
                    ptr = handle.$$.smartPtr;
                } else {
                    throwCannotConvert();
                }
                break;
            
            case 1: // INTRUSIVE
                throwBindingError('INTRUSIVE sharing policy not yet supported');
                break;
            
            case 2: // BY_EMVAL
                if (handle.$$.smartPtrType === this) {
                    ptr = handle.$$.smartPtr;
                } else {
                    var clonedHandle = handle.clone();
                    ptr = this.rawShare(
                        ptr,
                        __emval_register(function() {
                            clonedHandle.delete();
                        })
                    );
                    destructors.push(this.rawDestructor, ptr);
                }
                break;
            
            default:
                throwBindingError('Unsupporting sharing policy');
        }
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
    // ptr is a raw pointer (or a raw smartpointer)

    // rawPointer is a maybe-null raw pointer
    var rawPointer = this.getPointee(ptr);
    if (!rawPointer) {
        this.destructor(ptr);
        return null;
    }

    function makeDefaultHandle() {
        if (this.isSmartPointer) {
            return makeClassHandle(this.Handle.prototype, {
                ptrType: this.pointeeType,
                ptr: rawPointer,
                smartPtrType: this,
                smartPtr: ptr,
            });
        } else {
            return makeClassHandle(this.Handle.prototype, {
                ptrType: this,
                ptr: ptr,
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
        return makeClassHandle(toType.Handle.prototype, {
            ptrType: toType,
            ptr: dp,
            smartPtrType: this,
            smartPtr: ptr,
        });
    } else {
        return makeClassHandle(toType.Handle.prototype, {
            ptrType: toType,
            ptr: dp,
        });
    }
};

function makeClassHandle(prototype, record) {
    if (!record.ptrType || !record.ptr) {
        throwInternalError('makeClassHandle requires ptr and ptrType');
    }
    var hasSmartPtrType = !!record.smartPtrType;
    var hasSmartPtr = !!record.smartPtr;
    if (hasSmartPtrType !== hasSmartPtr) {
        throwInternalError('Both smartPtrType and smartPtr must be specified');
    }
    record.count = { value: 1 };
    return Object.create(prototype, {
        '$$': {
            value: record,
        },
    });
}

// root of all pointer and smart pointer handles in embind
function ClassHandle() {
}

function getInstanceTypeName(handle) {
    return handle.$$.ptrType.registeredClass.name;
}

ClassHandle.prototype.clone = function() {
    if (!this.$$.ptr) {
        throwBindingError(getInstanceTypeName(this) + ' instance already deleted');
    }

    var clone = Object.create(Object.getPrototypeOf(this), {
        '$$': {
            value: shallowCopy(this.$$),
        }
    });

    clone.$$.count.value += 1;
    return clone;
};

function runDestructor(handle) {
    var $$ = handle.$$;
    if ($$.smartPtr) {
        $$.smartPtrType.rawDestructor($$.smartPtr);
    } else {
        $$.ptrType.registeredClass.rawDestructor($$.ptr);
    }
}

ClassHandle.prototype['delete'] = function() {
    if (!this.$$.ptr) {
        throwBindingError(getInstanceTypeName(this) + ' instance already deleted');
    }

    this.$$.count.value -= 1;
    if (0 === this.$$.count.value) {
        runDestructor(this);
    }
    this.$$.smartPtr = undefined;
    this.$$.ptr = undefined;
};
        
function RegisteredClass(
    name,
    rawType,
    constructor,
    rawDestructor,
    baseClass,
    getActualType,
    upcast,
    downcast
) {
    this.name = name;
    this.rawType = rawType;
    this.constructor = constructor;
    this.rawDestructor = rawDestructor;
    this.baseClass = baseClass;
    this.getActualType = getActualType;
    this.upcast = upcast;
    this.downcast = downcast;
}

function shallowCopy(o) {
    var rv = {};
    for (var k in o) {
        rv[k] = o[k];
    }
    return rv;
}

function __embind_register_class(
    rawType,
    rawPointerType,
    rawConstPointerType,
    baseClassRawType,
    getActualType,
    upcast,
    downcast,
    name,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    getActualType = FUNCTION_TABLE[getActualType];
    upcast = FUNCTION_TABLE[upcast];
    downcast = FUNCTION_TABLE[downcast];
    var legalFunctionName = makeLegalFunctionName(name);

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

        var Handle = createNamedFunction(legalFunctionName, function(ptrType, ptr) {
            Object.defineProperty(this, '$$', {
                value: {
                    ptrType: ptrType,
                    count: { value: 1 },
                    ptr: ptr,
                }
            });
        });
        
        Handle.prototype = Object.create(basePrototype, {
            constructor: { value: Handle },
        });

        var registeredClass = new RegisteredClass(
            name,
            rawType,
            Handle,
            rawDestructor,
            baseClass,
            getActualType,
            upcast,
            downcast);

        // todo: clean this up!
        var type = new RegisteredPointer(
            name,
            rawType,
            registeredClass,
            Handle,
            true,
            false,
            false);
        registerType(rawType, type);

        var pointerType = new RegisteredPointer(
            name + '*',
            rawPointerType,
            registeredClass,
            Handle,
            false,
            false,
            false);
        registerType(rawPointerType, pointerType);

        var constPointerType = new RegisteredPointer(
            name + ' const*',
            rawConstPointerType,
            registeredClass,
            Handle,
            false,
            true,
            false);
        registerType(rawConstPointerType, constPointerType);

        registeredPointers[rawType] = {
            pointerType: pointerType,
            constPointerType: constPointerType
        };

        type.constructor = createNamedFunction(legalFunctionName, function() {
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

        exposePublicSymbol(legalFunctionName, type.constructor);
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

function downcastPointer(ptr, ptrClass, desiredClass) {
    if (ptrClass === desiredClass) {
        return ptr;
    }
    if (undefined === desiredClass.baseClass) {
        return null; // no conversion
    }
    // O(depth) stack space used
    return desiredClass.downcast(
        downcastPointer(ptr, ptrClass, desiredClass.baseClass));
}

function upcastPointer(ptr, ptrClass, desiredClass) {
    while (ptrClass !== desiredClass) {
        ptr = ptrClass.upcast(ptr);
        ptrClass = ptrClass.baseClass;
    }
    return ptr;
}

function validateThis(this_, classType, humanName) {
    if (!(this_ instanceof Object)) {
        throwBindingError(humanName + ' with invalid "this": ' + this_);
    }
    if (!(this_ instanceof classType.constructor)) {
        throwBindingError(humanName + ' incompatible with "this" of type ' + this_.constructor.name);
    }
    if (!this_.$$.ptr) {
        throwBindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
    }
    
    return upcastPointer(
        this_.$$.ptr,
        this_.$$.ptrType.registeredClass,
        classType.registeredClass);
}

function __embind_register_class_function(
    rawClassType,
    methodName,
    argCount,
    rawArgTypesAddr,
    isConst,
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
            if (arguments.length !== argCount - 1) {
                throwBindingError('emscripten binding method ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
            }

            var ptr = validateThis(this, classType, humanName);
            if (!isConst && this.$$.ptrType.isConst) {
                throwBindingError('Cannot call non-const method ' + humanName + ' on const reference');
            }

            var destructors = [];
            var args = new Array(argCount + 1);
            args[0] = ptr;
            args[1] = memberFunction;
            for (var i = 1; i < argCount; ++i) {
                args[i + 1] = argTypes[i].toWireType(destructors, arguments[i - 1]);
            }
            var rv = rawInvoker.apply(null, args);
            rv = argTypes[0].fromWireType(rv);
            runDestructors(destructors);
            return rv;
        };
    });
}

function __embind_register_class_class_function(
    rawClassType,
    methodName,
    argCount,
    rawArgTypesAddr,
    rawInvoker,
    fn
) {
    var classType = requireRegisteredType(rawClassType, 'class');
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = Pointer_stringify(methodName);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    whenDependentTypesAreResolved(rawArgTypes, function(argTypes) {
        var humanName = classType.name + '.' + methodName;
        classType.constructor[methodName] = makeInvoker(humanName, argCount, argTypes, rawInvoker, fn);
    });
}

function __embind_register_class_property(
    rawClassType,
    fieldName,
    rawFieldType,
    getter,
    setter,
    memberPointerSize,
    memberPointer
) {
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
                var ptr = validateThis(this, classType, humanName + ' getter');
                return fieldType.fromWireType(getter(ptr, memberPointer));
            },
            set: function(v) {
                var ptr = validateThis(this, classType, humanName + ' setter');
                var destructors = [];
                setter(ptr, memberPointer, fieldType.toWireType(destructors, v));
                runDestructors(destructors);
            },
            enumerable: true
        });
    });
}

var char_0 = '0'.charCodeAt(0);
var char_9 = '9'.charCodeAt(0);
function makeLegalFunctionName(name) {
    var rv = name.replace(/[^a-zA-Z0-9_]/g, '$');
    var f = rv.charCodeAt(0);
    if (f >= char_0 && f <= char_9) {
        return '_' + rv;
    } else {
        return rv;
    }
}

function __embind_register_smart_ptr(
    rawType,
    rawPointeeType,
    name,
    sharingPolicy,
    rawGetPointee,
    rawConstructor,
    rawShare,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawGetPointee = FUNCTION_TABLE[rawGetPointee];
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawShare = FUNCTION_TABLE[rawShare];
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    whenDependentTypesAreResolved([rawPointeeType], function(pointeeType) {
        pointeeType = pointeeType[0];

        var Handle = createNamedFunction(makeLegalFunctionName(name), function(smartPtrType, smartPtr, ptrType, ptr) {
            if (arguments.length !== 4) {
                throwBindingError("internal error");
            }
            Object.defineProperty(this, '$$', {
                value: {
                    count: {value: 1},
                    ptrType: ptrType,
                    ptr: ptr,
                    smartPtrType: registeredPointer,
                    smartPtr: smartPtr,
                },
            });
        });

        Handle.prototype = Object.create(pointeeType.Handle.prototype);

        var registeredPointer = new RegisteredPointer(
            name,
            rawType,
            pointeeType.registeredClass,
            Handle,
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
        registerType(rawType, registeredPointer);
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
            destructors.push(this.rawDestructor, ptr);
            return ptr;
        },
    });
}

