Packaging Emscripten
====================

This document is designed to aid in packaging emscripten.  For example within
linux distributions, or other downstream SDKs.

Firstly, we provide an install script in `tools/install.py`.  This allows
just the end-user parts of emscripten to be installed.  For example this avoids
including the `test/third_party` directory which is one of the biggest parts
of the source tree.

If you prefer, you can use `make install` or `make dist` which invokes
`tools/install.py`.

Dependencies
------------

One important thing to note here is that emscripten doesn't currently support
the stable LLVM releases.  It requires a build of LLVM from top-of-tree, or at
least very close to it.  This means that depending on a packaged version of LLVM
is unlikely to work.

The core
[DEPS](https://chromium.googlesource.com/emscripten-releases/+/refs/heads/master/DEPS)
file in the `emscripten-releases` repository contains all the information about
the various repositories that go into an emscripten release. These are the repos
used by our CI to build and test emscripten.  To find out which revisions went
into a given release you need to check out the DEPS file that the revision
corresponding to that release.  It has lines like these:

```
  'binaryen_revision': '06698d7a32cb4eeb24fea942e83d1b15e86a73e6',
  'emscripten_revision': '7224b7930ec2a6abca332300e247619e1aea1719',
  'llvm_project_revision': '33ef687d94604aeb73bedbcf3050524465a3439f',
```

These lines specify git hashes of the various repositories that make up the
release.

In principle you can build any git hash in that repo because all updates get
fully tested before they are committed. You can look at the
[CI UI](https://ci.chromium.org/p/emscripten-releases/g/main/console) to see
if our CI shows green for any hash, and if so, it should be safe to build.

Alternatively, you may want to build our official emscripten release tags, which
are the versions the emsdk lets users install. To find the mapping between the
emsdk versions and the revision of the emscripten-releases repository, the emsdk
has
[emscripten-releases-tags.json](https://github.com/emscripten-core/emsdk/blob/main/emscripten-releases-tags.json).
All versions listed there should be safe to build, as we check that the CI
was green on them.

To see how our CI builds things, the relevant script is
[build.py](https://github.com/WebAssembly/waterfall/blob/master/src/build.py).
In general, the repos you need to build are LLVM and Binaryen (as emscripten
itself doesn't have any binaries to build).

When packaging build results, you need the following executables:

  * From LLVM:
    * clang
    * clang++ (note: this is a symlink to clang)
    * wasm-ld
    * llc
    * llvm-nm
    * llvm-ar
    * llvm-as
    * llvm-dis
    * llvm-link
    * llvm-dwarfdump
  * From Binaryen:
    * wasm-emscripten-finalize
    * wasm-opt
    * wasm-dis
    * wasm-as
    * wasm2js
    * wasm-metadce

Node modules
------------

In addition to the dependencies listed above, emscripten also has node module
dependencies specified in `package.json`.  These should be be installed by
running `npm install` inside the emscripten directory.

This will create a `node_modules` subdirectory containing the required
dependencies.  This directory should be packaged along with emscripten.  We
don't currently support these modules being installed in other locations but
supporting system-wide node modules is something that could be added if it is
deemed useful.


Configuration
-------------

For most distributions it will make sense to ship a configuration file along
with emscripten.  If you don't do this then emscripten will attempt to guide the
user through the configuration process on first use, which is likely
undesirable in the packaged environment.

We suggest you ship a `.emscripten` configuration file inside the emscripten
directory itself.  Emscripten will look here first before looking for
`~/.emscripten`.

The key configuration setting are requires are:

 * `LLVM_ROOT`: The path to the LLVM binaries.
 * `BINARYEN_ROOT`: The path to binaryen (the binaries are expected in `/bin`
   under there; note that despite the name this differs from `LLVM_ROOT` which
   points directly to the binaries).
 * `NODE_JS`: The path to the node.js executable. This is needed internally.

Ports
-----

If you package emscripten and don't want the ports system to be used, you can
simply delete the `tools/ports/` directory.

Prebuilt libraries
------------------

Ideally a packaged installation can include a fully populated cache directory
containing pre-built libraries.  If you want to do this you can use
`embuilder build ALL` to populate the cache directory.  You can then ship the
`cache` directory inside the emscripten directory.  When shipping the cache
directory on a multi-user system where users cannot modify the `cache` you need
to be sure that all possible configurations of the libraries are built.
Currently that means running:

```
embuilder build ALL
embuilder build ALL --lto
embuilder build ALL --pic
embuilder build ALL --pic --lto
```

Existing Packages
-----------------

See https://emscripten.org/docs/getting_started/downloads.html#installation-using-packages
