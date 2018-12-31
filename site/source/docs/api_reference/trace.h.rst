.. _trace-h:

=======
trace.h
=======

The Emscripten tracing API provides some useful capabilities to better see
what is going on inside of your application, in particular with respect to
memory usage (which is otherwise not available to traditional browser
performance tools).

The tracing API can talk to a custom collection server (see `Running the Server`_
for more details) or it can talk with the `Google Web Tracing Framework`_.
When talking with the `Google Web Tracing Framework`_, a subset of the data
available is collected.


.. contents:: table of contents
   :local:
   :depth: 2

Usage
=====

Compiler Interaction
--------------------

When using the tracing API, you should pass ``--tracing`` to ``emcc`` at each
compile and link stage. This
will automatically include the ``library_trace.js`` library file as well as
set the preprocessor flag ``__EMSCRIPTEN_TRACING__``. If you are invoking
``clang`` directly to build your C / C++ code, then you will want to pass
``-D__EMSCRIPTEN_TRACING__`` when building code. When the preprocessor
flag ``__EMSCRIPTEN_TRACING__`` is not defined, the tracing API implementation
will be provided by inlined empty stubs.

Also, since enabling tracing modifies the implementation of ``dlmalloc.c``
in the ``libc`` implementation, it is advised that you manually clear your
cache before switching to using the tracing API. If you do not do this, then
you will not get full allocation details recorded.  You can clear the cache
with this ``emcc`` command::

    emcc --clear-cache

Initialization and Teardown
---------------------------

To initialize the tracing API, you call :c:func:`emscripten_trace_configure`:

.. code-block:: c

  emscripten_trace_configure("http://127.0.0.1:5000/", "MyApplication");

If you are simply going to use the tracing API with the `Google Web Tracing
Framework`_, then you can just call :c:func:`emscripten_trace_configure_for_google_wtf`
instead:

.. code-block:: c

  emscripten_trace_configure_for_google_wtf();

If you have the concept of a username or have some other way to identify
a given user of the application, then passing that to the tracing API
can make it easier to identify sessions in the collector server:

.. code-block:: c

  emscripten_trace_set_session_username(username);

To shut it down at application exit, you simply call
:c:func:`emscripten_trace_close`:

.. code-block:: c

  emscripten_trace_close();

Contexts
--------

Contexts are a way to tell the tracing API what part of your application
is currently running. Contexts are effectively maintained as a stack of
current contexts.

A context might be something as big as "running physics" or as small
as "updating animations on entity X".

The granularity of the context stack is up to the team instrumenting
their application. Some applications may find fine-grained contexts
more useful, while others are more comfortable with larger contexts.

Rather than getting a stack trace on every tracing call, we can often
look at the current context stack and record that instead, which is
much cheaper.

When contexts are fully implemented by the server, they will also be
used to track how much time is spent in each context (a primitive
profiling mechanism), as well as how much memory has been allocated
and freed while the context was active. This should help give a good
idea of which parts of your application are using more memory or
creating a lot of churn (and possibly heap fragmentation).

Recording context entry and exit is simple:

.. code-block:: c

  emscripten_trace_enter_context("Physics Update");
  ...
  emscripten_trace_exit_context();

Frames
------

It is important to record where your frame or event loop begins
and ends. This allows the tracing API to perform useful additional
analysis.

Noting the start of an event loop is as easy as:

.. code-block:: c

  emscripten_trace_record_frame_start();

And noting the end of the event loop is just as easy:

.. code-block:: c

  emscripten_trace_record_frame_end();

Annotating Allocations
----------------------

Each allocation and free operation should be recorded. Ideally,
the data type name will also be recorded, but this must currently
be done manually.

When building with ``--tracing`` and a cleared cache, the ``libc``
that Emscripten builds will automatically record all calls to
``malloc``, ``realloc`` and ``free``.

As for recording the data type name, after you've allocated the
memory, you can annotate the address:

.. code-block:: c

  emscripten_trace_annotate_address_type(model, "UI::Model");

Additionally, some applications may want to associate the size
of additional storage with an allocation. This can be done via
:c:func:`emscripten_trace_associate_storage_size`:

.. code-block:: c

  emscripten_trace_associate_storage_size(mesh, mesh->GetTotalMemoryUsage());

Overall Memory Usage
--------------------

Periodically, the overall heap layout and memory usage should
be reported to the trace API.

This is done with 2 calls:

.. code-block:: c

  emscripten_trace_report_memory_layout();
  emscripten_trace_report_off_heap_data();

Logging Messages
----------------

Messages can be logged and recorded via the Emscripten tracing API.
These messages can have both a channel and the actual message. The
channel name will help to categorize and filter messages within
the visualization interface. You should avoid allocating memory
on the heap while logging a message.

.. code-block:: c

  emscripten_trace_log_message("Application", "Started");

Over time, the visualization interface will improve to help you
better correlate these log messages with other views, such as
memory usage over time. Logging messages for things that may
cause large amounts of memory activity, like loading a new
model or game asset, is very useful when analyzing memory
usage behavior patterns.

