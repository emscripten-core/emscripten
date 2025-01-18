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
  $throwInternalError: (message) => { throw new InternalError(message); },

  $throwBindingError__deps: ['$BindingError'],
  $throwBindingError: (message) => { throw new BindingError(message); },

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
  $embind_init_charCodes: () => {
    var codes = new Array(256);
    for (var i = 0; i < 256; ++i) {
        codes[i] = String.fromCharCode(i);
    }
    embind_charCodes = codes;
  },
  $readLatin1String__deps: ['$embind_charCodes'],
  $readLatin1String: (ptr) => {
    var ret = "";
    var c = ptr;
    while (HEAPU8[c]) {
        ret += embind_charCodes[HEAPU8[c++]];
    }
    return ret;
  },
  $getTypeName__deps: ['$readLatin1String', '__getTypeName', 'free'],
  $getTypeName: (type) => {
    var ptr = ___getTypeName(type);
    var rv = readLatin1String(ptr);
    _free(ptr);
    return rv;
  },
  $getFunctionName__deps: [],
  $getFunctionName: (signature) => {
    signature = signature.trim();
    const argsIndex = signature.indexOf("(");
    if (argsIndex !== -1) {
#if ASSERTIONS
      assert(signature[signature.length - 1] == ")", "Parentheses for argument names should match.");
#endif
      return signature.substr(0, argsIndex);
    } else {
      return signature;
    }
  },
  $getFunctionArgsName__deps: [],
  $getFunctionArgsName: (signature) => {
    signature = signature.trim();
    const argsIndex = signature.indexOf("(") + 1;
    if (argsIndex !== 0) {
#if ASSERTIONS
      assert(signature[signature.length - 1] == ")", "Parentheses for argument names should match.");
#endif
      return signature.substr(argsIndex, signature.length - argsIndex - 1).replaceAll(" ", "").split(",").filter(n => n.length);
    } else {
      return [];
    }
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

  $requireRegisteredType__deps: [
    '$registeredTypes', '$getTypeName', '$throwBindingError'],
  $requireRegisteredType: (rawType, humanName) => {
    var impl = registeredTypes[rawType];
    if (undefined === impl) {
      throwBindingError(`${humanName} has unknown type ${getTypeName(rawType)}`);
    }
    return impl;
  },

  $usesDestructorStack(argTypes) {
    // Skip return value at index 0 - it's not deleted here.
    for (var i = 1; i < argTypes.length; ++i) {
      // The type does not define a destructor function - must use dynamic stack
      if (argTypes[i] !== null && argTypes[i].destructorFunction === undefined) {
        return true;
      }
    }
    return false;
  },

  // Many of the JS invoker functions are generic and can be reused for multiple
  // function bindings. This function needs to match createJsInvoker and create
  // a unique signature for any inputs that will create different invoker
  // function outputs.
  $createJsInvokerSignature(argTypes, isClassMethodFunc, returns, isAsync) {
    const signature = [
      isClassMethodFunc ? 't' : 'f',
      returns ? 't' : 'f',
      isAsync ? 't' : 'f'
    ];
    for (let i = isClassMethodFunc ? 1 : 2; i < argTypes.length; ++i) {
      const arg = argTypes[i];
      let destructorSig = '';
      if (arg.destructorFunction === undefined) {
        destructorSig = 'u';
      } else if (arg.destructorFunction === null) {
        destructorSig = 'n';
      } else {
        destructorSig = 't';
      }
      signature.push(destructorSig);
    }
    return signature.join('');
  },

  $checkArgCount(numArgs, minArgs, maxArgs, humanName, throwBindingError) {
    if (numArgs < minArgs || numArgs > maxArgs) {
      var argCountMessage = minArgs == maxArgs ? minArgs : `${minArgs} to ${maxArgs}`;
      throwBindingError(`function ${humanName} called with ${numArgs} arguments, expected ${argCountMessage}`);
    }
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

  $createJsInvoker__deps: ['$usesDestructorStack',
#if ASSERTIONS
    '$checkArgCount',
#endif
  ],
  $createJsInvoker(argTypes, isClassMethodFunc, returns, isAsync) {
    var needsDestructorStack = usesDestructorStack(argTypes);
    var argCount = argTypes.length - 2;
    var argsList = [];
    var argsListWired = ['fn'];
    if (isClassMethodFunc) {
      argsListWired.push('thisWired');
    }
    for (var i = 0; i < argCount; ++i) {
      argsList.push(`arg${i}`)
      argsListWired.push(`arg${i}Wired`)
    }
    argsList = argsList.join(',')
    argsListWired = argsListWired.join(',')

    var invokerFnBody = `return function (${argsList}) {\n`;

#if ASSERTIONS
    invokerFnBody += "checkArgCount(arguments.length, minArgs, maxArgs, humanName, throwBindingError);\n";
#endif

#if EMSCRIPTEN_TRACING
    invokerFnBody += `Module.emscripten_trace_enter_context('embind::' + humanName );\n`;
#endif

    if (needsDestructorStack) {
      invokerFnBody += "var destructors = [];\n";
    }

    var dtorStack = needsDestructorStack ? "destructors" : "null";
    var args1 = ["humanName", "throwBindingError", "invoker", "fn", "runDestructors", "retType", "classParam"];

#if EMSCRIPTEN_TRACING
    args1.push("Module");
#endif

    if (isClassMethodFunc) {
      invokerFnBody += `var thisWired = classParam['toWireType'](${dtorStack}, this);\n`;
    }

    for (var i = 0; i < argCount; ++i) {
      invokerFnBody += `var arg${i}Wired = argType${i}['toWireType'](${dtorStack}, arg${i});\n`;
      args1.push(`argType${i}`);
    }

    invokerFnBody += (returns || isAsync ? "var rv = ":"") + `invoker(${argsListWired});\n`;

    var returnVal = returns ? "rv" : "";
#if ASYNCIFY == 1
    args1.push("Asyncify");
#endif
#if ASYNCIFY
    invokerFnBody += `function onDone(${returnVal}) {\n`;
#endif

    if (needsDestructorStack) {
      invokerFnBody += "runDestructors(destructors);\n";
    } else {
      for (var i = isClassMethodFunc?1:2; i < argTypes.length; ++i) { // Skip return value at index 0 - it's not deleted here. Also skip class type if not a method.
        var paramName = (i === 1 ? "thisWired" : ("arg"+(i - 2)+"Wired"));
        if (argTypes[i].destructorFunction !== null) {
          invokerFnBody += `${paramName}_dtor(${paramName});\n`;
          args1.push(`${paramName}_dtor`);
        }
      }
    }

    if (returns) {
      invokerFnBody += "var ret = retType['fromWireType'](rv);\n" +
#if EMSCRIPTEN_TRACING
                       "Module.emscripten_trace_exit_context();\n" +
#endif
                       "return ret;\n";
    } else {
#if EMSCRIPTEN_TRACING
      invokerFnBody += "Module.emscripten_trace_exit_context();\n";
#endif
    }

#if ASYNCIFY == 1
    invokerFnBody += "}\n";
    invokerFnBody += `return Asyncify.currData ? Asyncify.whenDone().then(onDone) : onDone(${returnVal});\n`
#elif ASYNCIFY == 2
    invokerFnBody += "}\n";
    invokerFnBody += "return " + (isAsync ? "rv.then(onDone)" : `onDone(${returnVal})`) + ";";
#endif

    invokerFnBody += "}\n";

#if ASSERTIONS
    args1.push('checkArgCount', 'minArgs', 'maxArgs');
    invokerFnBody = `if (arguments.length !== ${args1.length}){ throw new Error(humanName + "Expected ${args1.length} closure arguments " + arguments.length + " given."); }\n${invokerFnBody}`;
#endif
    return [args1, invokerFnBody];
  }
};

addToLibrary(LibraryEmbindShared);
