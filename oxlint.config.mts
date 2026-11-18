import {defineConfig} from 'oxlint';
import {loadDefaultSettings} from './src/utility.ts';

// Emscripten settings are made available to the compiler as global
// variables.  Make sure oxlint knows about them.
const settings = loadDefaultSettings();
const settingsGlobals: Record<string, 'writable'> = {};
for (const name of Object.keys(settings)) {
  settingsGlobals[name] = 'writable';
}

export default defineConfig({
  plugins: ['typescript', 'oxc', 'unicorn', 'node', 'import', 'promise'],
  categories: {
    correctness: 'error',
    perf: 'error',
  },
  env: {
    browser: true,
    node: true,
  },
  globals: settingsGlobals,
  ignorePatterns: [
    '**/*.js',
    '**/*.cjs',
    'src/pthread_esm_startup.mjs',
    '**/out/',
    '**/dist/',
    '**/site/',
    '**/cache/',
    '**/third_party/',
    '**/test/',
    'tools/experimental/',
  ],
  rules: {
    'arrow-body-style': ['error', 'as-needed'],
    'prefer-object-has-own': 'error',
    'prefer-exponentiation-operator': 'error',
    'prefer-object-spread': 'error',
    'prefer-rest-params': 'error',
    'prefer-spread': 'error',
    'logical-assignment-operators': ['error', 'always'],
    'no-unused-vars': [
      'error',
      {
        argsIgnorePattern: '^_',
        destructuredArrayIgnorePattern: '^_',
      },
    ],
    'no-unexpected-multiline': 'error',
    'no-case-declarations': 'error',
    'no-empty': 'error',
    'no-fallthrough': 'error',
    'no-prototype-builtins': 'error',
    'no-redeclare': 'error',
    'no-regex-spaces': 'error',
    'no-undef': 'error',
    'no-useless-assignment': 'error',
    'preserve-caught-error': 'error',
  },
});
