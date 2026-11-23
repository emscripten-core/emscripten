import {defineConfig} from 'rolldown';

export default defineConfig({
  input: [
    'tools/compiler.mjs',
    'tools/preprocessor.mjs',
    'tools/acorn-optimizer.mjs',
    'tools/unsafe_optimizations.mjs',
    'tools/lz4-compress.mjs',
  ],
  platform: 'node',
  output: {
    dir: 'dist',
    format: 'esm',
    cleanDir: true,
    entryFileNames: '[name].mjs',
    chunkFileNames: '[name]-[hash].mjs',
  },
});
