.. _configuring-emscripten-settings:

==================================================================
Configuring Emscripten Settings when Manually Building from Source
==================================================================

.. note:: These instructions are only useful if you are building Emscripten **manually** from source! If you're using the :ref:`SDK <building-emscripten-from-source-using-the-sdk>` you should never manually update the settings, because the :ref:`emsdk` automatically overwrites the file with the appropriate compiler settings when you :term:`activate <Active Tool/SDK>` a tool or SDK.


The compiler settings used by Emscripten are defined in the :ref:`compiler configuration file (~/.emscripten) <compiler-configuration-file>`. These settings include paths to the tools (LLVM, Clang, Java, etc.) and the compiler's temporary directory for intermediate build files.

This article explains how to create and update the file when you are building Emscripten :ref:`manually <installing-from-source>` from source.


Creating the compiler configuration file
========================================

The settings file is created the first time a user runs :ref:`emcc <emccdoc>` (or any of the other Emscripten tools):

1. Navigate to the directory where you cloned the Emscripten repository.
2. Enter the command: 

	::
	
		./emcc --help

	You should get a ``Welcome to Emscripten!`` message. Behind the scenes, Emscripten generates a file called ``.emscripten`` in your home folder.
	
	
Emscripten makes a "best guess" at the correct locations for tools and updates the file appropriately. Where possible it will look for "system" apps (like Python and Java) and infer the location of the ``EMSCRIPTEN_ROOT`` (where :ref:`emcc <emccdoc>` is located) from the location of the command prompt. 

The file will probably not include the link to :term:`Fastcomp` (``LLVM_ROOT``) as a manual source build can create this anywhere.

Locating the compiler configuration file (.emscripten)
=======================================================

The settings file is created in the user's home directory: 

	- On Linux and Mac OS X this file is named **~/.emscripten**, where ~ is the user's home directory. 

		.. note:: Files with the "." prefix are hidden by default. You may need to change your view settings to find the file.


	- On Windows the file can be found at a path like: **C:/Users/yourusername_000/.emscripten**.


Compiler configuration file-format
==================================

.. note:: While the syntax is identical, the appearance of the default **.emscripten** file created by *emcc* is quite different than that created by :ref:`emsdk <compiler-configuration-file>`. This is because *emsdk* manages multiple target environments, and where possible hard codes the locations of those tools when a new environment is activated. The default file, by contrast, is managed by the user — and is designed to make that task as easy as possible.

The file simply assigns paths to a number of *variables* representing the main tools used by Emscripten. For example, if the user cloned Emscripten to the **C:/Users/username/Documents/GitHub/emscripten** directory, then the file would have the line: ::

	EMSCRIPTEN_ROOT = 'C:/Users/username/Documents/GitHub/emscripten'
	

The default *emcc* configuration file often gets the paths from environment variables if defined. If no variable is defined the system will also attempt to find "system executables". For example:  ::

	PYTHON = os.path.expanduser(os.getenv('PYTHON') or 'C:\\Python27\\python2.exe')

You can find out the other variable names from the default *.emscripten* file or the :ref:`example here <compiler-configuration-file>`. 

Editing the compiler configuration file
=======================================

The compiler configuration file can be edited with the text editor of your choice. As stated above, most default settings are likely to be correct. If you're building manually from source, you are most likely to have to update the variable ``LLVM_ROOT`` (for :term:`Fastcomp`).

		
#. Edit the variable ``LLVM_ROOT`` to point to the directory where you :ref:`built Fastcomp <llvm-update-compiler-configuration-file>`. This path is likely to be something like **<LLVM root>/build/Release/bin** or **<LLVM root>/build/bin**, where ``<LLVM root>`` is the path to the directory where you cloned LLVM:
   
	::
   
		LLVM_ROOT = 'os.path.expanduser(os.getenv('LLVM') or '/home/ubuntu/a-path/emscripten-fastcomp/build/bin')'

	.. note:: Use forward slashes!

#. Edit the variable ``TEMP_DIR`` to point to a valid path on your local system, e.g. ``TEMP_DIR = '/tmp'`` (``TEMP_DIR = 'c:/tmp'`` on Windows), and create that folder on the local filesystem if it doesn't exist.

#. You *may* need to edit the variable ``EMSCRIPTEN_ROOT`` to point to the Emscripten root folder, e.g.:
   
	::
   
		EMSCRIPTEN_ROOT = os.path.expanduser(os.getenv('EMSCRIPTEN') or '/home/ubuntu/yourpath/emscripten') # directory
 

.. comment .. The settings are now correct in the configuration file, but the paths and environment variables are not set in the command prompt/terminal. **HamishW** Follow up with Jukka on this.
 
After setting those paths, run ``emcc`` again. It should again perform the sanity checks to test the specified paths. There are further validation tests available at :ref:`verifying-the-emscripten-environment`.





	
	




	


 


