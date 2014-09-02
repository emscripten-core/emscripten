.. _getting-started-emscripten-from-visual-studio:

===================================================
Getting started using Emscripten from Visual Studio
===================================================

Windows Developers can develop, compile, and launch their Emscripten projects from `Microsoft Visual Studio 2010 <http://go.microsoft.com/?linkid=9709949>`_ (only). This topic shows you how.

.. warning:: Visual Studio 2011 and 2012 are not supported.

Integrating Emscripten and Visual Studio
========================================

Emscripten is integrated into Visual Studio (2010) using the :term:`vs-tool` plugin, which is automatically deployed by the :term:`Windows NSIS Installer Emscripten SDK` if *Visual Studio 2010* is present on your system at install time (it is also possible to :ref:`manually integrate Emscripten into Visual Studio 2010 <emscripten-and-vs2010-manual-integration-on-windows>`).

The *vs-tool* plugin adds an **Emscripten** configuration to the list of available *Solution Configurations* in Visual Studio. Activate this configuration for a solution/project to make Visual Studio run the project build through Emscripten, producing **.html** or **.js** output depending on the configured project properties.


How to build a Visual Studio solution using Emscripten
======================================================

First select the **Emscripten** configuration from the *Configuration Manager* dropdown as the active configuration. Then choose **Build Solution** (F7).


How to launch a project in a web browser from Visual Studio
===========================================================

To launch a project directly to a web browser from Visual Studio: 

#. Create a new environment variable ``EMCC_WEBBROWSER_EXE`` and set it to point to an executable you want to use to browse web pages generated using Emscripten via Visual Studio. Any browser can be used — below we specify *Mozilla Firefox*:

	::

		SETX EMCC_WEBBROWSER_EXE C:/Program Files (x86)/Mozilla Firefox/firefox.exe


#. Right-click on the project to run, choose **Set as Startup Project**, and select **Start without Debugging** (**Ctrl+F5**). 

This should launch the generated **.html** file to the browser you specified in ``EMCC_WEBBROWSER_EXE``.

.. note:: Visual Studio occasionally opens an "Executable for Debug Session" dialog when you hit **Ctrl+F5**. This can be ignored by clicking **Yes** and then **No**, or simply by hitting **Esc** and then tapping **Ctrl+F5** again.


Specifying project compiler settings
====================================

:ref:`Fastcomp <LLVM-Backend>` (Clang+LLVM) and :ref:`Emscripten compiler <emccdoc>` settings can be edited for the project in exactly the same way as other settings in Visual Studio. First make sure that the Emscripten platform configuration is active, right-click on a project, and then choose **Properties** to open the *Project properties* dialog.

The Project Properties should contain two tree nodes: *Clang C/C++* and *Emcc Linker*. These allow you to specify the desired compilation options.

.. _getting-started-emscripten-vs-existing-solution:

Porting an existing solution file to use Emscripten
===================================================

To add Emscripten as a build option for an existing project:

#. Open the *Configuration Manager* dialog for the solution.
#. In the *Active Solution Platform* dropdown, choose **New**.
#. In the *New* dialog, choose **Emscripten** as the platform name, and **Copy settings from Empty**. 

Now you should be able to switch between building the solution for Win32 and Emscripten by selecting the desired option from the *Configuration Manager* dropdown list.


Creating a new Visual Studio solution for Emscripten
====================================================

To create a new Visual Studio project for Emscripten, first create a **Visual C++ Empty Project** (a new project for the Win32 platform). Then convert that project to also build for Emscripten as outlined in the previous section: :ref:`getting-started-emscripten-vs-existing-solution`.
 