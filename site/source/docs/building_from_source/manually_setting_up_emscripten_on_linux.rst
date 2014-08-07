.. _manually-setting-up-emscripten-on-linux:

============================================================
Manually setting up Emscripten on Linux (under-construction)
============================================================

.. warning:: Under Construction - not started

This page contains basic instructions on how to set up Emscripten on a clean (Ubuntu 14.04.1 LTS 64bit) Linux box. After these steps, you can invoke emcc from the command line by calling 'python emcc'. 

**HamishW** confirm that you can't use paths set up by emscripten - you have to actually specify which emcc you want to run? on Linux.

.. tip: There is a :ref:`Portable SDK for Linux <portable-emscripten-sdk-linux-osx>` (and Mac OS X) which you can use instead of the process outlined here. As pre-built binaries of tools are not available on Linux, this SDK automatically clones and build the tools from the sources inside the **emsdk** directory. The benefit of this approach is that you automatically get the latest versions of clang built for you from source.


Installing required tools
============================================================

The system needs the following tools: *Python 2.x*, *Java*, *node.js*, *gcc 4.6*, *fastcomp* (LVVM-Clang), a *git* client, and the current Emscripten sources. 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects. 

Some of these are typically present on Linux systems by default (for example *Python*, *node.js* and *Java*). These instructions show how to confirm their presence and install them if needed.

1. Install *Python* (2.7.3 or above)

	- Python is probably already installed. Check by entering the following command on the terminal: ::
	
		python --version
		
	- If python is not present, install it using the *system package manager*: ::
	
		sudo apt-get install python2.7

	**HamishW** Add symlink for Python2? Anything else we need to do?


2. Install *node.js* (0.8 or above; 0.10.17 or above to run websocket-using servers in node)

	- *node.js* may already be installed. Check by entering the following command on the terminal: ::
	
		nodejs --version
		
	- If *node.js* is not present, install it using the *system package manager*: ::
	
		sudo apt-get install nodejs


3. Install gcc 4.6

	- The system must have a working compiler environment because :ref:`emsdk <emsdk>` builds software from the source.


4. Install Git

	- *Git* is needed on Linux, because tools are built directly from the repositories. 

	.. todo:: **HamishW** Add instructions for installing Git on Linux.

5. Clone the ``kripken/emscripten`` repository from github. The emscripten repository contains the main compiler tool for compiling C/C++ programs to JavaScript.

	-  Launch the GitHub for Windows Client. You may click "Skip Setup" if you don't have a github account.
	-  (optional) Select "Options" from the gear menu, and customize the default storage directory. A path without spaces can save you lots of pain down the line.
	-  In your web browser, open https://github.com/kripken/emscripten and press the "Clone in Windows" button.
	
		.. note:: If you already had a different git client, the repository URL is ``https://github.com/kripken/emscripten.git`` (e.g. ``git clone https://github.com/kripken/emscripten.git``)

6. Build :term:`Fastcomp` (LLVM/Clang) from source using the instructions :ref:`here <building-fastcomp-from-source>`.

7. Install Java (1.6.0_31 or later)

	- Java is optional. It is required to use Closure Compiler to minify code.
	- Java may already be installed. Check by entering the following command on the terminal: ::
	
		java -version
		
	- If *Java* is not present, install it using the *system package manager*: ::
	
		sudo apt-get install default-jre



	
Configuring Emscripten
============================================================

1. Open up a command prompt, and change to the directory where you cloned the Emscripten repository. By default with the GitHub client, this will be ``C:\Users\username\Documents\GitHub\emscripten``.
2. Run ``python emcc --help``

	-  You should get a ``Welcome to Emscripten!`` message. Behind the scenes, Emscripten generated a file called ``.emscripten`` in your Windows home folder.
	-  If you do not see this message, make sure that python is properly in your global PATH, and that you are in the Emscripten directory.

