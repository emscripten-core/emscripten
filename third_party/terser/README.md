Bundled version of terser 5.18.2 with emscripten patches
========================================================

We maintain a few downstream patches to terser which means we can't use the
version published in npm.

This `terser.js` bundle in this directory was built from our fork of terser
which lives at: https://github.com/emscripten-core/terser/

The current patches are stored in the `emscripten_patches_v4.8.0` branch.

To make changes to this code please submit patches to
https://github.com/emscripten-core/terser/ and then re-create this bundle
using the following steps:

  $ CI=1 npx rollup -c
  $ cp dist/bundle.min.js $EMSCRIPTEN_ROOT/third_party/terser/terser.js
