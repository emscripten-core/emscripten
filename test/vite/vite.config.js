export default {
  base: './',
  build: {
    rollupOptions: {
      onwarn(warning, defaultHandler) {
        // Treat externalized-for-browser-compatibility warnings as errors.
        // This catches require() in ESM output that vite silently externalizes.
        if (warning.message && warning.message.includes('externalized for browser compatibility')) {
          throw new Error(warning.message);
        }
        defaultHandler(warning);
      },
    },
  },
}
