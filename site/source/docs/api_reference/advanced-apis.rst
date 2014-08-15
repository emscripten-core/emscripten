.. _api-reference-advanced-apis:

=============
Advanced APIs
=============

This section lists APIs that are not suitable for general use, but which may be useful to developers in some circumstances. These include APIs that are difficult or complicated to use, or which are intended primarily for Emscripten developers.

.. contents:: Table of Contents
    :local:
    :depth: 1


.. _settings-js:

settings.js
============

`settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js>`_ contains default values and options used in various places by the compiler. 

.. Warning :: Many **settings.js** options are highly brittle - certain combinations of options, and combinations of certain options used with some source code, can cause Emscripten to fail badly. This is intended for use by "advanced users", and possibly even only people developing Emscripten itself.


The options in **settings.js** are normally set as command line parameters to *emcc*: ::

	emcc -s OPT=VALUE


While it is possible to edit **settings.js** manually, this is *highly discouraged*. In general **settings.js** defines low-level options that should not be modified. Note also that the compiler changes some options depending on other settings. For example, ``ASSERTIONS`` is enabled by default, but disabled in optimized builds (``-O1+``).

The small number of options that developers may have cause to change should be modified when the *emcc* tool is invoked. For example, ``EXPORTED_FUNCTIONS``: ::

	./emcc tests/hello_function.cpp -o function.html -s EXPORTED_FUNCTIONS="['_int_sqrt']"

	
allocate()
===========

``allocate()`` is documented in `preamble.js <https://github.com/kripken/emscripten/blob/master/src/preamble.js>`_ and marked as *internal* because it is difficult to use (it has been optimized for multiple syntaxes to save space in generated code). Normally developers should instead allocate memory using ``_malloc()``, initialize it with :js:func:`setValue`, etc., but this may be useful for advanced developers in certain cases.

.. todo:: **HamishW** It would be useful to have proper documentation of ``allocate()`` here. This has been deferred in the short term.
	
	
Module.Runtime
================

``Module.Runtime`` gives access to low-level runtime functionality. Some of these, for example ``Runtime.stackSave()`` and ``Runtime.stackRestore()`` may be useful for advanced users.

.. todo:: **HamishW** It would be useful to expand on what is offered by ``Module.Runtime``. This has been deferred.

