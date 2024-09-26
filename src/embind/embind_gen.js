// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
#include "embind/embind_shared.js"

var LibraryEmbind = {

  $moduleDefinitions: [],
  // Function signatures that have already been generated for JS generation.
  $emittedFunctions: 'new Set()',

  $PrimitiveType: class {
    constructor(typeId, name, destructorType) {
      this.typeId = typeId;
      this.name = name;
      this.destructorType = destructorType;
    }
  },
  $IntegerType: class {
    constructor(typeId) {
      this.typeId = typeId;
      this.destructorType = 'none';
    }
  },
  $Argument: class {
    constructor(name, type) {
      this.name = name;
      this.type = type;
    }
  },
  $UserType: class {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.destructorType = 'none'; // Same as emval.
    }
  },
  $OptionalType: class {
    constructor(type) {
      this.type = type;
      this.destructorType = 'none'; // Same as emval.
    }
  },
  $FunctionDefinition__deps: ['$createJsInvoker', '$createJsInvokerSignature', '$emittedFunctions'],
  $FunctionDefinition: class {
    constructor(name, returnType, argumentTypes, functionIndex, thisType = null, isNonnullReturn = false, isAsync = false) {
      this.name = name;
      this.returnType = returnType;
      this.argumentTypes = argumentTypes;
      this.functionIndex = functionIndex;
      this.thisType = thisType;
      this.isNonnullReturn = isNonnullReturn;
      this.isAsync = isAsync;
    }

    printSignature(nameMap, out) {
      out.push('(');
      const argOut = [];
      // Work backwards on the arguments, so optional types can be replaced
      // with TS optional params until we see the first non-optional argument.
      let seenNonOptional = false;
      for (let i = this.argumentTypes.length - 1; i >= 0; i--) {
        const arg = this.argumentTypes[i];
        let argType;
        let argName;
        if (arg.type instanceof OptionalType && !seenNonOptional) {
          argType = nameMap(arg.type.type);
          argName = arg.name + '?';
        } else {
          seenNonOptional = true;
          argType = nameMap(arg.type);
          argName = arg.name;
        }
        argOut.unshift(`${argName}: ${argType}`);
      }

      out.push(argOut.join(', '));
      let returnType = this.returnType;
      // Constructors can return a pointer, but it will be a non-null pointer.
      // Change the return type to the class type so the TS output doesn't
      // have `| null`.
      if (this.isNonnullReturn && this.returnType instanceof PointerDefinition) {
        returnType = this.returnType.classType;
      }
      out.push(`): ${nameMap(returnType, true)}`);
    }

    printFunction(nameMap, out) {
      out.push(`${this.name}`);
      this.printSignature(nameMap, out);
    }

    printModuleEntry(nameMap, out) {
      out.push('  ');
      this.printFunction(nameMap, out);
      out.push(';\n');
    }

    // Convert a type definition in this file to something that matches the type
    // object in embind.js `registerType()`.
    convertToEmbindType(type) {
      const ret = {
        name: type.name,
      };
      switch (type.destructorType) {
        case 'none':
          ret.destructorFunction = null;
          break;
        case 'function':
          ret.destructorFunction = true;
          break;
        case 'stack':
          // Intentionally empty since embind uses `undefined` for this type.
          break;
        default:
          throw new Error(`Bad destructor type '${type.destructorType}'`);
      }
      return ret;
    }

    printJs(out) {
      const argTypes = [this.convertToEmbindType(this.returnType)];
      if (this.thisType) {
        argTypes.push(this.convertToEmbindType(this.thisType));
      } else {
        argTypes.push(null);
      }
      for (const argType of this.argumentTypes) {
        argTypes.push(this.convertToEmbindType(argType.type));
      }
      const signature = createJsInvokerSignature(argTypes, !!this.thisType, this.returnType.name !== 'void', this.isAsync)
      if (emittedFunctions.has(signature)) {
        return;
      }
      emittedFunctions.add(signature);
      let [args, body] = createJsInvoker(argTypes, !!this.thisType, this.returnType.name !== 'void', this.isAsync);
      out.push(
        // The ${""} is hack to workaround the preprocessor replacing "function".
        `'${signature}': f${""}unction(${args.join(',')}) {\n${body}},`
      );
    }
  },
  $PointerDefinition: class {
    constructor(classType, isConst, isSmartPointer) {
      this.classType = classType;
      this.isConst = isConst;
      this.isSmartPointer = isSmartPointer;
      this.destructorType = 'none';
      if (isSmartPointer || classType.base) {
        this.destructorType = 'stack';
      }
    }
  },
  $ClassDefinition: class {
    constructor(typeId, name, base = null) {
      this.typeId = typeId;
      this.name = name;
      this.methods = [];
      this.staticMethods = [];
      this.staticProperties = [];
      this.constructors = [];
      this.base = base;
      this.properties = [];
      this.destructorType = 'none';
      if (base) {
        this.destructorType = 'stack';
      }
    }

    print(nameMap, out) {
      out.push(`export interface ${this.name}`);
      if (this.base) {
        out.push(` extends ${this.base.name}`);
      }
      out.push(' {\n');
      for (const property of this.properties) {
        const props = [];
        property.print(nameMap, props);
        for (const formattedProp of props) {
          out.push(`  ${formattedProp};\n`);
        }
      }
      for (const method of this.methods) {
        out.push('  ');
        method.printFunction(nameMap, out);
        out.push(';\n');
      }
      out.push('  delete(): void;\n');
      out.push('}\n\n');
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: {`);
      const entries = [];
      for (const construct of this.constructors) {
        const entry = [];
        entry.push('new');
        construct.printSignature(nameMap, entry);
        entries.push(entry.join(''));
      }
      for (const method of this.staticMethods) {
        const entry = [];
        method.printFunction(nameMap, entry);
        entries.push(entry.join(''));
      }
      for (const prop of this.staticProperties) {
        const entry = [];
        prop.print(nameMap, entry);
        entries.push(...entry);
      }
      if (entries.length) {
        out.push('\n');
        for (const entry of entries) {
          out.push(`    ${entry};\n`);
        }
        out.push('  ');
      }
      out.push('};\n');
    }

    printJs(out) {
      out.push(`// class ${this.name}\n`);
      if (this.constructors.length) {
        out.push(`// constructors\n`);
        for (const construct of this.constructors) {
          construct.printJs(out);
        }
      }
      if (this.staticMethods.length) {
        out.push(`// static methods\n`);
        for (const method of this.staticMethods) {
          method.printJs(out);
        }
      }
      if (this.methods.length) {
        out.push(`// methods\n`);
        for (const method of this.methods) {
          method.printJs(out);
        }
      }
      out.push('\n');
    }

  },
  $ClassProperty: class {
    constructor(type, name, readonly) {
      this.type = type;
      this.name = name;
      this.readonly = readonly;
    }

    print(nameMap, out) {
      const setType = nameMap(this.type, false);
      const getType = nameMap(this.type, true);
      if (this.readonly || setType === getType) {
        out.push(`${this.readonly ? 'readonly ' : ''}${this.name}: ${getType}`);
        return;
      }
      // The getter/setter types don't match, so generate each get/set definition.
      out.push(`get ${this.name}(): ${getType}`);
      out.push(`set ${this.name}(value: ${setType})`);
    }
  },
  $ConstantDefinition: class {
    constructor(type, name) {
      this.type = type;
      this.name = name;
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: ${nameMap(this.type)};\n`);
    }
  },
  $EnumDefinition: class {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.items = [];
      this.destructorType = 'none';
    }

    print(nameMap, out) {
      out.push(`export interface ${this.name}Value<T extends number> {\n`);
      out.push('  value: T;\n}\n');
      out.push(`export type ${this.name} = `);
      if (this.items.length === 0) {
        out.push('never/* Empty Enumerator */');
      } else {
        const outItems = [];
        for (const [name, value] of this.items) {
          outItems.push(`${this.name}Value<${value}>`);
        }
        out.push(outItems.join('|'));
      }
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
  $ValueArrayDefinition: class {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.elementTypeIds = [];
      this.elements = [];
      this.destructorType = 'function';
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
  $ValueObjectDefinition: class {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
      this.fieldTypeIds = [];
      this.fieldNames = [];
      this.fields = [];
      this.destructorType = 'function';
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
  $TsPrinter__deps: ['$OptionalType'],
  $TsPrinter: class {
    constructor(definitions) {
      this.definitions = definitions;
      const jsString = 'EmbindString'; // Type alias for multiple types.
      // The mapping is in the format of '<c++ name>' => ['toWireType', 'fromWireType']
      // or if the to/from wire types are the same use a single element.
      this.builtInToJsName = new Map([
        ['bool', ['boolean']],
        ['float', ['number']],
        ['double', ['number']],
#if MEMORY64
        ['long', ['bigint']],
        ['unsigned long', ['bigint']],
#endif
#if WASM_BIGINT
        ['int64_t', ['bigint']],
        ['uint64_t', ['bigint']],
#endif
        ['void', ['void']],
        ['std::string', [jsString, 'string']],
        ['std::basic_string<unsigned char>', [jsString, 'string']],
        ['std::wstring', ['string']],
        ['std::u16string', ['string']],
        ['std::u32string', ['string']],
        ['emscripten::val', ['any']],
      ]);
      // Signal that the type alias for EmbindString is needed.
      this.usedEmbindString = false;
    }

    typeToJsName(type, isFromWireType = false) {
      if (type instanceof IntegerType) {
        return 'number';
      }
      if (type instanceof PrimitiveType) {
        if (!this.builtInToJsName.has(type.name)) {
          throw new Error(`Missing primitive type to TS type for '${type.name}'`);
        }
        const [toWireType, fromWireType = toWireType] = this.builtInToJsName.get(type.name);
        const tsName = isFromWireType ? fromWireType : toWireType;
        if (tsName === 'EmbindString') {
          this.usedEmbindString = true;
        }
        return tsName;
      }
      if (type instanceof PointerDefinition) {
        return `${this.typeToJsName(type.classType)} | null`;
      }
      if (type instanceof OptionalType) {
        return `${this.typeToJsName(type.type)} | undefined`;
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
      out.push('interface EmbindModule {\n');
      for (const def of this.definitions) {
        if (!def.printModuleEntry) {
          continue;
        }
        def.printModuleEntry(this.typeToJsName.bind(this), out);
      }
      out.push('}\n');
      if (this.usedEmbindString) {
        out.unshift('type EmbindString = ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string;\n');
      }
      console.log(out.join(''));
    }
  },

  $JsPrinter: class {
    constructor(definitions) {
      this.definitions = definitions;
    }

    print() {
      const out = ['{\n'];
      for (const def of this.definitions) {
        if (!def.printJs) {
          continue;
        }
        def.printJs(out);
      }
      out.push('}')
      console.log(out.join(''));
    }
  },

  $registerType__deps: ['$sharedRegisterType'],
  $registerType: function(rawType, registeredInstance, options = {}) {
    return sharedRegisterType(rawType, registeredInstance, options);
  },
  $registerPrimitiveType__deps: ['$registerType', '$PrimitiveType'],
  $registerPrimitiveType: (id, name, destructorType) => {
    name = readLatin1String(name);
    registerType(id, new PrimitiveType(id, name, destructorType));
  },
  $registerIntegerType__deps: ['$registerType', '$IntegerType'],
  $registerIntegerType: (id) => {
    registerType(id, new IntegerType(id));
  },
  $createFunctionDefinition__deps: ['$FunctionDefinition', '$heap32VectorToArray', '$readLatin1String', '$Argument', '$whenDependentTypesAreResolved', '$getFunctionName', '$getFunctionArgsName', '$PointerDefinition', '$ClassDefinition'],
  $createFunctionDefinition: (name, argCount, rawArgTypesAddr, functionIndex, hasThis, isNonnullReturn, isAsync, cb) => {
    const argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = typeof name === 'string' ? name : readLatin1String(name);

    whenDependentTypesAreResolved([], argTypes, function (argTypes) {
      const argsName = getFunctionArgsName(name);
      name = getFunctionName(name);
      const returnType = argTypes[0];
      let thisType = null;
      let argStart = 1;
      if (hasThis) {
        thisType = argTypes[1];
        if (thisType instanceof PointerDefinition) {
          thisType = argTypes[1].classType;
        }
        if (!(thisType instanceof ClassDefinition)) {
          throw new Error('This type must be class definition for: ' + name);
        }
        argStart = 2;
      }
      if (argsName.length && argsName.length != (argTypes.length - hasThis - 1)) {
        throw new Error('Argument names should match number of parameters.');
      }

      const args = [];
      for (let i = argStart, x = 0; i < argTypes.length; i++) {
        if (x < argsName.length) {
          args.push(new Argument(argsName[x++], argTypes[i]));
        } else {
          args.push(new Argument(`_${i - argStart}`, argTypes[i]));
        }
      }
      const funcDef = new FunctionDefinition(name, returnType, args, functionIndex, thisType, isNonnullReturn, isAsync);
      cb(funcDef);
      return [];
    });
  },
  _embind_register_void__deps: ['$registerPrimitiveType'],
  _embind_register_void: (rawType, name) => {
    registerPrimitiveType(rawType, name, 'none');
  },
  _embind_register_bool__deps: ['$registerPrimitiveType'],
  _embind_register_bool: (rawType, name, trueValue, falseValue) => {
    registerPrimitiveType(rawType, name, 'none');
  },
  _embind_register_integer__deps: ['$registerIntegerType'],
  _embind_register_integer: (primitiveType, name, size, minRange, maxRange) => {
    registerIntegerType(primitiveType, name);
  },
  _embind_register_bigint: (primitiveType, name, size, minRange, maxRange) => {
    registerPrimitiveType(primitiveType, name, 'none');
  },
  _embind_register_float__deps: ['$registerPrimitiveType'],
  _embind_register_float: (rawType, name, size) => {
    registerPrimitiveType(rawType, name, 'none');
  },
  _embind_register_std_string__deps: ['$registerPrimitiveType'],
  _embind_register_std_string: (rawType, name) => {
    registerPrimitiveType(rawType, name, 'function');
  },
  _embind_register_std_wstring: (rawType, charSize, name) => {
    registerPrimitiveType(rawType, name, 'function');
  },
  _embind_register_emval__deps: ['$registerType', '$PrimitiveType'],
  _embind_register_emval: (rawType) => {
    registerType(rawType, new PrimitiveType(rawType, 'emscripten::val', 'none'));
  },
  _embind_register_user_type__deps: ['$registerType', '$readLatin1String', '$UserType'],
  _embind_register_user_type: (rawType, name) => {
    name = readLatin1String(name);
    registerType(rawType, new UserType(rawType, name));
  },
  _embind_register_optional__deps: ['_embind_register_emval', '$OptionalType'],
  _embind_register_optional: (rawOptionalType, rawType) => {
    whenDependentTypesAreResolved([rawOptionalType], [rawType], function(type) {
      type = type[0];
      return [new OptionalType(type)];
    });
  },
  _embind_register_memory_view: (rawType, dataTypeIndex, name) => {
    // TODO
  },
  _embind_register_function__deps: ['$moduleDefinitions', '$createFunctionDefinition'],
  _embind_register_function: (name, argCount, rawArgTypesAddr, signature, rawInvoker, fn, isAsync, isNonnullReturn) => {
    createFunctionDefinition(name, argCount, rawArgTypesAddr, fn, false, isNonnullReturn, isAsync, (funcDef) => {
      moduleDefinitions.push(funcDef);
    });
  },
  _embind_register_class__deps: ['$readLatin1String', '$ClassDefinition', '$whenDependentTypesAreResolved', '$moduleDefinitions', '$PointerDefinition'],
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
        const hasBase = base.length;
        const classDef = new ClassDefinition(rawType, name, hasBase ? base[0] : null);
        moduleDefinitions.push(classDef);

        const pointer = new PointerDefinition(classDef, false, false);
        const constPointer = new PointerDefinition(classDef, true, false);
        return [classDef, pointer, constPointer];
      }
    );

  },
  _embind_register_class_constructor__deps: ['$whenDependentTypesAreResolved', '$createFunctionDefinition'],
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
      createFunctionDefinition(`constructor ${classType.name}`, argCount, rawArgTypesAddr, rawConstructor, false, true, false, (funcDef) => {
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
          isAsync,
          isNonnullReturn) {
    createFunctionDefinition(methodName, argCount, rawArgTypesAddr, context, true, isNonnullReturn, isAsync, (funcDef) => {
      const classDef = funcDef.thisType;
      classDef.methods.push(funcDef);
    });
  },
  _embind_register_class_property__deps: [
    '$readLatin1String', '$whenDependentTypesAreResolved', '$ClassProperty'],
  _embind_register_class_property: function(classType,
                                            fieldName,
                                            getterReturnType,
                                            getterSignature,
                                            getter,
                                            getterContext,
                                            setterArgumentType,
                                            setterSignature,
                                            setter,
                                            setterContext) {
    fieldName = readLatin1String(fieldName);
    const readonly = setter === 0;
    if (!(readonly || getterReturnType === setterArgumentType)) {
      throw new error('Mismatched getter and setter types are not supported.');
    }

    whenDependentTypesAreResolved([], [classType], function(classType) {
      classType = classType[0];
      whenDependentTypesAreResolved([], [getterReturnType], function(types) {
        const prop = new ClassProperty(types[0], fieldName, readonly);
        classType.properties.push(prop);
        return [];
      });
      return [];
    });
  },
  _embind_register_class_class_function__deps: ['$createFunctionDefinition'],
  _embind_register_class_class_function: function(rawClassType,
                                                  methodName,
                                                  argCount,
                                                  rawArgTypesAddr,
                                                  invokerSignature,
                                                  rawInvoker,
                                                  fn,
                                                  isAsync,
                                                  isNonnullReturn) {
    whenDependentTypesAreResolved([], [rawClassType], function(classType) {
      classType = classType[0];
      createFunctionDefinition(methodName, argCount, rawArgTypesAddr, fn, false, isNonnullReturn, isAsync, (funcDef) => {
        classType.staticMethods.push(funcDef);
      });
      return [];
    });
  },
  _embind_register_class_class_property__deps: [
    '$readLatin1String', '$whenDependentTypesAreResolved', '$ClassProperty'],
  _embind_register_class_class_property: (rawClassType,
                                          fieldName,
                                          rawFieldType,
                                          rawFieldPtr,
                                          getterSignature,
                                          getter,
                                          setterSignature,
                                          setter) => {
    fieldName = readLatin1String(fieldName);
    whenDependentTypesAreResolved([], [rawClassType], function(classType) {
      classType = classType[0];
      whenDependentTypesAreResolved([], [rawFieldType], function(types) {
        const prop = new ClassProperty(types[0], fieldName);
        classType.staticProperties.push(prop);
        return [];
      });
      return [];
    });
  },
  // Stub function. This is called a when extending an object and not needed for TS generation.
  _embind_create_inheriting_constructor: (constructorName, wrapperType, properties) => {},
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
    if (getterReturnType !== setterArgumentType) {
      throw new Error('Mismatched getter and setter types are not supported.');
    }

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
    if (getterReturnType !== setterArgumentType) {
      throw new Error('Mismatched getter and setter types are not supported.');
    }

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
      const smartPointer = new PointerDefinition(pointeeType[0], false, true);
      return [smartPointer];
    });
  },

#if EMBIND_AOT
  $embindEmitAotJs__deps: ['$awaitingDependencies', '$throwBindingError', '$getTypeName', '$moduleDefinitions', '$JsPrinter'],
  $embindEmitAotJs__postset: 'addOnInit(embindEmitAotJs);',
  $embindEmitAotJs: () => {
    for (const typeId in awaitingDependencies) {
      throwBindingError(`Missing binding for type: '${getTypeName(typeId)}' typeId: ${typeId}`);
    }
    const printer = new JsPrinter(moduleDefinitions);
    printer.print();
  },
#else // EMBIND_AOT
  $embindEmitTypes__deps: ['$awaitingDependencies', '$throwBindingError', '$getTypeName', '$moduleDefinitions', '$TsPrinter'],
  $embindEmitTypes__postset: 'addOnInit(embindEmitTypes);',
  $embindEmitTypes: () => {
    for (const typeId in awaitingDependencies) {
      throwBindingError(`Missing binding for type: '${getTypeName(typeId)}' typeId: ${typeId}`);
    }
    const printer = new TsPrinter(moduleDefinitions);
    printer.print();
  },
#endif

  // Stub functions used by eval, but not needed for TS generation:
  $makeLegalFunctionName: () => { throw new Error('stub function should not be called'); },
  $newFunc: () => { throw new Error('stub function should not be called'); },
  $runDestructors: () => { throw new Error('stub function should not be called'); },
  $createNamedFunction: () => { throw new Error('stub function should not be called'); },
};

#if EMBIND_AOT
extraLibraryFuncs.push('$embindEmitAotJs');
#else
extraLibraryFuncs.push('$embindEmitTypes');
#endif

addToLibrary(LibraryEmbind);
