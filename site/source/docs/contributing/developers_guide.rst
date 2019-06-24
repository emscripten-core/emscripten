.. _Developer's-Guide:

=================
Developer's Guide
=================

This article provides information that is relevant to people working on Emscripten itself, as opposed to those that use Emscripten in their own projects.

.. tip:: The information will be less relevant if you're just using Emscripten, but may still be of interest.


Building Emscripten from source code
====================================

As a contributor you will need to :ref:`build Emscripten from source <installing-from-source>` in order to integrate fixes back into the main repositories.

Repositories and branches of interest
=====================================

The Emscripten main repository is https://github.com/emscripten-core/emscripten. There are two main branches:

-  **master** - The "master" branch. This is always safe to pull from and the test suite always passes.
-  **incoming** - The branch for new code. Code in **incoming** is merged with the **master** only after it is code reviewed and has passed all the automated tests.

Emscripten with the LLVM wasm backend can use normal upstream LLVM, at https://github.com/llvm/llvm-project (or, with the older fastcomp backend, it is in https://github.com/emscripten-core/emscripten-fastcomp and https://github.com/emscripten-core/emscripten-fastcomp-clang).

.. _developers-guide-submitting-patches:

Submitting patches
==================

Patches should be submitted as *pull requests* to the **incoming** branch.

.. note:: Before submitting your first patch, add yourself to the `AUTHORS <https://github.com/emscripten-core/emscripten/blob/incoming/AUTHORS>`_ file. By doing so, you agree to license your code under the project's :ref:`open source licenses (MIT/LLVM) <emscripten-license>`.

When submitting patches, please:

- Make pull requests to **incoming**, not master.
- Do not include merge commits in pull requests; include only commits with the new relevant code.
- :ref:`Run all the automatic tests <emscripten-test-suite>` and make sure they pass.  Some tests might not be required for very simple patches (for example, when just adding tests for new library functions). If you don't have time to run all the tests, at least run a random subset of them (say, 100), see that link.
- "Add an automatic test to `tests/runner.py <https://github.com/emscripten-core/emscripten/blob/master/tests/runner.py>`_ if you add any new functionality or fix a bug.
- Record the tests that were run in the pull request or issue.


Code reviews
============

`@kripken <https://github.com/kripken>`_ reviews all pull requests before merging.

Exceptions are sub-projects that are 'owned' by other people. These owners can push to incoming directly:

-  OpenAL and audio in general: `@ehsan <https://github.com/ehsan>`_
-  embind: `@imvu <https://github.com/imvu>`_
-  Windows stuff: `@juj <https://github.com/juj>`_


Compiler overview
=================

The :ref:`Emscripten Compiler Frontend (emcc) <emccdoc>` is a python script that manages the entire compilation process:

- *emcc* calls :term:`Clang` to convert C++ to bitcode, ``llvm-opt`` to optimize it, ``llvm-link`` to link it, etc.
- *emcc* then calls `emscripten.py <https://github.com/emscripten-core/emscripten/blob/master/emscripten.py>`_, which performs any final compilation to wasm (including invoking *wasm-emscripten-finalize* from Binaryen, for wasm) and emits the JavaScript.
- If optimizing wasm, *emcc* will then call Binaryen's optimizer, run meta-dce, and other useful things. It will also run JS optimizations on the JS that is emitted alongside the wasm.

Emscripten Test Suite
=====================

Emscripten has a :ref:`comprehensive test suite <emscripten-test-suite>`, which covers virtually all Emscripten functionality. These tests must all pass when you are :ref:`submitting patches <developers-guide-submitting-patches>`.

See also
========

- :ref:`Debugging`
- :ref:`Building-Projects`


