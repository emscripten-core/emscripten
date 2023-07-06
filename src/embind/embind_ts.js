// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
#include "embind/embind_shared.js"

var LibraryEmbind = {

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

      const argOut = [];
      for (const arg of this.argumentTypes) {
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
      out.push('  ');
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
      out.push(' {\n');
      for (const method of this.methods) {
        out.push('  ');
        method.printFunction(nameMap, out);
      }
      out.push('  delete(): void;\n');
      out.push('}\n\n');
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: {new`);
      // TODO Handle constructor overloading
      const constructor = this.constructors[this.constructors.length > 1 ? 1 : 0];
      constructor.printSignature(nameMap, out);
      out.push('};\n');
    }
  },
  $ConstantDefinition: class ConstantDefinition {
    constructor(type, name) {
      this.type = type;
      this.name = name;
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: ${nameMap(this.type)};\n`);
    }
  },
  $EnumDefinition: class EnumDefinition {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.items = [];
    }

    print(nameMap, out) {
      out.push(`export interface ${this.name}Value<T extends number> {\n`);
      out.push('  value: T;\n}\n');
      out.push(`export type ${this.name} = `);
      const outItems = [];
      for (const [name, value] of this.items) {
        outItems.push(`${this.name}Value<${value}>`);
      }
      out.push(outItems.join('|'));
      out.push(';\n\n');
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: {`);
      const outItems = [];
      for (const [name, value] of this.items) {
        outItems.push(`${name}: ${this.name}Value<${value}>`);
      }
      out.push(outItems.join(', '));
      out.push('};\n');
    }
  },
  $ValueArrayDefinition: class ValueArrayDefinition {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.elementTypeIds = [];
      this.elements = [];
    }

    print(nameMap, out) {
      out.push(`export type ${this.name} = [ `);
      const outElements = [];
      for (const type of this.elements) {
        outElements.push(nameMap(type));
      }
      out.push(outElements.join(', '))
      out.push(' ];\n\n');
    }
  },
  $ValueObjectDefinition: class ValueObjectDefinition {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.fieldTypeIds = [];
      this.fieldNames = [];
      this.fields = [];
    }

    print(nameMap, out) {
      out.push(`export type ${this.name} = {\n`);
      const outFields = [];
      for (const {name, type} of this.fields) {
        outFields.push(`  ${name}: ${nameMap(type)}`);
      }
      out.push(outFields.join(',\n'))
      out.push('\n};\n\n');
    }
  },
  $TsPrinter: class TsPrinter {
    constructor(definitions) {
      this.definitions = definitions;
      const jsString = 'ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string';
      this.builtInToJsName = new Map([
        ['bool', 'boolean'],
        ['char', 'number'],
        ['unsigned char', 'number'],
        ['int', 'number'],
        ['unsigned int', 'number'],
        ['unsigned long', 'number'],
        ['float', 'number'],
        ['double', 'number'],
        ['void', 'void'],
        ['std::string', jsString],
        ['std::basic_string<unsigned char>', jsString],
        ['emscripten::val', 'any'],
      ]);
    }

    typeToJsName(type) {
      if (type instanceof PrimitiveType) {
        if (!this.builtInToJsName.has(type.name)) {
          throw new Error(`Missing primitive type to TS type for '${type.name}'`);
        }
        return this.builtInToJsName.get(type.name)
      }
      return type.name;
    }

    print() {
      const out = [];
      for (const def of this.definitions) {
        if (!def.print) {
          continue;
        }
        def.print(this.typeToJsName.bind(this), out);
      }
      // Print module definitions
      out.push('export interface MainModule {\n');
      for (const def of this.definitions) {
        if (!def.printModuleEntry) {
          continue;
        }
        def.printModuleEntry(this.typeToJsName.bind(this), out);
      }
      out.push('}');
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
    const argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = readLatin1String(name);

    whenDependentTypesAreResolved([], argTypes, function(argTypes) {
      const returnType = argTypes[0];
      let thisType = null;
      let argStart = 1;
      if (hasThis) {
        thisType = argTypes[1];
        argStart = 2;
      }
      const args = [];
      for (let i = argStart; i < argTypes.length; i++) {
        args.push(new Argument(`_${i - argStart}`, argTypes[i]));
      }
      const funcDef = new FunctionDefinition(name, returnType, args, thisType);
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
        const classDef = new ClassDefinition(rawType, name, base.length ? base[0] : null);
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
      const classDef = funcDef.thisType;
      classDef.methods.push(funcDef);
    });
  },
  _embind_register_enum__deps: ['$readLatin1String', '$EnumDefinition', '$moduleDefinitions'],
  _embind_register_enum: function(rawType, name, size, isSigned) {
    name = readLatin1String(name);
    const enumDef = new EnumDefinition(rawType, name);
    registerType(rawType, enumDef);
    moduleDefinitions.push(enumDef);
  },
  _embind_register_enum_value__deps: ['$readLatin1String', '$requireRegisteredType'],
  _embind_register_enum_value: function(rawEnumType, name, enumValue) {
    name = readLatin1String(name);
    const enumDef = requireRegisteredType(rawEnumType, name);
    enumDef.items.push([name, enumValue]);
  },
  _embind_register_constant__deps: ['$readLatin1String', '$ConstantDefinition', '$whenDependentTypesAreResolved', '$moduleDefinitions'],
  _embind_register_constant: function(name, typeId, value) {
    name = readLatin1String(name);
    whenDependentTypesAreResolved([], [typeId], function(types) {
      const def = new ConstantDefinition(types[0], name);
      moduleDefinitions.push(def);
      return [];
    });
  },
  _embind_register_value_array__deps: [
    '$readLatin1String', '$ValueArrayDefinition', '$tupleRegistrations'],
  _embind_register_value_array: function(
    rawType,
    name,
    constructorSignature,
    rawConstructor,
    destructorSignature,
    rawDestructor
  ) {
    name = readLatin1String(name);
    const valueArray = new ValueArrayDefinition(rawType, name);
    tupleRegistrations[rawType] = valueArray;
  },
  _embind_register_value_array_element__deps: ['$tupleRegistrations'],
  _embind_register_value_array_element: function(
    rawTupleType,
    getterReturnType,
    getterSignature,
    getter,
    getterContext,
    setterArgumentType,
    setterSignature,
    setter,
    setterContext
  ) {
    const valueArray = tupleRegistrations[rawTupleType];
    assert(getterReturnType === setterArgumentType, 'Mismatched getter and setter types are not supported.');
    valueArray.elementTypeIds.push(getterReturnType);
  },
  _embind_finalize_value_array__deps: ['$whenDependentTypesAreResolved', '$moduleDefinitions', '$tupleRegistrations'],
  _embind_finalize_value_array: function(rawTupleType) {
    const valueArray = tupleRegistrations[rawTupleType];
    delete tupleRegistrations[rawTupleType];
    whenDependentTypesAreResolved([rawTupleType], valueArray.elementTypeIds, function(types) {
      moduleDefinitions.push(valueArray);
      valueArray.elements = types;
      return [valueArray];
    });
  },
  _embind_register_value_object__deps: ['$readLatin1String', '$ValueObjectDefinition', '$structRegistrations'],
  _embind_register_value_object: function(
    rawType,
    name,
    constructorSignature,
    rawConstructor,
    destructorSignature,
    rawDestructor
  ) {
    name = readLatin1String(name);
    const valueObject = new ValueObjectDefinition(rawType, name);
    structRegistrations[rawType] = valueObject;
  },
  _embind_register_value_object_field__deps: [
    '$readLatin1String', '$structRegistrations'],
  _embind_register_value_object_field: function(
    structType,
    fieldName,
    getterReturnType,
    getterSignature,
    getter,
    getterContext,
    setterArgumentType,
    setterSignature,
    setter,
    setterContext
  ) {
    const valueObject = structRegistrations[structType];
    assert(getterReturnType === setterArgumentType, 'Mismatched getter and setter types are not supported.');
    valueObject.fieldTypeIds.push(getterReturnType);
    valueObject.fieldNames.push(readLatin1String(fieldName));
  },
  _embind_finalize_value_object__deps: ['$moduleDefinitions', '$whenDependentTypesAreResolved', '$structRegistrations'],
  _embind_finalize_value_object: function(structType) {
    const valueObject = structRegistrations[structType];
    delete structRegistrations[structType];
    whenDependentTypesAreResolved([structType], valueObject.fieldTypeIds, function(types) {
      moduleDefinitions.push(valueObject);
      for (let i = 0; i < types.length; i++) {
        valueObject.fields.push({
          name: valueObject.fieldNames[i],
          type: types[i],
        });
      }
      return [valueObject];
    });
  },
  _embind_register_smart_ptr__deps: ['$whenDependentTypesAreResolved'],
  _embind_register_smart_ptr: function(rawType,
                                       rawPointeeType,
                                       name,
                                       sharingPolicy,
                                       getPointeeSignature,
                                       rawGetPointee,
                                       constructorSignature,
                                       rawConstructor,
                                       shareSignature,
                                       rawShare,
                                       destructorSignature,
                                       rawDestructor) {
    whenDependentTypesAreResolved([rawType], [rawPointeeType], function(pointeeType) {
      return [pointeeType[0]];
    });
  },

  $embindEmitTypes__deps: ['$awaitingDependencies', '$throwBindingError', '$getTypeName', '$moduleDefinitions', '$TsPrinter'],
  $embindEmitTypes__postset: 'addOnInit(embindEmitTypes);',
  $embindEmitTypes: function() {
    for (const typeId in awaitingDependencies) {
      throwBindingError(`Missing type definition for '${getTypeName(typeId)}'`);
    }
    const printer = new TsPrinter(moduleDefinitions);
    printer.print();
  },
};

DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$embindEmitTypes');

mergeInto(LibraryManager.library, LibraryEmbind);
