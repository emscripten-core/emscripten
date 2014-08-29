.. _Using-Web-Audio-API-from-Cpp-with-the-Embind-val-class:

==========================================================================
Using Web Audio API from C++ with the Embind::val class (wiki-import)
==========================================================================
.. note:: This article was migrated from the wiki (Mon, 04 Aug 2014 23:20) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

(The title of this page is supposed to say C++ but GitHub turns the pluses into spaces.)

Overview
========

embind provides a C++ class, ``emscripten::val``, that offers convenient access to JavaScript values.

With an :cpp:func:`emscripten::val`, you can call it, read and write properties, or coerce it to a C++ value like a ``bool``, ``int``, or ``std::string``.

Its specific capabilities are enumerated in :ref:`val-h`

Example
========

.. code-block:: cpp

	#include <emscripten/val.h>
	#include <stdio.h>
	#include <math.h>

	using namespace emscripten;

	// based on the excellent Web Audio tutorial at
	// http://stuartmemo.com/making-sine-square-sawtooth-and-triangle-waves/

	int main() {
		val AudioContext = val::global("AudioContext");
		if (!AudioContext.as<bool>()) {
			printf("No global AudioContext, trying webkitAudioContext\n");
			AudioContext = val::global("webkitAudioContext");
		}
		printf("Got an AudioContext\n");
		val context = AudioContext.new_();
		val oscillator = context.call<val>("createOscillator");

		printf("Configuring oscillator\n");
		oscillator.set("type", val("triangle"));
		oscillator["frequency"].set("value", val(261.63)); // Middle C

		printf("Playing\n");
		oscillator.call<void>("connect", context["destination"]);
		oscillator.call<void>("start", 0);

		printf("All done!\n");
	}


Compile with:

::

	em++ -O2 -Wall -Werror --bind -o oscillator.html oscillator.cpp
