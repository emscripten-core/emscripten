// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
  _fooVoid(): void;
  _fooInt(_0: number, _1: number): number;
  __start(): void;
}

export type MainModule = WasmModule;
