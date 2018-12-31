.. _emscripten-test-suite:

=====================
Emscripten Test Suite
=====================

Emscripten has a comprehensive test suite, which covers virtually all Emscripten functionality. These tests are an excellent resource for developers as they provide practical examples of most features, and are known to build successfully on the master branch. There are also :ref:`benchmark tests <emscripten-benchmark-tests>` that can be used to test how close Emscripten is getting to native speed.

This article explains how to run the test and benchmark suite, and provides an overview of what tests are available.

Running the whole test suite
============================

The whole core test suite can be run using the script `tests/runner.py <https://github.com/kripken/emscripten/blob/master/tests/runner.py>`_: 

.. code-block:: bash

    python tests/runner.py
	
.. note:: 

	- The core test suite is not the entire test suite, see :ref:`section about core test modes <emscripten-test-suite-modes>`
	- This may take several hours.
	- :term:`Node.js` cannot run all of the tests in the suite; if you need to run them all, you should get a recent trunk version of the `SpiderMonkey <https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Introduction_to_the_JavaScript_shell>`_ shell. On Windows you can install and activate *SpiderMonkey* using the :ref:`emsdk`.

Running specific tests
======================

You can also use *runner.py* to run different parts of the test suite, or individual tests. For example, you would run test named ``test_hello_world`` as shown:

.. code-block:: bash

    python tests/runner.py test_hello_world
	
Tests in the "core" test suite (``tests/test_core.py``) can be run as above. Other tests may need a prefix, for example ``browser.test_cubegeom`` for a test in ``tests/test_browser.py``. You can also specify an optional prefix for tests in core, to run them with extra options, for example ``asm2.test_hello_world`` will run ``hello_world`` using ``asm2`` opts (basically ``-O2``). See more examples in :ref:`emscripten-test-suite-list-of-tests`.

It is possible to pass a wildcard to match multiple tests by name. For example, the commands

.. code-block:: bash

    python tests/runner.py ALL.test_simd_* ALL.test_sse*

would run all the SIMD related tests in the suite.

Individual tests can be skipped, so

.. code-block:: bash

    python tests/runner.py browser.test_pthread_* skip:browser.test_pthread_gcc_atomic_fetch_and_op

would run all the multithreading tests except the one test ``browser.test_pthread_gcc_atomic_fetch_and_op``.

Running a bunch of random tests
===============================

You can run a random subset of the test suite, using something like

.. code-block:: bash

    python tests/runner.py random100

Replace ``100`` with another number as you prefer. This will run that number of random tests, and tell you the statistical likelihood of almost all the test suite passing assuming those tests do. This works just like election surveys do - given a small sample, we can predict fairly well that so-and-so percent of the public will vote for candidate A. In our case, the "candidates" are pass or fail, and we can predict how much of the test suite will pass given that sample. Assuming the sample tests all pass, we can say with high likelihood that most of the test suite will in fact pass. (Of course, this is no guarantee, and even a single test failure is serious, however, this gives a quick estimate that your patch does not cause significant and obvious breakage.)

Core test modes
===============

By default, calling the test runner without arguments will run the core test suite

.. code-block:: bash

    python tests/runner.py

The core test suite includes ``default`` (no optimizations), ``asm1`` (``-O1`` optimizations), and a bunch of other optimization and compiler flags, each of which is a different "mode". The core test suite is the bulk of the entire test suite, and it runs each test in each of those modes.

You can also run a specific mode or test in a mode, or a specific test across all modes:

.. code-block:: bash

	# Run all tests in asm1 mode	(-O1 optimizations).
	python tests/runner.py asm1

	# Run one test in asm1 mode	(-O1 optimizations).
	python tests/runner.py asm1.test_hello_world   
	
	# Run one test in all modes.
	python tests/runner.py ALL.test_hello_world 

The core test modes are documented at the end of `/tests/test_core.py <https://github.com/kripken/emscripten/blob/1.29.12/tests/test_core.py#L7421>`_.

.. _emscripten-test-suite-modes:

The core tests are the bulk of the entire test suite, in both number and time to run. To speed them up, you can run them in parallel using `/tests/parallel_test_core.py <https://github.com/kripken/emscripten/blob/master/tests/parallel_test_core.py>`_. That runs the test modes using a python process pool, emitting their outputs and stderrs to ``*.out, *.err`` for each mode.


Non-core test modes
===================

The main non-core test modes are ``other, browser, sockets, interactive, sanity``. See :ref:`emscripten-test-suite-list-of-tests` for how to run them.


.. _emscripten-benchmark-tests:

Benchmark tests
===============

You can view `Emscripten’s current benchmark test results <http://arewefastyet.com/#machine=11&view=breakdown&suite=asmjs-ubench>`_ online. These are created by compiling a sequence of benchmarks and running them several times, then reporting averaged statistics including a comparison of how fast the same code runs when compiled to a native executable.

You can run the tests yourself using the following command:

.. code-block:: bash

    python tests/runner.py benchmark

	
.. _emscripten-test-suite-list-of-tests:

Common tests
============

Below is a list of some common tests/example commands. These include a comment explaining what each test does.

.. code-block:: bash

	# Run all (core) tests
	python tests/runner.py                          

	# Run hello world test, in default mode
	python tests/runner.py test_hello_world

	# Run it in asm1 mode
	python tests/runner.py asm1.test_hello_world   
	
	# Run it in all modes
	python tests/runner.py ALL.test_hello_world 

	# Run all (core) tests in asm1 mode	
	python tests/runner.py asm1 

	# Run all "other" tests - that have no mode	
	python tests/runner.py other

	# Run a specific test in "other"	
	python tests/runner.py other.test_static_link 

	# Run all browser tests	
	python tests/runner.py browser
	
	# Run a specific browser test	
	python tests/runner.py browser.test_sdlglshader 
	
	# Run all network tests. Note that you can also run specific tests (sockets.test_*)
	python tests/runner.py sockets

	# Run all sanity tests. Note that you can also run specific tests (sanity.test_*)	
	python tests/runner.py sanity

	# Run all benchmarks. Note that you can also run specific tests (benchmark.test_*)	
	python tests/runner.py benchmark                


Debugging test failures
=======================

Setting the :ref:`debugging-EMCC_DEBUG` is useful for debugging tests, as it emits debug output and intermediate files from the compilation process:

.. code-block:: bash

	# On Windows, use "set" to set and un-set the EMCC_DEBUG environment variable:
	set EMCC_DEBUG=1 
	python tests/runner.py test_hello_world
	set EMCC_DEBUG=0
	
	# On Linux, you can do this all in one line
	EMCC_DEBUG=1 python tests/runner.py test_hello_world
	
	# EMCC_DEBUG=2 generates additional debug information.
	EMCC_DEBUG=2 python tests/runner.py test_hello_world


You can also specify ``EM_SAVE_DIR=1`` in the environment to save the temporary directory that the test runner uses into **/tmp/emscripten_temp/**. This is a test suite-specific feature, and is useful for tests that create temporary files.

The :ref:`Debugging` topic provides more guidance on how to debug Emscripten-generated code. 

