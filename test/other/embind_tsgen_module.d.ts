// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
  _main(_0: number, _1: number): number;
}

type EmbindString = ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string;
export interface ClassHandle {
  isAliasOf(other: ClassHandle): boolean;
  delete(): void;
  deleteLater(): this;
  isDeleted(): boolean;
  // @ts-ignore - If targeting lower than ESNext, this symbol might not exist.
  [Symbol.dispose](): void;
  clone(): this;
}
export interface Test extends ClassHandle {
  x: number;
  readonly y: number;
  get stringProperty(): string;
  set stringProperty(value: EmbindString);
  functionOne(_0: number, _1: number): number;
  functionTwo(_0: number, _1: number): number;
  functionFour(_0: boolean): number;
  functionFive(x: number, y: number): number;
  constFn(): number;
  longFn(_0: number): number;
  functionThree(_0: EmbindString): number;
  functionSix(str: EmbindString): number;
}

export interface Obj extends ClassHandle {
}

export interface FirstEnumValue<T extends number> {
  value: T;
}
export type FirstEnum = FirstEnumValue<0>|FirstEnumValue<1>|FirstEnumValue<2>;

export type SecondEnum = 0|1|2;

export type ThirdEnum = 'kValueAlpha'|'kValueBeta'|'kValueGamma';

export interface EmptyEnumValue<T extends number> {
  value: T;
}
export type EmptyEnum = never/* Empty Enumerator */;

export type ValArrIx = [ FirstEnum, FirstEnum, FirstEnum, FirstEnum ];

export interface IntVec extends ClassHandle, Iterable<number> {
  push_back(_0: number): void;
  resize(_0: number, _1: number): void;
  size(): number;
  get(_0: number): number | undefined;
  set(_0: number, _1: number): boolean;
}

export interface IterableClass extends ClassHandle, Iterable<number> {
  count(): number;
  at(_0: number): number;
}

export interface MapIntInt extends ClassHandle {
  keys(): IntVec;
  get(_0: number): number | undefined;
  set(_0: number, _1: number): void;
  size(): number;
}

export interface Foo extends ClassHandle {
  process(_0: Test): void;
}

export interface ClassWithConstructor extends ClassHandle {
  fn(_0: number): number;
}

export interface ClassWithTwoConstructors extends ClassHandle {
}

export interface ClassWithSmartPtrConstructor extends ClassHandle {
  fn(_0: number): number;
}

type AliasedVal = number;

export interface BaseClass extends ClassHandle {
  fn(_0: number): number;
}

export interface DerivedClass extends BaseClass {
  fn2(_0: number): number;
}

export interface Interface extends ClassHandle {
  invoke(_0: EmbindString): void;
}

export interface InterfaceWrapper extends Interface {
  notifyOnDestruction(): void;
}

export type ValArr = [ number, number, number ];

export type ValObj = {
  string: EmbindString,
  firstEnum: FirstEnum,
  secondEnum: SecondEnum,
  thirdEnum: ThirdEnum,
  optionalInt?: number | undefined,
  callback: (message: string) => void
};

interface EmbindModule {
  Test: {
    staticFunction(_0: number): number;
    staticFunctionWithParam(x: number): number;
    staticProperty: number;
    get staticStringProperty(): string;
    set staticStringProperty(value: EmbindString);
  };
  class_returning_fn(): Test;
  class_unique_ptr_returning_fn(): Test;
  Obj: {};
  getPointer(_0: Obj | null): Obj | null;
  getNonnullPointer(): Obj;
  a_class_instance: Test;
  an_enum: FirstEnum;
  FirstEnum: {kValueOne: FirstEnumValue<0>, kValueTwo: FirstEnumValue<1>, kValueThree: FirstEnumValue<2>};
  SecondEnum: {kValueA: 0, kValueB: 1, kValueC: 2};
  ThirdEnum: {kValueAlpha: 'kValueAlpha', kValueBeta: 'kValueBeta', kValueGamma: 'kValueGamma'};
  EmptyEnum: {};
  enum_returning_fn(): FirstEnum;
  num_enum_returning_fn(): SecondEnum;
  str_enum_returning_fn(): ThirdEnum;
  IntVec: {
    new(): IntVec;
  };
  IterableClass: {
    new(): IterableClass;
  };
  MapIntInt: {
    new(): MapIntInt;
  };
  Foo: {};
  ClassWithConstructor: {
    new(_0: number, _1: ValArr): ClassWithConstructor;
  };
  ClassWithTwoConstructors: {
    new(): ClassWithTwoConstructors;
    new(_0: number): ClassWithTwoConstructors;
  };
  ClassWithSmartPtrConstructor: {
    new(_0: number, _1: ValArr): ClassWithSmartPtrConstructor;
  };
  BaseClass: {};
  DerivedClass: {};
  Interface: {
    implement(_0: any): InterfaceWrapper;
    extend(_0: EmbindString, _1: any): any;
  };
  InterfaceWrapper: {};
  function_consuming_aliased_val(_0: AliasedVal): void;
  a_bool: boolean;
  an_int: number;
  optional_test(_0?: Foo): number | undefined;
  global_fn(_0: number, _1: number): number;
  optional_and_nonoptional_test(_0: Foo | undefined, _1: number): number | undefined;
  smart_ptr_function(_0: ClassWithSmartPtrConstructor | null): number;
  smart_ptr_function_with_params(foo: ClassWithSmartPtrConstructor | null): number;
  function_with_callback_param(_0: (message: string) => void): number;
  getValObj(): ValObj;
  setValObj(_0: ValObj): void;
  string_test(_0: EmbindString): string;
  optional_string_test(_0: EmbindString): string | undefined;
  wstring_test(_0: string): string;
}

export type MainModule = WasmModule & EmbindModule;
export default function MainModuleFactory (options?: unknown): Promise<MainModule>;
