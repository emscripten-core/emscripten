// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
  _fooVoid(): void;
  _fooInt(_0: number, _1: number): number;
  _main(_0: number, _1: number): number;
}

export type MainModule = WasmModule;
export default function MainModuleFactory (options?: unknown): MainModule;
