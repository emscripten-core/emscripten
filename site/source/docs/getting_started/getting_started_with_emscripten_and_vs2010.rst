.. getting-started-emscripten-from-vs2010:

===========================================================================================
Getting started using Emscripten from the Visual Studio 2010 IDE (under-construction)
===========================================================================================


To build a Visual Studio solution using the Emscripten toolchain, select the "Emscripten" configuration from the Configuration Manager dropdown as the active configuration, and choose Build Solution (F7).

To launch a project directly to a web browser from Visual Studio, right-click on the project to run, choose "Set as Startup Project", and select Start without Debugging (Ctrl+F5). This should launch the generated .html file to the browser you specified in EMCC\_WEBBROWSER\_EXE. A bug(?) in Visual Studio causes a "Executable for Debug Session" dialog to occasionally open up when you hit Ctrl+F5. This can be ignored by clicking Yes, then No, or simply by hitting Esc and then tapping Ctrl+F5 again.

llvm-clang and Emscripten compiler settings can be edited for the project in the "usual" Visual Studio fashion: First make sure that the Emscripten platform configuration is active, then right-click on a project, and choose Properties. In the Project properties dialog, two
tree nodes Clang C/C++ and Emcc Linker should exist, which allow you to specify the desired compilation options.

If you have an existing solution file you want to build using Emscripten, open the Configuration Manager.. dialog for the solution, and in the Active Solution Platform dropdown, choose New.... In the new dialog, choose Emscripten as the platform name, and Copy settings from
Empty. After this, you can switch between building the solution for Win32 and Emscripten from the Configuration Manager dropdown list.

When you want to create a new Visual Studio project for Emscripten, we recommend you start with a Visual C++ Empty Project. This will create a new project for the Win32 platform. Then convert that project for Emscripten as outlined in the previous paragraph.

