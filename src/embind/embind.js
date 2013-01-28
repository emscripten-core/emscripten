/*global Module*/
/*global Module*/
/*global _malloc, _free, _memcpy*/
/*global FUNCTION_TABLE, HEAP32*/
/*global Pointer_stringify, writeStringToMemory*/
/*global __emval_register, _emval_handle_array, __emval_decref*/
/*global ___getDynamicPointerType: false*/
/*global ___typeName:false*/
/*global ___staticPointerCast: false*/

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
var deferredRegistrations = [];

function requestDeferredRegistration(registrationFunction) {
    deferredRegistrations.push(registrationFunction);
}

function performDeferredRegistrations(){
    while(deferredRegistrations.length > 0) {
        var registrationFunction = deferredRegistrations.shift();
        registrationFunction();
    }
}

function createInheritedFunctionOrProperty(name, type, nameInBaseClass, baseClassType) {
    function upcastingWrapper(method) {
        return function() {
            var baseClassPtr = ___staticPointerCast(this.ptr, type.rawType, baseClassType.rawType);
            if (baseClassPtr === this.ptr) {
                return method.apply(this, arguments);
            } else {
                var handle = this.clone();
                try {
                    handle.ptr = baseClassPtr;
                    return method.apply(handle, arguments);
                } finally {
                    handle.delete();
                }
            }
        };
    }
    var baseClassPrototype = baseClassType.Handle.prototype;
    if (baseClassPrototype.constructor.memberType[nameInBaseClass] === 'field') {
        var baseClassDescriptor = Object.getOwnPropertyDescriptor(baseClassPrototype, nameInBaseClass);
        Object.defineProperty(type.Handle.prototype, name, {
            enumerable: true,
            get: upcastingWrapper(baseClassDescriptor.get),
            set: upcastingWrapper(baseClassDescriptor.set)
        });
    } else if (baseClassPrototype.constructor.memberType[nameInBaseClass] === 'method') {
        var baseClassMethod = baseClassPrototype[nameInBaseClass];
        type.Handle.prototype[name] = createNamedFunction(name, upcastingWrapper(baseClassMethod));
    }
}

function resolveType(type) {
    if (!type.resolved) {
        var baseClassType, name, baseProto;
        var inheritedNames = {};
        var rawBaseClassTypes =  Module.__getBaseClasses(type.rawType);
        for (var i = 0; i < rawBaseClassTypes.size(); i++) {
            var rawBaseClassType = rawBaseClassTypes.at(i);
            baseClassType = typeRegistry[rawBaseClassType];
            if (baseClassType) {
                resolveType(baseClassType);
                baseProto = baseClassType.Handle.prototype;
                for (name in baseProto) {
                    if (baseProto.hasOwnProperty(name) && baseClassType.Handle.memberType[name]) {
                        if (!(name in inheritedNames)) {
                            inheritedNames[name] = [];
                        }
                        inheritedNames[name].push(baseClassType);
                    }
                }
            }
        }
        for (name in inheritedNames) {
            if (inheritedNames.hasOwnProperty(name)) {
                if (!type.Handle.prototype.hasOwnProperty(name) && inheritedNames[name].length === 1) {
                    baseClassType = inheritedNames[name][0];
                    createInheritedFunctionOrProperty(name, type, name, baseClassType);
                } else {
                    for (var j = 0; j < inheritedNames[name].length; j++) {
                        baseClassType = inheritedNames[name][j];
                        createInheritedFunctionOrProperty(baseClassType.name+"_"+name, type, name, baseClassType);
                    }
                }
            }
        }
        type.resolved = true;
    }
}

function resolveBindings() {
    performDeferredRegistrations();
    for (var rawType in typeRegistry) {
        if (typeRegistry.hasOwnProperty(rawType)) {
            resolveType(typeRegistry[rawType]);
        }
    }
}