3. Edit ``C:\Users\username\.emscripten``:

   1. Edit the variable ``EMSCRIPTEN_ROOT`` to point to the Emscripten root folder, e.g. ``EMSCRIPTEN_ROOT = 'C:/Users/username/Documents/GitHub/emscripten'`` (Note: use forward slashes)
   2. Edit the variable ``LLVM_ROOT`` to point directly to the path where you installed clang.exe, e.g. ::
   
      ``LLVM_ROOT = 'C:/mingw64/bin'``
	  
   3. Edit the variable TEMP\_DIR to point to a valid Windows path on your local system, e.g. ``TEMP_DIR = 'C:/tmp'``, and create that folder on the local filesystem if it doesn't exist.
   4. If you are planning on using Cygwin and cygwin make in the future, copy the .emscripten file to your cygwin home folder as well, e.g. 
   copy C:.emscripten C: (There is a bug in Emscripten that it currently looks for the file .emscripten in two places (Win7 user home folder or cygwin user home folder), depending on the environment it is executed in, so as a current workaround these files must be present and identical on Windows. (see `issue #411 <https://github.com/kripken/emscripten/issues/411>`_))

   
How to locate file and update the configuration?

user/bin/

JAVA='/usr/bin/java'
Add Java 


Notes take from existing doc sources
============================================

Things you'll need
--------------------

- The Emscripten code, from GitHub (git clone git://github.com/kripken/emscripten.git. The master branch is fine, it is guaranteed to always be stable. We merge to master only after all tests pass.)
- Emscripten's LLVM and Clang. Emscripten now has an LLVM backend ("fastcomp"), which means you need to use our LLVM+Clang. See `Getting Fastcomp <LLVM-Backend#getting-fastcomp>`_ in the :doc:`LLVM Backend` page. 

	.. note::
		
		- The `LLVM Backend <https://github.com/kripken/emscripten/wiki/LLVM-Backend>`_ page has instructions for building Emscripten's LLVM+Clang. After you build it, run ``emcc -v``, which should print out the version number as well as run some basic tests.
		- It is possible but not recommended to disable  fastcomp and use a stock version of LLVM (see the link for more information).
		
- **Node.js** (0.8 or above; 0.10.17 or above to run websocket-using servers in node)
- Python 2.7.3
- Optionally, if you want to use Closure Compiler to minify your code as much as possible, you will also need Java.



Additional Notes
--------------------


- Node.js and LLVM should have convenient binaries for your OS, but installing them from source is easy, just compile them in their directories, you don't need to bother with installing them systemwide (you will point Emscripten to them in the next step, where you set up directories).


Linux: guides for manual building
---------------------------------

.. COMMENT - HAMISHW from the MDN - and I suspect no longer needed now we have LLVM Backend. Check and delete if needed. 


The following guides explain setting up Emscripten and its prerequisites manually on Linux:

- For help on Ubuntu, you can follow the :doc:`Getting Started on Ubuntu 12.10` guide for instructions on how to obtain the prerequisites and build Clang manually using CMake.
- For help on Debian, see this `guide by EarthServer <https://earthserver.com/Setting_up_emscripten_development_environment_on_Linux>`_.
- rhelmer has provided a Vagrant VM for Emscripten, see `emscripten-vagrant <https://github.com/rhelmer/emscripten-vagrant>`_.
- Dirk Krause created an `Amazon EC2 image <https://groups.google.com/forum/?fromgroups=#!topic/emscripten-discuss/H8kG0kP1eDE>`_ for Emscripten.

Configuring the basic Emscripten settings file
------------------------------------------------------------------

**HamsihW** What do to with this section depends on how specific it is to particular platforms.  Currently it is in the parent doc. May be separate doc before validation.


Linux - from the portable SDK installation notes
---------------------------------------------------

.. note:: Pre-built binaries of tools are not available on Linux. Installing a tool will automatically clone and build that tool from the sources inside the **emsdk** directory. *Emsdk* does not interact with Linux package managers on the behalf of the user, nor does it install any tools to the system. All file changes are done inside the **emsdk/** directory.

- The system must have a working compiler environment available (because *emsdk* builds software from the source). 
- *Python*, *node.js* or *Java* are not provided by *emsdk*. The user is expected to install these beforehand with the *system package manager*.
- *Git* is not installed automatically. Git is only needed if you want to use tools from one of the development branches **emscripten-incoming** or **emscripten-master**. 

.. todo:: **HamishW** Add instructions for installing Git on Linux.




Installing Node.js
-----------------------

