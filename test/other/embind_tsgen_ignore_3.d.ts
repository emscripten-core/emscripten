// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
declare namespace RuntimeExports {
    let HEAPF32: any;
    let HEAPF64: any;
    let HEAP_DATA_VIEW: any;
    let HEAP8: any;
    let HEAPU8: any;
    let HEAP16: any;
    let HEAPU16: any;
    let HEAP32: any;
    let HEAPU32: any;
    let HEAP64: any;
    let HEAPU64: any;
}
interface WasmModule {
  _main(_0: number, _1: number): number;
}

type EmbindString = ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string;
export interface Test {
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
  delete(): void;
}

export interface Obj {
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
  get(_0: number): number | undefined;
  set(_0: number, _1: number): boolean;
  delete(): void;
}

export interface MapIntInt {
  keys(): IntVec;
  get(_0: number): number | undefined;
  set(_0: number, _1: number): void;
  size(): number;
  delete(): void;
}

export interface Foo {
  process(_0: Test): void;
  delete(): void;
}

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

export type ValObj = {
  foo: Foo,
  bar: Bar,
  callback: (message: string) => void
};

export interface BaseClass {
  fn(_0: number): number;
  delete(): void;
}

export interface DerivedClass extends BaseClass {
  fn2(_0: number): number;
  delete(): void;
}

export interface Interface {
  invoke(_0: EmbindString): void;
  delete(): void;
}

export interface InterfaceWrapper extends Interface {
  notifyOnDestruction(): void;
  delete(): void;
}

export type ValArr = [ number, number, number ];

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
  an_enum: Bar;
  Bar: {valueOne: BarValue<0>, valueTwo: BarValue<1>, valueThree: BarValue<2>};
  EmptyEnum: {};
  enum_returning_fn(): Bar;
  IntVec: {
    new(): IntVec;
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
  a_bool: boolean;
  an_int: number;
  optional_test(_0?: Foo): number | undefined;
  global_fn(_0: number, _1: number): number;
  optional_and_nonoptional_test(_0: Foo | undefined, _1: number): number | undefined;
  smart_ptr_function(_0: ClassWithSmartPtrConstructor | null): number;
  smart_ptr_function_with_params(foo: ClassWithSmartPtrConstructor | null): number;
  function_with_callback_param(_0: (message: string) => void): number;
  string_test(_0: EmbindString): string;
  wstring_test(_0: string): string;
}

export type MainModule = WasmModule & typeof RuntimeExports & EmbindModule;
