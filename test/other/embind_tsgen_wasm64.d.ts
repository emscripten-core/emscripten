// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
}

interface EmbindModule {
  longFn(_0: bigint): bigint;
}

export type MainModule = WasmModule & EmbindModule;
