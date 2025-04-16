.. _Optimizing-WebGL:

================
Optimizing WebGL
================

Due to the extra validation that WebGL needs to impose to ensure web security, the CPU side overhead of running WebGL applications is known to be higher in comparison to native OpenGL applications. Because of this, porting graphics heavy applications can become bottlenecked on the CPU side when interfacing with GL functions. Therefore for best performance, special care should be taken to profile and optimize GL API usage in applications that are expected to be WebGL heavy. This optimization guide focuses on different techniques that have been found useful for improving WebGL performance.

There are so many GL hardware and driver vendors out there, as well as operating system combinations, such that generating a specific optimization guide is difficult. Some optimizations that are efficient on certain hardware/driver/OS combinations have been seen to not make much difference on drivers from another vendor. Fortunately it has been somewhat rare to find a conflicting scenario where certain optimization for one driver would have led to falling down a performance cliff on hardware from another GPU vendor. Most often when this happens it is due to a specific feature not being supported by a particular hardware, which causes the driver to resort to emulation. For example, in one case it was found that a native GL driver advertises having support for the ETC2 compressed texture format, even though the graphics hardware did not implement this, and in another case, it was found that using vertex shader primitive restart index would cause the GL driver to fall back to running vertex shaders in software. Unfortunately OpenGL specifications do not provide means for a driver to report these types of performance caveats, which is why being able to benchmark across a large variety of target hardware is almost necessary when optimizing GL. It is also useful to pay close attention to the web page console of the browser when running, since browsers can report extra performance warnings in the console logs.

It should also be acknowledged that some detected performance issues have been due to inefficiencies or outright performance bugs in browsers and their utilized software libraries, and have nothing to do with the underlying GL drivers or web security in general. When initially working on optimizing Emscripten ported GL codebases, most browsers were found to be inefficient with their WebGL stacks, but this was to be expected, since before Emscripten and asm.js, it was not even possible to perform such precise GL performance comparisons between native versus the web, such that a large number of performance critical issues fell through the gaps. This aspect has steadily been improving as more people stress WebGL with large Emscripten codebases, so some of the items in this guide might not be relevant in the future. If reading this guide in the future you do find something that seems like a net loss in all cases, please do submit a doc PR to discuss. Likewise, if certain GL access pattern is orders of magnitude slower on the web compared to native, it is likely to be a performance bug.

The following optimization tips list different situations that have been known to make an impact in practice, although it is advised to never perform an optimization blindly, but keep the profiler close at hand when experimenting.

Which GL Mode To Target?
========================

Emscripten allows targeting various different OpenGL and OpenGL ES API flavors with different linker flags.

