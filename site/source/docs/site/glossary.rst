========
Glossary
========

General
=======

.. glossary::
  :sorted:

  XHR
    Contraction of ``XMLHttpRequest``. Emscripten uses XHRs for asynchronously downloading binary data.

  LLVM backend
    A (*Clang*) compiler backend that converts the :term:`LLVM` Intermediate Representation (IR) to code for a specified machine or other languages. In the case of Emscripten, the specified target is JavaScript.

  Minifying
    `Minification <http://en.wikipedia.org/wiki/Minification_(programming)>`_ in JavaScript is the process of removing all unnecessary characters from source code without changing its functionality. At higher optimisation levels Emscripten uses the :term:`Closure Compiler` to minify Emscripten code.

  Relooping
    Recreate high-level loop and ``if`` structures from the low-level labels and branches that appear in LLVM assembly (definition taken from `this paper <https://github.com/emscripten-core/emscripten/blob/master/docs/paper.pdf?raw=true>`_).

  SDL
    `Simple DirectMedia Layer <https://www.libsdl.org/>`_ (SDL) is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D.

  Typed Arrays Mode 2
    *Typed Arrays Mode 2* is the name of the approach used for the current :ref:`emscripten-memory-model`. This is the only memory model supported by the (current) :ref:`Fastcomp <LLVM-Backend>` compiler and is the default memory model for the :ref:`old compiler <original-compiler-core>`.

    The original compiler supported a number of other memory models and compilation modes (see `Code Generation Modes <https://github.com/emscripten-core/emscripten/wiki/Code-Generation-Modes>`_) but *Typed Arrays Mode 2* proved to have, among other benefits, the greatest support for arbitrary code.


  Load-store consistency
    Load-Store Consistency (LSC), is the requirement that after a value with a specific type is written to a memory location, loads from that memory location will be of the same type. So if a variable contains a 32-bit floating point number, then both loads and stores to that variable will be of 32-bit floating point values, and not 16-bit unsigned integers or anything else.

    .. note:: This definition is taken from `Emscripten: An LLVM-to-JavaScript Compiler <https://github.com/emscripten-core/emscripten/blob/master/docs/paper.pdf?raw=true>`_ (section 2.1.1). There is additional detail in that paper.


Emscripten tools and dependencies
=================================

.. glossary::
  :sorted:

  Clang
    Clang is a compiler front end for C, C++, and other programming languages that uses :term:`LLVM` as its back end.

  emcc
    The :ref:`emccdoc`. Emscripten's drop-in replacement for a compiler like *gcc*.

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
    `Java <http://www.java.com/en/download/faq/whatis_java.xml>`_ is a programming language and computing platform. It is used by Emscripten for the code that performs some advanced optimisations. The required version is listed in the :ref:`toolchain requirements <central-list-of-emscripten-tools-and-dependencies>`.

  JavaScript
    `JavaScript <http://en.wikipedia.org/wiki/JavaScript>`_ (`ECMAScript <http://en.wikipedia.org/wiki/ECMAScript>`_) is a programming language that is primarily used as part of a web browser, providing programmatic access to objects within a host environment. With :term:`node.js`, it is also being used in server-side network programming.

    The `asm.js <http://asmjs.org/faq.html>`_ subset of JavaScript is Emscripten's target output language.

  Closure Compiler
    The closure compiler is used to minify Emscripten-generated code at higher optimisations.

  Git
    `Git <http://en.wikipedia.org/wiki/Git_(software)>`_ is a distributed revision control system. Emscripten is hosted on :term:`GitHub` and can be updated and modified using a git client.

  GitHub
    `GitHub <https://github.com/>`_ is a :term:`Git` repository web-based hosting service that also offers project-based collaboration features including wikis, task management, and bug tracking.

    The Emscripten project is hosted on GitHub.

  lli
  LLVM Interpreter
    The `LLVM interpreter (LLI) <http://llvm.org/releases/3.0/docs/CommandGuide/html/lli.html>`_ executes programs from :term:`LLVM` bitcode. This tool is not maintained and has odd errors and crashes.

    Emscripten provides an alternative tool, the :term:`LLVM Nativizer`.

  LLVM Nativizer
    The LLVM Nativizer (`tools/nativize_llvm.py <https://github.com/emscripten-core/emscripten/blob/master/tools/nativize_llvm.py>`_) compiles LLVM bitcode to a native executable. This links to the host libraries, so comparisons of output with Emscripten builds will not necessarily be identical.

    It performs a similar role to the :term:`LLVM Interpreter`.

    .. note:: Sometimes the output of the this tool will crash or fail. This tool is intended for developers fixing bugs in Emscripten.


SDK Terms
=========

The following terms are used when referring to the SDK and :ref:`emsdk`:

.. glossary::

  emsdk
    The :ref:`emsdk` is used to perform all SDK maintenance and can install, update, add, remove and :term:`activate <Active Tool/SDK>` :term:`SDKs <SDK>` and :term:`tools <Tool>`. Most operations are of the form ``./emsdk command``. To access the *emsdk* script, launch the :term:`Emscripten Command Prompt`.

  Tool
    The basic unit of software bundled in the :term:`SDK`. A Tool has a name and a version. For example, **clang-3.2-32bit** is a tool that contains the 32-bit version of the *Clang* v3.2 compiler. Other tools used by *Emscripten* include :term:`Java`, :term:`Git`, :term:`node.js`, etc.

  SDK
    A set of :term:`tools <Tool>`. For example, **sdk-1.5.6-32bit** is an SDK consisting of the tools: clang-3.2-32bit, node-0.10.17-32bit, python-2.7.5.1-32bit and emscripten-1.5.6.

    There are a number of different Emscripten SDK packages. These can be downloaded from :ref:`here <sdk-download-and-install>`.

  Active Tool/SDK
    The :term:`emsdk` can store multiple versions of :term:`tools <Tool>` and :term:`SDKs <SDK>`. The active tools/SDK is the set of tools that are used by default on the *Emscripten Command Prompt*. This compiler configuration is stored in a user-specific persistent file (**~/.emscripten**) and can be changed using *emsdk*.

  emsdk root directory
    The :term:`emsdk` can manage any number of :term:`tools <Tool>` and :term:`SDKs <SDK>`, and these are stored in :term:`subdirectories <SDK root directory>` of the *emsdk root directory*. The **emsdk root** is the directory specified when you first installed an SDK.

  SDK root directory
    The :term:`emsdk` can store any number of tools and SDKs. The *SDK root directory* is the directory used to store a particular :term:`SDK`. It is located as follows, with respect to the :term:`emsdk root directory`: **<emsdk root>\\emscripten\\<sdk root directory>\\**


Site / Sphinx
==============

.. glossary::
  :sorted:

  reStructured text
    Markup language used to define content on this site. See the `reStructured text primer <http://sphinx-doc.org/rest.html>`_.
