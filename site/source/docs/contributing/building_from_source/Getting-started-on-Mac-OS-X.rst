.. _Getting-started-on-Mac-OS-X:

=========================================
Getting started on Mac OS X (wiki-import)
=========================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

This guide instructs step-by-step on how to setup emscripten on a clean Mac OS X box. The steps have been tested against OS X version 10.8.2.

Note for `homebrew <https://github.com/Homebrew/homebrew>`__ users:
``brew install emscripten`` does all the steps below for you (including installation of the llvm fastcomp compiler and patching the python interpreter).

1. Install svn if you do not have it yet. It is distributed with XCode    Command Line Tools in OSX App Store. See http://superuser.com/questions/455214/where-is-svn-on-os-x-mountain-lion

	-  Install XCode from OSX App Store.
	-  In XCode->Preferences->Downloads, install Command Line Tools.
	-  Test that svn works from command line: type 'svn --version' in terminal.

2. Install git if you do not have it yet:

	-  Allow installation of unsigned packages, or installing git package won't succeed: https://www.my-private-network.co.uk/knowledge-base/apple-related-questions/osx-unsigned-apps.html
	-  Download and install git from http://git-scm.com/
	-  Test that git works from command line: type 'git --version' in terminal.

3. Install cmake if you do not have it yet:

	-  Download and install cmake-2.8.10.2-Darwin64-universal.dmg or newer from http://www.cmake.org/cmake/resources/software.html
	-  Test that cmake works from command line: type 'cmake --version' in terminal.

4. Build LLVM+Clang 3.2 from SVN repository: 

	::

		cd ~
		svn co http://llvm.org/svn/llvm-project/llvm/tags/RELEASE\_32/final llvm32
		cd llvm32/tools
		svn co http://llvm.org/svn/llvm-project/cfe/tags/RELEASE\_32/final clang
		cd ../..
		mkdir llvm32build
		cd llvm32build
		cmake -DCMAKE\_BUILD\_TYPE=Release -G "Unix Makefiles" ../llvm32
		make

	After these steps, Clang 3.2 will be built into ~/llvm32build/bin. The source tree ~/llvm32 is not needed anymore, if you want to conserve disk space.

	Note that XCode ships with a custom version of Clang (3.1svn or 3.2svn depending on XCode version). It may be possible to omit this step and just use the version provided by XCode, but the LLVM 3.2 RELEASE version is preferred, since that is the "officially supported version" the Emscripten OSX unit tests are run against.

5. Set up Clang 3.2 in PATH: 

	::

		cd ~
		echo "export PATH=~/llvm32build/bin:$PATH" >> .profile

	The above change is permanent and it persists between system restarts. It only affects the current user.

6. Close all terminal windows, and open a new one. Check that clang works ok in path: 

	::

		cd ~
		clang --version

	The command should output: clang version 3.2 (tags/RELEASE\_32/final 176107) Target: x86\_64-apple-darwin12.2.1 Thread model: posix

7. Delete old .emscripten file if it happened to exist: 

	::

		rm ~/.emscripten
		rm -rf ~/.emscripten\_cache

8. Set up node:

	-  Download and install node from http://nodejs.org/
	-  Test that node works from command line: type 'node --version' in terminal.

9. Obtain Emscripten: 

	::

		cd ~
		git clone https://github.com/kripken/emscripten.git

	By default, git will clone the emscripten 'incoming' branch, which is the branch where the newest developments of Emscripten occur. If you want to use a more stable branch, switch to the 'master' branch: - cd ~/emscripten - git checkout master

.. _getting-started-on-osx-install-python2:

10. Setup 'python2': (this step is needed to workaround a bug reported in `#763 <https://github.com/kripken/emscripten/issues/763>`__)

	-  In terminal, type 'python2 --version'. If you get a "command not found", type the following: ::
	
		cd /usr/bin
		sudo ln python python2
		sudo ln ../../System/Library/Frameworks/Python.framework/Versions/2.7/bin/python2.7 python22.7
		
	-  Issue 'python2 --version' again. It should now print Python 2.7.2

11. Perform emscripten "first run": 

	::

		cd ~/emscripten
		/emcc --help
	
	-  edit the file ~/.emscripten in a text editor to set up any required paths.

12. (Optional) Compile a simple test program to check that everything works: 

	::

		cd ~/emscripten
		./em++ tests/hello\_world.cpp
		node a.out.js

	The first run will most likely pop up an automatic prompt that asks to install java. Proceed, and after installation finishes, rerun the above commands. Running node should output: hello, world!

13. (Optional) Compile a simple WebGL program to check that .html output and GLES2/WebGL works: 

	::

		cd ~/emscripten
		./emcc tests/hello\_world\_gles.c -o hello\_world\_gles.html
		open hello\_world\_gles.html
		
	.. note:: If the Safari 6 browser pops up with a message "Could not create canvas :(", follow `these instructions <http://support.apple.com/kb/PH11926>`_ to enable WebGL support in Safari


14. (Optional) Run the full battery of tests to check that Emscripten is perfectly operational on the current platform: ::

	cd ~/emscripten
	python tests/runner.py
	python tests/runner.py benchmark

	Note that some tests will likely fail. Cross-reference the results with https://github.com/kripken/emscripten/issues?labels=tests to see if you are receiving currently unknown issues.