function registerType(rawType, name, registeredInstance) {
    if (!rawType) {
        throw new BindingError('type "' + name + '" must have a positive integer typeid pointer');
    }
    if (typeRegistry.hasOwnProperty(rawType)) {
        throw new BindingError("Cannot register type '" + name + "' twice");
    }
    registeredInstance.rawType = rawType;
    registeredInstance.name = name;
    typeRegistry[rawType] = registeredInstance;
    return registeredInstance;
}

function requireRegisteredType(rawType, humanName) {
    var impl = typeRegistry[rawType];
    if (undefined === impl) {
        throw new BindingError(humanName + " has unknown type " + typeName(rawType));
    }
    return impl;
}

function typeName(rawType) {
    return Pointer_stringify(___typeName(rawType));
}

function heap32VectorToArray(count, firstElement) {
    var array = [];
    for (var i = 0; i < count; i++) {
        array.push(HEAP32[(firstElement >> 2) + i]);
    }
    return array;
}

function requireArgumentTypes(rawArgTypes, name) {
    var argTypes = [];
    for (var i = 0; i < rawArgTypes.length; ++i) {
        if (i === 0) {
            argTypes[i] = requireRegisteredType(rawArgTypes[i], name + " return value");
        } else {
            argTypes[i] = requireRegisteredType(rawArgTypes[i], name + " parameter " + i);
        }
    }
    return argTypes;
}

function staticPointerCast(from, fromType, toType) {
    if (!from) {
        return from;
    }
    var to = ___staticPointerCast(from, fromType, toType);
    if (to <= 0) {
        throw new CastError("Pointer conversion is not available");
    }
    return to;
}

function RegisteredVoid() {
}

RegisteredVoid.prototype.fromWireType = function() {
    return undefined;
};

function __embind_register_void(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, name, new RegisteredVoid());
}

function RegisteredBool(trueValue, falseValue) {
    this.trueValue = trueValue;
    this.falseValue = falseValue;
}

RegisteredBool.prototype.toWireType = function(destructors, o) {
    return o ? this.trueValue : this.falseValue;
};

RegisteredBool.prototype.fromWireType = function(wt) {
            // ambiguous emscripten ABI: sometimes return values are
            // true or false, and sometimes integers (0 or 1)
            return !!wt;
};

function __embind_register_bool(rawType, name, trueValue, falseValue) {
    name = Pointer_stringify(name);
    registerType(rawType, name, new RegisteredBool(trueValue, falseValue));
}

function RegisteredInteger() {
}

RegisteredInteger.prototype.toWireType = function(destructors, value) {
    if (typeof value !== "number") {
        throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + this.name);
    }
    return value | 0;
};

RegisteredInteger.prototype.fromWireType = function(value) {
    return value;
};

function __embind_register_integer(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, name, new RegisteredInteger());
}

function RegisteredFloat() {
}

RegisteredFloat.prototype.toWireType = function(destructors, value) {
    if (typeof value !== "number") {
        throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' +this.name);
    }
    return value;
};

RegisteredFloat.prototype.fromWireType = function(value) {
    return value;
};

function __embind_register_float(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, name, new RegisteredFloat());
}

function RegisteredString(stringType, name) {

}

RegisteredString.prototype.toWireType = function(destructors, value) {
    var ptr = _malloc(value.length + 1);
    writeStringToMemory(value, ptr);
    destructors.push(_free);
    destructors.push(ptr);
    return ptr;
};

RegisteredString.prototype.fromWireType = function(value) {
    var rv = Pointer_stringify(value);
    _free(value);
    return rv;
};

function __embind_register_cstring(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, name, new RegisteredString());
}

function RegisteredEmval() {
}

RegisteredEmval.prototype.toWireType = function(destructors, value) {
    return __emval_register(value);
};

RegisteredEmval.prototype.fromWireType = function(handle) {
    var rv = _emval_handle_array[handle].value;
    __emval_decref(handle);
    return rv;
};

function __embind_register_emval(rawType, name) {
    name = Pointer_stringify(name);
    registerType(rawType, name, new RegisteredEmval());
}

var BindingError = Error;
var CastError = Error;
/** @expose */
Module.BindingError = BindingError;
Module.CastError = CastError;

