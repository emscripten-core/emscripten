// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
#include "libembind_shared.js"

var LibraryEmbind = {

  $moduleDefinitions: [],
  // Function signatures that have already been generated for JS generation.
  $emittedFunctions: 'new Set()',

  $PrimitiveType: class {
    constructor(typeId, name) {
      this.typeId = typeId;
      this.name = name;
    }
  },
  $IntegerType: class {
    constructor(typeId) {
      this.typeId = typeId;
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
    }
  },
  $UserTypeDefinition: class {
    constructor(typeId, name, definition) {
      this.typeId = typeId;
      this.name = name;
      this.definition = definition;
    }

    print(nameMap, out) {
      out.push(`export type ${this.name} = ${this.definition};\n\n`);
    }
  },
  $OptionalType: class {
    constructor(type) {
      this.type = type;
    }
  },
  $FunctionDefinition: class {
    hasPublicSymbol = true;
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
      returnType = nameMap(returnType, true);
      if (this.isAsync) {
        returnType = `Promise<${returnType}>`;
      }
      out.push(`): ${returnType}`);
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
      if (type.usesDestructor) {
        ret.usesDestructor = true;
      }
      if (type.argStackAlloc) {
        // Trivial value types stack-allocate their argument temporaries;
        // must mirror the runtime type object so the invoker signature and
        // generated shape match (see createJsInvokerSignature).
        ret.argStackAlloc = true;
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
      const signature = createJsInvokerSignature(argTypes, !!this.thisType, !this.returnType.isVoid, this.isAsync)
      if (emittedFunctions.has(signature)) {
        return;
      }
      emittedFunctions.add(signature);
      let invokerFactory = createJsInvoker(argTypes, !!this.thisType, !this.returnType.isVoid, this.isAsync);
      out.push(`'${signature}': ${invokerFactory},`);
    }
  },
  $PointerDefinition: class {
    constructor(classType, isSmartPointer = false) {
      this.classType = classType;
      this.usesDestructor = isSmartPointer;
    }
  },
  $ClassDefinition: class {
    hasPublicSymbol = true;
    constructor(typeId, name, base = null) {
      this.typeId = typeId;
      this.name = name;
      this.methods = [];
      this.staticMethods = [];
      this.staticProperties = [];
      this.constructors = [];
      this.base = base;
      this.properties = [];
      this.iterableElementType = null;
    }

    print(nameMap, out) {
      out.push(`export interface ${this.name}`);
      const extendsParts = [];
      if (this.base) {
        extendsParts.push(this.base.name);
      } else {
        extendsParts.push('ClassHandle');
      }
      if (this.iterableElementType) {
        extendsParts.push(`Iterable<${nameMap(this.iterableElementType, true)}>`);
      }
      out.push(` extends ${extendsParts.join(', ')}`);
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
    hasPublicSymbol = true;
    constructor(type, name) {
      this.type = type;
      this.name = name;
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: ${nameMap(this.type)};\n`);
    }
  },
  $EnumDefinition: class {
    hasPublicSymbol = true;
    constructor(typeId, name, valueType) {
      this.typeId = typeId;
      this.name = name;
      this.items = [];
      this.valueType = valueType;
    }

    print(nameMap, out) {
      if (this.valueType === 'object') {
        out.push(`export interface ${this.name}Value<T extends number> {\n`);
        out.push('  value: T;\n}\n');
      }
      out.push(`export type ${this.name} = `);
      if (!this.items.length) {
        out.push('never/* Empty Enumerator */');
      } else {
        const outItems = [];
        for (const [name, value] of this.items) {
          switch (this.valueType) {
            case 'object':
              outItems.push(`${this.name}Value<${value}>`);
              break;
            case 'number':
              outItems.push(`${value}`);
              break;
            case 'string':
              outItems.push(`'${name}'`);
              break;
          }
        }
        out.push(outItems.join('|'));
      }
      out.push(';\n\n');
    }

    printModuleEntry(nameMap, out) {
      out.push(`  ${this.name}: {`);
      const outItems = [];
      for (const [name, value] of this.items) {
        // Quote keys that aren't valid JS identifiers.
        const key = /^[a-zA-Z_$][\w$]*$/.test(name) ? name : `'${name}'`;
        switch (this.valueType) {
          case 'object':
            outItems.push(`${key}: ${this.name}Value<${value}>`);
            break;
          case 'number':
            outItems.push(`${key}: ${value}`);
            break;
          case 'string':
            outItems.push(`${key}: '${name}'`);
            break;
        }
      }
      out.push(outItems.join(', '));
      out.push('};\n');
    }
  },
  $ValueArrayDefinition: class {
    constructor(typeId, name, isTrivial) {
      this.typeId = typeId;
      this.name = name;
      this.elementTypeIds = [];
      this.elements = [];
      // Trivial types need no destructor call; their argument temporaries
      // live in the invoker's stack frame.
      this.usesDestructor = !isTrivial;
      this.argStackAlloc = !!isTrivial;
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
    constructor(typeId, name, isTrivial) {
      this.typeId = typeId;
      this.name = name;
      this.fieldTypeIds = [];
      this.fieldNames = [];
      this.fields = [];
      // See ValueArrayDefinition: trivial types stack-allocate.
      this.usesDestructor = !isTrivial;
      this.argStackAlloc = !!isTrivial;
    }

    print(nameMap, out) {
      out.push(`export type ${this.name} = {\n`);
      const outFields = [];
      for (const {name, type} of this.fields) {
        outFields.push(`  ${name}${type instanceof OptionalType ? '?' : ''}: ${nameMap(type)}`);
      }
      out.push(outFields.join(',\n'))
      out.push('\n};\n\n');
    }
  },
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
        ['long long', ['bigint']],
        ['unsigned long long', ['bigint']],
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
        return `${this.typeToJsName(type.classType, isFromWireType)} | null`;
      }
      if (type instanceof OptionalType) {
        return `${this.typeToJsName(type.type, isFromWireType)} | undefined`;
      }
      return type.name;
    }

    print() {
      const out = [];
      let hadClass = false;
      for (const def of this.definitions) {
        if (def instanceof ClassDefinition) {
          hadClass = true;
          break;
        }
      }
      if (hadClass) {
        out.push(
          'export interface ClassHandle {\n',
          '  isAliasOf(other: ClassHandle): boolean;\n',
          '  delete(): void;\n',
          '  deleteLater(): this;\n',
          '  isDeleted(): boolean;\n',
          '  // @ts-ignore - If targeting lower than ESNext, this symbol might not exist.\n',
          '  [Symbol.dispose](): void;\n',
          '  clone(): this;\n',
          '}\n',
        );
      }
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
      return out.join('');
    }
  },

  $JsPrinter: class {
    constructor(definitions) {
      this.definitions = definitions;
    }

    print() {
      const out = ['{\n'];
      const publicSymbols = [];
      for (const def of this.definitions) {
        if (def.hasPublicSymbol) {
          publicSymbols.push(def.name);
        }
        if (!def.printJs) {
          continue;
        }
        def.printJs(out);
      }
      out.push('}\n');
      return JSON.stringify({
        'invokers': out.join(''),
        publicSymbols,
      });
    }
  },

  $registerType: function(typeID, registeredInstance, options = {}) {
    return sharedRegisterType(typeID, registeredInstance, options);
  },
  $registerPrimitiveType: (id, name) => {
    name = AsciiToString(name);
    registerType(id, new PrimitiveType(id, name));
  },
  $registerIntegerType: (id) => {
    registerType(id, new IntegerType(id));
  },
  $createFunctionDefinition: (name, argCount, rawArgTypesAddr, functionIndex, hasThis, isNonnullReturn, isAsync, cb) => {
    const argTypeIDs = heap32VectorToArray(argCount, rawArgTypesAddr);
    name = typeof name === 'string' ? name : AsciiToString(name);

    whenDependentTypesAreResolved([], argTypeIDs, function (argTypes) {
      const argsName = getFunctionArgsName(name);
      name = getFunctionName(name);
      const returnType = argTypes[0];
      let thisType = null;
      let argStart = 1;
      if (hasThis) {
        thisType = argTypes[1];
        const classType = thisType instanceof PointerDefinition ? thisType.classType : thisType;
        if (!(classType instanceof ClassDefinition)) {
          throw new Error('This type must be class definition for: ' + name);
        }
        argStart = 2;
      }
      if (argsName && argsName.length != (argTypes.length - hasThis - 1)) {
        throw new Error('Argument names should match number of parameters.');
      }

      const args = [];
      for (let i = argStart, x = 0; i < argTypes.length; i++) {
        if (argsName) {
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
  _embind_register_void: (typeID, name) => {
    const voidType = new PrimitiveType(typeID, 'void');
    voidType.isVoid = true; // Match the marker property from the non-AOT mode.
    registerType(typeID, voidType);
  },
  _embind_register_bool: (typeID, name, trueValue, falseValue) => {
    registerPrimitiveType(typeID, name);
  },
  _embind_register_integer: (typeID, name, size, minRange, maxRange) => {
    registerIntegerType(typeID, name);
  },
  _embind_register_bigint: (typeID, name, size, minRange, maxRange) => {
    registerPrimitiveType(typeID, name);
  },
  _embind_register_float: (typeID, name, size) => {
    registerPrimitiveType(typeID, name);
  },
  _embind_register_std_string: (typeID, name) => {
    registerPrimitiveType(typeID, name);
  },
  _embind_register_std_wstring: (typeID, charSize, name) => {
    registerPrimitiveType(typeID, name);
  },
  _embind_register_emval: (typeID) => {
    registerType(typeID, new PrimitiveType(typeID, 'emscripten::val'));
  },
  _embind_register_user_type: (typeID, name) => {
    name = AsciiToString(name);
    registerType(typeID, new UserType(typeID, name));
  },
  _embind_register_user_type_definition: (typeID, name, definition) => {
    name = AsciiToString(name);
    definition = AsciiToString(definition);
    const userTypeDef = new UserTypeDefinition(typeID, name, definition);
    registerType(typeID, userTypeDef);
    moduleDefinitions.push(userTypeDef);
  },
  _embind_register_optional: (optionalTypeID, typeID) => {
    whenDependentTypesAreResolved([optionalTypeID], [typeID], function(type) {
      type = type[0];
      return [new OptionalType(type)];
    });
  },
  _embind_register_memory_view: (typeID, dataTypeIndex, name) => {
    // TODO
  },
  _embind_register_function: (name, argCount, rawArgTypesAddr, signature, rawInvoker, fn, isAsync, isNonnullReturn) => {
    createFunctionDefinition(name, argCount, rawArgTypesAddr, fn, false, isNonnullReturn, isAsync, (funcDef) => {
      moduleDefinitions.push(funcDef);
    });
  },
  _embind_register_class: function(typeID,
                                  pointerTypeID,
                                  constPointerTypeID,
                                  baseClassTypeID,
                                  getActualTypeSignature,
                                  getActualType,
                                  upcastSignature,
                                  upcast,
                                  downcastSignature,
                                  downcast,
                                  name,
                                  destructorSignature,
                                  rawDestructor) {
    name = AsciiToString(name);
    whenDependentTypesAreResolved(
      [typeID, pointerTypeID, constPointerTypeID],
      baseClassTypeID ? [baseClassTypeID] : [],
      function(base) {
        const hasBase = base.length;
        const classDef = new ClassDefinition(typeID, name, hasBase ? base[0] : null);
        moduleDefinitions.push(classDef);

        const pointer = new PointerDefinition(classDef);
        const constPointer = new PointerDefinition(classDef);
        return [classDef, pointer, constPointer];
      }
    );

  },
  _embind_register_iterable: (classTypeID, elementTypeID, sizeMethodName, getMethodName) => {
    whenDependentTypesAreResolved([], [classTypeID, elementTypeID], (types) => {
      const classType = types[0];
      const elementType = types[1];
      classType.iterableElementType = elementType;
      return [];
    });
  },
  _embind_register_class_constructor: function(
    classTypeID,
    argCount,
    rawArgTypesAddr,
    invokerSignature,
    invoker,
    rawConstructor
  ) {
    whenDependentTypesAreResolved([], [classTypeID], function(classType) {
      classType = classType[0];
      createFunctionDefinition(`constructor ${classType.name}`, argCount, rawArgTypesAddr, rawConstructor, false, true, false, (funcDef) => {
        classType.constructors.push(funcDef);
      });
      return [];
    });
  },
  _embind_register_class_function: function(classTypeID,
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
      const classDef = funcDef.thisType instanceof PointerDefinition ? funcDef.thisType.classType : funcDef.thisType;
      classDef.methods.push(funcDef);
    });
  },
  _embind_register_class_property: function(classTypeID,
                                            fieldName,
                                            getterReturnTypeID,
                                            getterSignature,
                                            getter,
                                            getterContext,
                                            setterArgumentTypeID,
                                            setterSignature,
                                            setter,
                                            setterContext) {
    fieldName = AsciiToString(fieldName);
    const readonly = !setter;
    if (!(readonly || getterReturnTypeID === setterArgumentTypeID)) {
      throw new error('Mismatched getter and setter types are not supported.');
    }

    whenDependentTypesAreResolved([], [classTypeID], function(classType) {
      classType = classType[0];
      whenDependentTypesAreResolved([], [getterReturnTypeID], function(types) {
        const prop = new ClassProperty(types[0], fieldName, readonly);
        classType.properties.push(prop);
        return [];
      });
      return [];
    });
  },
  _embind_register_class_class_function: function(classTypeID,
                                                  methodName,
                                                  argCount,
                                                  rawArgTypesAddr,
                                                  invokerSignature,
                                                  rawInvoker,
                                                  fn,
                                                  isAsync,
                                                  isNonnullReturn) {
    whenDependentTypesAreResolved([], [classTypeID], function(classType) {
      classType = classType[0];
      createFunctionDefinition(methodName, argCount, rawArgTypesAddr, fn, false, isNonnullReturn, isAsync, (funcDef) => {
        classType.staticMethods.push(funcDef);
      });
      return [];
    });
  },
  _embind_register_class_class_property: (classTypeID,
                                          fieldName,
                                          fieldTypeID,
                                          rawFieldPtr,
                                          getterSignature,
                                          getter,
                                          setterSignature,
                                          setter) => {
    fieldName = AsciiToString(fieldName);
    whenDependentTypesAreResolved([], [classTypeID], function(classType) {
      classType = classType[0];
      whenDependentTypesAreResolved([], [fieldTypeID], function(types) {
        const prop = new ClassProperty(types[0], fieldName);
        classType.staticProperties.push(prop);
        return [];
      });
      return [];
    });
  },
  // Stub function. This is called when extending an object and not needed for TS generation.
  _embind_create_inheriting_constructor: (constructorName, wrapperTypeID, properties) => {},
  _embind_register_enum: function(typeID, name, size, isSigned, rawValueType) {
    name = AsciiToString(name);
    const valueType = getEnumValueType(rawValueType);
    const enumDef = new EnumDefinition(typeID, name, valueType);
    registerType(typeID, enumDef);
    moduleDefinitions.push(enumDef);
  },
  _embind_register_enum_value: function(enumTypeID, name, enumValue) {
    name = AsciiToString(name);
    const enumDef = requireRegisteredType(enumTypeID, name);
    enumDef.items.push([name, enumValue]);
  },
  _embind_register_constant: function(name, typeID, value) {
    name = AsciiToString(name);
    whenDependentTypesAreResolved([], [typeID], function(types) {
      const def = new ConstantDefinition(types[0], name);
      moduleDefinitions.push(def);
      return [];
    });
  },
  _embind_register_value_array: function(
    typeID,
    name,
    constructorSignature,
    rawConstructor,
    destructorSignature,
    rawDestructor,
    valueSize,
    isTrivial
  ) {
    name = AsciiToString(name);
    const valueArray = new ValueArrayDefinition(typeID, name, isTrivial);
    tupleRegistrations[typeID] = valueArray;
  },
  _embind_register_value_array_element: function(
    tupleTypeID,
    getterReturnTypeID,
    getterSignature,
    getter,
    getterContext,
    setterArgumentTypeID,
    setterSignature,
    setter,
    setterContext
  ) {
    const valueArray = tupleRegistrations[tupleTypeID];
    if (getterReturnTypeID !== setterArgumentTypeID) {
      throw new Error('Mismatched getter and setter types are not supported.');
    }

    valueArray.elementTypeIds.push(getterReturnTypeID);
  },
  _embind_finalize_value_array: function(tupleTypeID) {
    const valueArray = tupleRegistrations[tupleTypeID];
    delete tupleRegistrations[tupleTypeID];
    whenDependentTypesAreResolved([tupleTypeID], valueArray.elementTypeIds, function(types) {
      moduleDefinitions.push(valueArray);
      valueArray.elements = types;
      return [valueArray];
    });
  },
  _embind_register_value_object: function(
    typeID,
    name,
    constructorSignature,
    rawConstructor,
    destructorSignature,
    rawDestructor,
    valueSize,
    isTrivial
  ) {
    name = AsciiToString(name);
    const valueObject = new ValueObjectDefinition(typeID, name, isTrivial);
    structRegistrations[typeID] = valueObject;
  },
  _embind_register_value_object_field: function(
    structTypeID,
    fieldName,
    getterReturnTypeID,
    getterSignature,
    getter,
    getterContext,
    setterArgumentTypeID,
    setterSignature,
    setter,
    setterContext
  ) {
    const valueObject = structRegistrations[structTypeID];
    if (getterReturnTypeID !== setterArgumentTypeID) {
      throw new Error('Mismatched getter and setter types are not supported.');
    }

    valueObject.fieldTypeIds.push(getterReturnTypeID);
    valueObject.fieldNames.push(AsciiToString(fieldName));
  },
  _embind_finalize_value_object: function(structTypeID) {
    const valueObject = structRegistrations[structTypeID];
    delete structRegistrations[structTypeID];
    whenDependentTypesAreResolved([structTypeID], valueObject.fieldTypeIds, function(types) {
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
  _embind_register_smart_ptr: function(typeID,
                                       pointeeTypeID,
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
    whenDependentTypesAreResolved([typeID], [pointeeTypeID], function(pointeeType) {
      const smartPointer = new PointerDefinition(pointeeType[0], true);
      return [smartPointer];
    });
  },

  $emitOutput__postset: () => { addAtPostCtor('emitOutput()'); },
  $emitOutput__force: true,
  $emitOutput: () => {
    for (const typeId in awaitingDependencies) {
      throwBindingError(`Missing binding for type: '${getTypeName(typeId)}' typeId: ${typeId}`);
    }
#if EMBIND_AOT
    const printer = new JsPrinter(moduleDefinitions);
#else
    const printer = new TsPrinter(moduleDefinitions);
#endif
    const output = printer.print();
    var fs = require('node:fs');
    fs.writeFileSync(process.argv[2], output + '\n');
  },

  // Stub functions used by eval, but not needed for TS generation:
  $makeLegalFunctionName: () => { throw new Error('stub function should not be called'); },
  $runDestructors: () => { throw new Error('stub function should not be called'); },
  $flushPendingDeletes: () => { throw new Error('stub function should not be called'); },
  $setDelayFunction: () => { throw new Error('stub function should not be called'); },
  $PureVirtualError: () => { throw new Error('stub function should not be called'); },
};

addToLibrary(LibraryEmbind);
