.. _emscripten-and-vs2010-manual-integration-on-windows:

=======================================================
Manually Integrating Emscripten into Visual Studio 2010
=======================================================

.. tip:: The :term:`vs-tool` plugin is installed automatically by the :term:`Windows NSIS Installer Emscripten SDK` if *Visual Studio 2010* is present on the target system at install time. These instructions are only needed if you're :ref:`manually building from source <installing-from-source>`, or for some other reason cannot use the SDK.

*vs-tool* is an *MSBuild* plugin that tells *Visual Studio* how to use the *Fastcomp* (LLVM + Clang) and Emscripten toolchain. The plugin allows existing Visual Studio **.sln** files to be directly built through the IDE using :ref:`emcc <emccdoc>`. The Microsoft compiler property pages are replaced by *Fastcomp*- and *emcc*- specific options. 

The following instructions explain how to manually set up *vs-tool*.

.. note:: Currently (August 2014) *vs-tool* `only supports <https://github.com/juj/vs-tool/issues/5>`_ Visual Studio 2010.

Install the vs-tool plugin
============================

1. Install `Visual Studio 2010 <http://www.visualstudio.com/downloads/download-visual-studio-vs>`_.

	.. note:: The `Express <http://go.microsoft.com/?linkid=9709949>`_ version has not been tested, but should work. :ref:`contact` if you have any problems integrating with VS2010 Express!

#. Clone the juj/vs-tool repository from github:

	::
	
		git clone https://github.com/juj/vs-tool

#.  Install the *vs-tool* plugin:

	#. Navigate to the **vs-tool** folder in the repository clone.
	#. Copy the folders **Clang** and **Emscripten** to **C:/Program Files (x86)/MSBuild/Microsoft.Cpp/v4.0/Platforms/**.
	
#.  Set up the required environment variables for *vs-tool*.

	#. Open the Environment Variables setting:
	
		- Windows 7: Open **Control Panel | System | Advanced System Settings | Environment Variables**.
		- Windows 8: Right click on the **Windows menu icon | System | Advanced System Settings | Environment Variables**.
		
	#. Create a new environment variable ``CLANG_BIN`` and set it to point to the folder where you built *Fastcomp* (path to **clang.exe**) — for example:
	
		:: 
		
			SETX CLANG_BIN C:/Projects/llvm-build/bin/Release
		
		This variable is how *vs-tool* locates the folder containing your *Fastcomp* compiler. Alternatively, this path can be set on a per-project basis in the *Visual Studio project property* pages.

	#. Create a new environment variable ``EMSCRIPTEN`` and set it to point to the Emscripten git repository root folder (the path where you copied **emcc.exe** and the other exe files), for example:
	
		::

			SETX EMSCRIPTEN C:/Projects/emscripten
			
		This variable is how *vs-tool* locates the Emscripten compiler. This path can also be overridden on a per-project basis in the Visual Studio property pages.

	#. Create a new environment variable ``EMCC_WEBBROWSER_EXE`` and set it to point to an executable you want to use to browse web pages generated using Emscripten via Visual Studio. This process is spawned when you tap **Start** (**Ctrl-F5**) in Visual Studio for an Emscripten-targeted project:
	
		::

			SETX EMCC_WEBBROWSER_EXE C:/Program Files (x86)/Mozilla Firefox/firefox.exe

#. Now you are all set to start developing Emscripten-based projects using *Visual Studio*. 

Test the installation
======================

To test the installation, open and build the Visual Studio solution file **/emscripten/tests/msvc10/tests_msvc10.sln**. 

That project builds a few select samples from inside the Emscripten **tests/** folder ("hello world", "hello gles", etc.), and is a good verification of the integration.


Learn more
==========

To start working with Emscripten using the IDE, see :ref:`getting-started-emscripten-from-visual-studio`.
 