.. _compiler-configuration-file:

==========================================================================
Emscripten Compiler Configuration File (.emscripten) (ready-for-review)
==========================================================================

Purpose
============================================

The *Compiler Configuration File* stores the :term:`active <Active Tool/SDK>` configuration on behalf of the :ref:`emsdk`. The active configuration is the specific SDK and tools that will be used if Emscripten in called on the :ref:`Emscripten Command Prompt <emcmdprompt>`. 


Example .emscripten file usage
==============================

The configuration file is named **.emscripten**. It is user-specific, and is located in the user's root directory (**~/.emscripten** on Linux).

The file should not be updated directly. Instead use the :ref:`emsdk` to activate specific SDKs and tools as needed (``emsdk activate <tool/SDK>``).

Below is a typical **.emscripten** file, taken from a user's root directory on Windows:

::

	import os
	SPIDERMONKEY_ENGINE = ''
	NODE_JS = 'node'
	LLVM_ROOT='C:/Program Files/Emscripten/clang/e1.21.0_64bit'
	NODE_JS='C:/Program Files/Emscripten/node/0.10.17_64bit/node.exe'
	PYTHON='C:/Program Files/Emscripten/python/2.7.5.3_64bit/python.exe'
	EMSCRIPTEN_ROOT='C:/Program Files/Emscripten/emscripten/1.21.0'
	JAVA='C:/Program Files/Emscripten/java/7.45_64bit/bin/java.exe'
	V8_ENGINE = ''
	TEMP_DIR = 'c:/users/hamis_~1/appdata/local/temp'
	COMPILER_ENGINE = NODE_JS
	JS_ENGINES = [NODE_JS]
