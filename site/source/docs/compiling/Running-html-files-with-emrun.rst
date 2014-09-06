.. _Running-html-files-with-emrun:

===========================================
Running html files with emrun (wiki-import)
===========================================

.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

When you target the web with emscripten via the ``-o out.html`` directive to emcc, emscripten will generate one or more outfiles with suffixes ``.html, .js, and .data``. Not all web browsers can run the .html file via a ``file://`` URL by double-clicking the html file because of default browser CORS rules (in Chrome, Opera and IE), but you need to deploy the files to a web server first. To solve this, you can use the ``emrun`` command line tool to run your page in a locally launched web server.

Features
--------

``emrun`` enables the following uses:

-  Launch your html page in a web browser from command line, behaving as if it was a native executable.
-  Capture application ``stdout`` and ``stderr`` streams during the run, and print them to terminal or log to file.
-  Pass command-line arguments to the application and read them from ``GET`` parameters in the launched URL, or ``argc`` and ``argv`` in main.
-  Detect when application quits via a call to C function ``exit(returncode);`` and pass that return code to terminal.
-  Use a custom browser to run, or even run a browser on an Android device connected to the local computer via adb.

To summarize, ``emrun`` is the tool to use for all kinds of command-line automation needs, for example when integrating your unit tests to run in an Emscripten environment in your project build farm.

Quick how-to
------------

Using ``emrun`` is simple. Here is how:

1. Rebuild your emscripten application and add the ``--emrun`` linker flag. This flag injects a piece of code in the generated ``Module`` that is needed to enable the ``stdout``, ``stderr`` and ``exit()`` capture to work. If you skip this step, you can still run any .html file with ``emrun``, but then the capture will just not work.

2. Open a terminal, navigate to the build output directory, and run ``emrun page.html``. This will spawn a new web server to host the page and launch your default system browser to visit that page. ``emrun`` will block until the page calls ``exit(returncode);``, after which it will quit back to shell with the provided process exit code.

That's all!

Choosing the browser to run
---------------------------

``emrun`` provides the ``--browser <filename-or-browser-alias>`` command line option that allows specifying which browser to run. If that flag is not specified, the default system browser is launched. If the parameter to ``--browser`` points to an existing file, then that file is spawned
as the browser process to run. Additionally, to simplify setup, one can pass predefined alias names of known browsers to ``--browser`` to launch a specific browser. To enumerate the list of detected browser aliases on your system, run the ``--list_browsers`` command, like follows:

::

    > emrun --list_browsers

    emrun has automatically found the following browsers in the default install locations on the system:

    - firefox: Mozilla Firefox 26.0.0.5087
    - firefox_beta: Mozilla Firefox 26.0.0.5077
    - firefox_aurora: Mozilla Firefox Aurora 28.0.0.5098
    - firefox_nightly: Mozilla Firefox Nightly 29.0.0.5098
    - chrome: Google Chrome 31.0.1650.63
    - chrome_canary: Google Chrome 34.0.1752.0
    - iexplore: Microsoft Internet Explorer 11.0.9600.16384
    - opera: Opera 18.0.1284.63

    You can pass the --browser <id> option to launch with the given browser above. Even if your browser was not detected, you can use --browser /path/to/browser/executable to launch with that browser.

For your browser to be detected, it should be installed in a default install location on the system (``%ProgramFiles%`` on Windows and ``/Applications/`` on OSX) or by adding it to the ``PATH`` for current user.

Then for example to run Firefox Nightly, you would launch ``emrun --browser firefox_nightly page.html``.

If you just want to launch a web server, similar to how ``python -m SimpleHTTPServer`` operates, you can pass the ``--no_browser`` command line flag, in which case ``emrun`` will not spawn a browser at all, but simply runs a web server.

Controlling web server operation
--------------------------------

``emrun`` spawns its own web server to host the target html file. You should note the following security implications:

