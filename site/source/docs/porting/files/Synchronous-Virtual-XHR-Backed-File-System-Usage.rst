.. _Synchronous-Virtual-XHR-Backed-File-System-Usage:

================================================
Synchronous Virtual XHR Backed File System Usage
================================================

Emscripten supports lazy loading of binary data from HTTP servers using :term:`XHR`. This functionality can be used to create a backend for synchronous file access from compiled code.

The backend can improve start up time as the whole file system does not need to be preloaded before compiled code is run. It can also be very efficient if the web server supports `byte serving <http://en.wikipedia.org/wiki/Byte_serving>`_ — in this case Emscripten can just read the parts of files that are actually needed. 

.. warning:: This mechanism is only possible in `Web Workers <https://developer.mozilla.org/en/docs/Web/Guide/Performance/Using_web_workers>`_ (due to browser limitations).

.. note:: If byte serving is not supported then Emscripten will have to load the whole file (however big) even if a single byte is read.


Test code
=========

An example of how to implement a synchronous virtual XHR backed file system is provided in the test code at `tests/test_browser.py <https://github.com/kripken/emscripten/blob/1.29.12/tests/test_browser.py#L1313>`_ (see ``test_chunked_synchronous_xhr``). The test case also contains an HTTP server (see `test_chunked_synchronous_xhr_server <https://github.com/kripken/emscripten/blob/1.29.12/tests/test_browser.py#L14>`_) showing CORS headers that might need to be set (if the resources are hosted from the same domain Emscripten runs from, there is no issue).

The tests use `checksummer.c <https://github.com/kripken/emscripten/blob/master/tests/checksummer.c>`_ as the Emscripten-compiled program. This is simply a vanilla C program using synchronous *libc* file system calls like ``fopen()``, ``fread()``, ``fclose()`` etc.

JavaScript code is added (using *emcc*'s :ref:`pre-js <emcc-pre-js>` option) to map the file system calls in **checksummer.c** to a file in the virtual file system. This file is *created* early in Emscripten initialisation using :js:func:`FS.createLazyFile`, but only loaded with content from the server when the file is first accessed by compiled code. The added JavaScript code also sets up communication between the web worker and the main thread. 


Instructions
============

#. 
	You will need to add JavaScript to the generated code to map the file accessed by your compiled native code and the server. 
	
	The test code simply creates a file in the virtual file system using :js:func:`FS.createLazyFile` and sets the compiled code to use the same file (**/bigfile**):

	.. include:: ../../../../../tests/test_browser.py
		:literal:
		:start-after: prejs_file.write(r"""
		:end-before: var doTrace = true;
		:code: javascript	

	.. note::
	
		- The compiled test code (in this case) gets the file name from command line arguments — these are set in Emscripten using :js:attr:`Module.arguments`.
		- The call to create the file is added to :js:attr:`Module.preInit`. This ensures that it is run before any compiled code.
		- The additional JavaScript is added using *emcc*'s :ref:`prejs <emcc-pre-js>` option.

#. 
	The added JavaScript should also include code to allow the web worker to communicate with the original thread. 

	The test code adds the following JavaScript to the web worker for this purpose. It uses ``postMessage()`` to send it's ``stdout`` back to the main thread. 

	.. include:: ../../../../../tests/test_browser.py
		:literal:
		:start-after: var doTrace = true;
		:end-before: """)
		:code: javascript	
		
	.. note:: If you use the above solution, the parent page should probably contain handwritten glue code to handle the ``stdout`` data.
	
#. 
	You will need a page that spawns the web worker. 
	
	The `test code <https://github.com/kripken/emscripten/blob/1.29.12/tests/test_browser.py#L1286>`_ that does this is shown below:

	.. include:: ../../../../../tests/test_browser.py
		:literal:
		:start-after: html_file.write(r"""
		:end-before: html_file.close()
		:code: html






    
