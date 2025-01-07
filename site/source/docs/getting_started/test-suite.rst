.. _emscripten-test-suite:

=====================
Emscripten Test Suite
=====================

Emscripten has a comprehensive test suite, which covers virtually all Emscripten
functionality. These tests are an excellent resource for developers as they
provide practical examples of most features, and are known to pass on the master
branch. In addition to correctness tests, there are also benchmarks that you can
run.

This article explains how to run the test and benchmark suite, and provides an
overview of what tests are available.

Setting up
==========

To run the tests, you need an emscripten setup, as it will run ``emcc`` and other
commands. See the :ref:`developer's guide <developers-guide-setting-up>` for
how best to do that.

Running tests
=============

Run the test suite runner (`test/runner <https://github.com/emscripten-core/emscripten/blob/main/test/runner.py>`_) with ``--help`` to see the help message:

.. code-block:: bash

    test/runner --help

The tests are divided into *modes*. You can run either an entire mode or an
individual test, or use wildcards to run some tests in some modes. For example:

.. code-block:: bash

  # run one test (in the default mode)
  test/runner test_foo

  # run a bunch of tests in one mode (here, all i64 tests in -O3)
  test/runner core3.test_*i64*

  # run all tests in a specific mode (here, wasm2gs -O1)
  test/runner wasm2js1

The *core* test modes (defined at the bottom of `test/test_core.py
<https://github.com/emscripten-core/emscripten/blob/main/test/test_core.py>`_)
let you run the tests in variety of different configurations and with different
optimization flags.  For example, wasm2js or wasm64.  There are also non-core
test suites, that run tests in more special manner (in particular, in those tests
it is not possible to say "run the test with a different optimization flag" -
that is what the core tests are for).  The non-core test suites include

 * `other`: Non-core tests running in the shell.
 * `browser`: Tests that run in a browser.
 * `sockets`: Networking tests that run in a browser.
 * `interactive`: Browser tests that are not fully automated, and require user interaction (these should be automated eventually).
 * `sanity`: Tests for emscripten setting itself up. This modifies your `.emscripten` file temporarily.
 * `benchmark`: Runs benchmarks, measuring speed and code size.

The wildcards we mentioned above work for non-core test modes too, for example:

.. code-block:: bash

  # run one browser test
  test/runner browser.test_sdl_image

  # run all SDL2 browser tests
  test/runner browser.test_sdl2*

  # run all browser tests
  test/runner browser

Skipping Tests
==============

An individual test can be skipped by passing the "skip:" prefix. E.g.

.. code-block:: bash

  test/runner other skip:other.test_cmake

Wildcards can also be passed in skip, so

.. code-block:: bash

  test/runner browser skip:browser.test_pthread_*

will run the whole browser suite except for all the pthread tests in it.

Exiting on first failure
========================

Sometimes it is useful to be able to iteratively fix one test at a time.  In
this case the ``--failfast`` option can be used to exit the test runner after
the first failure.

.. note:: This option only works with the serial test runner.  For test suites
   that are normally run in parallel you can force them to run serially using
   ``-j1``.

One a test is fixed you continue where you left off using ``--start-at`` option:

.. code-block:: bash

  test/runner browser --start-at test_foo --failfast

Running a bunch of random tests
===============================

You can run a random subset of the test suite, using something like

.. code-block:: bash

    test/runner random100

Replace ``100`` with another number as you prefer. This will run that number of
random tests, and tell you the statistical likelihood of almost all the test
suite passing assuming those tests do. This works just like election surveys do
- given a small sample, we can predict fairly well that so-and-so percent of the
public will vote for candidate A. In our case, the "candidates" are pass or
fail, and we can predict how much of the test suite will pass given that
sample. Assuming the sample tests all pass, we can say with high likelihood that
most of the test suite will in fact pass. (Of course, this is no guarantee, and
even a single test failure is serious, however, this gives a quick estimate that
your patch does not cause significant and obvious breakage.)

Important Tests
===============

Please see the bottom the file `test/test_core.py
<https://github.com/emscripten-core/emscripten/blob/main/test/test_core.py>`_
for the current test modes, as they may change slowly over time.  When you want
to run the entire test suite locally, these are currently the important
commands:

.. code-block:: bash

  # Run all core tests
  test/runner core*

  # Run "other" test suite
  test/runner other

  # Run "browser" test suite - this requires a web browser
  test/runner browser

  # Run "sockets" test suite - this requires a web browser too
  test/runner sockets

  # Run "sanity" test suite - this tests setting up emscripten during
  # first run, etc., and so it modifies your .emscripten file temporarily.
  test/runner sanity

  # Optionally, also run benchmarks to check for regressions
  test/runner benchmark

.. _benchmarking:

Benchmarking
============

Emscripten has a benchmark suite that measures both speed and code size, which
includes several interesting real-world codebases, from physics engines to
compression libraries to virtual machines. It also includes some existing
benchmarks such as CoreMark and LINPACK. See for example
`this post's section on speed <https://kripken.github.io/blog/wasm/2020/07/27/wasmboxc.html>`_
which gives an overview.

To run the benchmark suite, do:

.. code-block:: bash

  # Run all benchmarks
  test/runner benchmark

As with all the test suites, you can also run a specific benchmark:

.. code-block:: bash

  # Run one specific benchmark
  test/runner benchmark.test_skinning

You can also specify which benchmarkers are run by using the environment
variable `EMTEST_BENCHMARKERS`. It accepts a comma separated list of named
benchmarkers (names can be found in `named_benchmarkers` in
`test/test_benchmark.py`).

.. code-block:: bash

  # Run one specific benchmark and with clang and v8.
  EMTEST_BENCHMARKERS=clang,v8 test/runner benchmark.test_skinning

To further customize how the benchmarks are run, you will want to edit the file
`test/test_benchmark.py`. Some of the options include:

* ``DEFAULT_ARG`` is how long the benchmark should run (they all try to run for
  a similar amount of time for consistency).
* ``TEST_REPS`` is how many times to repeat each run (more will take longer, but
  should have less noise).
* ``PROFILING`` controls whether the builds are set up for profiling (which can
  increase code size, so it's not done by default).

Debugging test failures
=======================

Setting the :ref:`debugging-EMCC_DEBUG` is useful for debugging tests, as it
emits debug output and intermediate files (the files go in
**/tmp/emscripten_temp/**):

.. code-block:: bash

  # On Windows, use "set" to set and un-set the EMCC_DEBUG environment variable:
  set EMCC_DEBUG=1
  test/runner test_hello_world
  set EMCC_DEBUG=0

  # On Linux, you can do this all in one line
  EMCC_DEBUG=1 test/runner test_hello_world

  # EMCC_DEBUG=2 generates additional debug information.
  EMCC_DEBUG=2 test/runner test_hello_world


You can also specify ``--save-dir`` to save the temporary directory that the
test runner uses into **/out/test/**.  This is a test suite-specific
feature, and is useful for inspecting test outputs as well as temporary files
generated by the test.  By default, the temporary directory will be cleaned
between each test run, but you can add ``--no-clean`` to avoid this.

The :ref:`Debugging` topic provides more guidance on how to debug
Emscripten-generated code.
