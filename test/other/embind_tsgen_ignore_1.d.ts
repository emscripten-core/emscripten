// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
declare namespace RuntimeExports {
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
    function keepRuntimeAlive(): any;
    /** @constructor */
    function ExitStatus(status: any): void;
    let wasmMemory: any;
    let FS_createPath: any;
    function FS_createDataFile(parent: any, name: any, fileData: any, canRead: any, canWrite: any, canOwn: any): void;
    function FS_createPreloadedFile(parent: any, name: any, url: any, canRead: any, canWrite: any, onload: any, onerror: any, dontCreateFile: any, canOwn: any, preFinish: any): void;
    function FS_unlink(path: any): any;
    let FS_createLazyFile: any;
    let FS_createDevice: any;
    let addRunDependency: any;
    let removeRunDependency: any;
}
interface WasmModule {
  _pthread_self(): number;
  _main(_0: number, _1: number): number;
  __emscripten_tls_init(): number;
  __emscripten_proxy_main(_0: number, _1: number): number;
  __embind_initialize_bindings(): void;
  __emscripten_thread_init(_0: number, _1: number, _2: number, _3: number, _4: number, _5: number): void;
  __emscripten_thread_crashed(): void;
  __emscripten_thread_exit(_0: number): void;
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
  MapIntInt: {new(): MapIntInt};
  Foo: {};
  ClassWithConstructor: {new(_0: number, _1: ValArr): ClassWithConstructor};
  ClassWithTwoConstructors: {new(): ClassWithTwoConstructors; new(_0: number): ClassWithTwoConstructors};
  ClassWithSmartPtrConstructor: {new(_0: number, _1: ValArr): ClassWithSmartPtrConstructor};
  BaseClass: {};
  DerivedClass: {};
  a_bool: boolean;
  an_int: number;
  optional_test(_0: Foo | undefined): number | undefined;
  global_fn(_0: number, _1: number): number;
  smart_ptr_function(_0: ClassWithSmartPtrConstructor): number;
  smart_ptr_function_with_params(foo: ClassWithSmartPtrConstructor): number;
  function_with_callback_param(_0: (message: string) => void): number;
  string_test(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): string;
  wstring_test(_0: string): string;
}
export type MainModule = WasmModule & typeof RuntimeExports & EmbindModule;
