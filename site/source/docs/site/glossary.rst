===============================
Glossary (under-construction) 
===============================



Emscripten Tools
==================

.. glossary::
	:sorted:

	vs-tool
		Visual Studio 2010 plugin to integrate MinGW, Clang and Emscripten to the VS IDE.  Automatically added by the :term:`Windows NSIS Installer Emscripten SDK` if Visual Studio 2010 is present on the target system. 
		
		.. note:: At time of writing this only supports Visual Studio 2010 (check `here <https://github.com/juj/vs-tool/issues/5>`_ to see if the VS 2012 support has been added).


	Clang
		**HamishW**
		
	emcc
		**HamishW**
	
	emsdk
		**HamishW**
		
	Emscripten Command Prompt
		**HamishW**
		
	Compiler Configuration File
		The :ref:`Compiler Configuration File <compiler-configuration-file>` stores the :term:`active <Active Tool/SDK>` tools and SDKs as defined using :term:`emsdk activate <emsdk>`.
		
	LLVM
		**HamishW**
		
	node
		**HamishW**
	
	


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