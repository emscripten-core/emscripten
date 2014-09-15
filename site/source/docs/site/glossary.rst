===============================
Glossary (ready-for-review) 
===============================


.. glossary::
	:sorted:

	XHR
		Contraction of ``XMLHttpRequest``. Emscripten uses XHRs for asynchronously downloading binary data. 
		
	LLVM backend
		A (*Clang*) compiler backend that convert the :term:`LLVM` Intermediate Representation (IR) to code for a specified machine or other languages. In the case of Emscripten, the specified target is JavaScript.
		
	Minifying
		`Minification <http://en.wikipedia.org/wiki/Minification_(programming)>`_ in JavaScript, is the process of removing all unnecessary characters from source code without changing its functionality. A higher optimisation levels Emscripten uses the :term:`Closure Compiler` to minify Emscripten code.
		
	Relooping
		Recreate high-level loop and ``if`` structures from the low-level labels and branches that appear in LLVM assembly (definition from `this paper <https://github.com/kripken/emscripten/blob/master/docs/paper.pdf?raw=true>`_).

	SDL
		`Simple DirectMedia Layer <https://www.libsdl.org/>`_ (SDL) is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D.
	
	Typed Arrays Mode 2
		*Typed Arrays Mode 2* is the name of the approach used for the current :ref:`emscripten-memory-model`. This is the only memory model supported by the (current) :ref:`Fastcomp <LLVM-Backend>` compiler and it is the default memory model for the :ref:`old compiler <original-compiler-core>`. 
		
		The original compiler supported a number of other memory models and compilation modes (see `wiki here <https://github.com/kripken/emscripten/wiki/Code-Generation-Modes>`_) but *Typed Arrays Mode 2* proved to have, among other benefits, the greatest support for arbitrary code.
		
		

Emscripten tools and dependencies
=======================================

.. glossary::
	:sorted:

	vs-tool
		Visual Studio 2010 plugin to integrate MinGW, Clang and Emscripten to the VS IDE.  Automatically added by the :term:`Windows NSIS Installer Emscripten SDK` if Visual Studio 2010 is present on the target system. 
		
		.. note:: At time of writing this only supports Visual Studio 2010 (check `here <https://github.com/juj/vs-tool/issues/5>`_ to see if the VS 2012 support has been added).


	Clang
		Clang is a compiler front end for C, C++, and other programming languages that uses :term:`LLVM` as its back end.
		
	emcc
		The :ref:`emccdoc`. Emscripten's drop in replacement for a compiler like *gcc*.
	
	emsdk
		The :ref:`emsdk` is used to perform all SDK maintenance and can install, update, add, remove and active SDKs and tools.
		
	Emscripten Command Prompt
		The :ref:`emcmdprompt` is used to call Emscripten tools from the command line on Windows.
		
	Compiler Configuration File
		The :ref:`Compiler Configuration File <compiler-configuration-file>` stores the :term:`active <Active Tool/SDK>` tools and SDKs as defined using :term:`emsdk activate <emsdk>`.
		
	LLVM
		`LLVM <http://en.wikipedia.org/wiki/LLVM>`_ is a compiler infrastructure designed to allow optimization of programs written in arbitrary programming languages.
		
	Fastcomp
		:ref:`Fastcomp <LLVM-Backend>` is Emscripten's current compiler core.
		
	node.js
		**Node.js** is a cross-platform runtime environment for server-side and networking applications written in JavaScript. Essentially it allows you to run JavaScript applications outside of a browser context.
	
	Python
		Python is a scripting language used to write many of Emscripten's tools. The required version is listed in the :ref:`toolchain requirements <central-list-of-emscripten-tools-and-dependencies>`.
	
	Java
		Java is required to run the code for some advanced optimisations. The required version is listed in the :ref:`toolchain requirements <central-list-of-emscripten-tools-and-dependencies>`.
		
	Closure Compiler
		The closure compiler is used to minify Emscripten-generated code at higher optimisations.
		
	Git
		Git is a distributed revision control system. Emscripten is hosted on Github and can be updated and modified using a git client.


SDK Terms
=========

The following terms are used when referring to the SDK and :ref:`emsdk`:

.. glossary::

	emsdk
		The :ref:`emsdk <emsdk>` management script is used to control which SDK and tools are present and :term:`active <Active Tool/SDK>` on an installation. Most operations are of the form ``emsdk command``. To access the *emsdk* script, launch the *Emscripten Command Prompt*.

	Tool
		The basic unit of software bundled in the :term:`SDK`. A Tool has a name and a version. For example, **clang-3.2-32bit** is a tool that contains the 32-bit version of the *Clang* v3.2 compiler. Other tools include *Emscripten*, *Java*, *Git*, *Node*, etc.
		
	SDK
		A set of :term:`tools <Tool>`. For example, **sdk-1.5.6-32bit** is an SDK consisting of the tools: clang-3.2-32bit, node-0.10.17-32bit, python-2.7.5.1-32bit and emscripten-1.5.6. 
		
		There are a number of different packages of the Emscripten SDKs including the :term:`Portable Emscripten SDK` and :term:`Windows NSIS Installer Emscripten SDK`. SDKs can be downloaded from :ref:`here <sdk-download-and-install>`.
		
	Active Tool/SDK
		The :term:`emsdk` can store multiple versions of :term:`tools <Tool>` and :term:`SDKs <SDK>`. The active tools/SDK is the set of tools that are used by default on the *Emscripten Command Prompt*. This compiler configuration is stored in a user-specific persistent file (**~/.emscripten**) and can be changed using *emsdk*.
		
	Portable Emscripten SDK
		A portable, no-installer, version of the SDK package. It is identical to the :term:`NSIS SDK installer <Windows NSIS Installer Emscripten SDK>`, except that it does not interact with the Windows registry. This allows Emscripten to be used on a computer without administrative privileges, and means that the installation to be migrated from one location (directory or computer) to another by simply copying/zipping up the directory contents.
		
	Windows NSIS Installer Emscripten SDK
		A Windows NSIS installer of the Emscripten SDK. This registers the Emscripten SDK as a ‘standard’ Windows application, allowing it to be installed and removed from Windows like any other app. This installer also automatically deploys :term:`vs-tool` if Visual Studio 2010 is present on the target system.
		
	emsdk root directory
		The :term:`emsdk` can manage any number of :term:`tools <Tool>` and :term:`SDKs <SDK>`, and these are stored in :term:`subdirectories <SDK root directory>` of the *emsdk root directory*. The **emssdk root** is the directory specified when you first installed an SDK.
		
	SDK root directory
		The :term:`emsdk` can store any number of tools and SDKs. The *SDK root directory* is the directory used to store a particular :term:`SDK`. It is located with respect to the :term:`emsdk root directory` as shown: **<emsdk root>\\emscripten\\<sdk root directory>\\**

		
		
Site / Sphinx
==============		

.. glossary::
	:sorted:

	reStructured text
		Markup language used to define content on this site. See the `reStructured text primer <http://sphinx-doc.org/rest.html>`_.