.. _Using-Emscripten-on-Windows:

=========================================
Using Emscripten on Windows (wiki-import)
=========================================

.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

This page describes how to use Emscripten on Windows.

Setting up the Emscripten Toolchain on Windows
----------------------------------------------

This section contains basic instructions on how to start using Emscripten on a clean Windows box. After these steps, you can invoke emcc from the command line by calling 'python emcc'. 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects.

Installing required tools
~~~~~~~~~~~~~~~~~~~~~~~~~

1. Install Python 2.x (not 3.x) from http://www.python.org/.

	- e.g. `python-2.7.5.amd64.msi <http://python.org/ftp/python/2.7.5/python-2.7.5.amd64.msi>`_ for a 64-bit install.
	- If you plan to use the Visual Studio integration, do NOT install the 64-bit version. Get the 32-bit version of python from `python-2.7.5.msi <http://python.org/ftp/python/2.7.5/python-2.7.5.msi>`_.
	- Add the Python bin directory to your PATH. (Open up System Settings, Advanced system properties, click Environment Variables. Add "C:27" to the PATH, separated by a semicolon.) If on the other hand ``python.exe`` appears in "C:27" and not under bin, just add "C:27".

2. Install node.js from http://nodejs.org/

	-  e.g. `node-v0.10.17-x64.msi <http://nodejs.org/dist/v0.10.17/x64/node-v0.10.17-x64.msi>`_
	-  If you plan to use the Visual Studio integration, do NOT install the 64-bit version. Get the 32-bit version of node.js from `node-v0.10.17-x86.msi <http://nodejs.org/dist/v0.10.17/node-v0.10.17-x86.msi>`_.

3. Setup gcc 4.6 and llvm-clang 3.2.

	-  If you like building the tools yourself or plan to use the Visual Studio integration (for which you need 32-bit versions of the tools) see below for how to build LLVM/Clang from source.
	-  Otherwise download the following 64-bit builds:

		-  `x86\_64-w64-mingw32-gcc-4.6.3-2-release-win64\_rubenvb.7z <http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/rubenvb/gcc-4.6-release/x86_64-w64-mingw32-gcc-4.6.3-2-release-win64_rubenvb.7z/download>`_
		-  `x86\_64-w64-mingw32-clang-3.2-release-win64\_rubenvb.7z <http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/rubenvb/clang-3.2-release/x86_64-w64-mingw32-clang-3.2-release-win64_rubenvb.7z/download>`_

	-  Extract both of them using `7zip <http://7-zip.org>`__ to the same location, e.g. C:64

4. If you do not already have a git client, Install `Github for Windows <http://windows.github.com/>`_.
5. Clone the ``kripken/emscripten`` repository from github. The emscripten repository contains the main compiler tool for compiling C/C++ programs to JavaScript.

	-  Launch the GitHub for Windows Client. You may click "Skip Setup" if you don't have a github account.
	-  (optional) Select "Options" from the gear menu, and customize the default storage directory. A path without spaces can save you lots of pain down the line.
	-  In your web browser, open https://github.com/kripken/emscripten and press the "Clone in Windows" button.
	
		.. note:: If you already had a different git client, the repository URL is ``https://github.com/kripken/emscripten.git`` (e.g. ``git clone https://github.com/kripken/emscripten.git``)

	
Configuring Emscripten
~~~~~~~~~~~~~~~~~~~~~~

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

   
Integrating Emscripten to Visual Studio 2010
--------------------------------------------

It is possible to integrate the Emscripten compiler directly to Visual Studio. This allows existing VS .sln files to be directly built using *emcc*. The Microsoft compiler property pages are replaced by *Clang* and emcc -specific options.

1. Install Visual Studio 2010

	.. note:: Express version has not been tested, but should be ok. Please drop a line on Emscripten IRC channel or mailing list to report your experience with VS2010 Express!

2. Clone the juj/vs-tool repository from github:

	-  git clone https://github.com/juj/vs-tool
	-  The vs-tool repository contains a plugin to Visual Studio 2010 (or rather, to MSBuild) that instructs Visual Studio how to use the llvm-clang and emscripten toolchain.

