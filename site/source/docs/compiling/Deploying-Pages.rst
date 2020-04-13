.. _Deploying-Pages:

===================================
Deploying Emscripten Compiled Pages
===================================

Emscripten compiled output can either be run directly in a JS shell from command line, or hosted on a web page. When hosting asm.js and WebAssembly compiled pages as .html for browsers to execute, Emscripten provides a default `HTML shell file` that serves as a launcher to run the code, simplified to get started with development. However when getting ready to release and host the content on a web site, a number of extra features and customizations are likely needed to polish the visitor experience. This guide highlights things to pay attention to when deploying sites to the public.

Build Files and Custom Shell
============================

Emscripten build output consists of two essential parts: 1) the low level compiled code module and 2) the JavaScript runtime to interact with it. If targeting WebAssembly which is done with linker flags ``-s WASM=1 -o out.html``, the compiled code is stored in a file ``out.wasm`` and the runtime lives in a file ``out.js``. When targeting asm.js there exists an additional binary file ``out.mem`` that contains the static memory section of the compiled code. This part is embedded in the ``out.wasm`` file when targeting WebAssembly.

By default when targeting asm.js, the compiled code and the runtime are fused in the same ``out.js`` file. For moderately large asm.js projects, it is recommended to use the ``--separate-asm`` flag to separate the compiled code to its own ``out.asm.js`` file, which enables browsers to optimize memory usage for the compiled asm.js code.

Additional build output files can also exist, depending on which features are used. If the Emscripten file packager is used, a binary ``out.data`` package is generated, along with an associated ``out.data.js`` loader file. Also Emscripten pthreads and Fetch APIs have their own associated Web Worker related script ``.js`` output files.

Developers can choose to output either to JavaScript or HTML. If outputting JavaScript (``emcc -o out.js``), the developer is expected to manually create the ``out.html`` main page in which the code is run in browsers. When targeting HTML with ``emcc -o out.html`` (the recommended build mode), Emscripten will generate the HTML shell file automatically. This shell file can be customized by using the ``emcc -o out.html --shell-file path/to/custom_shell.html`` linker directive. Copy the `default minimal HTML shell file <https://github.com/emscripten-core/emscripten/blob/master/src/shell_minimal.html>`_ from Emscripten repository to your project tree to get a good starting template for a customized shell file.

The following sections offer tips for improving the site experience.

Optimizing Download Sizes
=========================

The biggest slowdown to speedy page loading is most often the need to download large amounts of asset data for the project, especially if the page is heavy on WebGL textures or geometry. Compiled code generally takes up more space than handwritten JavaScript, but machine code compresses efficiently. Therefore when hosting asm.js and WebAssembly, it is critical to ensure that all content is transferred using gzip compression, which all browsers and CDNs nowadays support built-in. Gzip compressing ``.wasm`` files give on average 60-75% size reductions compared to uncompressed ones, so it practically never makes sense to serve uncompressed files.

- To serve gzip-compressed assets on a CDN, use a gzip compression tool and precompress asset files offline before uploading to the CDN. Some web servers support compressing files on the fly, but for static asset content, that should be avoided since it can be costly for the server CPU to keep recompressing the files. Adjust the configuration of the web server to host the precompressed files with the HTTP response header ``Content-Encoding: gzip``. This instructs web browsers that the downloaded content should be transparently uncompressed before handing the data off to the page itself.

- WebAssembly has now shipped in `Firefox 52 and Chrome 57 <http://caniuse.com/#feat=wasm>`_. Emscripten also supports targeting WebAssembly, by using the ``-s WASM=1`` linker flag. WebAssembly is an evolution of asm.js, and if your project already successfully compiles to asm.js, it is likely to already work with WebAssembly as well. Compressed WebAssembly output files can be around 20% smaller than compressed asm.js files, but for builds with debugging and profiling information, the difference can even be up to 50%, so the benefit is large.

- Make sure that gzip compression does not confuse the MIME types that the assets are served with. All JavaScript files (precompressed or not) should be best served with the HTTP response header ``Content-Type: application/javascript``, and all asset files (``.data``, ``.mem``) should be served with the header ``Content-Type: application/octet-stream``. WebAssembly ``.wasm`` files should be served with ``Content-Type: application/wasm``.

- Try to minimize the amount of preloaded asset data that is downloaded up front with the Emscripten ``--preload-file`` linker flag. This data file is loaded up before the Emscripten compiled application starts to execute ``main()`` function at all, so all files that are stored in this package can greatly slow down the time to start up. Prefer to break up downloaded asset files to multiple separate packages and using asynchronous asset download APIs in Emscripten which can operate while the application is running.

