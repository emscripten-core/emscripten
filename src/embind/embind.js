/*global Module*/
/*global _malloc, _free, _memcpy*/
/*global FUNCTION_TABLE, HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32*/
/*global readLatin1String*/
/*global __emval_register, _emval_handle_array, __emval_decref*/
/*global ___getTypeName*/

var InternalError = Module.InternalError = extendError(Error, 'InternalError');
var BindingError = Module.BindingError = extendError(Error, 'BindingError');
var UnboundTypeError = Module.UnboundTypeError = extendError(BindingError, 'UnboundTypeError');

function throwInternalError(message) {
    throw new InternalError(message);
}

function throwBindingError(message) {
    throw new BindingError(message);
}

function throwUnboundTypeError(message, types) {
    var unboundTypes = [];
    var seen = {};
    function visit(type) {
        if (seen[type]) {
            return;
        }
        if (registeredTypes[type]) {
            return;
        }
        if (typeDependencies[type]) {
            typeDependencies[type].forEach(visit);
            return;
        }
        unboundTypes.push(type);
        seen[type] = true;
    }
    types.forEach(visit);
    
    throw new UnboundTypeError(message + ': ' + unboundTypes.map(getTypeName).join([', ']));
}

// Creates a function overload resolution table to the given method 'methodName' in the given prototype,
// if the overload table doesn't yet exist.
function ensureOverloadTable(proto, methodName, humanName) {
    if (undefined === proto[methodName].overloadTable) {
        var prevFunc = proto[methodName];
        // Inject an overload resolver function that routes to the appropriate overload based on the number of arguments.
        proto[methodName] = function() {
            // TODO This check can be removed in -O3 level "unsafe" optimizations.
            if (!proto[methodName].overloadTable.hasOwnProperty(arguments.length)) {
                throwBindingError("Function '" + humanName + "' called with an invalid number of arguments (" + arguments.length + ") - expects one of (" + proto[methodName].overloadTable + ")!");
            }
            return proto[methodName].overloadTable[arguments.length].apply(this, arguments);
        };
        // Move the previous function into the overload table.
        proto[methodName].overloadTable = [];
        proto[methodName].overloadTable[prevFunc.argCount] = prevFunc;
    }            
}

/* Registers a symbol (function, class, enum, ...) as part of the Module JS object so that
   hand-written code is able to access that symbol via 'Module.name'.
   name: The name of the symbol that's being exposed.
   value: The object itself to expose (function, class, ...)
   numArguments: For functions, specifies the number of arguments the function takes in. For other types, unused and undefined.

   To implement support for multiple overloads of a function, an 'overload selector' function is used. That selector function chooses
   the appropriate overload to call from an function overload table. This selector function is only used if multiple overloads are
   actually registered, since it carries a slight performance penalty. */
function exposePublicSymbol(name, value, numArguments) {
    if (Module.hasOwnProperty(name)) {
        if (undefined === numArguments || (undefined !== Module[name].overloadTable && undefined !== Module[name].overloadTable[numArguments])) {
            throwBindingError("Cannot register public name '" + name + "' twice");
        }
        
        // We are exposing a function with the same name as an existing function. Create an overload table and a function selector
        // that routes between the two.
        ensureOverloadTable(Module, name, name);
        if (Module.hasOwnProperty(numArguments)) {
            throwBindingError("Cannot register multiple overloads of a function with the same number of arguments (" + numArguments + ")!");
        }
        // Add the new function into the overload table.
        Module[name].overloadTable[numArguments] = value;
    }
    else {
        Module[name] = value;
        if (undefined !== numArguments) {
            Module[name].numArguments = numArguments;
        }
    }
}

function replacePublicSymbol(name, value, numArguments) {
    if (!Module.hasOwnProperty(name)) {
        throwInternalError('Replacing nonexistant public symbol');
    }
    // If there's an overload table for this symbol, replace the symbol in the overload table instead.
    if (undefined !== Module[name].overloadTable && undefined !== numArguments) {
        Module[name].overloadTable[numArguments] = value;
    }
    else {
        Module[name] = value;
    }
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
        "    \"use strict\";" +
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

// typeID -> { toWireType: ..., fromWireType: ... }
var registeredTypes = {};

// typeID -> [callback]
var awaitingDependencies = {};

// typeID -> [dependentTypes]
var typeDependencies = {};

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
    delete typeDependencies[rawType];

    if (awaitingDependencies.hasOwnProperty(rawType)) {
        var callbacks = awaitingDependencies[rawType];
        delete awaitingDependencies[rawType];
        callbacks.forEach(function(cb) {
            cb();
        });
    }
}

