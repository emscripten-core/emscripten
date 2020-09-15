.. _Developer's-Guide:

=================
Developer's Guide
=================

This article provides information that is relevant to people who want to
contribute to Emscripten. We welcome contributions from anyone that is
interested in helping out!

.. tip:: The information will be less relevant if you're just using Emscripten, but may still be of interest.

.. _developers-guide-setting-up:

Setting up
==========

For contributing to core Emscripten code, such as ``emcc.py``, you don't need to
build any binaries as ``emcc.py`` is in Python, and the core JS generation is
in JavaScript. You do still need binaries for LLVM and Binaryen, which you can
get using the emsdk:

::

    emsdk install tot-upstream
    emsdk activate tot-upstream

That gets a "tip-of-tree" build of the very latest binaries. You can use those
binaries with a checkout of the core Emscripten repository, simply by calling
``emcc.py`` from that checkout, and it will use the binaries from the emsdk.

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

.. note::
   Together with your first patch, add yourself to the
   `AUTHORS <https://github.com/emscripten-core/emscripten/blob/master/AUTHORS>`_
   file. By doing so, you agree to license your code under the project's
   :ref:`open source licenses (MIT/LLVM) <emscripten-license>`.

When submitting patches, please:

- Add an automatic test if you add any new functionality or fix a bug. Search
  in ``tests/*.py`` for related tests, as often the simplest thing is to add to
  an existing one. If you're not sure how to test your code, feel free to ask
  for help.
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
- **emcc** then calls `emscripten.py <https://github.com/emscripten-core/emscripten/blob/master/emscripten.py>`_
  which performs the final transformation to wasm (including invoking
  **wasm-emscripten-finalize** from Binaryen) and calls the JS compiler
  (see ``src/compiler.js`` and related files) which emits the JS.
- If optimizing wasm, **emcc** will then call **wasm-opt**, run meta-dce, and
  other useful things. It will also run JS optimizations on the JS that is
  emitted alongside the wasm.

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
investigate if it's an Emscripten regression or something else.

The normal ``git bisect`` workflow can work if you just need to bisect a single
repository. For example, you can bisect only on the emscripten repo if you are on
a range that all works with the same versions of LLVM and Binaryen (which was
mentioned earlier, are two separate codebases that are depended on).

If you have a large bisection range, you generally can't bisect a single repo.
You can still bisect, though! To do that you need the emsdk and to understand
how the
`release process <https://github.com/emscripten-core/emscripten/blob/master/docs/process.md#release-processes>`_
works for all the repos together. The key "trick" is that::

     emsdk install tot

can install an arbitrary build of emscripten: it installs the one identified
in ``emscripten-releases-tot.txt``. You can therefore bisect on the
`releases repo <https://chromium.googlesource.com/emscripten-releases>`_ which
has a DEPS file that specifies what version of all the various repos are
in which release. At each bisection step in this repo the git hash identifies a
particular `tot` release (which when it was built, was tip-of-tree).

For the actual bisection you can use
`git bisect <https://git-scm.com/docs/git-bisect>`_. Each step in will download
a complete build which is not a trivial download. However, at least the number
of such steps will be logarithmic!

This bisects down to a single commit in the releases repo. That commit will
generally update a single sub-repo from one commit to another. That will often
be a very short list or even a single commit. If it's more than one, you can
bisect there while using a fixed build for the other repos.

See also
========

- :ref:`Debugging`
- :ref:`Building-Projects`


