// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
}

export interface Test {
  x: number;
  readonly y: number;
  functionOne(_0: number, _1: number): number;
  functionTwo(_0: number, _1: number): number;
  functionFour(_0: boolean): number;
  functionFive(x: number, y: number): number;
  constFn(): number;
  longFn(_0: number): number;
  functionThree(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): number;
  functionSix(str: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): number;
  delete(): void;
}

export interface BarValue<T extends number> {
  value: T;
}
export type Bar = BarValue<0>|BarValue<1>|BarValue<2>;

export interface EmptyEnumValue<T extends number> {
  value: T;
}
export type EmptyEnum = never/* Empty Enumerator */;

export type ValArrIx = [ Bar, Bar, Bar, Bar ];

export interface IntVec {
  push_back(_0: number): void;
  resize(_0: number, _1: number): void;
  size(): number;
  set(_0: number, _1: number): boolean;
  get(_0: number): any;
  delete(): void;
}

export interface Foo {
  process(_0: Test): void;
  delete(): void;
}

export type ValObj = {
  foo: Foo,
  bar: Bar
};

export interface ClassWithConstructor {
  fn(_0: number): number;
  delete(): void;
}

export interface ClassWithTwoConstructors {
  delete(): void;
}

export interface ClassWithSmartPtrConstructor {
  fn(_0: number): number;
  delete(): void;
}

export interface BaseClass {
  fn(_0: number): number;
  delete(): void;
}

export interface DerivedClass extends BaseClass {
  fn2(_0: number): number;
  delete(): void;
}

export type ValArr = [ number, number, number ];

interface EmbindModule {
  Test: {staticFunction(_0: number): number; staticFunctionWithParam(x: number): number; staticProperty: number};
  class_returning_fn(): Test;
  class_unique_ptr_returning_fn(): Test;
  a_class_instance: Test;
  an_enum: Bar;
  Bar: {valueOne: BarValue<0>, valueTwo: BarValue<1>, valueThree: BarValue<2>};
  EmptyEnum: {};
  enum_returning_fn(): Bar;
  IntVec: {new(): IntVec};
  Foo: {};
  ClassWithConstructor: {new(_0: number, _1: ValArr): ClassWithConstructor};
  ClassWithTwoConstructors: {new(): ClassWithTwoConstructors; new(_0: number): ClassWithTwoConstructors};
  ClassWithSmartPtrConstructor: {new(_0: number, _1: ValArr): ClassWithSmartPtrConstructor};
  BaseClass: {};
  DerivedClass: {};
  a_bool: boolean;
  an_int: number;
  global_fn(_0: number, _1: number): number;
  optional_test(_0: Foo | undefined): number | undefined;
  smart_ptr_function(_0: ClassWithSmartPtrConstructor): number;
  smart_ptr_function_with_params(foo: ClassWithSmartPtrConstructor): number;
  function_with_callback_param(_0: (message: string) => void): number;
  string_test(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): string;
  wstring_test(_0: string): string;
}
export type MainModule = WasmModule & EmbindModule;
