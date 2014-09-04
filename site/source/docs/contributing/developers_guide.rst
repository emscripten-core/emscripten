.. _Developer's-Guide:

===============================
Developer's Guide (wiki-import)
===============================

This page is for people working on Emscripten itself, as opposed to those that use Emscripten on their own projects. If you are just using Emscripten, you probably don't need to know this stuff (but it might help out sometime too).

Basic useful stuff
==================

Here are some example commands for the test suite, with comments explaining what they do

::

    python tests/runner.py test_hello_world         # run hello world test, in default mode
    python tests/runner.py asm1.test_hello_world    # run it in asm1 mode (see test_core.py end for definition)
    python tests/runner.py ALL.test_hello_world     # run it in all modes
    python tests/runner.py asm1                     # run all (core) tests in asm1 mode
    python tests/runner.py                          # run all (core) tests

    python tests/runner.py other                    # run "other" tests - that have no mode
    python tests/runner.py other.test_static_link   # run a specific test in "other"

    python tests/runner.py browser                  # run browser tests
    python tests/runner.py browser.test_sdlglshader # run a specific browser test

    python tests/runner.py sockets                  # run network tests. sockets.test_... works too

    python tests/runner.py sanity                   # run sanity tests. sanity.test_... works too

    python tests/runner.py benchmark                # run benchmarks. benchmark.test_... works too

Running a specific test
=======================

-  Check for the test name in tests/test\_core.py
-  Then, run it using the test runner like this — ``python tests/runner.py test_name``

Test failures
=============

To debug a test failure, the following might help:

::

    EMCC_DEBUG=1 ./tests/runner.py test_hello_world

will make emcc emit debug output, and also store its temp files in **/tmp/emscripten_temp/**. emcc-\* will contain, in order, the files it
emits. ``EMCC_DEBUG=2`` will emit even more files. Note that ``EMCC_DEBUG`` works on emcc in general, not just in the test suite.

``EM_SAVE_DIR=1`` is a test suite specific command that uses **/tmp/emscripten_temp/** as the test dir, so that all the files created by the test are retained (note that the dir is not cleaned out beforehand).

General compiler overview
=========================

- emcc is a python script that manages the entire compilation process.
- emcc will call clang to convert C++ to bitcode, llvm opt to optimize it, llvm-link to link, etc. etc.
- emcc calls emscripten.py which does the LLVM IR to JavaScript conversion process.

	- emscripten.py is a python script that runs the core compiler, either src/compiler.js (old compiler) or the LLVM backend (new fastcomp).
	- emscripten.py receives the core compiler output, modifies it slightly (some regexps) and then adds some necessary code around it, generating the basic emitted JavaScript. This is called emcc-2-original when running EMCC\_DEBUG=1 and saving the temp files.

- emcc will run tools/js_optimizer.py to further process and optimize the generated JavaScript.

	- js\_optimizer.py is a python script that breaks up a JavaScript file into the relevant parts for optimization (the generated code, as opposed to glue code) and calls js-optimizer.js to actually optimize it.
	- js-optimizer.js is a node script using UglifyJS which parses and transforms JavaScript into better JavaScript.

Articles you should read
==========================

- :ref:`Debugging`
- :ref:`Building-Projects`

Bisection
=========

As of fastcomp, version numbers should help bisect across the 3 repos we now have. See the :ref:`LLVM-Backend` page for details on how version numbers are used and where they are found.

Debugging tricks
================

-  You can run

::

    python tools/js_optimizer.py FILENAME PASSES..

on JavaScript output from emscripten. For example, ``asm safeHeap`` will apply safe heap annotations to the code; you can then manually add the ``SAFE_HEAP_LOAD`` and STORE methods from src/preamble.js, and now all your loads and stores are instrumented into calls.

-  You can make a program's execution 100% deterministic (not rely on ``Math.random`` or ``Date.now`` or anything like that) by including the src/deterministic.js file.

