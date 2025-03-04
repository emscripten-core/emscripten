/**
 * @license
 * Copyright 2025 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $MAPFS__deps: ['$stringToUTF8OnStack', 'wasmfs_create_map_backend', 'wasmfs_map_create_manifest', 'wasmfs_map_add_to_manifest'],
  $MAPFS: {
    createBackend(opts) {
      var manifest = 0;
      if (opts['manifest']) {
        manifest = _wasmfs_map_create_manifest();
        Object.entries(opts['manifest']).forEach(([path, dest]) => {
          withStackSave(() => {
            _wasmfs_map_add_to_manifest(manifest,
                                        stringToUTF8OnStack(path),
                                        stringToUTF8OnStack(dest));
          });
        });
        return _wasmfs_create_map_backend(manifest);
      };
    },
  },
});

if (!WASMFS) {
  error("using -lmapfs.js requires using WasmFS (-sWASMFS)");
}
