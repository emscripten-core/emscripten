================================================
Contributing (ready-for-review) 
================================================

Anyone can contribute to Emscripten. Feel free to :doc:`join the discussion <contact>` and share your suggestions, ideas, and `bug reports <bug-reports>`_!

If you find Emscripten useful and want to contribute, check out the information and suggestions below.


Getting started
===============

A good starting point is to work on the `open issues on Github <https://github.com/kripken/emscripten/issues?state=open>`_. Many issues can be resolved without an in-depth knowledge of compiler internals, and this is a great way to learn more about the project. 

.. tip:: We really appreciate your help. Every closed issue means more time for experienced developers to work on compiler internals.


Branches of interest
====================

-  **master** - The "master" branch. This is always safe to pull from and the test suite always passes.
-  **incoming** - Branch for new code. Code in **incoming** is merged with the **master** only after it is code-reviewed and has passed all the automated tests.
-  **llvmsvn** - Branch for work to support a new version of LLVM. Activity typically begins near the end of an LLVM 6-month dev cycle. When LLVM launches the new version, we merge this branch to master and incoming, at which point our support officially moves to that new LLVM version. Only one version of LVVM is supported at a time.


Submitting patches
=====================

Patches should be submitted as *pull requests* to the **incoming** branch. 

.. note:: Before submitting your first patch, add yourself to the `AUTHORS <https://github.com/kripken/emscripten/blob/incoming/AUTHORS>`_ file. By doing so, you agree to license your code under the project's `open source licenses (MIT/LLVM) <emscripten-license>`_.

When submitting patches, please:

- Make pull requests to **incoming**, not master.  
- Do not include merge commits in pull requests; include only commits with the new relevant code.
- Run all the automatic tests and make sure they pass **tests/runner.py**.  Some tests might not be required for very simple patches (for example, when just adding tests for new library functions).
- If you add any new functionality or fix a bug, add an automatic test to **tests/runner.py**.
- Record the tests that were run in the pull request or issue.


Code reviews
============

`@kripken <https://github.com/kripken>`_ reviews all pull requests before merging. 

Exceptions are sub-projects that are 'owned' by other people. These developer/owners can push to incoming directly:

-  OpenAL and audio in general: `@ehsan <https://github.com/ehsan>`_
-  embind: `@imvu <https://github.com/imvu>`_
-  Windows stuff: `@juj <https://github.com/juj>`_



Next steps
==========

After you've fixed some bugs and have a better understanding of the project, you may wish to contribute more "deeply". Please share your development ideas with the :doc:`community <contact>` so that they can be evolved and planned.

Emscripten internal developers should also read the :doc:`PLACEHOLDER Developer's-Guide`.