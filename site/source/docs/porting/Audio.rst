.. _Audio:

=====
Audio
=====

Emscripten supports OpenAL 1.1 "out of the box", with no additional compilation flag required. The implementation uses the Web Audio API.

You can reasonably expect ported OpenAL applications to "just work" with no additional effort.  
However, there are some implementation-specific aspects which are worth considering - these are documented here.


.. _Audio-openal-extensions-g:

Supported OpenAL extensions
===========================

The following extensions are supported by Emscripten's OpenAL implementation.

- ``ALC_SOFT_pause_device``;
- ``ALC_SOFT_HRTF``;
- ``AL_EXT_float32``;
- ``AL_SOFT_loop_points``;
- ``AL_SOFT_source_length``;
- ``AL_EXT_source_distance_model``;
- ``AL_SOFT_source_spatialize``;

.. warning:: This doesn't mean you should just assume their presence ! For correctness, you should **always** check if an extension is supported before using it, like a good application would do.


.. _Audio-guidelines-g:

Guidelines for Audio on Emscripten
==================================

First: Do not loop on OpenAL calls.

Put simply, avoid completely this kind of code :

.. code-block:: c

    while(nframes < THE_NUMBER_OF_FRAMES_WE_WANT)
        alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &nframes);


The reason for this is simple: Your application needs to yield to the Javascript main loop for audio processing to take place (See :ref:`Browser main loop <emscripten-runtime-environment-main-loop>`).

The above snippet usually works because most OpenAL implementations own and manage one or more separate threads. This is not the case in Emscripten.

What you must do instead is perform each such query only once per "main loop iteration" (i.e the callback you provide via :c:func:`emscripten_set_main_loop` or :c:func:`emscripten_set_main_loop_arg`).


Second: Avoid creating and destroying resources relentlessly.

On some browsers (except, apparently, Firefox), garbage collection does not occur as much as it should. Knowing this, your app should ideally at most use one ``ALCcontext`` for playback and one ``ALCdevice`` for capture.

Even if you're planning to suspend audio capture or playback for extended periods of time, it is probably better to keep contexts in memory (unlike in native apps where you should rather destroy them) to avoid risking memory leaks caused by poorly garbage-collected Javascript implementations.

Of course, the current OpenAL implementation strives to be memory-efficient, but it has no control over lower-level mechanisms involved.


.. _Audio-openal-capture-behavior-g:

Emscripten-specific capture behavior
====================================

Attempting to open an input stream to the user's audio capture device
results in the asynchronous appearance of a small browser-specific dialog
asking for the user's permission.

.. note:: 
    On Firefox, the user is also given the opportunity to pick from a list of devices at that moment.
    On other browsers, this is normally a simple "Allow/Deny" button pair on a toast.

With this in mind, when ``alcCaptureOpenDevice()`` is called with valid and 
supported parameters, a "proxy" device is returned, which successfully 
captures no sample until the user clicks "Allow" on said dialog.

That means, when calling ``alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &nframes)``, ``nframes`` remains set to zero until the user clicks "Allow". You might want to make your app handle this specific behavior.

If the user clicks "Deny", the device is invalidated (because this is somewhat
similar to unplugging the physical device) and calls to ``alcCapture*`` functions on that
device then consistently fail with ``ALC_INVALID_DEVICE``.
Your application must be prepared to handle this properly.

.. note::
    Some browsers "remember" this choice and apply it automatically every time it would be asked again instead.
    There's no way for the implementation to detect this behavior.


.. _Audio-openal-capture-details-g:

Useful implementation details of OpenAL capture
===============================================

Internally, Web Audio's capture data is always backed by a Javascript ``Float32Array``.  
Thus, ``AL_FORMAT_MONO_FLOAT32`` and ``AL_FORMAT_STEREO_FLOAT32`` are the only formats which do not require running "type conversion passes" on acquired buffers.

Note, however, that :

- It's probably pointless to pick a stereo format for capture;
- float32 samples use more memory than the standard 16-bit or 8-bit integer formats;
- Most microphones aren't very high quality.
  Especially for real-time apps such as games, users will likely tolerate lower quality recordings.

Also, the actual sample rate at which samples are acquired from the device is unpredictable and depends on many factors, including the user's browser and setup. If this sample rate does not match the one your app requests, the implementation is required to perform resampling on your behalf.  

That sample rate is given by ``audioCtx.sampleRate``, where ``audioCtx`` is the ``AudioContext`` object used internally by the relevant capture ``ALCdevice``.
Currently, Emscripten provides no direct way for applications to access this value, but this might be provided through an Emscripten-specific OpenAL extension (which is not here yet because it requires registration).

Right now though there's a quick and *de facto* reliable way to do this (C example):

.. code-block:: c

    #ifdef __EMSCRIPTEN__
    
    #include <emscripten.h>

    // Avoid calling this more than once! Caching the value is up to you.
    unsigned query_sample_rate_of_audiocontexts() {
        return EM_ASM_INT_V({
            var AudioContext = window.AudioContext || window.webkitAudioContext;
            var ctx = new AudioContext();
            var sr = ctx.sampleRate;
            ctx.close();
            return sr;
        });
    }
    #endif

It is reasonable to expect this sample rate to be either 44100Hz or 48000Hz. You can test it right now by copying the javascript part into your browser's console if you're curious.


.. _Audio-improving-g:

Improving and extending the implementation
==========================================

Currently, the OpenAL capture implementation performs naïve linear resampling because it is simple and the small loss of quality was deemed acceptable.

However, you're welcome to improve this by contributing if you so wish!

Likewise, if you'd like to see a particlar extension implemented (whether it is registered or not), then your best bet is to either file an issue (provided no related prior issue existed for this), or contribute! See :ref:`Contributing <contributing>` for guidelines.
