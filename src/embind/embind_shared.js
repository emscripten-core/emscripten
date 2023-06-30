// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
var LibraryEmbindShared = {
  $InternalError__postset: "InternalError = Module['InternalError'] = class InternalError extends Error { constructor(message) { super(message); this.name = 'InternalError'; }}",
  $InternalError:  undefined,
  $BindingError__postset: "BindingError = Module['BindingError'] = class BindingError extends Error { constructor(message) { super(message); this.name = 'BindingError'; }}",
  $BindingError: undefined,

  $throwInternalError__deps: ['$InternalError'],
  $throwInternalError: function(message) {
    throw new InternalError(message);
  },

  $throwBindingError__deps: ['$BindingError'],
  $throwBindingError: function(message) {
    throw new BindingError(message);
  },

  // typeID -> { toWireType: ..., fromWireType: ... }
  $registeredTypes:  {},

  // typeID -> [callback]
  $awaitingDependencies: {},

  // typeID -> [dependentTypes]
  $typeDependencies: {},

  $tupleRegistrations: {},

  $structRegistrations: {},

  $sharedRegisterType__deps: [
    '$awaitingDependencies', '$registeredTypes',
    '$typeDependencies', '$throwBindingError',
    '$whenDependentTypesAreResolved'],
  $sharedRegisterType__docs: '/** @param {Object=} options */',
  $sharedRegisterType: function(rawType, registeredInstance, options = {}) {
    var name = registeredInstance.name;
    if (!rawType) {
      throwBindingError(`type "${name}" must have a positive integer typeid pointer`);
    }
    if (registeredTypes.hasOwnProperty(rawType)) {
      if (options.ignoreDuplicateRegistrations) {
        return;
      } else {
        throwBindingError(`Cannot register type '${name}' twice`);
      }
    }

    registeredTypes[rawType] = registeredInstance;
    delete typeDependencies[rawType];

    if (awaitingDependencies.hasOwnProperty(rawType)) {
      var callbacks = awaitingDependencies[rawType];
      delete awaitingDependencies[rawType];
      callbacks.forEach((cb) => cb());
    }
  },

  $whenDependentTypesAreResolved__deps: [
    '$awaitingDependencies', '$registeredTypes',
    '$typeDependencies', '$throwInternalError'],
  $whenDependentTypesAreResolved: function(myTypes, dependentTypes, getTypeConverters) {
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
    dependentTypes.forEach((dt, i) => {
      if (registeredTypes.hasOwnProperty(dt)) {
        typeConverters[i] = registeredTypes[dt];
      } else {
        unregisteredTypes.push(dt);
        if (!awaitingDependencies.hasOwnProperty(dt)) {
          awaitingDependencies[dt] = [];
        }
        awaitingDependencies[dt].push(() => {
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
  },

  $embind_charCodes__deps: ['$embind_init_charCodes'],
  $embind_charCodes__postset: "embind_init_charCodes()",
  $embind_charCodes: undefined,
  $embind_init_charCodes: function() {
    var codes = new Array(256);
    for (var i = 0; i < 256; ++i) {
        codes[i] = String.fromCharCode(i);
    }
    embind_charCodes = codes;
  },
  $readLatin1String__deps: ['$embind_charCodes'],
  $readLatin1String: function(ptr) {
    var ret = "";
    var c = ptr;
    while (HEAPU8[c]) {
        ret += embind_charCodes[HEAPU8[c++]];
    }
    return ret;
  },
  $getTypeName__deps: ['$readLatin1String', '__getTypeName', 'free'],
  $getTypeName: function(type) {
    var ptr = ___getTypeName(type);
    var rv = readLatin1String(ptr);
    _free(ptr);
    return rv;
  },

  $heap32VectorToArray: function(count, firstElement) {
    var array = [];
    for (var i = 0; i < count; i++) {
        // TODO(https://github.com/emscripten-core/emscripten/issues/17310):
        // Find a way to hoist the `>> 2` or `>> 3` out of this loop.
        array.push({{{ makeGetValue('firstElement', `i * ${POINTER_SIZE}`, '*') }}});
    }
    return array;
  },

  $requireRegisteredType__deps: [
    '$registeredTypes', '$getTypeName', '$throwBindingError'],
  $requireRegisteredType: function(rawType, humanName) {
    var impl = registeredTypes[rawType];
    if (undefined === impl) {
        throwBindingError(humanName + " has unknown type " + getTypeName(rawType));
    }
    return impl;
  }
};

mergeInto(LibraryManager.library, LibraryEmbindShared);
