==========================================
Advanced/Internal APIs (ready-for-review) 
==========================================

This section lists APIs which are not considered suitable for general use, but which may be useful to some developers in some circumstances. These include APIs that are difficult or complicated to use, or which are intended primarily for Emscripten developers.

.. contents:: Table of Contents
    :local:
    :depth: 1




settings.js
============

`settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js>`_ contains default values and options used in various places by the compiler. 

.. Warning :: Many **settings.js** options are highly brittle - certain combination of options and some options used with some source code, can cause Emscripten to fail badly. This is intended for use by "power users", and possibly even only people developing Emscripten itself.

The options are normally set as command line parameters to *emcc*: ::

	emcc -s OPT=VALUE


While it is possible to edit **settings.js** manually, this is *highly discouraged*. In general **settings.js** defines low-level options that should not be modified. Note also that the compiler changes some options depending on other settings. For example, ``ASSERTIONS`` is disabled in optimized builds (``-O1+``), but enabled by default in default (non-optimized) builds.

The small number of options that people do need to change should be modified when the tool is invoked. For example, ``EXPORTED_FUNCTIONS``: ::

	./emcc tests/hello_function.cpp -o function.html -s EXPORTED_FUNCTIONS="['_int_sqrt']"

	
allocate()
===========

``allocate()`` is documented in `preamble.js <https://github.com/kripken/emscripten/blob/master/src/preamble.js>`_ and marked as *internal* because it is difficult to use (it has been optimized for multiple syntaxes to save space in generated code). Normally developers should instead allocate memory using ``_malloc()``, initialize it with :js:func:`setValue`, etc., but this may be useful for advanced developers in certain cases.
	
	
Module.Runtime
================

``Module.Runtime`` gives access to some low-level things in the runtime. Some of these, for example ``Runtime.stackSave()`` and ``Runtime.stackRestore()`` may be useful for advanced users.



emscripten_jcache_printf_()
===========================

``emscripten_jcache_printf_()`` is an internal API documented in `emscripten.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/emscripten.h>`_.