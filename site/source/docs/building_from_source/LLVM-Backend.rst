.. _LLVM-Backend:

====================================================
LLVM Backend ("Fastcomp") (under-construction)
====================================================

*Fastcomp* is the default compiler core for Emscripten. It is a new :term:`LLVM backend` that converts the LLVM Intermediate Representation (IR) created by *Clang* (from C/C++) into JavaScript. 

The backend is still too new to be in the upstream LLVM repository. As such, builds from Linux distributions will **not** contain *Fastcomp*, and Emscripten will report an error if you try to use them. Instead you will need to use the Emscripten fork of LLVM, which you can either build from source or get as part of the :ref:`Emscripten SDK <sdk-download-and-install>`.

This article explains how to get *Fastcomp*  — either from the SDK or by building it from source. There is also a :ref:`fastcomp-faq` at the very end for troubleshooting *Fastcomp* problems.


.. note:: *Fastcomp* was turned on by default in version 1.12.1. It replaces the :ref:`original compiler core <original-compiler-core>`, which is now "deprecated". While it is possible to manually disable Fastcomp and build the original compiler from source (binaries are no longer provided), this is discouraged.

	The article also explains the reasons for changing to *Fastcomp*, and how you can use the old compiler if needed. 

.. todo:: Emscripten tool (emcc) links to original wiki page when you use the wrong clang. Should point to here instead.


Getting Fastcomp
================

*Fastcomp* (Clang) is part of the SDK, and the binaries are automatically installed during installation (except on Linux, where pre-built binaries are not supplied so the SDK builds them for you). 

It is also possible to build Fastcomp from source as a :ref:`fully manual process <building-fastcomp-from-source>` or :ref:`using the SDK <building-fastcomp-from-source-using-the-sdk>` (recommended).

.. _building-fastcomp-from-source-using-the-sdk:

Building Fastcomp from source using the SDK
============================================

The SDK (:term:`emsdk`) supports building *Fastcomp* from :ref:`the master or incoming branches <emsdk-master-or-incoming-sdk>`, or from :ref:`your own fork <emsdk-howto-use-own-fork>` of these branches. The former allows you to get the latest and greatest code, while the later lets you additionally make and contribute your own modifications. 

.. tip:: This approach is recommended over :ref:`building-fastcomp-from-source` because the SDK does all the work of getting the correct sources and tools for each branch, and guides you through the build process. 

You can see the *Clang* SDK targets for this using ``./emsdk list``: ::

	clang-incoming-32bit
	clang-incoming-64bit
	clang-master-32bit
	clang-master-64bit

.. note:: 

	- To build from source you first need to install a :ref:`compiler-toolchain` (see link for installation instructions). 
	- If you are building a large project, you will need a 64-bit build of LLVM and Clang. Compiling and optimizing can take more memory than is available to the 32-bit build.
	
To get and build the latest upstream git development branch (``incoming``), run the following:

::

	# Install git. Skip if git is already installed.
	./emsdk install git-1.8.3 
	
	# Clone+pull the latest **/kripken/emscripten-fastcomp-clang** and **/kripken/emscripten-fastcomp**
	./emsdk install clang-incoming-64bit
	
	# Set the "incoming SDK" as the active version.
	./emsdk activate clang-incoming-64bit	

If you want to use the upstream stable branch ``master``, then replace ``-incoming-`` with ``-master-`` in the commands above. This is all you need to do to get the latest version of *Fastcomp*.

.. note:: On Windows, *git* may fail with the error message: 

	::

		Unable to find remote helper for 'https' when cloning a repository with https:// url. 
		
	The workaround is to uninstall git from *emsdk* (``emsdk uninstall git-1.8.3``)  and install `Git for Windows <http://msysgit.github.io>`_. This issue is reported `here <https://github.com/juj/emsdk/issues/13>`_.
	
