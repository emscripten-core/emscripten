import globals from 'globals';
import js from '@eslint/js';
import { FlatCompat } from '@eslint/eslintrc';
import { loadDefaultSettings } from './src/utility.mjs';

const compat = new FlatCompat({
  baseDirectory: import.meta.dirname,
  recommendedConfig: js.configs.recommended,
  allConfig: js.configs.all
});


// Emscripten settings are made available to the compiler as global
// variables.  Make sure eslint knows about them.
const settings = loadDefaultSettings();
const settingsGlobals = {};
for (const name of Object.keys(settings)) {
  settingsGlobals[name] = 'writable';
}

export default [{
  ignores: [
    '**/out/',
    '**/site/',
    '**/cache/',
    '**/third_party/',
    '**/test/',
    'src/polyfill/',
    'src/lib/',
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
    'src/growableHeap.js',
    'src/emrun_prejs.js',
    'src/arrayUtils.js',
    'src/deterministic.js',
    'src/base64Decode.js',
    'src/proxyWorker.js',
    'src/proxyClient.js',
    'src/IDBStore.js',
    'src/URIUtils.js',
    'tools/experimental',
  ],
}, ...compat.extends('prettier'), js.configs.recommended, {
  languageOptions: {
    globals: {
      ...globals.browser,
      ...globals.node,
      ...settingsGlobals,
    },

    ecmaVersion: 'latest',
    sourceType: 'module',
  },

  rules: {
    'max-len': 'off',
    'no-multi-spaces': 'off',
    'require-jsdoc': 'off',
    'arrow-body-style': ['error', 'as-needed'],
    'space-infix-ops': 'error',
    'no-prototype-builtins': 'off',

    quotes: ['error', 'single', {
      avoidEscape: true,
    }],
  },
}, {
  files: ['**/*.mjs'],

  rules: {
    'no-undef': 'error',
    'no-unused-vars': ['error', {
      vars: 'all',
      args: 'none',
      ignoreRestSiblings: false,
      destructuredArrayIgnorePattern: '^_',
    }],
  },
}];
