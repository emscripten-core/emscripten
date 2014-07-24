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