.. todo:: **HamishW** Check whether the bug (https://github.com/juj/emsdk/issues/13) is fixed and remove the above note if it is.

If you're :ref:`contributing` to Emscripten you will instead want to use a fork of the *incoming* branch on Github (so that you can make pull requests back to the project). 

The way this works is that you first install the ``clang-incoming-64bit`` SDK target (as explained above). Then you use familiar *git* commands to replace this branch with the information from your own fork. These commands are the same that are used when :ref:`building-fastcomp-from-source`:

::

	cd clang/fastcomp/src
	
	## Get your fork of LLVM
	
	# Add a git remote link to your LLVM repository fork.
	git remote add myllvmremote https://github.com/mygituseraccount/emscripten-fastcomp.git
	
	# Obtain the changes in the remote.
	git fetch myllvmremote
	
	# Checkout the code in your LLVM fork to replace the original repository code
	git checkout -b myllvmremote --track myremote/incoming

	
	## Get your fork of Clang
	
	# Add a git remote link to your Clang repository fork.
	git remote add myclangremote https://github.com/mygituseraccount/emscripten-fastcomp-clang.git
	
	# Obtain the changes in the remote.
	git fetch myclangremote
	
	# Checkout the code in your Clang fork to replace the original repository code
	git checkout -b myclangremote --track myclangremote/incoming
	
	# At this point the clang-incoming-64bit target now points to your fork.
	
You can switch back and forth between remotes (the original sources and your fork, or the incoming and master branches) using the ``git checkout`` command as usual.



.. _building-fastcomp-from-source:

Manually building Fastcomp from source
======================================

.. note:: 

	- To build from source you first need to install a :ref:`compiler-toolchain` (see link for installation instructions). 
	- If you are building a large project, you will need a 64-bit build of LLVM and Clang. Compiling and optimizing can take more memory than is available to the 32-bit build.


To build the Fastcomp code from source "manually" (without the benefit of the SDK environment): 

-  Clone the fastcomp LLVM repository (https://github.com/kripken/emscripten-fastcomp>): 

	::
	
		git clone https://github.com/kripken/emscripten-fastcomp

	.. note:: It doesn't matter where you clone *fastcomp* because Emscripten gets the information from the :ref:`compiler configuration file (~/.emscripten) <compiler-configuration-file>`. We show how to update this file later in these instructions:
	
		
- Navigate to the **tools** directory (**emscripten-fastcomp/tools**) and clone the `kripken/emscripten-fastcomp-clang <https://github.com/kripken/emscripten-fastcomp-clang>`_ repository into a **clang** subdirectory: 

	::
	
		cd tools
		git clone https://github.com/kripken/emscripten-fastcomp-clang clang

	.. warning:: You **must** clone it into a directory named **clang** as shown, so that :term:`Clang` is present in **tools/clang**! 
	
	.. note:: This repository has changed. Early in fastcomp development we used a different directory.
	
-  Create a *build* directory (we highly recommend creating a separate build directory):

	- Navigate back to the root of the llvm clone (**/emscripten-fastcomp**)
	- Create a new directory "**build**", and then navigate into it:
	
		::
		
			cd ..
			mkdir build
			cd build
	
-  Configure the build using *either* the *configure* script or *cmake* (configure is located in the parent directory):
	
	- Using *configure*: 
		
	::
	
		../configure --enable-optimized --disable-assertions --enable-targets=host,js
			
	-  Using *CMake*: 

		::
			
			cmake .. -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;JSBackend" -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_EXAMPLES=OFF -DCLANG_INCLUDE_TESTS=OFF
				
		.. note:: replace X86 if you are on something else..
	   
-  Determine the number of available cores on your system (Emscripten can run many operations in parallel, so using more cores may have a significant impact on compilation time):

	- On Mac OS X you can get the number of cores using: **Apple menu | About this mac | System report**. The **Hardware overview** on the resulting dialog includes a *Total number of cores* entry.
	- On Linux you can find the number of cores by entering the following command on the terminal: ``cat /proc/cpuinfo | grep "^cpu cores" | uniq``.
	- On Windows the number of cores is listed on the **Task Manager | Performance Tab**. YOu can open the *Task Manager* by entering **Ctrl + Shift + Esc** from the Desktop.

- Call *make* to build the sources, specifying the number of available cores:

	::
		
		make -j 4
	
		
		

.. _llvm-update-compiler-configuration-file:
			
-  Update the :ref:`~/.emscripten <compiler-configuration-file>` file, specifying the location of *fastcomp* in using the ``LLVM_ROOT`` variable. The path should be set to the location of the *clang* binary under the **build** directory. This will be something like **<LLVM root>/build/Release/bin** or **<LLVM root>/build/bin**: 

	::
	
		LLVM_ROOT='/home/ubuntu/yourpath/emscripten-fastcomp/build/bin'
		
	.. note:: If **~/.emscripten** does not yet exist, you can create it by running ``./emcc --help`` in your **emscripten** directory (assuming Emscripten has already been downloaded).

Branches
---------

You should use the **same** branch (*incoming*, or *master*) for building all three repositories:

- Emscripten: `emscripten <https://github.com/kripken/emscripten>`_.
- Emscripten's LLVM fork: `emscripten-fastcomp <https://github.com/kripken/emscripten-fastcomp>`_.
- Emscripten's *Clang* fork `emscripten-fastcomp-clang <https://github.com/kripken/emscripten-fastcomp-clang>`_. 

Mixing *incoming* and *master* branches may result in errors when building the three repositories.

Run ``emcc -v`` to check if the branches are synchronized. 

.. note:: ``emcc -v`` checks the code in the repositories, not the builds. Before building make sure that you fetch the latest changes to LLVM and Clang.

Version numbers
++++++++++++++++

Bisecting across multiple git trees can be hard. We use version numbers to help synchronize points between them:

- `tools/shared.py <https://github.com/kripken/emscripten/blob/master/tools/shared.py>`_ in Emscripten
- `emscripten-version.txt <https://github.com/kripken/emscripten-fastcomp/blob/master/emscripten-version.txt>`_ in fastcomp (llvm)
- `emscripten-version.txt <https://github.com/kripken/emscripten-fastcomp-clang/blob/master/emscripten-version.txt>`_ in fastcomp-clang (clang)

Version numbers are typically ``X.Y.Z`` where

- ``X`` is a major number (changes very rarely)
- ``Y`` is a release number (changes each time we merge incoming to master, so these numbers indicate points where all tests passed), and
- ``Z`` is minor update that is just a sync point between the repos, or is needed when libc changes in emscripten (version changes clear the cache).

Additional compilation notes
----------------------------

-  Windows: To build 64 bit using *CMmake* and *Visual Studio*, use the ``-G "Visual Studio 10 Win64"`` directive. 

	.. note:: Visual Studio 2011 and 2012 don't work yet.
	
-  Windows: To build using *MinGW* replace the ``-G`` directive above with ``-G "MinGW Makefiles"``, and run *mingw32-make* to build (not tested yet).


Backend code structure
----------------------

The backend is in `emscripten-fastcomp <https://github.com/kripken/emscripten-fastcomp>`_. The main file is `JSBackend.cpp <https://github.com/kripken/emscripten-fastcomp/blob/incoming/lib/Target/JSBackend/JSBackend.cpp>`_ but the the other files in that directory (`lib/Target/JSBackend/ <https://github.com/kripken/emscripten-fastcomp/tree/incoming/lib/Target/JSBackend>`_) are also important.


.. _original-compiler-core:

Original compiler core (deprecated)
===================================

The original compiler supported dozens of different code generation modes (no-typed arrays, typed arrays in various modes, **asm.js** vs. **non-asm.js**, etc.), many of which proved not to be particularly efficient. Over time, the compiler became harder to maintain and was susceptible to unpredictable compiler slow-downs. *Fastcomp* was created with a core focus on **asm.js** code generation, which has been shown to give the best results.




Why did this change happen?
---------------------------

*Fastcomp* is a much better compiler:

- It is much more streamlined than the original compiler. It focusses on **asm.js** code generation, which has been shown to give the best results.
- It is much faster and has more predictable performance (often 4x faster or more).
- It requires much less memory.
- It generates better code — as an LLVM backend it integrates more tightly with LLVM. 


Are there downsides?
---------------------------

The main downside is that Emscripten can no longer use a stock build of LLVM, because we have made changes that must be built with LLVM. We hope that the new Emscripten backend will eventually become part of the upstream LLVM, and hence become available in stock builds.

.. note:: You actually **can** still use a stock build, but only because you can make Emscripten use the original compiler if needed. This is not a good idea as the original compiler is nowhere near as fast or as well tested.

There are also a few features that were present in the original compiler that are not present in *Fastcomp* (see the next section).


Features not present in Fastcomp
----------------------------------------

Some features that were present in the original compiler that are not present in *Fastcomp* include:

-  Various deprecated **settings.js** options (e.g. ``FORCE_ALIGNMENT``, ``HEAP_INIT``, etc.) have no effect. You should receive a compile-time error if you use a setting which is not yet supported.
-  Linking of **asm.js** shared modules has not yet been ported. This is not deprecated, but may need to be reconsidered.

	.. note:: Normal static linking as used by almost all projects works fine, it is just specifically the options ``MAIN_MODULE`` and ``SIDE_MODULE`` that do not work. 

	
How to disable Fastcomp
---------------------------

.. warning:: You should **NOT** disable Fastcomp. If you "really must", this will have the following downsides:

	-  Not taking advantage of the benefits of *Fastcomp* (much faster compilation, better generated code).
	-  The old compiler is deprecated, and consequently less tested.

The original compiler is still present, and you may want to use it if you need a feature that is not yet present in *Fastcomp*. There should be very few such features, as almost everything that is not deprecated or planned to be rewritten has already been ported. 

However, if you do need to, you can use the old compiler by turning off *Fastcomp*. Specifically, by setting ``EMCC_FAST_COMPILER=0`` when you build:
::

    EMCC_FAST_COMPILER=0 emcc [..]


When you disable *Fastcomp* you can use **either** a build from the *Fastcomp* repositories, **or** a stock LLVM build. The latter is less tested, but should work in principle: Disabling *Fastcomp* does not use anything new in the *Fastcomp* repo (neither the new backend, nor the new target triple).

You can check whether *Fastcomp* is enabled by looking at debug output. For example, run ``EMCC_DEBUG=1 emcc tests/hello_world.c`` — if *Fastcomp* is on, then among the output will be:

::

    DEBUG    root: emscript: llvm backend: ...
    DEBUG    root:   emscript: llvm backend took

That shows both the command used to run the backend, and how much time it took. If *Fastcomp* is off on the other hand, the old compiler is used, and you will instead see:

::

    DEBUG    root: emscript: ll=>js
    DEBUG    root:   emscript: scan took ...
    ...
    DEBUG    root: emcc step "emscript (llvm=>js)" took ...

This shows that the old compiler (``ll=>js``) is called, as well as how much time each step takes, and the total time. Again, this is the output for the **old** compiler, so hopefully you will never see it!



.. _fastcomp-faq:

FAQ
===

-  I see ``WARNING: Linking two modules of different target triples`` [..] ``'asmjs-unknown-emscripten' and 'le32-unknown-nacl'``..?
-  You are linking together bitcode files compiled with the old compiler (or older versions of *Fastcomp*) with bitcode files from the new one. This may work in some cases but is dangerous and should be avoided. To fix it, just recompile all your bitcode with the new compiler.