- Asset sizes for WebGL applications are often dominated by the amount of textures, such that using compressed texture formats helps to shrink down asset sizes. The web can be quite a different platform to target compared to native platforms, since on the web one cannot necessarily assume that a specific compressed texture format would be supported on the visitor's hardware, especially if developing a site that should work both on mobile and desktop browsers. The best practice to support a wide range of hardware is to generate multiple sets of compressed textures, one for each supported platform, and download the appropriate one based on the formats that the WebGL context supports.

- If multiple screen sizes are being targeted, e.g. in desktop and mobile form factors, consider separating textures out to SD and HD variants to make the page load up quicker for mobile devices that have a smaller display resolution.

Optimizing Page Startup Times
=============================

In addition to downloading the page, other parts of the startup sequence can sometimes also be slow. Things to consider here are:

- If targeting asm.js and running on Firefox or Edge, the web page console displays a log message after the asm.js module has been compiled. This log message includes timing information about how long the compilation took. Asm.js compilation begins the moment the asm.js script source file is added to the DOM, and once it finishes, the ``onload`` event of the script tag is called. This can be used to time how long asm.js compilation takes on Safari, Opera and Chrome.

- It is recommended to migrate to WebAssembly to speed up compiled code startup times in browsers. WebAssembly modules are much faster to parse and compile compared to asm.js. Additionally, compiled ``WebAssembly.Module`` objects can be manually persisted to IndexedDB, which avoids the compilation step altogether on the second run. (see next section)

- Occassionally it can be easy to misattribute slow startup to asm.js/WebAssembly compilation, when the actual cause of slowness can in fact be in executing the ``main()`` function entry point of the application itself. This is because these two actions are run closely back to back to each other. It is worthwhile to be precise to profile these two actions separately, check out the ``function callMain()`` in ``src/preamble.js`` which kicks off the execution of application ``main()`` code. If executing ``main()`` takes too long time, consider splitting it out to separate operations that are driven by multiple ``setTimeout()`` calls or by the ``emscripten_set_main_loop()`` event loop.

- To speed up network transfers, experience shows that the under regular network conditions, the fastest approach is to aggressively start all network downloads simultaneously in parallel (assuming there are only a handful of them), as opposed to e.g. downloading a single input file at a time before starting the next one. Therefore to maximize network transfer speed, try to write the main HTML page of the application to start all needed network downloads in parallel instead of queueing them up for sequential transfer.

- In the case when the first time load of the page is dominated by network transfers, it is useful to take advantage of the fact that the CPU is otherwise mostly idle while waiting for downloads to finish. This CPU time can be utilized to perform other heavy tasks. An ideal candidate for this is to download and compile the asm.js/WebAssembly module already while downloading other page assets.

- A currently known issue on Windows based systems is that compiling WebGL shaders can be slow. This is also a primary candidate for an action to be performed in parallel while downloading other assets for the page.

Providing a Quick Second Time Load
==================================

While the first run experience of visiting a page can take some time to finish all downloads, the second run experience of the page can be made much faster by making sure that the results of the first visit are cached by the browser.

- All browsers have an implementation defined limit (20MB or 50MB) for assets, and files larger than that will bypass the browser's built-in web caches altogether. Therefore it is recommended that large ``.data`` files are manually cached to IndexedDB by the main page. The Emscripten linker option ``--use-preload-cache`` can be used to have Emscripten implement this, although it can be desirable to manually manage this on the html page in a custom manner, since that allows taking control of which database the assets are cached to, and what kind of scheme will be used to evict data from it.

- Compilation results of asm.js modules are cached automatically by the browser, and there is little control over this. WebAssembly on the other hand supports explicit caching of compiled ``WebAssembly.Module`` objects to IndexedDB. This feature should be always leveraged, since it allows skipping the whole compilation process on the second page visit.

- If the compiled C/C++ code itself performs any computation e.g. in ``main()`` that could be skipped on the second load, use either IndexedDB or the localStorage APIs to cache the results of this computation across page runs. IndexedDB is suitable for storing large files, but it works asynchronously. The localStorage API on the other hand is fully synchronous, but its usage is restricted to storing small cookie style data fields only.

- When implementing IndexedDB based caching, it is good to note that as an asynchronous API that performs disk accesses, IndexedDB operations have some latency as well. Therefore if performing multiple read operations at startup, it is good to fire all of them up in parallel where possible, to reduce latency.

- Another important point with persisting data is that for best practices towards the user, it is good to provide an explicit visual identification when using IndexedDB or localStorage to persist large amounts of data, and offer an easy mechanism to clear or uninstall that data. This is because currently browsers do not implement convenient UIs for fine grained deletion of data from these storages, but clearing data often is presented as a "clear cache from all pages" type of option.

Reserving Memory for Compiled Code
==================================

