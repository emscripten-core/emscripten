import { importMetaAssets } from '@web/rollup-plugin-import-meta-assets';

export default {
  input: 'src/index.mjs',
  output: {
    dir: 'dist',
    format: 'es',
  },
  plugins: [importMetaAssets()],
};