Tasks
-----

Specific tasks can be recorded and analyzed. A task is typically
a unit of work that is not repeating. It may be suspended or
blocked due to having portions performed asynchronously.

An example of a task is loading an asset which usually involves
chains of callbacks.

The application should keep track of task IDs (integers) and
ensure that they are unique.

The task ID need not be passed to every trace call involving
tasks as most calls operate on the current task.

Tasks can be started and stopped with:

.. code-block:: c

  emscripten_trace_task_start(taskID, name);
  emscripten_trace_task_end();

If a task is suspended / blocked, this can be noted via:

.. code-block:: c

  emscripten_trace_task_suspend("loading via HTTP");

And when it is resumed:

.. code-block:: c

  emscripten_trace_task_resume(taskID, "parsing");

It is common to need to associate additional data with the
current task for use when examining task data later. An example
of this would be the URL of an asset that was loaded:

.. code-block:: c

  emscripten_trace_task_associate_data("url", url);

Reporting Errors
----------------

Errors encountered by the application can be reported to the tracing
API as an ancillary service:

.. code-block:: c

  emscripten_trace_report_error("Assertion failed: ...");

This feature is included as an indication of the future direction
of the Emscripten tracing API.

Running the Server
==================

* Obtain a copy of the `emscripten-trace-collector`_ server.
* Follow the directions in the `README.rst`.

Design Notes
============

Client / Server Design
----------------------

The Emscripten tracing API gathers data from instrumented code and transmits
it to a collector server. The server also performs data analysis and
provides a web interface for viewing the collected data.

This client / server design is intended to allow the tool to run without
interfering with the browser on lower-end hardware where memory might
be at a premium, like 32 bit Windows machines.

This design also allows for a single server to be run to collect data
from a variety of clients.

Data Batching
-------------

Data is batched and sent to the server in chunks, roughly once or twice
per second. This avoids having to open a new connection to the server
for every single event being recorded.

Do Not Perturb The Heap
-----------------------

When using the Emscripten tracing API, you should be careful that you do
not perform operations that would perturb the heap. For example, you shouldn't
allocate a string to pass to :c:func:`emscripten_trace_log_message` as
that would result in the allocation being tracked and possibly
disturbing the behavior or results that you are trying to analyze.

For this reason, the Emscripten tracing API also keeps all of its own
data off of the Emscripten heap and performs no writes to the Emscripten
heap.

Functions
=========

.. c:function:: void emscripten_trace_configure(const char *collector_url, const char *application)

   :param collector_url: The base URL for the collector server.
   :type collector_url: const char*
   :param application: The name of the application being traced.
   :type application: const char*
   :rtype: void

   Configure the connection to the collector server.

   This should be one of the very first things that is done after the
   application has started.

   In most cases, the ``collector_url`` will be ``http://127.0.0.1:5000/``.

.. c:function:: void emscripten_trace_configure_for_google_wtf(void)

   :rtype: void

   Configure tracing to communicate with the `Google Web Tracing Framework`_.

   Not all features of the tracing are available within the Google WTF
   tools. (Currently, only contexts, log messages and marks.)

.. c:function:: void emscripten_trace_set_enabled(bool enabled)

   :param enabled: Whether or not tracing is enabled.
   :type enabled: bool
   :rtype: void

   Set whether or not tracing is enabled. Using this option to disable
   tracing will likely result in inaccurate data being collected about
   memory usage.

.. c:function:: void emscripten_trace_set_session_username(const char *username)

   :param username: The username of the person running the application.
   :type username: const char*
   :rtype: void

   This is useful when a collector server is being used by multiple
   people and you want to be able to identify individual sessions
   by a means other than their timestamped session ID.

   This can be set after tracing has already started, so it is fine
   to set this after the user has gone through a login or authentication
   process.

.. c:function:: void emscripten_trace_record_frame_start(void)

   :rtype: void

   This should be called at the start of the frame / event loop.

   The current timestamp is associated with this data.

   The server uses this to track frame times (and therefore frames
   per second), as well as accounting for memory operations that
   happen during the frame processing.

.. c:function:: void emscripten_trace_record_frame_end(void)

   :rtype: void

   This should be called at the end of the frame / event loop.

   The current timestamp is associated with this data.

   The server uses this to stop accruing memory operations and
   elapsed time to the frame.

.. c:function:: void emscripten_trace_log_message(const char *channel, const char *message)

   :param channel: The category of the timeline event being emitted.
   :type channel: const char*
   :param message: The description for the timeline event being emitted.
   :type message: const char*
   :rtype: void

   Record a log message. This is useful for noting events or actions
   which have occurred which might be advantageous to have correlated
   against memory usage or changes in frame rate.

   The current timestamp is associated with this data.

   *The server doesn't yet do enough with this data. This will improve
   in the future.*

