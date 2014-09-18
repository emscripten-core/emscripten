.. _Developer's-Guide:

====================================
Developer's Guide (ready-for-review)
====================================

This article provides information that is relevant to people working on Emscripten itself, as opposed to those that use Emscripten on their own projects. 

.. tip:: The information will be less relevant if you're just using Emscripten, but may still be of interest.


Building Emscripten from source code
====================================

As a contributor you will need to build Emscripten from source code, in order to integrate fixes back into the main repositories.

We have :ref:`instructions explaining how to build from source <installing-from-source>` for all supported platforms, both using the SDK and manually.


Repositories and branches of interest
=====================================

The Emscripten main repository is https://github.com/kripken/emscripten. There are two main branches:

-  **master** - The "master" branch. This is always safe to pull from and the test suite always passes.
-  **incoming** - Branch for new code. Code in **incoming** is merged with the **master** only after it is code-reviewed and has passed all the automated tests.


Emscripten's compiler core (:ref:`Fastcomp <LLVM-Backend>`) is maintained in two other repositories, which use the same master/incoming branch approach:

- https://github.com/kripken/emscripten-fastcomp (Emscripten LLVM fork)
- https://github.com/kripken/emscripten-fastcomp-clang (Emscripten Clang fork)

When :ref:`building Emscripten from source <installing-from-source>` you should use the same branch (incoming, or master) for building all three repositories. The topic :ref:`building-fastcomp-from-source-branches` explains how to ensure that the versions are kept in sync.


.. _developers-guide-submitting-patches:

Submitting patches
==================

Patches should be submitted as *pull requests* to the **incoming** branch. 

.. note:: Before submitting your first patch, add yourself to the `AUTHORS <https://github.com/kripken/emscripten/blob/incoming/AUTHORS>`_ file. By doing so, you agree to license your code under the project's :ref:`open source licenses (MIT/LLVM) <emscripten-license>`.

When submitting patches, please:

- Make pull requests to **incoming**, not master.  
- Do not include merge commits in pull requests; include only commits with the new relevant code.
- :ref:`Run all the automatic tests <emscripten-test-suite>` and make sure they pass.  Some tests might not be required for very simple patches (for example, when just adding tests for new library functions).
- If you add any new functionality or fix a bug, add an automatic test to `tests/runner.py <https://github.com/kripken/emscripten/blob/master/tests/runner.py>`_.
- Record the tests that were run in the pull request or issue.


Code reviews
============

`@kripken <https://github.com/kripken>`_ reviews all pull requests before merging. 

Exceptions are sub-projects that are 'owned' by other people. These owners can push to incoming directly:

-  OpenAL and audio in general: `@ehsan <https://github.com/ehsan>`_
-  embind: `@imvu <https://github.com/imvu>`_
-  Windows stuff: `@juj <https://github.com/juj>`_


Compiler overview
=========================

The :ref:`Emscripten Compiler Frontend (emcc) <emccdoc>` is a python script that manages the entire compilation process:

- *emcc* calls :term:`Clang` to convert C++ to bitcode, ``llvm-opt`` to optimize it, ``llvm-link`` to link it, etc.
- *emcc* then calls `emscripten.py <https://github.com/kripken/emscripten/blob/master/emscripten.py>`_ which performs the LLVM IR to JavaScript conversion:

	- **emscripten.py** is a python script that runs the core compiler. Currently this is :ref:`Fastcomp <LLVM-Backend>` (previously it was the :ref:`old compiler <original-compiler-core>` — `src/compiler.js <https://github.com/kripken/emscripten/blob/master/src/compiler.js>`_).
	- **emscripten.py** receives the core compiler output, modifies it slightly (some regexps) and then adds some necessary code around it. This generates the basic emitted JavaScript, which is called **emcc-2-original** in the intermediate files saved in :ref:`debug mode <debugging-EMCC_DEBUG>`.

- *emcc* runs `tools/js_optimizer.py <https://github.com/kripken/emscripten/blob/master/tools/js_optimizer.py>`_ to further process and optimize the generated JavaScript.

	- **js_optimizer.py** is a python script that breaks up a JavaScript file into the relevant parts for optimization (the generated code, as opposed to glue code) and calls `js-optimizer.js <https://github.com/kripken/emscripten/blob/master/tools/js-optimizer.js>`_ to actually optimize it.
	- **js-optimizer.js** is a :term:`node.js` script using ``UglifyJS`` that parses and transforms JavaScript into better JavaScript.

Emscripten Test Suite
=====================

Emscripten has a :ref:`comprehensive test suite <emscripten-test-suite>`, which covers virtually all Emscripten functionality. These tests must all pass when you are :ref:`submitting patches <developers-guide-submitting-patches>`. 

Articles you should read
========================

- :ref:`Debugging`
- :ref:`Building-Projects`