function runDestructors(destructors) {
    while (destructors.length) {
        var ptr = destructors.pop();
        var del = destructors.pop();
        del(ptr);
    }
}

function refreshSmartPointee(handle) {
    if (handle && handle.smartPointer) {
        handle.ptr = handle.type.smartPointerType.getPointee(handle.smartPointer);
    }
}

function makeInvoker(name, argCount, argTypes, invoker, fn) {
    if (!FUNCTION_TABLE[fn]) {
        throw new BindingError('function '+name+' is not defined');
    }
    return function() {
        if (arguments.length !== argCount - 1) {
            throw new BindingError('function ' + name + ' called with ' + arguments.length + ' arguments, expected ' + (argCount - 1));
        }
        var destructors = [];
        var args = new Array(argCount);
        args[0] = fn;
        for (var i = 1; i < argCount; ++i) {
            args[i] = argTypes[i].toWireType(destructors, arguments[i-1]);
        }
        var rv = invoker.apply(null, args);
        if (argTypes[0].fromWireTypeAutoDowncast) {
            rv = argTypes[0].fromWireTypeAutoDowncast(rv);
        } else {
            rv = argTypes[0].fromWireType(rv);
        }
        runDestructors(destructors);
        for (i = 1; i < argCount; i++) {
            refreshSmartPointee(arguments[i-1]);
        }
        return rv;
    };
}

function __embind_register_function(name, argCount, rawArgTypesAddr, rawInvoker, fn) {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = Pointer_stringify(name);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    requestDeferredRegistration(function() {
        var argTypes = requireArgumentTypes(rawArgTypes, name);
        exposePublicSymbol(name, makeInvoker(name, argCount, argTypes, rawInvoker, fn));
    });
}

function RegisteredTuple(rawConstructor, rawDestructor) {
    this.rawConstructor = rawConstructor;
    this.rawDestructor = rawDestructor;
    this.elements = [];
}

RegisteredTuple.prototype.toWireType = function(destructors, o) {
    var len = this.elements.length;
    if (len !== o.length) {
        throw new TypeError("Incorrect number of tuple elements");
    }
    var ptr = this.rawConstructor();
    for (var i = 0; i < len; ++i) {
        this.elements[i].write(ptr, o[i]);
    }
    destructors.push(this.rawDestructor);
    destructors.push(ptr);
    return ptr;
};

RegisteredTuple.prototype.fromWireType = function(ptr) {
    var len = this.elements.length;
    var rv = new Array(len);
    for (var i = 0; i < len; ++i) {
        rv[i] = this.elements[i].read(ptr);
    }
    this.rawDestructor(ptr);
    return rv;
};

