.. _verifying-the-emscripten-environment:

================================================
Verifying the Emscripten Development Environment
================================================

After you've :ref:`installed the SDK <sdk-download-and-install>` or :ref:`built an Emscripten development environment from sources <installing-from-source>`, the compiler should just work! This section shows how to verify that the environment has been set up correctly, and how to troubleshoot installation problems when they do occur.


Testing the environment
=======================

Sanity tests
------------

The first step in verifying the environment is to run Emscripten with the version command (``-v``). The command prints out information about the toolchain and runs some basic sanity tests to check that the required tools are available. 

Open a terminal in the directory in which you installed Emscripten (on Windows open the :ref:`Emscripten Command Prompt <emcmdprompt>`). Then call the :ref:`Emscripten Compiler Frontend (emcc) <emccdoc>` as shown: 

::

	./emcc -v

.. note:: On Windows, invoke the tool with **emsdk** instead of **./emsdk**.
	
For example, the following output reports an installation where Java is missing: 

.. code-block:: javascript
	:emphasize-lines: 6

	emcc (Emscripten GCC-like replacement + linker emulating GNU ld ) 1.21.0
	clang version 3.3
	Target: x86_64-pc-win32
	Thread model: posix
	INFO     root: (Emscripten: Running sanity checks)
	WARNING  root: java does not seem to exist, required for closure compiler. -O2 and above will fail. You need to define JAVA in ~/.emscripten

At this point you need to :ref:`Install and activate <fixing-missing-components-emcc>` any missing components. When everything is set up properly, ``./emcc -v`` should give no warnings, and if you just enter ``./emcc`` (without any input files), it should only give the following warning: ::

	WARNING  root: no input files

	
Build a basic example
---------------------

The next test is to actually build some code! On the command prompt navigate to the Emscripten directory for the current SDK and try to build the **hello_world.cpp** test code: 

::

	./emcc tests/hello_world.cpp
	
This command should complete without warnings and you should find the newly-compiled JavaScript file (**a.out.js**) in the current directory.

If compiling succeeds, you're ready for the :ref:`Tutorial`. If not, check out the troubleshooting instructions below.


Run the full test suite
------------------------

Emscripten has a comprehensive test suite which may be used to further validate all or parts of the toolchain. For more information, see :ref:`emscripten-test-suite`.

	
.. _troubleshooting-emscripten-environment:

Troubleshooting
===============

First run ``./emcc -v`` and examine the output to find missing components. You can also try ``./emcc --clear-cache`` to empty the :ref:`compiler's internal cache <emcc-clear-cache>` and reset it to a known good state. 

.. _fixing-missing-components-emcc:

Installing missing components 
-----------------------------

Missing tools can often be added using the :ref:`emsdk`. For example, to fix a warning that Java is missing, locate it in the repository, install it, and then set it as active: ::
	
	#List all the components. Look for the missing component (in this case "java-7.45-64bit")
	./emsdk list
	
	#Install the missing component 
	./emsdk install java-7.45-64bit
	
	#Set the component as active
	./emsdk activate java-7.45-64bit

If you're :ref:`building Emscripten manually from source <installing-from-source>`, see that link for information on how to obtain all dependencies.


Other common problems
---------------------

Other common problems to check for are:

   -  Errors in the paths in :ref:`.emscripten <compiler-configuration-file>`. These are less likely if you update the file using :ref:`emsdk <emsdk>`.
   -  Using older versions of Node or JavaScript engines. Use the default versions for the SDK as listed with :ref:`emsdk list <emsdk>`.
   -  Using older versions of LLVM. The correct versions come with the SDK, but if you're building the environment from source see :ref:`LLVM-Backend` for the proper repos for LLVM and Clang.

If none of the above is helpful, then please :ref:`contact us <contact>` for help.
