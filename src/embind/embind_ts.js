// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryEmbind = {
#include embind/embind_shared.js
  ,

  $moduleDefinitions: [],

  $PrimitiveType: class PrimitiveType {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
    }
  },
  $Argument: class Argument {
    constructor(name, type) {
      this.name = name;
      this.type = type;
    }
  },
  $FunctionDefinition: class FunctionDefinition {
    constructor(name, returnType, argumentTypes, thisType = null) {
      this.name = name;
      this.returnType = returnType;
      this.argumentTypes = argumentTypes;
      this.thisType = thisType;
    }

    printSignature(nameMap, out) {
      out.push('(');

      let argOut = [];
      for (let arg of this.argumentTypes) {
        argOut.push(`${arg.name}: ${nameMap(arg.type)}`);
      }
      out.push(argOut.join(', '));
      out.push(`): ${nameMap(this.returnType)}`);
    }

    printFunction(nameMap, out) {
      out.push(`${this.name}`);
      this.printSignature(nameMap, out);
      out.push(';\n');
    }

    printModuleEntry(nameMap, out) {
      out.push("  ");
      this.printFunction(nameMap, out);
    }
  },
  $ClassDefinition: class ClassDefinition {
    constructor(typeId, name, base = null) {
      this.typeId = typeId;
      this.name = name;
      this.methods = [];
      this.constructors = [
        new FunctionDefinition('default', this, [])
      ];
      this.base = base;
    }

    print(nameMap, out) {
      out.push(`export interface ${this.name}`);
      if (this.base) {
        out.push(` extends ${this.base.name}`);
      }
      out.push(" {\n");
      for (let method of this.methods) {
        out.push("  ");
        method.printFunction(nameMap, out);
      }
      out.push("  delete(): void;\n");
      out.push("}\n\n");
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: {new`);
      // TODO Handle constructor overloading
      let constructor = this.constructors[this.constructors.length > 1 ? 1 : 0];
      constructor.printSignature(nameMap, out);
      out.push('};\n');
    }
  },
  $TsPrinter: class TsPrinter {
    constructor(definitions) {
      this.definitions = definitions;
      const jsString = 'ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string';
      this.builtInToJsName = new Map([
        ["bool", "boolean"],
        ["char", "number"],
        ["unsigned char", "number"],
        ["int", "number"],
        ["unsigned int", "number"],
        ["unsigned long", "number"],
        ["float", "number"],
        ["double", "number"],
        ["void", "void"],
        ["std::string", jsString],
        ["std::basic_string<unsigned char>", jsString],
        ["emscripten::val", "any"],
      ]);
    }

    typeToJsName(type) {
      if (type instanceof PrimitiveType) {
        if (!this.builtInToJsName.has(type.name)) {
          throw new Error(`Missing primitive type to TS type for "${type.name}"`);
        }
        return this.builtInToJsName.get(type.name)
      }
      return type.name;
    }

    print() {
      var out = [];
      for (let def of this.definitions) {
        if (!def.print) {
          continue;
        }
        def.print(this.typeToJsName.bind(this), out);
      }
      // Print module definitions
      out.push("export interface MainModule {\n");
      for (let def of this.definitions) {
        if (!def.printModuleEntry) {
          continue;
        }
        def.printModuleEntry(this.typeToJsName.bind(this), out);
      }
      out.push("}");
      console.log(out.join(''));
    }
  },

  $registerType__deps: ['$sharedRegisterType'],
  $registerType: function(rawType, registeredInstance, options = {}) {
    return sharedRegisterType(rawType, registeredInstance, options);
  },
  $registerPrimitiveType__deps: ['$registerType', '$PrimitiveType'],
  $registerPrimitiveType: function(id, name) {
    name = readLatin1String(name);
    registerType(id, new PrimitiveType(id, name));
  },
  $createFunctionDefinition__deps: ['$FunctionDefinition', '$heap32VectorToArray', '$readLatin1String', '$Argument', '$whenDependentTypesAreResolved'],
  $createFunctionDefinition: function(name, argCount, rawArgTypesAddr, hasThis, cb) {
    var argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = readLatin1String(name);

    whenDependentTypesAreResolved([], argTypes, function(argTypes) {
      var returnType = argTypes[0];
      var thisType = null;
      var argStart = 1;
      if (hasThis) {
        thisType = argTypes[1];
        argStart = 2;
      }
      let args = [];
      for (var i = argStart; i < argTypes.length; i++) {
        args.push(new Argument(`_${i - argStart}`, argTypes[i]));
      }
      var funcDef = new FunctionDefinition(name, returnType, args, thisType);
      cb(funcDef);
      return [];
    });
  },
  _embind_register_void__deps: ['$registerPrimitiveType'],
  _embind_register_void: function(rawType, name) {
    registerPrimitiveType(rawType, name);
  },
  _embind_register_bool__deps: ['$registerPrimitiveType'],
  _embind_register_bool: function(rawType, name, size, trueValue, falseValue) {
    registerPrimitiveType(rawType, name);
  },
  _embind_register_integer__deps: ['$registerPrimitiveType'],
  _embind_register_integer: function(primitiveType, name, size, minRange, maxRange) {
    registerPrimitiveType(primitiveType, name);
  },
  _embind_register_bigint: function(primitiveType, name, size, minRange, maxRange) {
    registerPrimitiveType(primitiveType, name);
  },
  _embind_register_float__deps: ['$registerPrimitiveType'],
  _embind_register_float: function(rawType, name, size) {
    registerPrimitiveType(rawType, name);
  },
  _embind_register_std_string__deps: ['$registerPrimitiveType'],
  _embind_register_std_string: function(rawType, name) {
    registerPrimitiveType(rawType, name);
  },
  _embind_register_std_wstring: function(rawType, charSize, name) {
    registerPrimitiveType(rawType, name);
  },
  _embind_register_emval: function(rawType, name) {
    registerPrimitiveType(rawType, name);
  },
  _embind_register_memory_view: function(rawType, dataTypeIndex, name) {
    // TODO
  },
  _embind_register_function__deps: ['$moduleDefinitions', '$createFunctionDefinition'],
  _embind_register_function: function(name, argCount, rawArgTypesAddr, signature, rawInvoker, fn, isAsync) {
    createFunctionDefinition(name, argCount, rawArgTypesAddr, false, (funcDef) => {
      moduleDefinitions.push(funcDef);
    });
  },
  _embind_register_class__deps: ['$readLatin1String', '$ClassDefinition', '$whenDependentTypesAreResolved', '$moduleDefinitions'],
  _embind_register_class: function(rawType,
                                  rawPointerType,
                                  rawConstPointerType,
                                  baseClassRawType,
                                  getActualTypeSignature,
                                  getActualType,
                                  upcastSignature,
                                  upcast,
                                  downcastSignature,
                                  downcast,
                                  name,
                                  destructorSignature,
                                  rawDestructor) {
    name = readLatin1String(name);
    whenDependentTypesAreResolved(
      [rawType, rawPointerType, rawConstPointerType],
      baseClassRawType ? [baseClassRawType] : [],
      function(base) {
        var classDef = new ClassDefinition(rawType, name, base.length ? base[0] : null);
        moduleDefinitions.push(classDef);
        return [classDef, classDef, classDef];
      }
    );

  },
  _embind_register_class_constructor__deps: ['$whenDependentTypesAreResolved'],
  _embind_register_class_constructor: function(
    rawClassType,
    argCount,
    rawArgTypesAddr,
    invokerSignature,
    invoker,
    rawConstructor
  ) {
    whenDependentTypesAreResolved([], [rawClassType], function(classType) {
      classType = classType[0];
      createFunctionDefinition(`constructor ${classType.name}`, argCount, rawArgTypesAddr, false, (funcDef) => {
        classType.constructors.push(funcDef);
      });
      return [];
    });
  },
  _embind_register_class_function__deps: ['$createFunctionDefinition'],
  _embind_register_class_function: function(rawClassType,
          methodName,
          argCount,
          rawArgTypesAddr, // [ReturnType, ThisType, Args...]
          invokerSignature,
          rawInvoker,
          context,
          isPureVirtual,
          isAsync) {
    createFunctionDefinition(methodName, argCount, rawArgTypesAddr, true, (funcDef) => {
      let classDef = funcDef.thisType;
      classDef.methods.push(funcDef);
    });
  },

  _embind_initialize_done__deps: ['$awaitingDependencies', '$throwBindingError', '$getTypeName', '$moduleDefinitions', '$TsPrinter'],
  _embind_initialize_done: function() {
    for (var typeId in awaitingDependencies) {
      throwBindingError(`Missing type definition for '${getTypeName(typeId)}'`);
    }
    var printer = new TsPrinter(moduleDefinitions);
    printer.print();
  },
};

mergeInto(LibraryManager.library, LibraryEmbind);