function whenDependentTypesAreResolved(myTypes, dependentTypes, getTypeConverters) {
    myTypes.forEach(function(type) {
        typeDependencies[type] = dependentTypes;
    });

    function onComplete(typeConverters) {
        var myTypeConverters = getTypeConverters(typeConverters);
        if (myTypeConverters.length !== myTypes.length) {
            throwInternalError('Mismatched type converter count');
        }
        for (var i = 0; i < myTypes.length; ++i) {
            registerType(myTypes[i], myTypeConverters[i]);
        }
    }

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

var __charCodes = [];

function readLatin1String(ptr) {
  if (__charCodes.length === 0) {
    for (var charCodeI = 0; charCodeI < 127; charCodeI++) {
      __charCodes.push(String.fromCharCode(charCodeI));
    }
  }
  
  var ret = "";
  var c = ptr;
  while (HEAPU8[c]) {
    ret += __charCodes[HEAPU8[c++]];
  }
  return ret;
}

function getTypeName(type) {
    var ptr = ___getTypeName(type);
    var rv = readLatin1String(ptr);
    _free(ptr);
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
        throwBindingError(humanName + " has unknown type " + getTypeName(rawType));
    }
    return impl;
}

function __embind_register_void(rawType, name) {
    name = readLatin1String(name);
    registerType(rawType, {
        name: name,
        fromWireType: function() {
            return undefined;
        },
    });
}

function __embind_register_bool(rawType, name, trueValue, falseValue) {
    name = readLatin1String(name);
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

// When converting a number from JS to C++ side, the valid range of the number is
// [minRange, maxRange], inclusive.
function __embind_register_integer(primitiveType, name, minRange, maxRange) {
    name = readLatin1String(name);
    if (maxRange === -1) { // LLVM doesn't have signed and unsigned 32-bit types, so u32 literals come out as 'i32 -1'. Always treat those as max u32.
        maxRange = 4294967295;
    }
    registerType(primitiveType, {
        name: name,
        minRange: minRange,
        maxRange: maxRange,
        fromWireType: function(value) {
            return value;
        },
        toWireType: function(destructors, value) {
            // todo: Here we have an opportunity for -O3 level "unsafe" optimizations: we could
            // avoid the following two if()s and assume value is of proper type.
            if (typeof value !== "number") {
                throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + this.name);
            }
            if (value < minRange || value > maxRange) {
                throw new TypeError('Passing a number "' + _embind_repr(value) + '" from JS side to C/C++ side to an argument of type "' + name + '", which is outside the valid range [' + minRange + ', ' + maxRange + ']!');
            }
            return value | 0;
        },
    });
}

function __embind_register_float(rawType, name) {
    name = readLatin1String(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(value) {
            return value;
        },
        toWireType: function(destructors, value) {
            // todo: Here we have an opportunity for -O3 level "unsafe" optimizations: we could
            // avoid the following if() and assume value is of proper type.
            if (typeof value !== "number") {
                throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' +this.name);
            }
            return value;
        },
    });
}