function __embind_register_tuple(rawType, name, rawConstructor, rawDestructor) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    registerType(rawType, name, new RegisteredTuple(rawConstructor, rawDestructor));
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
    requestDeferredRegistration(function() {
        var tupleType = requireRegisteredType(rawTupleType, 'tuple');
        var type = requireRegisteredType(rawType, "element " + tupleType.name + "[" + tupleType.elements.length + "]");
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
    rawStaticGetter = FUNCTION_TABLE[rawStaticGetter];
    getter = copyMemberPointer(getter, getterSize);
    rawStaticSetter = FUNCTION_TABLE[rawStaticSetter];
    setter = copyMemberPointer(setter, setterSize);
    requestDeferredRegistration(function() {
        var tupleType = requireRegisteredType(rawTupleType, 'tuple');
        var elementType = requireRegisteredType(rawElementType, "element " + tupleType.name + "[" + tupleType.elements.length + "]");
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

function RegisteredStruct(rawConstructor, rawDestructor) {
    this.rawConstructor = rawConstructor;
    this.rawDestructor = rawDestructor;
    this.fields = {};
}

RegisteredStruct.prototype.toWireType = function(destructors, o) {
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
    destructors.push(this.rawDestructor);
    destructors.push(ptr);
    return ptr;
};

RegisteredStruct.prototype.fromWireType = function(ptr) {
    var fields = this.fields;
    var rv = {};
    for (var i in fields) {
        rv[i] = fields[i].read(ptr);
    }
    this.rawDestructor(ptr);
    return rv;
};

function __embind_register_struct(
    rawType,
    name,
    rawConstructor,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    registerType(rawType, name, new RegisteredStruct(rawConstructor, rawDestructor));
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
    fieldName = Pointer_stringify(fieldName);
    rawGetter = FUNCTION_TABLE[rawGetter];
    rawSetter = FUNCTION_TABLE[rawSetter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);
    requestDeferredRegistration(function() {
        var structType = requireRegisteredType(rawStructType, 'struct');
        var fieldType = requireRegisteredType(rawFieldType, 'field "' + structType.name + '.' + fieldName + '"');
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

function RegisteredPointer(Handle, isPolymorphic, isSmartPointer, rawGetPointee, rawConstructor, rawDestructor) {
    this.Handle = Handle;
    this.isPolymorphic = isPolymorphic;
    this.isSmartPointer = isSmartPointer;
    this.rawGetPointee = rawGetPointee;
    this.rawConstructor = rawConstructor;
    this.rawDestructor = rawDestructor;
}

RegisteredPointer.prototype.toWireType = function(destructors, handle) {
    var fromRawType;
    if (!handle) {
        return null;
    }
    if (handle.type.isPolymorphic) {
        fromRawType = handle.type.getDynamicRawPointerType(handle.ptr);
    } else {
        fromRawType = handle.type.rawType;
    }
    if (fromRawType === this.type.rawType) {
        return this.isSmartPointer ? handle.smartPointer : handle.ptr;
    }
    var ptr = staticPointerCast(handle.ptr, fromRawType, this.type.rawType);
    if (this.isSmartPointer) {
        // todo: if ptr == handle.ptr, there's no need to allocate a new smartPtr!
        var smartPtr = _malloc(16); // todo: can we get C++ to tell us the size of the pointer?
        handle.type.smartPointerType.rawConstructor(smartPtr, ptr, handle.smartPointer);
        ptr = smartPtr;
        destructors.push(handle.type.smartPointerType.rawDestructor);
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
    if (this.isPolymorphic) {
        if (this.rawGetPointee) { // todo: did you mean isSmartPtr?
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
    if (type && type !== this.type.rawType) {
        var derivation = Module.__getDerivationPath(type, this.type.rawType);
        for (var i = 0; i < derivation.size(); i++) {
            downcastType = typeRegistry[derivation.at(i)];
            if (downcastType) {
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
        var fromType = this.type;
        if (this.isSmartPointer) {
            handle = toType.smartPointerType.fromWireType(ptr);
        } else {
            handle = toType.fromWireType(ptr);
        }
        handle.ptr = staticPointerCast(handle.ptr, fromType.rawType, toType.rawType);
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
    var pointeeType = requireRegisteredType(rawPointeeType, 'class');
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    rawGetPointee = FUNCTION_TABLE[rawGetPointee];

    if (name == "") {
        name = pointeeType.name + "Ptr";
    }

    var Handle = createNamedFunction(name, function(ptr) {
        this.count = {value: 1};
        this.smartPointer = ptr; // std::shared_ptr<T>*
        this.ptr = rawGetPointee(ptr); // T*
        this.type = pointeeType;
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
            rawDestructor(this.smartPointer);
        }
        this.smartPointer = undefined;
        this.ptr = undefined;
    };
    var registeredPointer = new RegisteredPointer(Handle, pointeeType.isPolymorphic, true, rawGetPointee, rawConstructor, rawDestructor);
    registeredPointer.type = pointeeType;
    pointeeType.smartPointerType = registerType(rawType, name, registeredPointer);
}

function RegisteredRawPointer(isPolymorphic, classType, Handle) {
    this.isPolymorphic = isPolymorphic;
    this.pointeeType = classType;
    this.Handle = Handle;
}

RegisteredRawPointer.prototype.toWireType = function(destructors, o) {
    return o.ptr;
};

RegisteredRawPointer.prototype.fromWireType = function(ptr) {
    return new this.Handle(ptr);
};

RegisteredRawPointer.prototype.fromWireTypeAutoDowncast = function(ptr) {
    if (this.isPolymorphic) {
        var toRawType = ___getDynamicPointerType(ptr);
        if (toRawType === null || toRawType === this.pointeeType.rawType) {
            return new this.Handle(ptr);
        }
        var derivation = Module.__getDerivationPath(toRawType, this.pointeeType.rawType);
        var candidateType = null;
        for (var i = 0; i < derivation.size(); i++) {
            candidateType = typeRegistry[derivation.at(i)];
            if (candidateType) {
                break;
            }
        }
        derivation.delete();
        if (candidateType === null) {
            return new this.Handle(ptr);
        }
        var handle = candidateType.fromWireType(ptr);
        handle.ptr = ___staticPointerCast(handle.ptr, this.pointeeType.rawType, candidateType.rawType);
        // todo: can come back -1 or -2!! Throw appropriate exception
        return handle;
    } else {
        return new this.Handle(ptr);
    }
};

function RegisteredClassInstance(constructor, Handle) {
    this.constructor = constructor;
    this.Handle = Handle;
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
            Object.defineProperty(arr, 'delete', {
                writable: false,
                enumerable: false,
                configurable: false,
                value: function() {
                    var needsToBeDeleted = elementType.hasOwnProperty('Handle');
                    for (var i = 0; i < arr.length; i++) {
                        if (needsToBeDeleted) {
                            arr[i].delete();
                        }
                    }
                }
            });

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
            runDestructors(destructors);

            destructors.push(destructor);
            destructors.push(vec);
            return vec;
        }
    });
}

RegisteredClassInstance.prototype.toWireType = function(destructors, o) {
    return o.ptr;
};

RegisteredClassInstance.prototype.fromWireType = function(ptr) {
    return new this.Handle(ptr);
};

function RegisteredRawConstPointer() {
}

RegisteredRawConstPointer.prototype.toWireType = function(destructors, o) {
    return o.ptr;
};

// TODO: null pointers are always zero (not a Handle) in Javascript
function __embind_register_class(
    rawType,
    rawPointerType,
    rawConstPointerType,
    isPolymorphic,
    name,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    var Handle = createNamedFunction(name, function(ptr) {
        var h = function() {
            if(h.operator_call !== undefined) {
                return h.operator_call.apply(h, arguments);
            } else {
                throw new BindingError(name + ' does not define call operator');
            }
        };
        
        h.count = {value: 1, ptr: ptr };
        h.ptr = ptr;
        h.type = type; // set below

        for(var prop in Handle.prototype) {
            if (Handle.prototype.hasOwnProperty(prop)) {
                var dp = Object.getOwnPropertyDescriptor(Handle.prototype, prop);
                Object.defineProperty(h, prop, dp);
            }
        }

        return h;
    });

    Handle.prototype.clone = function() {
        if (!this.ptr) {
            throw new BindingError(type.name + ' instance already deleted');
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

    // todo: test delete with upcast and downcast multiply derived pointers
    // todo: then replace this.count.ptr below with this.ptr and make sure it fails
    Handle.prototype['delete'] = function() {
        if (!this.ptr) {
            throw new BindingError(type.name + ' instance already deleted'); // todo: but 'type' hasn't been resolved!?!
        }

        this.count.value -= 1;
        if (0 === this.count.value) {
            rawDestructor(this.count.ptr);
        }
        this.ptr = undefined;
    };
    Handle.memberType = {};

    // todo: clean this up!
    var registeredClass = new RegisteredPointer(Handle, isPolymorphic, false);
    var type = registerType(rawType, name, registeredClass);
    registeredClass.type = type;

    registeredClass = new RegisteredPointer(Handle, isPolymorphic, false);
    registerType(rawPointerType, name + '*', registeredClass);
    registeredClass.type = type;
    // todo: implement const pointers (no modification Javascript side)
    registeredClass = new RegisteredPointer(Handle, isPolymorphic, false);
    registerType(rawConstPointerType, name + ' const*', registeredClass);
    registeredClass.type = type;

    type.constructor = createNamedFunction(type.name, function() {
        var body = type.constructor.body;
        return body.apply(this, arguments);
    });
    type.constructor.prototype = type.Handle.prototype;
    type.constructor.type = type;

    exposePublicSymbol(name, type.constructor);
}

function __embind_register_class_constructor(
    rawClassType,
    argCount,
    rawArgTypesAddr,
    rawConstructor
) {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        var humanName = 'constructor ' + classType.name;
        var argTypes = requireArgumentTypes(rawArgTypes, humanName);
        classType.constructor.body = function() {
            if (arguments.length !== argCount - 1) {
                throw new BindingError('emscripten binding ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
            }
            var destructors = [];
            var args = new Array(argCount-1);
            for (var i = 1; i < argCount; ++i) {
                args[i-1] = argTypes[i].toWireType(destructors, arguments[i-1]);
            }

            var ptr = rawConstructor.apply(null, args);
            runDestructors(destructors);

            return classType.Handle.call(this, ptr);
        };
    });
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
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        var humanName = classType.name + '.' + methodName;
        var argTypes = requireArgumentTypes(rawArgTypes, 'method ' + humanName);
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
            var rv = rawInvoker.apply(null, args);
            if (argTypes[0].fromWireTypeAutoDowncast) {
                rv = argTypes[0].fromWireTypeAutoDowncast(rv);
            } else {
                rv = argTypes[0].fromWireType(rv);
            }
            runDestructors(destructors);
            for (i = 1; i < argCount; i++) {
                refreshSmartPointee(arguments[i-1]);
            }
            return rv;
        };
        classType.Handle.memberType[methodName] = "method";
    });
}

function __embind_register_cast_method(
    rawClassType,
    isPolymorphic,
    methodName,
    rawRawReturnType,
    rawSharedReturnType,
    rawRawCaster,
    rawSharedCaster
) {
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        methodName = Pointer_stringify(methodName);
        var humanName = classType.name + '.' + methodName;
        var rawReturnType = requireRegisteredType(rawRawReturnType, 'method ' + humanName + ' return value');
        var sharedReturnType = requireRegisteredType(rawSharedReturnType, 'method ' + humanName + ' shared pointer return value');
        var rawCaster = FUNCTION_TABLE[rawRawCaster];
        var sharedCaster = FUNCTION_TABLE[rawSharedCaster];
        classType.Handle.prototype[methodName] = function() {
            if (!this.ptr) {
                throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
            }
            if (arguments.length !== 0) {
                throw new BindingError('emscripten binding method ' + humanName + ' called with arguments, none expected');
            }
            if (isPolymorphic) {
                // todo: this is all only to validate the cast -- cache the result
                var runtimeType = ___getDynamicPointerType(this.ptr);
                var derivation = Module.__getDerivationPath(rawRawReturnType, runtimeType); // downcast is valid
                var size = derivation.size();
                derivation.delete();
                if (size === 0) {
                    derivation = Module.__getDerivationPath(runtimeType, rawRawReturnType); // upcast is valid
                    size = derivation.size();
                    derivation.delete();
                    if (size === 0) {
                        throw new CastError("Pointer conversion is not available");
                    }
                }
            }
            var args;
            var ptr;
            var rv;
            if (this.smartPointer) {
                args = new Array(2);
                ptr = _malloc(8);
                args[0] = ptr;
                args[1] = this.smartPointer;
                sharedCaster.apply(null,args); // need a smart pointer raw invoker
                rv = sharedReturnType.fromWireType(ptr);
            } else {
                args = new Array(1);
                args[0] = this.ptr;
                ptr = rawCaster.apply(null, args);
                rv = rawReturnType.fromWireType(ptr);
                rv.count = this.count;
                this.count.value ++;
            }
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
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = Pointer_stringify(methodName);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        var humanName = classType.name + '.' + methodName;
        var argTypes = requireArgumentTypes(rawArgTypes, 'classmethod ' + humanName);
        classType.constructor[methodName] = makeInvoker(humanName, argCount, argTypes, rawInvoker, fn);
    });
}

function __embind_register_class_operator_call(
    rawClassType,
    argCount,
    rawArgTypesAddr,
    rawInvoker
) {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        var humanName = classType.name + '.' + 'operator_call';
        var argTypes = requireArgumentTypes(rawArgTypes, 'method ' + humanName);

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

            var rv = argTypes[0].fromWireType(rawInvoker.apply(null, args));
            runDestructors(destructors);
            return rv;
        };
    });
}

function __embind_register_class_operator_array_get(
    rawClassType,
    elementType,
    indexType,
    rawInvoker
) {
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        indexType = requireRegisteredType(indexType, 'array access index ' + classType.name);
        elementType = requireRegisteredType(elementType, 'array access element' + classType.name);
        var humanName = classType.name + '.' + 'operator_array_get';
        classType.Handle.prototype.array_get = function() {
            if (!this.ptr) {
                throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
            }

            if (arguments.length !== 1) {
                throw new BindingError('emscripten binding method ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + 1);
            }

            var destructors = [];
            var args = new Array(2);
            args[0] = this.ptr;
            args[1] = indexType.toWireType(destructors, arguments[0]);

            var rv = elementType.fromWireType(rawInvoker.apply(null, args));
            runDestructors(destructors);
            return rv;
        };
    });
}

function __embind_register_class_operator_array_set(
    rawClassType,
    elementType,
    rawIndexType,
    rawInvoker
) {
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        var indexType = requireRegisteredType(rawIndexType, 'array access index ' + classType.name);
        elementType = requireRegisteredType(elementType, 'array access element ' + classType.name);
        var humanName = classType.name + '.' + 'operator_array_get';
        classType.Handle.prototype.array_set = function() {
            if (!this.ptr) {
                throw new BindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
            }

            if (arguments.length !== 2) {
                throw new BindingError('emscripten binding method ' + humanName + ' called with ' + arguments.length + ' arguments, expected ' + 2);
            }

            var destructors = [];
            var args = new Array(2);
            args[0] = this.ptr;
            args[1] = indexType.toWireType(destructors, arguments[0]);
            args[2] = elementType.toWireType(destructors, arguments[1]);

            var rv = elementType.fromWireType(rawInvoker.apply(null, args));
            runDestructors(destructors);
            return rv;
        };
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
    fieldName = Pointer_stringify(fieldName);
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];
    memberPointer = copyMemberPointer(memberPointer, memberPointerSize);
    requestDeferredRegistration(function() {
        var classType = requireRegisteredType(rawClassType, 'class');
        var humanName = classType.name + '.' + fieldName;
        var fieldType = requireRegisteredType(rawFieldType, 'field ' + humanName);
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
        classType.Handle.memberType[fieldName] = "field";
    });
}

function RegisteredEnum() {
    this.constructor = function() {};
    this.constructor.values = {};
}

RegisteredEnum.prototype.toWireType = function(destructors, c) {
    return c.value;
};

RegisteredEnum.prototype.fromWireType = function(c) {
    return this.constructor.values[c];
};

function __embind_register_enum(
    rawType,
    name
) {
    name = Pointer_stringify(name);
    var newEnum = new RegisteredEnum();
    registerType(rawType, name, newEnum);
    exposePublicSymbol(name, newEnum.constructor);
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

function RegisteredInterface(rawConstructor, rawDestructor) {
    this.rawConstructor = rawConstructor;
    this.rawDestructor = rawDestructor;
}

RegisteredInterface.prototype.toWireType = function(destructors, o) {
    var handle = __emval_register(o);
    var ptr = this.rawConstructor(handle);
    destructors.push(this.rawDestructor);
    destructors.push(ptr);
    return ptr;
};

function __embind_register_interface(
    rawType,
    name,
    rawConstructor,
    rawDestructor
) {
    name = Pointer_stringify(name);
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    registerType(rawType, name, new RegisteredInterface(rawConstructor, rawDestructor));
}