.. c:function:: void emscripten_trace_mark(const char *message)

   :param message: The name of the mark begin emitted.
   :type message: const char *
   :rtype: void

   Record a mark in the timeline. This is primary for use with the
   `Google Web Tracing Framework`_.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_report_error(const char *error)

   :param error: The error message being reported.
   :type error: const char*
   :rtype: void

   The API will obtain the current callstack and include that in the report
   to the server.

   The current timestamp is associated with this data.

   This could be used for various things including capturing JavaScript and
   web-worker errors, as well as failed assertions or other run-time errors
   from within the C/C++ code.

.. c:function:: void emscripten_trace_record_allocation(const void *address, int32_t size)

   :param address: Memory address which has been allocated.
   :type address: const void*
   :param size: Size of the memory block allocated.
   :type size: int32_t
   :rtype: void

   This must be called for each and every memory allocation. The best place to
   do this is within the ``dlmalloc`` implementation in Emscripten.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_record_reallocation(const void *old_address, const void *new_address, int32_t size)

   :param old_address: Old address of the memory block which has been reallocated.
   :type old_address: const void*
   :param new_address: New address of the memory block which has been reallocated.
   :type new_address: const void*
   :param size: New size of the memory block reallocated.
   :type size: int32_t
   :rtype: void

   This must be called for each and every memory re-allocation. The best place to
   do this is within the ``dlmalloc`` implementation in Emscripten.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_record_free(const void *address)

   :param address: Memory address which is being freed.
   :type address: const void*
   :rtype: void

   This must be called for each and every ``free`` operation. The best place
   to do this is within the ``dlmalloc`` implementation in Emscripten.

   The current timestamp is associated with this data.

   It is also important that this not be called multiple times for a single
   ``free`` operation.

.. c:function:: void emscripten_trace_annotate_address_type(const void *address, const char *type)

   :param address: Memory address which should be annotated.
   :type address: const void*
   :param type: The name of the data type being allocated.
   :type type: const char*
   :rtype: void

   Annotate an address with the name of the data type that is
   stored there. This is used by the server to help breakdown
   what is in memory.

.. c:function:: void emscripten_trace_associate_storage_size(const void *address, int32_t size)

   :param address: Memory address which should be annotated.
   :type address: const void*
   :param size: Size of the memory associated with this allocation.
   :type size: int32_t
   :rtype: void

   Associate an amount of additional storage with this address. This
   does not represent the size of the allocation itself, but rather
   associated memory that should be taken into account when looking
   at the size of this object.

   This associated storage is application specific in nature.

   An example is when an object contains a vector or string, you may
   want to be aware of that when analyzing memory usage and this
   provides a way to let the server be aware of that additional
   storage.

.. c:function:: void emscripten_trace_report_memory_layout(void)

   :rtype: void

   This should be called periodically to report the usage of the
   normal Emscripten heap. This provides details of both the stack
   and the dynamic memory usage as well as the total memory size.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_report_off_heap_data(void)

   :rtype: void

   This should be called periodically to report memory usage that is
   not part of the normal Emscripten heap. This is currently used
   to report OpenAL memory usage.

   The current timestamp is associated with this data.

   *The server does not yet display this data.*

.. c:function:: void emscripten_trace_enter_context(const char *name)

   :param name: Context name.
   :type name: const char*
   :rtype: void

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_exit_context(void)

   :rtype: void

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_task_start(int task_id, const char *name);

   :param task_id: Task ID
   :type task_id: int
   :param name: Task name
   :type name: const char*
   :rtype: void

   A task is initiated. The task ID should be unique over the lifetime of
   the application. It should be managed / tracked by the application.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_task_associate_data(const char *key, const char *value);

   :param key: Key
   :type key: const char*
   :param value: Value
   :type value: const char*
   :rtype: void

   Associate a key / value pair with the current task.

.. c:function:: void emscripten_trace_task_suspend(const char *explanation);

   :param explanation: Why the task is suspending.
   :type explanation: const char*
   :rtype: void

   The current task is suspended.

   The explanation should indicate why the task is being suspended
   so that this information can be made available when viewing the
   task's history.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_task_resume(int task_id, const char *explanation);

   :param task_id: Task ID
   :type task_id: int
   :param explanation: Why the task is being resumed.
   :type explanation: const char*
   :rtype: void

   The task identified by ``task_id`` is resumed and made the current task.

   The explanation should indicate what the task is being resumed to do
   so that this information can be made available when viewing the task's
   history.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_task_end(void);

   :rtype: void

   The current task is ended.

   The current timestamp is associated with this data.

.. c:function:: void emscripten_trace_close(void)

   :rtype: void

   This should be closed during application termination. It helps ensure
   is flushed to the server and terminates the tracing code.

.. _emscripten-trace-collector: https://github.com/waywardmonkeys/emscripten-trace-collector
.. _README.rst: https://github.com/waywardmonkeys/emscripten-trace-collector/blob/master/README.rst
.. _Google Web Tracing Framework: http://google.github.io/tracing-framework/
