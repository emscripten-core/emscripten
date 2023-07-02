export interface Test {
  functionOne(_0: number, _1: number): number;
  functionTwo(_0: number, _1: number): number;
  functionFour(_0: boolean): number;
  constFn(): number;
  functionThree(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): number;
  delete(): void;
}

export interface MainModule {
  Test: {new(): Test};
  global_fn(_0: number, _1: number): number;
}
