/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// A babel plugin to remove the leading `node:` prefix from all imports.

export default function ({ types: t, targets }) {
  // Skip this plugin for Node.js >= 16
  if (+targets().node.split('.')[0] >= 16) {
    return {};
  }

  return {
    name: 'strip-node-prefix',
    visitor: {
      // e.g. `import fs from 'node:fs'`
      ImportDeclaration({ node }) {
        if (node.source.value.startsWith('node:')) {
          node.source.value = node.source.value.slice(5);
        }
      },

      // e.g. `await import('node:fs')`
      // Note: only here for reference, it's mangled with EMSCRIPTEN$AWAIT$IMPORT below.
      ImportExpression({ node }) {
        if (t.isStringLiteral(node.source) && node.source.value.startsWith('node:')) {
          node.source.value = node.source.value.slice(5);
        }
      },

      // e.g. `require('node:fs')` or EMSCRIPTEN$AWAIT$IMPORT('node:fs')
      CallExpression({ node }) {
        if (
          (t.isIdentifier(node.callee, { name: 'require' }) ||
            // Special placeholder for `await import`
            // FIXME: Remove after PR https://github.com/emscripten-core/emscripten/pull/23730 is landed.
            t.isIdentifier(node.callee, { name: 'EMSCRIPTEN$AWAIT$IMPORT' })) &&
          t.isStringLiteral(node.arguments[0]) &&
          node.arguments[0].value.startsWith('node:')
        ) {
          node.arguments[0].value = node.arguments[0].value.slice(5);
        }
      },
    },
  };
}
