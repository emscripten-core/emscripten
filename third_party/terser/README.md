Bundled version of terser 4.1.0 with emscripten patches
=======================================================

We maintain a few downstream patches to terser which means we can't use the
version published in npm.

This `terser.js` bundle in this directory was built from our fork of terser
which lives at: https://github.com/emscripten-core/terser/

The current patches are stored in the `emscripten_patches_v4.8.0` branch.

To make changes to this code please submit patches to
https://github.com/emscripten-core/terser/ and then re-create this bundle
using the following steps:

  $ CI=1 npm run build
  $ cp dist/bundle.min.js $EMSCRIPTEN_ROOT/third_party/terser/terser.js

Despite its name, `dist/bundle.min.js` should not be minified (due to the `CI=1`
in the command avove).
