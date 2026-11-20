// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
var LibraryEmbindShared = {
  $InternalError: class extends Error {
    constructor(message) {
      super(message);
      this.name = 'InternalError';
    }
  },
  $BindingError: class extends Error {
    constructor(message) {
      super(message);
      this.name = 'BindingError';
    }
  },

  $throwInternalError: (message) => { throw new InternalError(message); },

  $throwBindingError: (message) => { throw new BindingError(message); },

  // typeID -> { toWireType: ..., fromWireType: ... }
  $registeredTypes:  {},

  // typeID -> [callback]
  $awaitingDependencies: {},

  // typeID -> [dependentTypes]
  $typeDependencies: {},

  $tupleRegistrations: {},

  $structRegistrations: {},

  $sharedRegisterType__docs: '/** @param {Object=} options */',
  $sharedRegisterType: function(typeID, registeredInstance, options = {}) {
    var name = registeredInstance.name;
    if (!typeID) {
      throwBindingError(`type "${name}" must have a positive integer typeid pointer`);
    }
    if (registeredTypes[typeID]) {
      if (options.ignoreDuplicateRegistrations) {
        return;
      } else {
        throwBindingError(`Cannot register type '${name}' twice`);
      }
    }

    registeredTypes[typeID] = registeredInstance;
    delete typeDependencies[typeID];

    if (awaitingDependencies[typeID]) {
      var callbacks = awaitingDependencies[typeID];
      delete awaitingDependencies[typeID];
      callbacks.forEach((cb) => cb());
    }
  },

  $whenDependentTypesAreResolved: (myTypes, dependentTypes, getTypeConverters) => {
    myTypes.forEach((type) => typeDependencies[type] = dependentTypes);

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
    for (let [i, dt] of dependentTypes.entries()) {
      if (registeredTypes[dt]) {
        typeConverters[i] = registeredTypes[dt];
      } else {
        unregisteredTypes.push(dt);
        (awaitingDependencies[dt] ??= []).push(() => {
          typeConverters[i] = registeredTypes[dt];
          ++registered;
          if (registered === unregisteredTypes.length) {
            onComplete(typeConverters);
          }
        });
      }
    }
    if (0 === unregisteredTypes.length) {
      onComplete(typeConverters);
    }
  },

  $getTypeName__deps: ['__getTypeName', 'free'],
  $getTypeName: (typeID) => {
    var ptr = ___getTypeName(typeID);
    var rv = AsciiToString(ptr);
    _free(ptr);
    return rv;
  },
  $getFunctionName__deps: [],
  $getFunctionName: (signature) => {
    signature = signature.trim();
    const argsIndex = signature.indexOf('(');
    if (argsIndex === -1) return signature;
#if ASSERTIONS
    assert(signature.endsWith(')'), 'Parentheses for argument names should match.');
#endif
    return signature.slice(0, argsIndex);
  },
  $getFunctionArgsName__deps: [],
  $getFunctionArgsName: (signature) => {
    signature = signature.trim();
    const argsIndex = signature.indexOf('(');
    if (argsIndex == -1) return; // Return undefined to mean we don't have any argument names
#if ASSERTIONS
    assert(signature.endsWith(')'), 'Parentheses for argument names should match.');
#endif
    return signature.slice(argsIndex + 1, -1).replaceAll(' ', '').split(',').filter(n => n.length);
  },
  $heap32VectorToArray: (count, firstElement) => {
    var array = [];
    for (var i = 0; i < count; i++) {
      // TODO(https://github.com/emscripten-core/emscripten/issues/17310):
      // Find a way to hoist the `>> 2` or `>> 3` out of this loop.
      array.push({{{ makeGetValue('firstElement', `i * ${POINTER_SIZE}`, '*') }}});
    }
    return array;
  },

  $requireRegisteredType: (typeID, humanName) => {
    var impl = registeredTypes[typeID];
    if (undefined === impl) {
      throwBindingError(`${humanName} has unknown type ${getTypeName(typeID)}`);
    }
    return impl;
  },

  $usesDestructorStack(argTypes) {
    // Skip return value at index 0 - it's not deleted here.
    for (var i = 1; i < argTypes.length; ++i) {
      if (argTypes[i]?.usesDestructor) {
        return true;
      }
    }
    return false;
  },

  // Trivially constructible/destructible value types (argStackAlloc) place
  // their argument temporaries on the wasm stack when the invoker brackets
  // the call in stackSave/stackRestore, skipping the per-call heap temp and
  // destructor bookkeeping entirely.
  $argsUseStackAlloc(argTypes) {
    // Skip return value at index 0 - only arguments stack-allocate.
    for (var i = 1; i < argTypes.length; ++i) {
      if (argTypes[i] !== null && argTypes[i].argStackAlloc) {
        return true;
      }
    }
    return false;
  },

  $checkArgCount(numArgs, minArgs, maxArgs, humanName, throwBindingError) {
    if (numArgs < minArgs || numArgs > maxArgs) {
      var argCountMessage = minArgs == maxArgs ? minArgs : `${minArgs} to ${maxArgs}`;
      throwBindingError(`function ${humanName} called with ${numArgs} arguments, expected ${argCountMessage}`);
    }
  },

  $getEnumValueType(rawValueType) {
    // This must match the values of enum_value_type in wire.h
    return !rawValueType ? 'object' : (rawValueType === 1 ? 'number' : 'string');
  },

  $getRequiredArgCount(argTypes) {
    var requiredArgCount = argTypes.length - 2;
    for (var i = argTypes.length - 1; i >= 2; --i) {
      if (!argTypes[i].optional) {
        break;
      }
      requiredArgCount--;
    }
    return requiredArgCount;
  },
};

addToLibrary(LibraryEmbindShared);