By default, if no special GL related linker flags are chosen, Emscripten targets the WebGL 1 API, which user code accesses by including OpenGL ES 2.0 headers in C/C++ code (``#include <GLES2/gl2.h>`` and ``#include <GLES2/gl2ext.h>``). This mode works like GLES 2, with the exception that a number of WebGL specific changes and restrictions are applied. For a close to complete reference of differences between WebGL 1 and OpenGL ES 2, refer to `WebGL 1 Specification: Differences Between WebGL and OpenGL ES 2.0 <https://www.khronos.org/registry/webgl/specs/latest/1.0/#6>`_.

- If your application renders geometry from client side memory, it will need to build with the linker flag ``-sFULL_ES2``. This mode is convenient to ease porting of new codebases, however WebGL itself does not support rendering from client side memory, so this feature is emulated. For best performance, use VBOs instead and build without the ``-sFULL_ES2`` linker flag.

- If your application targets old desktop OpenGL APIs, it might work when built with the ``-sLEGACY_GL_EMULATION`` flag. However, when building in this mode, even if it works, do not expect good performance. If the application is slow in this mode and it only uses fixed pipeline and no shaders at all, it is also possible to pair ``-sLEGACY_GL_EMULATION`` with ``-sGL_FFP_ONLY`` linker flag to attempt to recover some performance. Although in general it is recommended to spend the effort to port the application to use WebGL 1/OpenGL ES 2 instead.

- When targeting OpenGL ES 3, if one needs to render from client side memory, or the use of ``glMapBuffer*()`` API is needed, pass the linker flag ``-sFULL_ES3`` to emulate these features, which core WebGL 2 does not have. This emulation is expected to hurt performance, so using VBOs is recommended instead.

- Even if your application does not need any WebGL 2/OpenGL ES 3 features, consider porting the application to run on WebGL 2, because JavaScript side performance in WebGL 2 has been optimized to generate no temporary garbage, which has been observed to give a solid 3-7% speed improvement, as well as reducing potential stuttering at render time. To enable these optimizations, build with the linker flag ``-sMAX_WEBGL_VERSION=2`` and make sure to create a WebGL 2 context at GL startup time (OpenGL ES 3 context if using EGL).

How To Profile WebGL
====================

A mix of tools are available for measuring GL performance. In general, here it is recommended that developers do not restrict the focus to only searching for web browser specific profiling tools, but in practice native profilers have been found to work equally well, if not even better. The only drawback when using a native profiler is that some intimate knowledge of how WebGL is implemented in the browser can be critical, or it might be difficult to understand the call streams going to the GPU otherwise.

- To get an overview of how much time is spent in different WebGL entry points, use Firefox with its `Gecko Profiler add-on <https://developer.mozilla.org/en-US/docs/Mozilla/Performance/Profiling_with_the_Built-in_Profiler>`_. This profiler tool is able to show timing data across the whole stack of executed code: handwritten JavaScript, asm.js/WebAssembly and native Firefox C/C++ browser code, which makes it very valuable compared to other profiling tools.

- Useful native tools that are able to profile CPU overhead are `AMD CodeXL <http://gpuopen.com/compute-product/codexl/>`_, `Intel VTune Amplifier <https://software.intel.com/en-us/intel-vtune-amplifier-xe>`_ and `macOS Instruments <https://developer.apple.com/library/content/documentation/DeveloperTools/Conceptual/InstrumentsUserGuide/>`_. If the browser performance profiling tool suggests that a large portion of time is spent inside the browser entry points itself, these tools can be useful in locating where the hotspots are. However when using native CPU profiling tools, it is necessary to build the browser code manually from source in order to obtain symbol information data (e.g. .pdb files on Windows), which these tools look up locally. When debugging Firefox in this way, disabling the multiprocess architecture in Firefox is useful to obtain traces that run the content process in the same thread as the browser itself. Navigate Firefox browser to page ``about:config`` and set the pref ``browser.tabs.remote.autostart.2`` to ``false``, and restart the browser.

- For debugging GPU side API call traces, the `NVidia Nsight <https://developer.nvidia.com/nvidia-nsight-visual-studio-edition>`_, `Intel Graphics Performance Analyzers <https://software.intel.com/en-us/gpa>`_, `Visual Studio Graphics Debugger <https://msdn.microsoft.com/en-us/library/hh315751.aspx>`_ and `AMD CodeXL <http://gpuopen.com/compute-product/codexl/>`_ can be useful tools. On Windows, Firefox is able to use either OpenGL or Direct3D to render WebGL content. Direct3D is the default, but e.g. AMD CodeXL only traces OpenGL call streams. In order to use AMD CodeXL to trace WebGL API calls in Firefox, navigate the browser to ``about:config`` and set the pref ``webgl.disable-angle`` to ``true`` and reload the page.

Avoid Redundant Calls
=====================

In WebGL, every single GL function call has some amount of overhead, even those that are seemingly simple and do nearly nothing. This is because WebGL implementations need to validate each call since the underlying native OpenGL specifications provide no guarantees about security that could be relied upon on the web. Additionally in asm.js/WebAssembly side, each WebGL call generates an FFI transition (a jump between executing code in asm.js context and executing code in browser's native C++ context), which has a slightly higher overhead than a regular function call inside asm.js/WebAssembly. Therefore on the web, it is generally best for CPU side performance to attempt to minimize the number of calls that is made to WebGL. The following tips can be applied here.

- Optimize the renderer and input assets at high level to avoid redundant calls. Refactor the design if needed so that the renderer is able to better reason what kind of state changes are relevant and which ones are not needed. The best kind of cache is one that is unnecessary, so if the high level renderer is able to keep the GL call stream lean, that will produce the fastest results. However, in cases when that is difficult to achieve, some types of lower level caching can be effective, discussed below.

- Cache GL state inside the renderer code, and avoid doing redundant calls to set the same state multiple times if it has not changed. For example, some engines might blindly reconfigure depth testing or alpha blending modes before each draw call, or reset the shader program for each call.

- Avoid all types of renderer patterns which reset the GL to some specific "ground state" after certain operations. Commonly seen occurrences are to ``glBindBuffer(GL_ARRAY_BUFFER, 0)``, ``glUseProgram(0)``, or ``for(i in 0 -> max_attributes) glDisableVertexAttribArray(i);`` after issuing each draw call to revert to a known fixed configuration. Instead, lazily change only the GL state that is needed when transitioning from one draw call to another.

- Consider lazily setting GL state only when it needs to take effect. For example, in the following call stream

   .. code:: cpp

    // First draw
    glBindBuffer(...);
    glVertexAttribPointer(...);
    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glDrawArrays(...);

    // Second draw (back-to-back)
    glBindBuffer(...);
    glVertexAttribPointer(...);
    glActiveTexture(0); // (*)
    glBindTexture(GL_TEXTURE_2D, texture1); // (*)
    glActiveTexture(1); // (*)
    glBindTexture(GL_TEXTURE_2D, texture2); // (*)
    glDrawArrays(...);

all the four API calls marked with a star are redundant, but simple state caching is not quite enough to detect this. A lazier state cache mechanism will be able to detect these types of changes. However, when implementing deeply lazy state caches, it is recommended to do so only after having profiling data to motivate the optimization, because applying lazy caching techniques to all GL state prior to render can become costly as well for other reasons, and performance may be wasted if the renderer already is good at avoiding resubmitting redundant calls. Just right amount of caching can require a bit of tuning to find the balance.

A good rule of thumb is that a renderer that inherently avoids redundant state calls in the first place by high level design is generally more efficient than one that relies heavily on state caching at the low level.

Techniques To Minimize API Calls
================================

In addition to removing API calls that are outright redundant, it is good to also pay attention to how to minimize state changes using other techniques. The following checklist offers some possibilities.

- When rendering to offscreen render targets, use multiple FBOs so that switching render targets only requires a single glBindFramebuffer() call. This avoids having to perform multiple calls each frame to set FBO state.

- Avoid mutating FBO state, but prefer to set up multiple immutable/static FBOs, which do not change state. Changing FBO state causes revalidation of that FBO combination in the browser, but immutable FBOs only need to be validated once at creation.

- Use VAOs whenever possible to avoid having to call to several GL functions to set up vertex attributes for rendering.

- Batch up glUniform* calls to arrays of uniforms and update them in one ``glUniform4fv()`` array call, instead of calling ``glUniform4f()`` multiple times to update each one individually. Or better yet, use Uniform Buffer Objects in WebGL 2.

- Do not call ``glGetUniformLocation()`` at render time, but query the locations once per shader program at startup and cache them.

- Use instanced rendering whenever applicable.

- Consider atlasing several textures into one to enable better geometry batching and instancing opportunities.

- Consider culling renderables more aggressively compared to native GL platforms, if not already being as tight as possible.

Avoid GPU-CPU Sync Points
=========================

The most important aspect of efficient GPU usage is to make sure that the CPU will never need to block on the GPU during render time, and vice versa. These types of stalls create extremely costly CPU-GPU sync points, which lead to poor utilization of both resources. Generally a hint of this type of a scenario happening can be detected by observing overall GPU and CPU utilization rates. If a GPU profiler is claiming that the GPU is idle for large portions of the time, but a CPU profiler is claiming that the CPU in turn is idle, or that certain GL functions take a very long time to complete, it suggests that frames are not being efficiently submitted to the GPU, but GPU-CPU sync(s) occur somewhere during draw call submission. Unfortunately OpenGL specifications do not provide any performance guarantees of which GL calls may cause a stall, so look out for the following behavior and experiment by changing these and reprofiling the effects.

 - Avoid creating new GL resources at render time. This means optimizing out calls to ``glGen*()`` and ``glCreate*()`` functions (``glGenTextures()``, ``glGenBuffers()``, ``glCreateShader()`` and so on) at render time. If new resources are needed, try to create and upload them a couple of frames before attempting to render using them.

 - Likewise, do not delete any GL resources that have just been rendered with. The functions ``glDelete*()`` can introduce a full pipeline flush if the driver detects that any of the resources are in use. It is better to delete resources at loading time only.

 - Never call ``glGetError()`` or ``glCheckFramebufferStatus()`` at render time. These functions should be restricted to be checked at loading time only, since both of these can do a full pipeline sync.

 - Similarly, do not call any of the ``glGet*()`` API functions at render time, but query them at startup and loading time, and refer to cached results at render time.

 - Try to avoid compiling shaders at render time, both ``glCompileShader()`` and ``glLinkProgram()`` can be extremely slow.

 - Do not call ``glReadPixels()`` to copy texture contents back to main memory at render time. If necessary, use the WebGL 2 ``GL_PIXEL_PACK_BUFFER`` binding target instead to copy a GPU surface to an offscreen target first, and only later ``glReadPixels()`` the contents of that surface back to main memory.

GPU Driver Friendly Memory Access Behavior
==========================================

Transferring memory between the CPU and the GPU is a common source of GL performance issues. This is because creating new GL resources can be slow, and uploading or downloading data can block the CPU if the data is not ready, or if an old version of the data is still needed before being able to overwrite it with a new version.

- Prefer interleaved vertex data in a single VBO over multiple VBOs that contain planar attributes. This improves GPU vertex cache behavior, and avoids multiple redundant ``glBindBuffer()`` calls when setting up vertex attribute pointers for rendering.

- Avoid calling ``glBufferData()`` or ``glTexImage2D/3D()`` to resize the contents of a buffer or a texture at runtime. When increasing or decreasing dynamic VBO sizes, use std::vector-style geometric array grow semantics to avoid having to resize every frame.

- Prefer calling ``glBufferSubData()`` and ``glTexSubImage2D/3D()`` when updating buffer texture data, even when the whole contents of the texture or the buffer changes. If the size of a buffer would shrink, do not eagerly re-create the storage, but simply ignore the excess size.

- For dynamic vertex buffer data, consider double- or even triple-buffering VBOs each frame, to avoid uploading a VBO that is still in use. Prefer using ``GL_DYNAMIC`` vertex buffers over ``GL_STREAM``.

When The GPU Is The Bottleneck
==============================

After having verified that CPU-GPU pipeline sync bubbles do not occur, and rendering is still GPU bound, the following optimizations can be useful.

- Multiple additive lighting draw passes of geometry in a forward lighting renderer can be straightforward to implement, but the amount of GL API calls this generates can be too costly. In such cases, consider computing multiple light contributions in one shader pass, even if this would create no-op arithmetic operations in shaders when some objects are not affected by certain lights.

- Use lowest possible fragment shader precision when it is enough (lowp). Optimize shaders aggressively beforehand at offline authoring time, do not expect that the GPU GLSL driver would do any optimizations on the fly. This is particularly important for mobile GPU drivers.

- Sort renderables by target FBO first, then by shader program, and third to minimize any other needed GL state changes or to minimize overdraw, depending on whether the program is CPU or GPU bound. This helps tile based renderers. Call WebGL 2 ``glDiscardFramebuffer()`` when the contents of an FBO are no longer needed.

- Use a GPU profiler, or implement custom fragment shaders that can help profiling how much overdraw the rendered scene has. A large amount of overdraw not only generates extra work, but the sequential dependencies between rendering to the same blocks of display memory slow down parallel rendering. If rendering a 3D scene with depth buffering enabled, consider sorting the scene from front to back to minimize overdraw and redundant per pixel fill bandwidth. If using very complex fragment shaders in a 3D scene, consider doing a depth prepass to reduce the number of actually rasterized color fragments to an absolute minimum.

Optimizing Load Times and Other Best Practices
==============================================

Finally, a number of miscellaneous optimizations have been proven to be effective.

- On the web one cannot generally expect which compressed texture formats will be available. Author textures to multiple compressed texture bundles, e.g. one per format, and download the appropriate one in at runtime to minimize excessive downloads. Store textures and other assets to IndexedDB to avoid having to redownload on subsequent runs. The Emscripten linker flag ``-sGL_PREINITIALIZED_CONTEXT`` can help in authoring a html shell page that performs such texture format checks up front.

- Consider compiling shaders in parallel to when other assets are being downloaded. This can help hide slow shader compilation times.

- Test for WebGL support on the user's browser early in the page load process before downloading a large amount of assets. It can be frustrating for the user to have to wait to download multiple megabytes of assets, only then to get an error message of WebGL not being available after the wait.

- Check the WebGL context error reason if WebGL initialization fails, using the ``"webglcontextcreationerror"`` callback. Browsers can give good diagnostics in the context creation error handler to allow diagnosing what the root cause is.

- Pay close attention to the visible size of the canvas (the CSS pixels size of the DOM element) versus the physical render target size of the initialized WebGL context on the canvas, and make sure these two match in order to render 1:1 pixel perfect content.

- Probe context creation with the ``failIfMajorPerformanceCaveat`` flag to detect when rendering on software, and cut down on graphics fidelity in such cases.

- Make sure to initialize the WebGL context with just the minimum amount of features that are needed. `WebGL context creation parameters <https://www.khronos.org/registry/webgl/specs/1.0/#WEBGLCONTEXTATTRIBUTES>`_ include support for alpha, depth, stencil and MSAA, and most often e.g. support for alpha blending the canvas against the HTML page background is not needed, and should be disabled.

- Avoid using any of the ``*glGetProcAddress()`` API functions. Emscripten provides static linking to all of the GL API functions, even for all WebGL extensions. The ``*glGetProcAddress()`` API is only provided for compatibility to ease porting of existing code, but accessing WebGL via calling dynamically obtained function pointers is noticeably slower than direct function calls, due to extra function pointer security validation that dynamic dispatching has to do in asm.js/WebAssembly. Since Emscripten provides all of the GL entry points statically linked in, it is recommended to take advantage of this for best performance.

- Always use ``requestAnimationFrame()`` loops to render animation instead of the ``setTimeout()`` API. This gives the smoothest scheduling on the animation ticks.

Migrating To WebGL 2
====================

Compared to WebGL 1, the new WebGL 2 API provides essentially free API optimizations that are activated simply by targeting WebGL 2. This speedup comes from the fact that the WebGL 2 API was revised from the perspective of JavaScript bindings, and it is now possible to use WebGL without having to allocate temporary objects that would contribute to JS garbage collector pressure. These new entry points fit nicer together with asm.js and WebAssembly applications, and make the WebGL API a bit leaner to use. As a case study, updating Unreal Engine 4 to target WebGL 2, with no other engine modifications, yielded 7% faster throughput performance.

Because of this source of free performance, it is heavily recommended that all developers migrate to target WebGL 2 even if no other WebGL 2 features are needed, if performance is a concern. WebGL 2 is available starting from Firefox 51 and Chrome 58 (see `#4945 <https://github.com/emscripten-core/emscripten/pull/4945>`_). See also `caniuse: WebGL 2 <http://caniuse.com/#feat=webgl2>`_ table. With a little care, it is possible to simultaneously target both WebGL 1 and WebGL 2 APIs, and leverage the best performance when available, but gracefully fall back on less compatible GPUs.

When working with these two specifications, it is good to remember that `WebGL 1 <https://www.khronos.org/registry/webgl/specs/latest/1.0/>`_ is based on the `OpenGL ES 2.0 specification <https://www.khronos.org/opengles/2_X/>`_, and `WebGL 2 <https://www.khronos.org/registry/webgl/specs/latest/2.0/>`_ is based on the `OpenGL ES 3.0 specification <https://www.khronos.org/registry/OpenGL-Refpages/es3.0/>`_.

Migration to WebGL 2 is slightly complicated by the fact that WebGL, just like OpenGL ES, is not a backwards compatible API. That is, WebGL 1/OpenGL ES 2 applications do not generally work just by initializing a newer version of the GL context to run on WebGL 2/OpenGL ES 3.0. The reason for this is that a number of backwards compatibility breaking changes have been introduced between the two versions. However, these changes are more superficial/cosmetic rather than functional, and feature-wise, WebGL2/OpenGL ES 3.0 encompasses all features that exist in WebGL 1/OpenGL ES 2. Only the way that the different API functions are invoked has changed.

To migrate from WebGL 1 to WebGL 2, pay attention to the following list of known backwards incompatibilities.

- In WebGL 2, a number of WebGL 1.0 extensions have been incorporated to the core WebGL 2 API, and those extensions are no longer advertised to exist when querying for the list of different WebGL extensions. For example, the presence of instanced rendering in WebGL 1 is provided by the `ANGLE_instanced_arrays <https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/>`_ extension, but this is a WebGL 2 core feature, and is therefore no longer reported in the list of GL extensions. If targeting both WebGL 1 and WebGL 2 simultaneously in an application, remember to check both the extension and the core context version number when detecting the presence of a feature.

- A side effect of the above is that when the functionality was merged to core, the specific function names to call for the feature has changed, i.e. on WebGL1/GLES 2 contexts, one would call the function ``glDrawBuffersEXT()``, but with WebGL2/GLES 3.0, one should call the unsuffixed function ``glDrawBuffers()`` instead.

- The full list of WebGL 1 extensions that were adopted to the core WebGL 2 specification is:

   .. code:: cpp

    ANGLE_instanced_arrays
    EXT_blend_minmax
    EXT_color_buffer_half_float
    EXT_frag_depth
    EXT_sRGB
    EXT_shader_texture_lod
    OES_element_index_uint
    OES_standard_derivatives
    OES_texture_float
    OES_texture_half_float
    OES_texture_half_float_linear
    OES_vertex_array_object
    WEBGL_color_buffer_float
    WEBGL_depth_texture
    WEBGL_draw_buffers

These extensions were adopted without any functional changes, so when initializing a WebGL2/GLES 3.0 context, these can be used directly without checking for the presence of an extension.

- A notable addition is that WebGL 2 introduced a new GLSL shader language format. In WebGL 1, one authors shaders in `OpenGL ES Shading Language, Version 1.00 <https://www.khronos.org/registry/webgl/specs/1.0/#refsGLES20GLSL>`_, using ``#version 100`` version pragma in shader code. WebGL 2 introduced new shader language version, `The OpenGL ES Shading Language, Version 3.00 <https://www.khronos.org/registry/webgl/specs/latest/2.0/#4.3>`_, which is identified by the pragma directive ``#version 300 es`` in shader code.

- In WebGL 2/GLES 3.0, one can keep using either WebGL 1/GLES 2 ``#version 100`` shaders, or migrate to using WebGL 2/GLES 3.0 ``#version 300 es`` shaders. Note however that WebGL 2 has a backwards breaking incompatibility that the WebGL extensions ``OES_standard_derivatives`` and ``EXT_shader_texture_lod`` are no longer available in ``#version 100`` shaders, because those features are no longer present as extensions. ``#version 100`` shaders that use those extensions must be rewritten to ``#version 300 es`` format instead. Emscripten provides a linker flag ``-sWEBGL2_BACKWARDS_COMPATIBILITY_EMULATION`` which performs a string search-replace based automatic migration of ``#version 100`` shaders to ``#version 300 es`` format when either of these extensions are detected to attempt to hide this breakage in backwards compatibility.

- In WebGL 2/GLES 3.0, a number of texture format enums changed for texture formats introduced by extensions. It is no longer possible to use so called `unsized` texture formats from WebGL 1/GLES 2 extensions, but instead, the new sized variants of the formats must be used for the ``internalFormat`` field. For example, instead of creating a texture with ``format=GL_DEPTH_COMPONENT, type=GL_UNSIGNED_INT, internalFormat=GL_DEPTH_COMPONENT``, it is required to specify the size in the ``internalFormat`` field, i.e. ``format=GL_DEPTH_COMPONENT, type=GL_UNSIGNED_INT, internalFormat=GL_DEPTH_COMPONENT24``.

- A particular gotcha with WebGL 2/GLES 3.0 texture formats is that the enum value for the half float (float16) texture type changed value when the WebGL 1/GLES 2 extension ``OES_texture_half_float`` was subsumed to the core WebGL 2/GLES 3.0 specification. In WebGL1/GLES 2, half floats were denoted by the value ``GL_HALF_FLOAT_OES=0x8d61``, but in WebGL2/GLES 3.0, the enum value ``GL_HALF_FLOAT=0x140b`` is used, contrast to other texture type extensions where inclusion to the core specification generally preserved the value of the enum that is used.

Overall, to ease simultaneously targeting both WebGL1/GLES 2 and WebGL2/GLES 3.0 contexts, Emscripten provides a linker flag ``-sWEBGL2_BACKWARDS_COMPATIBILITY_EMULATION``, which hides the above differences behind automatically detected migration, to allow existing WebGL 1 content to transparently also target WebGL 2 for the free speed boost it provides.

If you find a missing item in this emulation, or have comments to improve this guide, please submit feedback to the `Emscripten bug tracker <https://github.com/emscripten-core/emscripten/issues>`_.
