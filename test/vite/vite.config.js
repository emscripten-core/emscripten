export default {
  base: './',
  build: {
    rollupOptions: {
      onwarn(warning, defaultHandler) {
        // Vite externalizes node built-in imports (node:fs, etc.) for browser
        // compatibility. This is expected for dynamic import() calls guarded
        // by ENVIRONMENT_IS_NODE. However, require() calls in ESM output are
        // truly broken — vite cannot handle them. Detect require()-based
        // externalization by checking for imports that don't use the node: scheme.
        if (warning.message && warning.message.includes('externalized for browser compatibility')) {
          // Accept node: scheme imports (dynamic import with bundler hints)
          var match = warning.message.match(/Module "([^"]+)"/);
          if (match && !match[1].startsWith('node:')) {
            throw new Error(warning.message);
          }
        }
        defaultHandler(warning);
      },
    },
  },
}