-  The web server is a generic file server that by default serves all files in the directory where the .html file resides, and all directories under that directory tree.
-  The web server will be visible to other computers on the same network.

The following command line flags control how ``emrun`` spawns the web server: 

- ``--no_server``: Does not launch a web server at all. The target file is run via ``file://`` protocol, if possible. 
- ``--serve_after_close``: Does not quit ``emrun`` after the user was detected to close the web browser, but keeps serving. Use this flag for example when you want to visit the page multiple times or with different browsers during the same run. 
- ``--serve_after_exit``: Does not quit ``emrun`` after the page finishes with a call to ``exit(returncode);``, but keeps serving. 
- ``--serve_root <path>``: Chooses a custom directory to use as the root directory for the spawned web server. By default, the directory where the .html file resides in is used. 
- ``--port <number>``: Chooses the TCP port where the web server listens to. The default port is ``6931``. 
- ``--silence_timeout <seconds>``: If the application does not print anything to ``stdout`` or ``stderr`` in this many seconds, the page/browser is assumed to be hung, and ``emrun`` will quit. Disabled by default. 
- ``--timeout <seconds>``: If the whole page run lasts longer than this many seconds, the page/browser is assumed to be hung, and ``emrun`` will quit. Disabled by default. 
- ``--timeout_returncode <code>``: Specifies the process return code that ``emrun`` quits with if a page run timeout occurs. By default ``99999``.

Controlling log output
----------------------

The following command line flags affect messaging to logs:

-  ``--verbose``: Prints detailed information about ``emrun`` internal steps, for debugging purposes.
-  ``--log_stdout <filename>``: Instead of printing to terminal, writes all ``stdout`` messages from the application to the given file.
-  ``--lot_stderr <filename>``: Same as above, but for the ``stderr`` stream.
-  ``--system_info``: Before launching, prints detailed information about the current system. Useful for automated runs to capture hardware information to logs.
-  ``--browser_info``: Before launching, prints information about which browser is about to be launched.
-  ``--log_html``: If specified, application output is reformatted as html markup friendly.
-  ``--no_emrun_detect``: Hides the friendly reminder message that warns if target .html file is detected to not have been built with ``--emrun``.

Cleaning up after the run
-------------------------

Especially when automating operation for build servers, it is important to be able to clean up properly after each run. The following command line flags enable this:

-  ``--kill_start``: If specified, all instances of the target browser process are terminated before starting the run. Pass this flag if you want to force that no old (hung) instances of the target browser processes could exist at the time of the launch that could interfere with the run. By default disabled.
-  ``--kill_exit``: If specified, all instances of the target browser process are terminated when ``emrun`` quits. Pass this flag to force that browser pages are not left around running when the run is over. By default disabled.

It is important to understand that these operations can cause data loss, since these actions cause processes to be forcibly terminated. Do not pass these flags when running a browser that could have multiple tabs or windows open that you do other work in, since they will all be wiped out. You were warned.

Running web pages on an Android device
--------------------------------------

It is also possible to use ``emrun`` to automate web browser runs on Android. For that to work, you need 

- An Android phone connected to the local system via USB and its developer mode enabled. No need to root the phone. 
- The ``adb`` tool installed to the system and present in ``PATH`` environment variable. 
- Check that adb is working by calling ``adb devices`` to see that your device is listed. 
- Install any browser apk to the device that you want to be able to run.

To run on Android, add the ``--android`` command line flag, and use the ``--browser <id>`` command line flag to explicitly choose the browser to run. Omitting ``--browser`` (to launch a default Android browser) is not supported. The following browser ids have been tested to work: ``firefox, firefox_beta, firefox_aurora, firefox_nightly, chrome, chrome_beta, opera``.

The following browser ids are also supported, but have known issues: 
- ``opera_mini``: The browser launches, but for some reason it times out when trying to load any page. 
- ``dolphin``: Works, but does not support WebGL.

Otherwise, ``emrun`` works as usual, and the other command line flags can be combined.
