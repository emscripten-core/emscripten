import emscriptenPlugin from 'rollup-plugin-emscripten';

export default {
  input: 'index.mjs',
  output: {
    dir: 'dist',
    format: 'es'
  },
  plugins: [
    emscriptenPlugin({
      'input': 'library.mjs',
      'wasmMetaDCE': 'BINARYEN_PATH'
    })
  ]
};
