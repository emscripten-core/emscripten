================================
Getting Started with Emscripten
================================

.. note:: This is fragment from the wiki topic Emscripten SDK. Not yet got a home.

The tools in the Emscripten toolchain can be accessed in various ways.

Which one you use depends on your preference.

Command line usage
-------------------

The Emscripten compiler is available on the command line by invoking
``emcc`` or ``em++``. They are located in the folder
``emsdk/emscripten/<version>/`` in the SDK.

The root directory of the Emscripten SDK contains scripts
``emsdk_env.bat`` (Windows) and ``emsdk_env.sh`` (Linux, OSX) which set
up ``PATH`` and other environment variables for the current terminal.
After calling these scripts, ``emcc``, ``clang``, etc. are all
accessible from the command line.

Check out the tutorial! See the Emscripten `Tutorial <https://github.com/kripken/emscripten/wiki/Tutorial>`__ page for help on how to get going with the tools from command line.

Windows: Emscripten Command Prompt
--------------------------------------

Start the Emscripten Command Prompt from Start Menu -> All Programs -> Emscripten -> Emscripten Command Prompt. This will spawn a new command prompt that has all the tools for the currently activated SDK version set to PATH. The Emscripten Command Prompt is analogous to the Visual Studio Command Prompt that ships with installations of Visual Studio.

Windows: Use Visual Studio 2010
--------------------------------------

After installing the vs-tool plugin, a new 'Emscripten' platform will appear to the list of all Solution Platforms in Visual Studio. To activate the Emscripten platform, right-click on the solution in the Solution Explorer, choose Configuration Manager... -> Active solution platform... -> New... -> Emscripten. After that, activating the Emscripten platform for the solution will make Visual Studio run the project build through Emscripten, producing .html or .js output, depending on the project properties you set up.

Note: If you copied the Emscripten platform properties from the Win32 platform, be sure to go and clean up any leftover Win32-specific #defines and other configuration from the Emscripten platform!


Raw HTML for toggle text
=========================

There is also a good example of a Sphinx plugin that could add this directive: http://scopatz.github.io/hiddencode/

.. raw:: html

	<a href="javascript:hideshow(document.getElementById('adiv1'))">Installation instructions1</a>

	<div id="adiv1" style="display:none;">
	1.The NSIS installers register the Emscripten SDK as a 'standard' Windows application. To install the SDK, download an NSIS .exe file (see above), double-click on it, and run through the installer to perform the installation. After the installer finishes, the full Emscripten toolchain will be available in the directory that was chosen during the installation, and no other steps are necessary. If your system has Visual Studio 2010 installed, the vs-tool MSBuild plugin will be automatically installed as well.
	</div>

.. raw:: html
	<script type="text/javascript">
	function hideshow(which){
	if (!document.getElementById)
		return
	if (which.style.display=="none")
		which.style.display="block"
	else
		which.style.display="none"
	}
	
	</script>	

Some text that probably belongs in installing SDK from source.
========================================================================

This was cut from an SDK installation topic:

"Run ``emsdk_env.bat`` (Windows) or ``source ./emsdk_env.sh`` (Linux and OSX) to set up the environment for the calling terminal." from topic "How do I check the installation status and version of the SDK and tools?". I think this is part of manual setup and doesn't belong here.

Text from tutorial that might belong in "introducting emscripten" and troubleshooting
===========================================================================================================


Cross-compiling
---------------------------

The main 'under the hood' topic to be aware of is that **emcc is a cross-compiler**: You are on a 'normal' OS, running native code, but using emcc you are building for a different environment, JavaScript.

Other examples of cross-compiling are building for an ARM phone on an x86 desktop, etc. When cross-compiling, the thing to keep in mind is that you need to build with settings for the target platform, not the one you are currently on. For that reason, Emscripten (and other cross-compilers) ship with a complete build environment, including system headers and so forth. When you run emcc, it does **not** use your /usr/include directory, instead it uses the system headers bundled with Emscripten (in system/include). One thing to be aware of is if you build a project that has hardcoded includes, for example
``-I/usr/include/something``: Using system headers that way is dangerous when you are cross-compiling, since the headers are meant for your local system, not for the platform you are actually building for.

Emscripten options
---------------------------

The Emscripten compiler (the core code called by emcc that translates LLVM assembly to JavaScript) has various options, which sometimes are useful to modify. To see the options look in ``src/settings.js``, they appear there with descriptions of what they do in comments. To modify a setting, use the ``-s`` option to emcc, for example

::

    emcc source.cpp -s TOTAL_STACK=10000000

This invocation of emcc will generate JavaScript that sets aside a lot of space for the stack.