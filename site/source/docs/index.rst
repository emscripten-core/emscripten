:orphan:

.. _documentation-home:

========================
Emscripten Documentation
========================


This comprehensive documentation set contains everything you need to know to use Emscripten.

**Getting started:**

- :ref:`introducing-emscripten-index` explains what Emscripten does, why it is needed, its limitations and its licensing. It will help you understand whether Emscripten is the right tool for you.
- :ref:`getting-started-index` walks you through downloading, installing and using the Emscripten SDK.

**Emscripten Fundamentals:**

- :ref:`integrating-porting-index` illustrates the main differences between the native and Emscripten runtime environments, and explains the changes you need to make to prepare your C/C++ code for the Web.
- :ref:`Optimizing-Code` shows how to optimise your code for size and performance.
- :ref:`Optimizing-WebGL` gives tips for how to maximize WebGL rendering performance for your page.
- :ref:`compiling-and-running-projects-index` demonstrates how to integrate Emscripten into your existing project build system.

**Contributing:**

- :ref:`contributing-to-emscripten-index` explains how you can contribute to the project.
- :ref:`installing-from-source` explains how to build Emscripten from sources on GitHub (this is useful for contributors).
- :ref:`about-this-site` describes the documentation tools and writing conventions used to create this site.

**Reference:**

- :ref:`api-reference-index` is a reference for the Emscripten toolchain.
- :ref:`settings-reference` is a reference of all the Emscripten compiler settings.
- :ref:`tools-reference` is a reference for the Emscripten integration APIs.
- :ref:`Sanitizers` shows how to debug with sanitizers.
- :ref:`Module-Splitting` is a guide to splitting modules and deferring the
  loading of code to improve startup time.

The full hierarchy of articles, opened to the second level, is shown below.

.. toctree::
  :maxdepth: 2

  introducing_emscripten/index
  getting_started/index
  porting/index
  optimizing/Optimizing-Code
  optimizing/Optimizing-WebGL
  optimizing/Profiling-Toolchain
  optimizing/Module-Splitting
  compiling/index
  building_from_source/index
  contributing/index
  api_reference/index
  tools_reference/index
  debugging/Sanitizers
  site/index
