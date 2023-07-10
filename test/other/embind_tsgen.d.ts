export interface Test {
  functionOne(_0: number, _1: number): number;
  functionTwo(_0: number, _1: number): number;
  functionFour(_0: boolean): number;
  constFn(): number;
  functionThree(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): number;
  delete(): void;
}

export interface BarValue<T extends number> {
  value: T;
}
export type Bar = BarValue<0>|BarValue<1>|BarValue<2>;

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

export interface MainModule {
  Test: {new(): Test};
  class_returning_fn(): Test;
  class_unique_ptr_returning_fn(): Test;
  a_class_instance: Test;
  an_enum: Bar;
  Bar: {valueOne: BarValue<0>, valueTwo: BarValue<1>, valueThree: BarValue<2>};
  enum_returning_fn(): Bar;
  IntVec: {new(): IntVec};
  Foo: {new(): Foo};
  ClassWithConstructor: {new(_0: number, _1: ValArr): ClassWithConstructor};
  ClassWithSmartPtrConstructor: {new(_0: number, _1: ValArr): ClassWithSmartPtrConstructor};
  BaseClass: {new(): BaseClass};
  DerivedClass: {new(): DerivedClass};
  a_bool: boolean;
  an_int: number;
  global_fn(_0: number, _1: number): number;
  smart_ptr_function(_0: ClassWithSmartPtrConstructor): number;
}
