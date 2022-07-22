.. _Profiling-Toolchain:

=======================
Profiling the Toolchain
=======================

The toolchain interacts with a moderate amount of external tools and sublibraries, the exact set generally depends on which compilation and linker flags were used. If you are seeing abnormal compilation times, or if you are developing the Emscripten toolchain itself, it may be useful to profile the toolchain performance itself as it compiles your project. Emscripten has a built-in toolchain wide ``emprofile.py`` profiler that can be used for this purpose.

Quick Example
=============

To try out the toolchain profiler, run the following set of commands:

.. code-block:: bash

    cd path/to/emscripten
    export EMPROFILE=1
    emcc test/hello_world.c -O3 -o a.html
    emprofile

On Windows, replace the ``export`` keyword with ``set`` instead. The last command should generate a HTML file of form ``toolchain_profiler.results_yyyymmdd_hhmm.html`` that can be opened in the web browser to view the results.

Details
=======

The toolchain profiler is active whenever the toolchain is invoked with the environment variable ``EMPROFILE=1`` being set. In this mode, each called tool will accumulate profiling instrumentation data to a set of .json files under the Emscripten temp directory.

Profiling Tool Commands
-----------------------

The command ``tools/emprofile.py --clear`` deletes all previously stored profiling data. Call this command to erase the profiling session to a fresh empty state. To start profiling, call Emscripten tool commands with the environment variable ``EMPROFILE=1`` set either system-wide as shown in the example, or on a per command basis, like this:

.. code-block:: bash

    emprofile --clear
    EMPROFILE=1 emcc -c foo.c a.o
    EMPROFILE=1 emcc a.o -O3 -o a.html
    emprofile --outfile=myresults.html

Any number of commands can be profiled within one session, and when ``emprofile`` is finally called, it will pick up records from all Emscripten tool invocations up to that point, graph them, and clear the recorded profiling data for the next run.

The output HTML filename can be chosen with the optional ``--outfile=myresults.html`` parameter.

Instrumenting Python Scripts
============================

Python Profiling Blocks
-----------------------

Graphing the subprocess start and end times alone might sometimes be a bit too coarse view into what is happening. In Python code, it is possible to hierarchically annotate individual blocks of code to break down execution into custom tasks. These blocks will be shown in blue in the output graph. To add a custom profiling block, use the Python ``with`` keyword to add a ``profile_block`` section:

.. code-block:: python

    with ToolchainProfiler.profile_block('my_custom_task'):
      do_some_tasks()
      call_another_function()
      more_code()

    this_is_outside_the_block()

This will show the three functions in the same scope under a block 'my_custom_task' drawn in blue in the profiling swimlane.

In some cases it may be cumbersome to wrap the code inside a ``with`` section. For these scenarios, it is also possible to use low level C-style ``enter_block`` and ``exit_block`` statements.

.. code-block:: python

    ToolchainProfiler.enter_block('my_code_block')
    try:
      do_some_tasks()
      call_another_function()
      more_code()
    finally:
      ToolchainProfiler.exit_block('my_code_block')

However when using this form one must be cautious to ensure that each call to ``ToolchainProfiler.enter_block()`` is matched by exactly one call to ``ToolchainProfiler.exit_block()`` in all code flows, so wrapping the code in a ``try-finally`` statement is a good idea.
