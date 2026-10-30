import {defineConfig} from 'rolldown';

export default defineConfig({
  input: [
    'tools/compiler.ts',
    'tools/preprocessor.ts',
    'tools/acorn-optimizer.ts',
    'tools/unsafe_optimizations.ts',
    'tools/lz4-compress.ts',
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
