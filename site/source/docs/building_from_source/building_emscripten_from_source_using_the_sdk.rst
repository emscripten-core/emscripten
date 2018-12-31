.. _building-emscripten-from-source-using-the-sdk:

=============================================
Building Emscripten from Source using the SDK
=============================================

The SDK can be used to fetch and build Emscripten from the latest sources in the Emscripten and :ref:`Fastcomp <LLVM-Backend>` repositories. This allows you to obtain new features and defect fixes as soon as they are pushed to Github. 

:ref:`Contributors <contributing>` can replace these sources with their own forks of Emscripten and *Fastcomp*, build them, and then use the normal process to pull changes into the main repository.

The instructions below explain how use the SDK to build Emscripten from both the main repositories and your own forks (for all supported platforms).

.. note:: The SDK is recommended if you need to manage both SDK and source-built environments within the same user login. 

	For :ref:`contributors <contributing>` the actual process of getting and building the sources for your fork is very similar to the :ref:`manual build process <installing-from-source>`. However the setup is easier because the SDK automatically installs and validates most of the needed tools and dependencies, and manages the compiler configuration file for you.


What you'll need
================

First :ref:`download and install the SDK <sdk-download-and-install>`. The SDK automatically installs and activates *most* of the :ref:`central-list-of-emscripten-tools-and-dependencies`. 

Then :ref:`download and install the compiler toolchain <compiler-toolchain>` for your platform.


.. _building-emscripten-from-the-main-repositories-using-the-sdk:

Building Emscripten from the main repositories
==============================================

The :ref:`emsdk` controls which tool and SDK "targets" are installed and activated. To start using the latest upstream git development branch (``incoming``), run the following commands:

::
	
	# Clone, pull, and build the latest "sdk" Emscripten environment
	./emsdk install sdk-incoming-64bit
		
	# Set the "incoming SDK" as the active version.
	./emsdk activate sdk-incoming-64bit	
	
.. todo:: **HamishW** Check whether the bug (https://github.com/juj/emsdk/issues/13) is fixed and remove the windows specific line if it is.

If you want to use the upstream stable branch ``master``, replace ``-incoming-`` with ``-master-`` in the commands above.

.. note:: The SDK supplies a number of other "repository" targets. These targets are viewed by entering ``./emsdk list`` on the command prompt:

	- ``sdk-incoming-64bit`` is the target for a complete 64-bit Emscripten environment build from the incoming branch of the Emscripten and *Fastcomp* repositories. This is equivalent to:
		
		::
			
			emsdk install clang-incoming-64bit node-0.10.17-64bit python-2.7.5.3-64bit java-7.45-64bit emscripten-incoming git-1.8.3
		
		Other ``sdk-`` targets are provided for the master branch and 32bit builds.
	
	- ``emscripten-incoming`` and ``emscripten-master`` are targets for the Emscripten repository (only)
	- ``clang-incoming-64bit`` is the target for getting and building Fastcomp (only) from its repositories. Other ``clang-`` targets are provided for the master branch and 32bit builds.



Building Emscripten using your own fork
=======================================

:ref:`Emscripten Contributors <contributing>` need to be able to build Emscripten from their own fork — in order to use pull requests to merge changes back into Emscripten. This is achieved by first :ref:`installing source from the main repositories <building-emscripten-from-the-main-repositories-using-the-sdk>` (as described in the previous section), and then replacing the sources with those from the fork.

.. note:: You don't have to replace all of the code as shown below. If you're only working on *Fastcomp*, then there is no need to get your own fork of *Emscripten*.

After installing the main repositories:

#. Get your fork of Fastcomp:

	::
	
		# From your emsdk "root" (where emsdk is installed). Navigate to the LLVM clone:
		cd clang/fastcomp/src
		
		# Add a git remote link to your LLVM repository fork
		git remote add myremote https://github.com/mygituseraccount/emscripten-fastcomp.git
		

		# Fetch the changes in the remote.
		git fetch myremote
		
		# Checkout the code in your LLVM fork to replace the original repository code
		git checkout -b myremote --track myremote/incoming
 
 
#. Get your fork of Fastcomp-Clang:

	::
	
		# Navigate to the tools/clang directory ( <emsdk root>/clang/fastcomp/src/tools/clang )
		cd tools/clang
		
		# Add a git remote link to your Clang repository fork
		git remote add myremote https://github.com/mygituseraccount/emscripten-fastcomp-clang.git
		

		# Fetch the changes in the remote.
		git fetch myremote
		
		# Checkout the code in your Clang fork to replace the original repository code
		git checkout -b myremote --track myremote/incoming

#. Build Fastcomp:

	#. The SDK builds *Fastcomp* into a branch- and build-specific directory — in this case **<emsdk root>/clang/fastcomp/build_incoming_64**. We need to build to the same location.

		::  
		
			#Navigate to the /clang/fastcomp/build_incoming_64
			cd ../../../build_incoming_64
		
		
		
	#. Configure the build using *cmake*:
	
		::
				
			cmake .. -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;JSBackend" -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_EXAMPLES=OFF -DCLANG_INCLUDE_TESTS=OFF
			
		.. note:: On Windows you will need Visual Studio 2012 or newer to build.
	   
	#. Determine the number of available cores on your system (Emscripten can run many operations in parallel, so using more cores may have a significant impact on compilation time):

		- On Mac OS X you can get the number of cores using: **Apple menu | About this mac | System report**. The **Hardware overview** on the resulting dialog includes a *Total number of cores* entry.
		- On Linux you can find the number of cores by entering the following command on the terminal: ``cat /proc/cpuinfo | grep "^cpu cores" | uniq``.
		- On Windows the number of cores is listed on the **Task Manager | Performance Tab**. You can open the *Task Manager* by entering **Ctrl + Shift + Esc** from the Desktop.

	#. Call *make* to build the sources, specifying the number of available cores:

		::
			
			make -j1
			
		At this point Fastcomp is compiled, and available in the same location as the original.


#. Get your fork of Emscripten:

	::
	
		# From your emsdk "root" (where emsdk is installed). Navigate to the Emscripten clone:
		cd emscripten/incoming
		
		# Add a git remote link to your Emscripten repository fork
		git remote add myremote https://github.com/mygituseraccount/emscripten.git
		

		# Fetch the changes in the remote.
		git fetch myremote
		
		# Checkout the code in your LLVM fork to replace the original repository code
		git checkout -b myremote --track myremote/incoming


You can switch back and forth between remotes (the original sources and your fork, or the incoming and master branches) using the ``git checkout`` command. You can switch between the SDK and source using the ``./emsdk activate <target>`` command.