3.  Install vs-tool itself.
4.  Navigate to folder vs-tool
5.  Copy the folders Clang and Emscripten to C:Files (x86).Cpp.0\\.
6.  Set up the required environment variables for vs-tool.
7.  Windows 7: Open Control Panel -- System -- Advanced System Settings -- Environment Variables.
8.  Create a new environment variable 'CLANG\_BIN' and set it to point to the folder where you built clang to (path to clang.exe) E.g.

    -  SETX CLANG\_BIN C:-build
    -  This is how vs-tool locates the folder to your llvm-clang compiler. Alternatively, this path can be set per-project in Visual Studio project property pages.

9.  Create a new environment variable 'EMSCRIPTEN' and set it to point to the Emscripten git repository root folder (the path where you copied emcc.exe and other exes to). E.g.

    -  SETX EMSCRIPTEN C:
    -  This is how vs-tool locates the Emscripten compiler. This path
       can also be overridden per-project in VS property pages.

10. Create a new environment variable 'EMCC\_WEBBROWSER\_EXE' and set it to point to an executable you want to use to browse Emscripten-generated web pages from Visual Studio. This process is spawned when you tap Start (Ctrl-F5) in Visual Studio for an Emscripten-targeted project.

    -  SETX EMCC\_WEBBROWSER\_EXE C:Files (x86)Firefox.exe

11. Now you are all set to start developing Emscripten-based projects from Visual Studio. To test the installation, open the Visual Studio solution file 10\_msvc10.sln.


Tips for using Emscripten from the Visual Studio 2010 IDE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To build a Visual Studio solution using the Emscripten toolchain, select the "Emscripten" configuration from the Configuration Manager dropdown as the active configuration, and choose Build Solution (F7).

To launch a project directly to a web browser from Visual Studio, right-click on the project to run, choose "Set as Startup Project", and select Start without Debugging (Ctrl+F5). This should launch the generated .html file to the browser you specified in EMCC\_WEBBROWSER\_EXE. A bug(?) in Visual Studio causes a "Executable for Debug Session" dialog to occasionally open up when you hit Ctrl+F5. This can be ignored by clicking Yes, then No, or simply by hitting Esc and then tapping Ctrl+F5 again.

llvm-clang and Emscripten compiler settings can be edited for the project in the "usual" Visual Studio fashion: First make sure that the Emscripten platform configuration is active, then right-click on a project, and choose Properties. In the Project properties dialog, two
tree nodes Clang C/C++ and Emcc Linker should exist, which allow you to specify the desired compilation options.

If you have an existing solution file you want to build using Emscripten, open the Configuration Manager.. dialog for the solution, and in the Active Solution Platform dropdown, choose New.... In the new dialog, choose Emscripten as the platform name, and Copy settings from
Empty. After this, you can switch between building the solution for Win32 and Emscripten from the Configuration Manager dropdown list.

When you want to create a new Visual Studio project for Emscripten, we recommend you start with a Visual C++ Empty Project. This will create a new project for the Win32 platform. Then convert that project for Emscripten as outlined in the previous paragraph.

Building LLVM/Clang from Source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In case the above-referenced prebuilt binaries do not work for you, you can build LLVM and Clang from source.

To install and set up ``llvm-clang 3.2``, open a Visual Studio 2010 command prompt and type:

1. ``svn co http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_32/final llvm``
2. ``cd llvm\tools``
3. ``svn co http://llvm.org/svn/llvm-project/cfe/tags/RELEASE_32/final clang``
4. ``cd ..\\..``
5. ``mkdir build``
6. ``cd build``
7. ``cmake -G "Visual Studio 10" ..\llvm``
8. ``msbuild LLVM.sln /p:Configuration=Release``

If you have problems with the above, see more help here: http://clang.llvm.org/get\_started.html

.. note:: Instead of checking out the trunk as the guide suggests, check out the latest tagged release of Clang (3.2 at the time of writing). An older version will not most likely work with Emscripten.