function __embind_register_std_string(rawType, name) {
    name = readLatin1String(name);
    registerType(rawType, {
        name: name,
        fromWireType: function(value) {
            var length = HEAPU32[value >> 2];
            var a = new Array(length);
            for (var i = 0; i < length; ++i) {
                a[i] = String.fromCharCode(HEAPU8[value + 4 + i]);
            }
            _free(value);
            return a.join('');
        },
        toWireType: function(destructors, value) {
            if (value instanceof ArrayBuffer) {
                value = new Uint8Array(value);
            }

            function getTAElement(ta, index) {
                return ta[index];
            }
            function getStringElement(string, index) {
                return string.charCodeAt(index);
            }
            var getElement;
            if (value instanceof Uint8Array) {
                getElement = getTAElement;
            } else if (value instanceof Int8Array) {
                getElement = getTAElement;
            } else if (typeof value === 'string') {
                getElement = getStringElement;
            } else {
                throwBindingError('Cannot pass non-string to std::string');
            }

            // assumes 4-byte alignment
            var length = value.length;
            var ptr = _malloc(4 + length);
            HEAPU32[ptr >> 2] = length;
            for (var i = 0; i < length; ++i) {
                var charCode = getElement(value, i);
                if (charCode > 255) {
                    _free(ptr);
                    throwBindingError('String has UTF-16 code units that do not fit in 8 bits');
                }
                HEAPU8[ptr + 4 + i] = charCode;
            }
            destructors.push(_free, ptr);
            return ptr;
        },
    });
}

function __embind_register_std_wstring(rawType, charSize, name) {
    name = readLatin1String(name);
    var HEAP, shift;
    if (charSize === 2) {
        HEAP = HEAPU16;
        shift = 1;
    } else if (charSize === 4) {
        HEAP = HEAPU32;
        shift = 2;
    }
    registerType(rawType, {
        name: name,
        fromWireType: function(value) {
            var length = HEAPU32[value >> 2];
            var a = new Array(length);
            var start = (value + 4) >> shift;
            for (var i = 0; i < length; ++i) {
                a[i] = String.fromCharCode(HEAP[start + i]);
            }
            _free(value);
            return a.join('');
        },
        toWireType: function(destructors, value) {
            // assumes 4-byte alignment
            var length = value.length;
            var ptr = _malloc(4 + length * charSize);
            HEAPU32[ptr >> 2] = length;
            var start = (ptr + 4) >> shift;
            for (var i = 0; i < length; ++i) {
                HEAP[start + i] = value.charCodeAt(i);
            }
            destructors.push(_free, ptr);
            return ptr;
        },
    });
}

function __embind_register_emval(rawType, name) {
    name = readLatin1String(name);
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
    return createNamedFunction(makeLegalFunctionName(name), function() {
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
    });
}

function __embind_register_function(name, argCount, rawArgTypesAddr, rawInvoker, fn) {
    var argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = readLatin1String(name);
    rawInvoker = FUNCTION_TABLE[rawInvoker];

    exposePublicSymbol(name, function() {
        throwUnboundTypeError('Cannot call ' + name + ' due to unbound types', argTypes);
    }, argCount - 1);

    whenDependentTypesAreResolved([], argTypes, function(argTypes) {
        replacePublicSymbol(name, makeInvoker(name, argCount, argTypes, rawInvoker, fn), argCount - 1);
        return [];
    });
}

var tupleRegistrations = {};

function __embind_register_tuple(rawType, name, rawConstructor, rawDestructor) {
    tupleRegistrations[rawType] = {
        name: readLatin1String(name),
        rawConstructor: FUNCTION_TABLE[rawConstructor],
        rawDestructor: FUNCTION_TABLE[rawDestructor],
        elements: [],
    };
}

function __embind_register_tuple_element(
    rawTupleType,
    getterReturnType,
    getter,
    getterContext,
    setterArgumentType,
    setter,
    setterContext
) {
    tupleRegistrations[rawTupleType].elements.push({
        getterReturnType: getterReturnType,
        getter: FUNCTION_TABLE[getter],
        getterContext: getterContext,
        setterArgumentType: setterArgumentType,
        setter: FUNCTION_TABLE[setter],
        setterContext: setterContext,
    });
}

