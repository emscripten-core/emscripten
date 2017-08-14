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
    tools/emprofile.py --reset
    export EM_PROFILE_TOOLCHAIN=1
    emcc tests/hello_world.c -O3 -o a.html
    tools/emprofile.py --graph

On Windows, replace the ``export`` keyword with ``set`` instead. The last command should generate a HTML file of form ``toolchain_profiler.results_yyyymmdd_hhmm.html`` that can be opened in the web browser to view the results.

Details
=======

The toolchain profiler is active whenever the toolchain is invoked with the environment variable ``EM_PROFILE_TOOLCHAIN=1`` being set. In this mode, each called tool will accumulate profiling instrumentation data to a set of .json files under the Emscripten temp directory.

Profiling Tool Commands
-----------------------

The command ``tools/emprofile.py --reset`` deletes all previously stored profiling data. Call this command to erase the profiling session to a fresh empty state. To start profiling, call Emscripten tool commands with the environment variable ``EM_PROFILE_TOOLCHAIN=1`` set either system-wide as shown in the example, or on a per command basis, like this:

.. code-block:: bash

    cd path/to/emscripten
    tools/emprofile.py --reset
    EM_PROFILE_TOOLCHAIN=1 emcc tests/hello_world.c -o a.bc
    EM_PROFILE_TOOLCHAIN=1 emcc a.bc -O3 -o a.html
    tools/emprofile.py --graph --outfile=myresults.html

Any number of commands can be profiled within one session, and when ``tools/emprofile.py --graph`` is finally called, it will pick up records from all Emscripten tool invocations up to that point. Calling ``--graph`` also clears the recorded profiling data.

The output HTML filename can be chosen with the optional ``--outfile=myresults.html`` parameter.

Instrumenting Python Scripts
============================

In order for the toolchain to work, each "top level" Python script (a script that is directly called from the command line, or by a subprocess spawn) should have the following preamble in the beginning of the script:

.. code-block:: python

    from tools.toolchain_profiler import ToolchainProfiler
    if __name__ == '__main__':
      ToolchainProfiler.record_process_start()

Additionally, at the end of the script when the script is about to exit, it should do so by explicitly calling either the ``sys.exit(<returncode>)`` function, or by calling the ``ToolchainProfiler.record_process_exit(<returncode>)`` function, whichever is more convenient for the script. The function ``ToolchainProfiler.record_process_exit()`` does not exit by itself, but only records that the process is quitting.

These two blocks ensure that the toolchain profiler will be aware of all tool invocations that occur. In the graphed output, the process spawns will be shown in green color.

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
