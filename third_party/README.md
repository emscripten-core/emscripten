# Emscripten Third Party Code

This directory contains vendored third-party libraries and tools used by
Emscripten's compiler, runtime, and tools.

- **`jni/`**: JNI headers (`jni.h`) for `emjvm`.
- **`leb128/`**: LEB128 integer compression module. See [leb128/README.md](file:///usr/local/google/home/sbc/dev/wasm/emscripten/third_party/leb128/README.md).
- **`mini-lz4.js`**: MiniLZ4 block decoding and encoding (based on [node-lz4](https://github.com/pierrec/node-lz4) by Pierre Curto).
- **`ply/`**: Python PLY (Lex-Yacc) library. See [ply/README](file:///usr/local/google/home/sbc/dev/wasm/emscripten/third_party/ply/README).
- **`stb_image.h`**: Public domain image loader (`v2.08`) by Sean Barrett ([stb_image.h](http://nothings.org/stb_image.h)).
- **`terser/`**: Bundled down-stream fork of Terser. See [terser/README.md](file:///usr/local/google/home/sbc/dev/wasm/emscripten/third_party/terser/README.md).
- **`WebIDL.py`**: WebIDL parser from Mozilla (`mozilla-central`).
