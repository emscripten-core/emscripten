.. _contributing:

================================================
Contributing (ready-for-review) 
================================================

Anyone can contribute to Emscripten — if you find it useful and want to help improve the project, follow the suggestions below. Feel free to file :ref:`bug reports <bug-reports>`, :ref:`join the discussion <contact>` and share your own ideas with the community!


Getting started
===============

A good starting point is to work on the `open issues on Github <https://github.com/kripken/emscripten/issues?state=open>`_. Many issues can be resolved without an in-depth knowledge of compiler internals, and this is a great way to learn more about the project. 

.. tip:: We really appreciate your help. Every existing issue closed means more time for the core contributors to work on new features, optimizations and other enhancements.

As a new contributor you should read the :ref:`Developer's-Guide`, and you may also need to :ref:`install Emscripten from source <installing-from-source>`.


Branches of interest
====================

-  **master** - The "master" branch. This is always safe to pull from and the test suite always passes.
-  **incoming** - Branch for new code. Code in **incoming** is merged with the **master** only after it is code-reviewed and has passed all the automated tests.


Submitting patches
=====================

Patches should be submitted as *pull requests* to the **incoming** branch. 

.. note:: Before submitting your first patch, add yourself to the `AUTHORS <https://github.com/kripken/emscripten/blob/incoming/AUTHORS>`_ file. By doing so, you agree to license your code under the project's :ref:`open source licenses (MIT/LLVM) <emscripten-license>`.

When submitting patches, please:

- Make pull requests to **incoming**, not master.  
- Do not include merge commits in pull requests; include only commits with the new relevant code.
- :ref:`Run all the automatic tests <running-emscripten-tests>` and make sure they pass **tests/runner.py**.  Some tests might not be required for very simple patches (for example, when just adding tests for new library functions).
- If you add any new functionality or fix a bug, add an automatic test to **tests/runner.py**.
- Record the tests that were run in the pull request or issue.


Code reviews
============

`@kripken <https://github.com/kripken>`_ reviews all pull requests before merging. 

Exceptions are sub-projects that are 'owned' by other people. These owners can push to incoming directly:

-  OpenAL and audio in general: `@ehsan <https://github.com/ehsan>`_
-  embind: `@imvu <https://github.com/imvu>`_
-  Windows stuff: `@juj <https://github.com/juj>`_



Next steps
==========

Community members make a huge difference to this project. Please :ref:`get in touch with the community <contact>` to share your ideas and work out where you can make the most difference.



