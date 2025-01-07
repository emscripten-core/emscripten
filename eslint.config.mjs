import globals from 'globals';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import js from '@eslint/js';
import { FlatCompat } from '@eslint/eslintrc';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const compat = new FlatCompat({
  baseDirectory: __dirname,
  recommendedConfig: js.configs.recommended,
  allConfig: js.configs.all
});

export default [{
  ignores: [
    '**/out/',
    '**/site/',
    '**/cache/',
    '**/third_party/',
    '**/test/',
    'src/polyfill/',
    'src/library*.js',
    'src/runtime_*.js',
    'src/shell*.js',
    'src/preamble*.js',
    'src/postamble*.js',
    'src/closure-externs/',
    'src/embind/',
    'src/emrun_postjs.js',
    'src/wasm_worker.js',
    'src/audio_worklet.js',
    'src/wasm2js.js',
    'src/webGLClient.js',
    'src/webGLWorker.js',
    'src/*_shell_read.js',
    'src/wasm_offset_converter.js',
    'src/threadprofiler.js',
    'src/cpuprofiler.js',
    'src/memoryprofiler.js',
    'src/gl-matrix.js',
    'src/headless.js',
    'src/headlessCanvas.js',
    'src/emscripten-source-map.min.js',
    'src/source_map_support.js',
    'src/Fetch.js',
    'src/settings.js',
    'src/settings_internal.js',
    'src/arrayUtils.js',
    'src/deterministic.js',
    'src/base64Utils.js',
    'src/base64Decode.js',
    'src/proxyWorker.js',
    'src/proxyClient.js',
    'src/IDBStore.js',
    'src/URIUtils.js',
    'tools/experimental',
  ],
}, ...compat.extends('prettier'), {
  languageOptions: {
    globals: {
      ...globals.browser,
      ...globals.node,
    },

    ecmaVersion: 13,
    sourceType: 'module',
  },

  rules: {
    'max-len': 'off',
    'no-multi-spaces': 'off',
    'require-jsdoc': 'off',
    'arrow-body-style': ['error', 'as-needed'],
    'space-infix-ops': 'error',

    quotes: ['error', 'single', {
      avoidEscape: true,
    }],
  },
}, {
  files: ['**/*.mjs'],

  rules: {
    'no-unused-vars': ['error', {
      vars: 'all',
      args: 'none',
      ignoreRestSiblings: false,
      destructuredArrayIgnorePattern: '^_',
    }],
  },
}];