An inherent property of asm.js and WebAssembly applications is that they need a linear block of memory to represent the application `heap`. This is often the single largest memory allocation that an Emscripten compiled page does, and therefore is the one that is at the biggest risk of failing if the user's system is low on memory.

Because this memory allocation needs to be contiguous, it can happen that the user's browser process does have enough memory, but only the address space of the process is too fragmented, and there is not enough linear address space available to satisfy the allocation. To avoid this issue, the best practice is to allocate the ``WebAssembly.Memory`` object (``ArrayBuffer`` for asm.js) up front at the top of the main page, before any other allocations or page script load actions are done. This ensures that the allocation has best chances to succeed. See the fields ``Module['buffer']`` and ``Module['wasmMemory']`` for more information.

Additionally, it is possible to opt in to content process isolation specifically for a web page that needs this kind of a large allocation. To utilize this machinery, specify the HTTP response header ``Large-Allocation: <MBytes>`` when serving the main html page. This support is currently implemented in Firefox 53.

Last, it is easy to accidentally cling to large unneeded blocks of memory after the page has loaded. For example, in WebAssembly, once the WebAssembly Module has been instantiated to a ``WebAssembly.Instance`` object, the original ``WebAssembly.Module`` object is no longer needed in memory, and it is best to clear all references to it so that the garbage collector can reclaim it, because the Module object can be dozens of megabytes in size. Similar, make sure that all XHRed files, asset data and large scripts are not referenced anymore when not used. Check out the browser's memory profiling tool, and the ``about:memory`` page in Firefox to perform memory profiling to ensure that memory is not being wasted.

Robust Error Handling
=====================

To provide the best possible user experience, make sure that the different ways that the page can fail are taken into account, and good error reporting is provided to the user. In particular, proceed through the following checklist for best practices.

- Aim to fail as early as possible. A large source of frustration for users comes from scenarios where user's system is not ready to run the given page, but the error only becomes apparent after having waited for a minute to download 100MB worth of assets. For example, try to allocate the needed heap memory up front before actually loading up the page. This way if the memory allocation fails, the failure is immediate and no asset downloads need to be attempted at all.

- If a particular browser is known to not be supported, resist the temptation to read ``navigator.userAgent`` field to gate users with that browser, if at all possible. For example, if your page needs WebGL 2 but Safari is known not to support it, do not exclude out Safari users with a following type of check:

   .. code:: js

    if (navigator.userAgent.indexOf('Safari') != -1) alert('Your browser does not support WebGL 2!');

but instead, detect the actual errors:

   .. code:: js

    if (!canvas.getContext('webgl2')) alert('Your browser does not support WebGL 2!'); // And look for webglcontextcreationerror here for an error reason.

This way the page will be future compatible once support for the particular feature later becomes available.

- Test various failure cases up front by simulating different issues and browser limitations. For example, on Firefox, it is possible to manually disable WebGL 2 by navigating to ``about:config`` and setting the preference ``webgl.enable-webgl2`` to ``false``. This allows you to debug what kind of error reporting your page will present to the user in such a scenario. To disable WebGL support altogether for testing purposes, set the preference ``webgl.disabled`` to ``true``.

- When working with IndexedDB, prepare to handle out of quota errors when user is about to run out of free disk space or allowed quota for the domain.

- Simulate out of memory errors by allocating unrealistically much memory for ``WebAssembly.Memory`` object and for the preloaded file packages, if using any. Make sure that out of memory errors are flagged correctly as such (and reported to the user or to an error database).

- Simulate download timeouts either intrusively by programmatically aborting XHR downloads, physically disconnecting network access, or by using external tools such as Fiddler. These types of tools can show up a lot of unexpected failure cases and help diagnose that the error handling path for such scenarios is as desired.

- Use a network limiter tool to constrain download or upload bandwidth speeds to simulate slow network connections. This can uncover bugs related to timing dependencies for network transfers. For example, a small network transfer may be implicitly assumed to finish before a large one, but that might not always be the case.

- When developing the page locally, perform testing by using a local web server and not just via ``file://`` URLs. The script ``emrun.py`` in Emscripten source tree is designed to serve as an ad hoc web server for this purpose. Emrun is preconfigured to handle serving gzip compressed files (with suffix ``.gz``), and enables support for the ``Large-Allocation`` header, and allows command line automation runs of compiled pages.