function __embind_finalize_tuple(rawTupleType) {
    var reg = tupleRegistrations[rawTupleType];
    delete tupleRegistrations[rawTupleType];
    var elements = reg.elements;
    var elementsLength = elements.length;
    var elementTypes = elements.map(function(elt) { return elt.getterReturnType; }).
                concat(elements.map(function(elt) { return elt.setterArgumentType; }));

    var rawConstructor = reg.rawConstructor;
    var rawDestructor = reg.rawDestructor;
 
    whenDependentTypesAreResolved([rawTupleType], elementTypes, function(elementTypes) {
        elements.forEach(function(elt, i) {
            var getterReturnType = elementTypes[i];
            var getter = elt.getter;
            var getterContext = elt.getterContext;
            var setterArgumentType = elementTypes[i + elementsLength];
            var setter = elt.setter;
            var setterContext = elt.setterContext;
            elt.read = function(ptr) {
                return getterReturnType.fromWireType(getter(getterContext, ptr));
            };
            elt.write = function(ptr, o) {
                var destructors = [];
                setter(setterContext, ptr, setterArgumentType.toWireType(destructors, o));
                runDestructors(destructors);
            };
        });

        return [{
            name: reg.name,
            fromWireType: function(ptr) {
                var rv = new Array(elementsLength);
                for (var i = 0; i < elementsLength; ++i) {
                    rv[i] = elements[i].read(ptr);
                }
                rawDestructor(ptr);
                return rv;
            },
            toWireType: function(destructors, o) {
                if (elementsLength !== o.length) {
                    throw new TypeError("Incorrect number of tuple elements");
                }
                var ptr = rawConstructor();
                for (var i = 0; i < elementsLength; ++i) {
                    elements[i].write(ptr, o[i]);
                }
                destructors.push(rawDestructor, ptr);
                return ptr;
            },
        }];
    });
}

var structRegistrations = {};

function __embind_register_struct(
    rawType,
    name,
    rawConstructor,
    rawDestructor
) {
    structRegistrations[rawType] = {
        name: readLatin1String(name),
        rawConstructor: FUNCTION_TABLE[rawConstructor],
        rawDestructor: FUNCTION_TABLE[rawDestructor],
        fields: [],
    };
}

function __embind_register_struct_field(
    structType,
    fieldName,
    getterReturnType,
    getter,
    getterContext,
    setterArgumentType,
    setter,
    setterContext
) {
    structRegistrations[structType].fields.push({
        fieldName: readLatin1String(fieldName),
        getterReturnType: getterReturnType,
        getter: FUNCTION_TABLE[getter],
        getterContext: getterContext,
        setterArgumentType: setterArgumentType,
        setter: FUNCTION_TABLE[setter],
        setterContext: setterContext,
    });
}

function __embind_finalize_struct(structType) {
    var reg = structRegistrations[structType];
    delete structRegistrations[structType];

    var rawConstructor = reg.rawConstructor;
    var rawDestructor = reg.rawDestructor;
    var fieldRecords = reg.fields;
    var fieldTypes = fieldRecords.map(function(field) { return field.getterReturnType; }).
              concat(fieldRecords.map(function(field) { return field.setterArgumentType; }));
    whenDependentTypesAreResolved([structType], fieldTypes, function(fieldTypes) {
        var fields = {};
        fieldRecords.forEach(function(field, i) {
            var fieldName = field.fieldName;
            var getterReturnType = fieldTypes[i];
            var getter = field.getter;
            var getterContext = field.getterContext;
            var setterArgumentType = fieldTypes[i + fieldRecords.length];
            var setter = field.setter;
            var setterContext = field.setterContext;
            fields[fieldName] = {
                read: function(ptr) {
                    return getterReturnType.fromWireType(
                        getter(getterContext, ptr));
                },
                write: function(ptr, o) {
                    var destructors = [];
                    setter(setterContext, ptr, setterArgumentType.toWireType(destructors, o));
                    runDestructors(destructors);
                }
            };
        });

        return [{
            name: reg.name,
            fromWireType: function(ptr) {
                var rv = {};
                for (var i in fields) {
                    rv[i] = fields[i].read(ptr);
                }
                rawDestructor(ptr);
                return rv;
            },
            toWireType: function(destructors, o) {
                // todo: Here we have an opportunity for -O3 level "unsafe" optimizations:
                // assume all fields are present without checking.
                for (var fieldName in fields) {
                    if (!(fieldName in o)) {
                        throw new TypeError('Missing field');
                    }
                }
                var ptr = rawConstructor();
                for (fieldName in fields) {
                    fields[fieldName].write(ptr, o[fieldName]);
                }
                destructors.push(rawDestructor, ptr);
                return ptr;
            },
        }];
    });
}

