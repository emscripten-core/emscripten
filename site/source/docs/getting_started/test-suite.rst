.. _emscripten-test-suite:

=====================
Emscripten Test Suite
=====================

Emscripten has a comprehensive test suite, which covers virtually all Emscripten functionality. These tests are an excellent resource for developers as they provide practical examples of most features, and are known to pass on the master branch (and almost always on the incoming branch). In addition to correctness tests, there are also benchmarks that you can run.

This article explains how to run the test and benchmark suite, and provides an overview of what tests are available.

Running tests
=============

Run the test suite runner (`tests/runner.py <https://github.com/emscripten-core/emscripten/blob/master/tests/runner.py>`_) with no arguments to see the help message:

.. code-block:: bash

    python tests/runner.py

The tests are divided into *modes*. You can run either an entire mode or an individual test, or use wildcards to run some tests in some modes. For example:

.. code-block:: bash

  # run one test (in the default mode)
  python tests/runner.py test_loop

  # run one test in a specific mode (here, asm.js -O2)
  python tests/runner.py asm2.test_loop

  # run a test in a bunch of modes (here, all asm.js modes)
  python tests/runner.py asm*.test_loop

  # run a bunch of tests in one mode (here, all i64 tests in wasm -O3)
  python tests/runner.py binaryen3.test_*i64*

  # run all tests in a specific mode (here, asm.js -O1)
  python tests/runner.py asm1

The *core* test modes (``asm*`` and ``binaryen*``, defined in ``tests/test_core.py``) let you run a specific test in either asm.js or wasm, and with different optimization flags. There are also non-core test modes, that run tests in more special manner (in particular, in those tests it is not possible to say "run the test with a different optimization flag" - that is what the core tests are for). The non-core test modes include

 * `other`: Non-core tests running in the shell.
 * `browser`: Tests that run in a browser.
 * `sockets`: Networking tests that run in a browser.
 * `interactive`: Browser tests that are not fully automated, and require user interaction (these should be automated eventually).
 * `sanity`: Tests for emscripten setting itself up. This modifies your `.emscripten` file temporarily.
 * `benchmark`: Runs benchmarks, measuring speed and code size.

The wildcards we mentioned above work for non-core test modes too, for example:

.. code-block:: bash

  # run one browser test
  python tests/runner.py browser.test_sdl_image

  # run all SDL2 browser tests
  python tests/runner.py browser.test_sdl2*

  # run all browser tests
  python tests/runner.py browser

Skipping Tests
==============

An individual test can be skipped by passing the "skip:" prefix. E.g.

.. code-block:: bash

  python tests/runner.py other skip:other.test_cmake

Wildcards can also be passed in skip, so

.. code-block:: bash

  python tests/runner.py browser skip:browser.test_pthread_*

will run the whole browser suite except for all the pthread tests in it.

Running a bunch of random tests
===============================

You can run a random subset of the test suite, using something like

.. code-block:: bash

    python tests/runner.py random100

Replace ``100`` with another number as you prefer. This will run that number of random tests, and tell you the statistical likelihood of almost all the test suite passing assuming those tests do. This works just like election surveys do - given a small sample, we can predict fairly well that so-and-so percent of the public will vote for candidate A. In our case, the "candidates" are pass or fail, and we can predict how much of the test suite will pass given that sample. Assuming the sample tests all pass, we can say with high likelihood that most of the test suite will in fact pass. (Of course, this is no guarantee, and even a single test failure is serious, however, this gives a quick estimate that your patch does not cause significant and obvious breakage.)

Important Tests
===============

When you want to run the entire test suite locally, these are the important commands:

.. code-block:: bash

  # Run all core asm.js and wasm tests
  python tests/runner.py asm* binaryen*

  # Run "other" test suite
  python tests/runner.py other

  # Run "browser" test suite - this requires a web browser
  python tests/runner.py browser

  # Run "sockets" test suite - this requires a web browser too
  python tests/runner.py sockets

  # Run "sanity" test suite - this tests setting up emscripten during
  # first run, etc., and so it modifies your .emscripten file temporarily.
  python tests/runner.py sanity

  # Optionally, also run benchmarks to check for regressions
  python tests/runner.py benchmark

.. _benchmarking:

Benchmarking
============

Emscripten has a benchmark suite that measures both speed and code size. To run it, do:

.. code-block:: bash

  # Run all benchmarks
  python tests/runner.py benchmark

Usually you will want to customize the python in `tests/test_benchmark.py` to run the benchmarks you want, see ``benchmarkers`` in the source code.

Debugging test failures
=======================

Setting the :ref:`debugging-EMCC_DEBUG` is useful for debugging tests, as it emits debug output and intermediate files (the files go in **/tmp/emscripten_temp/**):

.. code-block:: bash

  # On Windows, use "set" to set and un-set the EMCC_DEBUG environment variable:
  set EMCC_DEBUG=1
  python tests/runner.py test_hello_world
  set EMCC_DEBUG=0

  # On Linux, you can do this all in one line
  EMCC_DEBUG=1 python tests/runner.py test_hello_world

  # EMCC_DEBUG=2 generates additional debug information.
  EMCC_DEBUG=2 python tests/runner.py test_hello_world


You can also specify ``EMTEST_SAVE_DIR=1`` in the environment to save the temporary directory that the test runner uses into **/tmp/emscripten_test/**. This is a test suite-specific feature, and is useful for tests that create temporary files.

The :ref:`Debugging` topic provides more guidance on how to debug Emscripten-generated code.