- Catch all exceptions that come from within entry points that call to compiled asm.js and WebAssembly code. There are three distinct exception classes that compiled code can throw:

    1. C++ exceptions that are represented by a thrown integer and not caught by the C++ program. This integer points to a memory location in the application heap that contains pointer to the thrown object.

    2. Exceptions caused by Emscripten runtime calling the ``abort()`` function. These correspond to a fatal error that execution of the compiled code cannot recover from. For example, this can occur when calling an invalid function pointer.

    3. Traps caused by compiled WebAssembly code. These correspond to fatal errors coming from the WebAssembly VM. This can occur for example when performing an integer division by zero, or when converting a large floating point number to an integer when the float is out of range of the numbers representable by that integer type. See the linker flag ``-s BINARYEN_TRAP_MODE`` for more details.

- Implement a final "catch all" error handler on the page by implementing a ``window.onerror`` script. This will be called as a last resort if no other source handled an exception that was raised on the page. See `window.onerror <https://developer.mozilla.org/en-US/docs/Web/API/GlobalEventHandlers/onerror#window.onerror>`_ documentaton on MDN.

- Do not let the html page "freeze" and bury the error message in the web page console, because most users will not know how to find it there. Strive to provide meaningful error reports to the user on the main html page, preferably with hints on how to act. If updating a browser version or GPU drivers, or freeing up some space on disk might have a chance to help the page to run, let the user know what they could try out. If the error in question is completely unexpected, consider providing a link or an email address where to report the issue to.

- Provide meaningful and interactive loading progress indicators to show the user whether the loading progress is still proceeding and what is going to happen next. Try to prevent leading the user to a `"I wonder if it is still loading or if it hung?"` state of mind.

Prepare for The Web Environment
===============================

When planning a testing matrix before pushing a site live, the following items can be a good idea to review.

- Web page behavior can be subtly different when run as a top level window vs when run in an iframe. Make sure to test both scenarios if these are applicable.

- Test both 32 bit and 64 bit browsers, especially simulate out of memory scenarios on 32 bit browsers.

- Be aware of the `HTTP Cross-Origin Access Control <https://developer.mozilla.org/en-US/docs/Web/HTTP/Access_control_CORS>`_ rules and how they pertain to the site architecture you are hosting.

- Be aware of the `Content Security Policy <https://developer.mozilla.org/en-US/docs/Web/HTTP/CSP>`_ rules and make note of what kind of CSP policy the site is planned to run with.

- Be mindful of the `Mixed Content Security <https://developer.mozilla.org/en-US/docs/Web/Security/Mixed_content>`_ restrictions that browsers impose.

- Make sure that the site runs well in private browsing (incognito) mode. For example, this will prevent the site from persisting data to IndexedDB.

- Test that the page works well when put into a background tab. Use the ``blur``, ``focus`` and ``visibilitychange`` DOM events to react to page hide and show events. This is relevant in particular for applications that perform audio playback.

- If the page uses WebGL, make sure that it is able to gracefully handle the WebGL context loss event. Use the `WebGL_lose_context <https://www.khronos.org/registry/webgl/extensions/WEBGL_lose_context/>`_ developer extension to programmatically trigger context loss events when testing.

- Verify that the page works as intended on displays with different ``window.devicePixelRatio`` (DPI) settings, in particular when using WebGL. See `Khronos.org: HandlingHighDPI <https://www.khronos.org/webgl/wiki/HandlingHighDPI>`_. On Windows and macOS, try changing the desktop display scaling setting to test different values of ``window.devicePixelRatio`` that the browser reports.

- Test out that different page zoom levels do not break the site layout, especially when navigating to the page with the browser window already pre-zoomed.

- Likewise, verify that the page layout does not break when resizing the browser window, or when visiting the site having already initially sized the browser window to very small or large size, or to a disproportionate aspect ratio.

- Especially if targeting mobile, be aware of the `<meta viewport> tag <https://developer.mozilla.org/en-US/docs/Mozilla/Mobile/Viewport_meta_tag>`_ for how to develop a site layout that works well on mobile.

- If the page uses WebGL, test out different GPUs on target platforms. In particular, verify the site behavior when simulating the lack of any needed WebGL extensions, and compressed texture format support.

- If using the ``requestAnimationFrame()`` API (i.e. ``emscripten_set_main_loop()`` function) to drive rendering, be aware that the rate at which the function is called is not always 60 Hz, but can vary at runtime e.g. when moving the browser window from one display to another in a multimonitor setup, if the displays have different refresh rates. Update intervals such as 75Hz, 90Hz, 100Hz, 120Hz, 144Hz and 200Hz are becoming more common.

- Simulate lack of any special APIs that the page might need, e.g. Gamepad, Acceleration or Touch Events, and make sure that appropriate error flow is handled in those cases as well.

If you have good tips or suggestsions to share, please help improve this guide by posting feedback to the `Emscripten bug tracker <https://github.com/emscripten-core/emscripten/issues>`_ or the `emscripten-discuss <https://groups.google.com/forum/#!forum/emscripten-discuss>`_ mailing list.
