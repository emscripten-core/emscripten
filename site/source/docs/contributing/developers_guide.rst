.. _Developer's-Guide:

=================
Developer's Guide
=================

This article provides information that is relevant to people who want to
contribute to Emscripten. We welcome contributions from anyone that is
interested in helping out!

.. tip:: The information will be less relevant if you're just using Emscripten,
   but may still be of interest.

.. _developers-guide-setting-up:

Setting up
==========

For contributing to core Emscripten code, such as ``emcc.py``, you don't need to
build any binaries as ``emcc.py`` is in Python, and the core JS generation is
in JavaScript. You do still need binaries for LLVM and Binaryen, which you can
get using the emsdk:

::

    emsdk install tot
    emsdk activate tot

This with install the latest "tip-of-tree" binaries needed to run Emscripten.
You can use these emsdk-provided binaries with a git checkout of the Emscripten
repository.  To do this, you can either edit your local ``.emscripten`` config
file, or set ``EM_CONFIG=/path/to/emsdk/.emscripten`` in your environment.

If you do want to contribute to LLVM or Binaryen, or to test modifications
to them, you can
:ref:`build them from source <installing-from-source>`.

Repositories and branches of interest
=====================================

The Emscripten main repository is https://github.com/emscripten-core/emscripten.

Aside from the Emscripten repo, the other codebases of interest are LLVM
and Binaryen, which Emscripten invokes, and
:ref:`have their own repos <installing-from-source>`.

.. _developers-guide-submitting-patches:

Submitting patches
==================

Patches should be submitted as *pull requests* in the normal way on GitHub.

When submitting patches, please:

- Add an automatic test if you add any new functionality or fix a bug. Search
  in ``test/*.py`` for related tests, as often the simplest thing is to add to
  an existing one. If you're not sure how to test your code, feel free to ask
  for help.
- Pay attention to our coding style specified in
  `.clang-format <https://github.com/emscripten-core/emscripten/blob/main/.clang-format>`_.
- We normally squash and merge PRs, which means the PR turns into a single
  commit on the target branch. Because of that, it's ok to have merge commits
  in the PR itself, as they get removed. Please put a good description for
  the final commit in the PR description, and we'll use it when squashing.

Code reviews
============

One of the core developers will review a pull request before merging it. If
several days pass without any comments on your PR, please comment in the PR
which will ping them. (If that happens, sorry! Sometimes things get missed.)

Compiler overview
=================

The :ref:`Emscripten Compiler Frontend (emcc) <emccdoc>` is a python script that manages the entire compilation process:

- **emcc** calls :term:`Clang` to compile C++ and ``wasm-ld`` to link it. It
  builds and integrates with the Emscripten system libraries, both the
  compiled ones and the ones implemented in JS.
- **emcc** then calls `emscripten.py <https://github.com/emscripten-core/emscripten/blob/main/emscripten.py>`_
  which performs the final transformation to Wasm (including invoking
  **wasm-emscripten-finalize** from Binaryen) and calls the JS compiler
  (see ``src/compiler.mjs`` and related files) which emits the JS.
- If optimizing Wasm, **emcc** will then call **wasm-opt**, run meta-dce, and
  other useful things. It will also run JS optimizations on the JS that is
  emitted alongside the Wasm.

Emscripten Test Suite
=====================

Emscripten has a :ref:`comprehensive test suite <emscripten-test-suite>`, which
covers virtually all Emscripten functionality. These tests are run on CI
automatically when you create a pull request, and they should all pass. If you
run into trouble with a test failure you can't fix, please let the developers
know.

Bisecting
=========

If you find a regression, bisection is often the fastest way to figure out what
went wrong. This is true not just for finding an actual regression in Emscripten
but also if your project stopped working when you upgrade, and you need to
investigate if it's an Emscripten regression or something else. The rest of
this section covers bisection on Emscripten itself. It is hopefully useful for
both people using Emscripten as well as Emscripten developers.

If you have a large bisection range - for example, that covers more than one
version of Emscripten - then you probably have changes across multiple repos
(Emscripten, LLVM, and Binaryen). In that case the easiest and fastest thing
is to bisect using **emsdk builds**. Each step of the bisection will download
a build produced by the emscripten releases builders. Using this approach you
don't need to compile anything yourself, so it can be very fast!

To do this, you need a basic understanding of Emscripten's
`release process <https://github.com/emscripten-core/emscripten/blob/main/docs/process.md#release-processes>`_
The key idea is that::

     emsdk install [HASH]

can install an arbitrary build of emscripten from any point in the past (assuming
the build succeeded). Each build is identified by a hash (a long string of numbers
and characters), which is a hash of a commit in the
`releases repo <https://chromium.googlesource.com/emscripten-releases>`_.
The mapping of Emscripten release numbers to such hashes is tracked by
`emscripten-releases-tags.json in the emsdk repo <https://github.com/emscripten-core/emsdk/blob/main/emscripten-releases-tags.json>`_.

With that background, the bisection process would look like this:

1. Find the hashes to bisect between. You may already know them if you found
   the problem on ``tot`` builds. If instead you only know Emscripten version
   numbers, use ``emscripten-releases-tags.json`` to find the hashes.
2. Using those hashes, do a normal ``git bisect`` on the ``emscripten-releases``
   repo.
3. In each step of the bisection, download the binary build for the current
   commit hash (in the ``emscripten-releases`` repo that you are bisecting on)
   using ``emsdk install HASH``. Then test your code and do
   ``git bisect good`` or ``git bisect bad`` accordingly, and keep bisecting
   until you find the first bad commit.

The first bad commit is a single change in the releases repo. That commit will
generally update a single sub-repo (Emscripten, LLVM, or Binaryen) to add
one or more new changes. Often that list will be very short or even a single
commit, and you can see which actual commit caused the problem. When filing
a bug, mentioning such a bisection result can greatly speed things up (even if
that commit contains multiple changes).

If that commit contains multiple changes then you can optionally bisect
further on the specific repo (as all the changes will normally be in just
one of them, with the others kept fixed). Doing this will require rebuilding
locally, which was not needed in the main bisection described in this
section.

Working with C structs and defines
==================================

If you change the layout of C structs or modify C defines that are used in
JavaScript library files you may need to modify ``tools/struct_info.json``.  Any
time that file is modified or a struct layout is changed you will need to run
``./tools/gen_struct_info.py`` to re-generate the information used by
JavaScript. Note that you need to run both ``./tools/gen_struct_info.py`` and
``./tools/gen_struct_info.py --wasm64``.

The ``test_gen_struct_info`` test will fail if you forget to do this.

See also
========

- :ref:`Debugging`
- :ref:`Building-Projects`
