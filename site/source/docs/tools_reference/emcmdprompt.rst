.. _emcmdprompt:

===================================================
Emscripten Windows Command Prompt (emcmdprompt.bat)
===================================================

The *Emscripten Command Prompt* is used to call Emscripten tools from the command line on Windows.  

The prompt is added to the *Windows Start Menu* programs by the :term:`NSIS Installer <Windows NSIS Installer Emscripten SDK>`. It can be launched by typing **Emscripten** in the Windows 8 start screen, and then selecting the **Emscripten Command Prompt** option. The prompt can also be launched from the file system by locating and opening the file **emcmdprompt.bat**.

.. note:: The *Emscripten Command Prompt* is configured with the correct system paths and settings to point to the :term:`active <Active Tool/SDK>` Emscripten tools. From within the prompt you can call :ref:`emsdk <emsdk>` to change the current SDK or tools.

.. _emcmdprompt-command-line-syntax:

Command line syntax
============================================

The tool is not *intended* to be run from the command line. 

However, if **emcmdprompt.bat** is called on the command line, a nested Emscripten command prompt is spawned within the current prompt (note that this is not necessarily obvious). You can then use this prompt to access the SDK tools. To exit the nested command prompt, type **exit** and then press the **Enter** key. 