function RegisteredPointer(
    name,
    registeredClass,
    isReference,
    isConst,

    // smart pointer properties
    isSmartPointer,
    pointeeType,
    sharingPolicy,
    rawGetPointee,
    rawConstructor,
    rawShare,
    rawDestructor
) {
    this.name = name;
    this.registeredClass = registeredClass;
    this.isReference = isReference;
    this.isConst = isConst;

    // smart pointer properties
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
        throwBindingError('Expected null or instance of ' + this.name + ', got ' + _embind_repr(handle));
    }

    if (!handle.$$.ptr) {
        throwBindingError('Cannot pass deleted object');
    }

    // TODO: this is not strictly true
    // We could support BY_EMVAL conversions from raw pointers to smart pointers
    // because the smart pointer can hold a reference to the handle
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
                ptr = handle.$$.smartPtr;
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
            return makeClassHandle(this.registeredClass.instancePrototype, {
                ptrType: this.pointeeType,
                ptr: rawPointer,
                smartPtrType: this,
                smartPtr: ptr,
            });
        } else {
            return makeClassHandle(this.registeredClass.instancePrototype, {
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
        return makeClassHandle(toType.registeredClass.instancePrototype, {
            ptrType: toType,
            ptr: dp,
            smartPtrType: this,
            smartPtr: ptr,
        });
    } else {
        return makeClassHandle(toType.registeredClass.instancePrototype, {
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
    constructor,
    instancePrototype,
    rawDestructor,
    baseClass,
    getActualType,
    upcast,
    downcast
) {
    this.name = name;
    this.constructor = constructor;
    this.instancePrototype = instancePrototype;
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
    name = readLatin1String(name);
    rawDestructor = FUNCTION_TABLE[rawDestructor];
    getActualType = FUNCTION_TABLE[getActualType];
    upcast = FUNCTION_TABLE[upcast];
    downcast = FUNCTION_TABLE[downcast];
    var legalFunctionName = makeLegalFunctionName(name);

    exposePublicSymbol(legalFunctionName, function() {
        // this code cannot run if baseClassRawType is zero
        throwUnboundTypeError('Cannot construct ' + name + ' due to unbound types', [baseClassRawType]);
    });

    whenDependentTypesAreResolved(
        [rawType, rawPointerType, rawConstPointerType],
        baseClassRawType ? [baseClassRawType] : [],
        function(base) {
            base = base[0];

            var baseClass;
            var basePrototype;
            if (baseClassRawType) {
                baseClass = base.registeredClass;
                basePrototype = baseClass.instancePrototype;
            } else {
                basePrototype = ClassHandle.prototype;
            }

            var constructor = createNamedFunction(legalFunctionName, function() {
                if (Object.getPrototypeOf(this) !== instancePrototype) {
                    throw new BindingError("Use 'new' to construct " + name);
                }
                if (undefined === registeredClass.constructor_body) {
                    throw new BindingError(name + " has no accessible constructor");
                }
                var body = registeredClass.constructor_body[arguments.length];
                if (undefined === body) {
                    throw new BindingError("Tried to invoke ctor of " + name + " with invalid number of parameters (" + arguments.length + ") - expected (" + Object.keys(registeredClass.constructor_body).toString() + ") parameters instead!");
                }
                return body.apply(this, arguments);
            });

            var instancePrototype = Object.create(basePrototype, {
                constructor: { value: constructor },
            });

            constructor.prototype = instancePrototype;

            var registeredClass = new RegisteredClass(
                name,
                constructor,
                instancePrototype,
                rawDestructor,
                baseClass,
                getActualType,
                upcast,
                downcast);

            var referenceConverter = new RegisteredPointer(
                name,
                registeredClass,
                true,
                false,
                false);
        
            var pointerConverter = new RegisteredPointer(
                name + '*',
                registeredClass,
                false,
                false,
                false);

            var constPointerConverter = new RegisteredPointer(
                name + ' const*',
                registeredClass,
                false,
                true,
                false);

            registeredPointers[rawType] = {
                pointerType: pointerConverter,
                constPointerType: constPointerConverter
            };

            replacePublicSymbol(legalFunctionName, constructor);

            return [referenceConverter, pointerConverter, constPointerConverter];
        }
    );
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

    whenDependentTypesAreResolved([], [rawClassType], function(classType) {
        classType = classType[0];
        var humanName = 'constructor ' + classType.name;

        if (undefined === classType.registeredClass.constructor_body) {
            classType.registeredClass.constructor_body = [];
        }
        if (undefined !== classType.registeredClass.constructor_body[argCount - 1]) {
            throw new BindingError("Cannot register multiple constructors with identical number of parameters (" + (argCount-1) + ") for class '" + classType.name + "'! Overload resolution is currently only performed using the parameter count, not actual type info!");
        }
        classType.registeredClass.constructor_body[argCount - 1] = function() {
            throwUnboundTypeError('Cannot construct ' + classType.name + ' due to unbound types', rawArgTypes);
        };

        whenDependentTypesAreResolved([], rawArgTypes, function(argTypes) {
            classType.registeredClass.constructor_body[argCount - 1] = function() {
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
            return [];
        });
        return [];
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
    if (!(this_ instanceof classType.registeredClass.constructor)) {
        throwBindingError(humanName + ' incompatible with "this" of type ' + this_.constructor.name);
    }
    if (!this_.$$.ptr) {
        throwBindingError('cannot call emscripten binding method ' + humanName + ' on deleted object');
    }

    // todo: kill this
    return upcastPointer(
        this_.$$.ptr,
        this_.$$.ptrType.registeredClass,
        classType.registeredClass);
}

function __embind_register_class_function(
    rawClassType,
    methodName,
    argCount,
    rawArgTypesAddr, // [ReturnType, ThisType, Args...]
    rawInvoker,
    context
) {
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = readLatin1String(methodName);
    rawInvoker = FUNCTION_TABLE[rawInvoker];

    whenDependentTypesAreResolved([], [rawClassType], function(classType) {
        classType = classType[0];
        var humanName = classType.name + '.' + methodName;

        var unboundTypesHandler = function() {
                throwUnboundTypeError('Cannot call ' + humanName + ' due to unbound types', rawArgTypes);
            };

        var proto = classType.registeredClass.instancePrototype;
        var method = proto[methodName];
        if (undefined === method || (undefined === method.overloadTable && method.className !== classType.name && method.argCount === argCount-2)) {
            // This is the first overload to be registered, OR we are replacing a function in the base class with a function in the derived class.
            unboundTypesHandler.argCount = argCount-2;
            unboundTypesHandler.className = classType.name;
            proto[methodName] = unboundTypesHandler;
        } else {
            // There was an existing function with the same name registered. Set up a function overload routing table.
            ensureOverloadTable(proto, methodName, humanName);
            proto[methodName].overloadTable[argCount-2] = unboundTypesHandler;
        }

        whenDependentTypesAreResolved([], rawArgTypes, function(argTypes) {
            var memberFunction = createNamedFunction(makeLegalFunctionName(humanName), function() {
                if (arguments.length !== argCount - 2) {
                    throwBindingError(humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-2));
                }

                var destructors = [];
                var args = new Array(argCount + 1);
                args[0] = context;
                args[1] = argTypes[1].toWireType(destructors, this);
                for (var i = 2; i < argCount; ++i) {
                    args[i] = argTypes[i].toWireType(destructors, arguments[i - 2]);
                }
                var rv = rawInvoker.apply(null, args);
                rv = argTypes[0].fromWireType(rv);
                runDestructors(destructors);
                return rv;
            });

            // Replace the initial unbound-handler-stub function with the appropriate member function, now that all types
            // are resolved. If multiple overloads are registered for this function, the function goes into an overload table.
            if (undefined === proto[methodName].overloadTable) {
                proto[methodName] = memberFunction;
            } else {
                proto[methodName].overloadTable[argCount-2] = memberFunction;
            }

            return [];
        });
        return [];
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
    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    methodName = readLatin1String(methodName);
    rawInvoker = FUNCTION_TABLE[rawInvoker];
    whenDependentTypesAreResolved([], [rawClassType], function(classType) {
        classType = classType[0];
        var humanName = classType.name + '.' + methodName;

        var unboundTypesHandler = function() {
                throwUnboundTypeError('Cannot call ' + humanName + ' due to unbound types', rawArgTypes);
            };

        var proto = classType.registeredClass.constructor;
        if (undefined === proto[methodName]) {
            // This is the first function to be registered with this name.
            unboundTypesHandler.argCount = argCount-1;
            proto[methodName] = unboundTypesHandler;
        } else {
            // There was an existing function with the same name registered. Set up a function overload routing table.
            ensureOverloadTable(proto, methodName, humanName);
            proto[methodName].overloadTable[argCount-1] = unboundTypesHandler;
        }

        whenDependentTypesAreResolved([], rawArgTypes, function(argTypes) {
            // Replace the initial unbound-types-handler stub with the proper function. If multiple overloads are registered,
            // the function handlers go into an overload table.
            var func = makeInvoker(humanName, argCount, argTypes, rawInvoker, fn);
            if (undefined === proto[methodName].overloadTable) {
                proto[methodName] = func;
            } else {
                proto[methodName].overloadTable[argCount-1] = func;
            }
            return [];
        });
        return [];
    });
}

function __embind_register_class_property(
    classType,
    fieldName,
    getterReturnType,
    getter,
    getterContext,
    setterArgumentType,
    setter,
    setterContext
) {
    fieldName = readLatin1String(fieldName);
    getter = FUNCTION_TABLE[getter];
    setter = FUNCTION_TABLE[setter];

    whenDependentTypesAreResolved([], [classType], function(classType) {
        classType = classType[0];
        var humanName = classType.name + '.' + fieldName;

        Object.defineProperty(classType.registeredClass.instancePrototype, fieldName, {
            get: function() {
                throwUnboundTypeError('Cannot access ' + humanName + ' due to unbound types', [getterReturnType, setterArgumentType]);
            },
            set: function() {
                throwUnboundTypeError('Cannot access ' + humanName + ' due to unbound types', [getterReturnType, setterArgumentType]);
            },
            enumerable: true,
            configurable: true
        });

        whenDependentTypesAreResolved([], [getterReturnType, setterArgumentType], function(types) {
            var getterReturnType = types[0];
            var setterArgumentType = types[1];

            Object.defineProperty(classType.registeredClass.instancePrototype, fieldName, {
                get: function() {
                    var ptr = validateThis(this, classType, humanName + ' getter');
                    return getterReturnType.fromWireType(getter(getterContext, ptr));
                },
                set: function(v) {
                    var ptr = validateThis(this, classType, humanName + ' setter');
                    var destructors = [];
                    setter(setterContext, ptr, setterArgumentType.toWireType(destructors, v));
                    runDestructors(destructors);
                },
                enumerable: true
            });
            return [];
        });

        return [];
    });
}

var char_0 = '0'.charCodeAt(0);
var char_9 = '9'.charCodeAt(0);
function makeLegalFunctionName(name) {
    name = name.replace(/[^a-zA-Z0-9_]/g, '$');
    var f = name.charCodeAt(0);
    if (f >= char_0 && f <= char_9) {
        return '_' + name;
    } else {
        return name;
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
    name = readLatin1String(name);
    rawGetPointee = FUNCTION_TABLE[rawGetPointee];
    rawConstructor = FUNCTION_TABLE[rawConstructor];
    rawShare = FUNCTION_TABLE[rawShare];
    rawDestructor = FUNCTION_TABLE[rawDestructor];

    whenDependentTypesAreResolved([rawType], [rawPointeeType], function(pointeeType) {
        pointeeType = pointeeType[0];

        var registeredPointer = new RegisteredPointer(
            name,
            pointeeType.registeredClass,
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
        return [registeredPointer];
    });
}

function __embind_register_enum(
    rawType,
    name
) {
    name = readLatin1String(name);

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
    name = readLatin1String(name);

    var Enum = enumType.constructor;

    var Value = Object.create(enumType.constructor.prototype, {
        value: {value: enumValue},
        constructor: {value: createNamedFunction(enumType.name + '_' + name, function() {})},
    });
    Enum.values[enumValue] = Value;
    Enum[name] = Value;
}

function __embind_register_constant(name, type, value) {
    name = readLatin1String(name);
    whenDependentTypesAreResolved([], [type], function(type) {
        type = type[0];
        Module[name] = type.fromWireType(value);
        return [];
    });
}
