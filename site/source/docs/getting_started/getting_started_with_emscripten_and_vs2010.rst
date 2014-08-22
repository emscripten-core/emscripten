.. _getting-started-emscripten-from-vs2010:

===========================================================================================
Getting started using Emscripten from the Visual Studio 2010 IDE (under-construction)
===========================================================================================

Windows Developers who want to use an IDE can develop and compile their Emscripten projects from `Microsoft Visual Studio 2010 <http://go.microsoft.com/?linkid=9709949>`_. This article shows you how.

.. note:: Visual Studio 2011 and 2012 are not supported.

Integrating Emscripten and Visual Studio
========================================

The integration is enabled the :term:`vs-tool` plugin.

This plugin is automatically deployed by the :term:`Windows NSIS Installer Emscripten SDK` if *Visual Studio 2010* is present on your system. It is also possible to :ref:`manually integrate Emscripten into Visual Studio 2010 <emscripten-and-vs2010-manual-integration-on-windows>`.

The *vs-tool* plugin adds an 'Emscripten' configuration to the list of available *Solution Configurations* in Visual Studio. Activate this configuration for a solution/project to make Visual Studio run the project build through Emscripten, producing **.html** or **.js** output depending on the configured project properties.

How to build a Visual Studio solution using Emscripten
================================================================================

First select the "Emscripten" configuration from the Configuration Manager dropdown as the active configuration, and choose Build Solution (F7).


How to launch a project in a web browser from Visual Studio
================================================================================

To launch a project directly to a web browser from Visual Studio, right-click on the project to run, choose "Set as Startup Project", and select Start without Debugging (Ctrl+F5). This should launch the generated .html file to the browser you specified in EMCC\_WEBBROWSER\_EXE. A bug(?) in Visual Studio causes a "Executable for Debug Session" dialog to occasionally open up when you hit Ctrl+F5. This can be ignored by clicking Yes, then No, or simply by hitting Esc and then tapping Ctrl+F5 again.

llvm-clang and Emscripten compiler settings can be edited for the project in the "usual" Visual Studio fashion: First make sure that the Emscripten platform configuration is active, then right-click on a project, and choose Properties. In the Project properties dialog, two
tree nodes Clang C/C++ and Emcc Linker should exist, which allow you to specify the desired compilation options.

If you have an existing solution file you want to build using Emscripten, open the Configuration Manager.. dialog for the solution, and in the Active Solution Platform dropdown, choose New.... In the new dialog, choose Emscripten as the platform name, and Copy settings from
Empty. After this, you can switch between building the solution for Win32 and Emscripten from the Configuration Manager dropdown list.

When you want to create a new Visual Studio project for Emscripten, we recommend you start with a Visual C++ Empty Project. This will create a new project for the Win32 platform. Then convert that project for Emscripten as outlined in the previous paragraph.


