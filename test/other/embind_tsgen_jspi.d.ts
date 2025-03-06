// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
}

interface EmbindModule {
  sleep(): Promise<void>;
}

export type MainModule = WasmModule & EmbindModule;
