import metaUrlPlugin from '@chialab/esbuild-plugin-meta-url';
import * as esbuild from 'esbuild';

await esbuild.build({
  entryPoints: ['src/index.mjs'],
  bundle: true,
  outdir: 'dist',
  format: 'esm',
  logLevel: 'info',
  plugins: [metaUrlPlugin()],
});
