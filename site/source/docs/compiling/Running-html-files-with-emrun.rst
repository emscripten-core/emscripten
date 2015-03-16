.. _Running-html-files-with-emrun:

=============================
Running HTML files with emrun
=============================

*emrun* is a command line tool that can run generated HTML pages via a locally launched web server. This is helpful when using web browsers that cannot run a generated **.html** file via a ``file://`` URL (e.g. by double-clicking the file) because of the default browser CORS rules.

*emrun* also enables all kinds of command-line automation, for example, integrating unit tests into an Emscripten environment in a project build farm.

Features
========

*emrun* supports the following uses:

-  Launch your Emscripten-generated HTML page in a web browser from the command line.
-  Capture application ``stdout`` and ``stderr`` streams during the run, and print them to a terminal or log them to a file.
-  Pass command-line arguments to the application and read them from ``GET`` parameters in the launched URL, or ``argc`` and ``argv`` in ``main()``.
-  Detect when a launched application quits via a call to C's ``exit(returncode)``, then pass the specified return code to the terminal.
-  Choose which installed browser to run, or even run a browser on an Android device connected to the local computer through *adb*.


Quick how-to
============

Using *emrun* is simple:

#. Rebuild your Emscripten application and add the ``--emrun`` :ref:`linker flag <emcc-emrun>`. 

	This flag injects code into the generated :ref:`Module` to enable capture of ``stdout``, ``stderr`` and ``exit()``. 

	.. note:: If you skip this step, you can still run any **.html** file with *emrun*, but the capture will not work.

#. Open a terminal, navigate to the build output directory, and call ``emrun page.html``. 

	This will spawn a new web server to host the page and launch your default system browser to visit that page. *emrun* will block until the page calls ``exit(returncode)``, after which it will quit back to shell with the given process exit code.


Choosing the browser to run
===========================

The ``--browser <filename-or-browser-alias>`` command line option allows you to launch an HTML file using a particular browser, by specifying either its "browser alias" or the full path to its executable (if the flag is not specified, the default system browser is launched). 

To enumerate the list of browser aliases on your system, use the ``--list_browsers`` command:

.. code-block:: bash

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

.. note:: For your browser to be detected, it should be installed in the default install location on the system (``%ProgramFiles%`` on Windows and ``/Applications/`` on OSX) or by adding it to the ``PATH`` for the current user.

You can pass the ``--browser <alias>`` option to launch with a given browser. For example, to run the *Firefox Nightly* browser you would call:

.. code-block:: bash

	emrun --browser firefox_nightly page.html
	
To launch using a browser's filename use:

.. code-block:: bash

	--browser /path/to/browser/executable page.html

If you just want to launch a web server you can pass the ``--no_browser`` command line flag. In this case *emrun* will run the server without spawning the browser (this is similar to using Python's `SimpleHTTPServer <http://www.pythonforbeginners.com/modules-in-python/how-to-use-simplehttpserver/>`_).


Security implications
=====================

*emrun* spawns its own web server to host the target **.html** file. This has the following security implications:

-  The web server is a generic file server that by default serves all files in the directory where the **.html** file resides, and all directories under that directory tree.
-  The web server will be visible to other computers on the same network.


Controlling web server operation
================================

The following command line flags control how *emrun* spawns the web server: 

- ``--no_server``: Do not launch a web server. The target file is run via the ``file://`` protocol, if possible. 
- ``--serve_after_close``: Do not quit *emrun*; continue running the server even after the user closes the web browser. Use this flag when you want to visit the page multiple times or with different browsers during the same run. 
- ``--serve_after_exit``: Do not quit *emrun*; continue running the server after the page finishes with a call to ``exit(returncode)``. 
- ``--serve_root <path>``: Specify a custom directory to use as the root directory for the spawned web server. By default, the directory where the **.html** file resides is used. 
- ``--port <number>``: Specify the web server TCP port. The default port is ``6931``. 
- ``--silence_timeout <seconds>``: Specify the *emrun* silence timeout. If the application does not print anything to ``stdout`` or ``stderr`` in this many seconds, the page/browser is assumed to be hung, and *emrun* will quit. This is disabled by default. 
- ``--timeout <seconds>``: Specify the *emrun* timeout. If the whole page run lasts longer than this many seconds, the page/browser is assumed to be hung, and *emrun* will quit. This is disabled by default. 
- ``--timeout_returncode <code>``: Specifies the process return code that *emrun* quits with if a page run timeout occurs. By default this is ``99999``.


Controlling log output
======================

The following command line flags affect logging output:

- ``--verbose``: Print detailed information about *emrun* internal steps.
- ``--log_stdout <filename>``: Write all ``stdout`` messages from the application to the named file (instead of printing to terminal).
- ``--lot_stderr <filename>``: Write all ``stderr`` messages from the application to the named file (instead of printing to terminal).
- ``--system_info``: Print detailed information about the current system before launching. This is useful during automated runs when you want to capture hardware information to logs.
- ``--browser_info``: Print information about which browser is about to be launched.
- ``--log_html``: Reformat application output as HTML markup.
- ``--no_emrun_detect``: Hide the warning message that is launched if a target **.html** file is detected to not have been built with ``--emrun``.


Cleaning up after the run
=========================

These command line flags allow you to clean up open browser processes before starting a new run — this is important for automated testing on build servers:

-  ``--kill_start``: Terminate all instances of the target browser process before starting the run. Pass this flag to ensure that no old (hung) instances of the target browser process exist that could interfere with the current run. This is disabled by default.
-  ``--kill_exit``: Terminate all instances of the target browser process when *emrun* quits. Pass this flag to ensure that browser pages closed when the run is over. This is disabled by default. Note that it may be necessary to explicitly use the ``--browser=/path/to/browser`` command line option when using ``--kill_exit``, or otherwise the termination might not function properly.

.. warning:: These operations cause the browser process to be forcibly terminated.  Any windows or tabs you have open will be closed, including any that might contain unsaved data. 


Running web pages in Firefox
============================

When running web pages via ``emrun`` using Firefox, you may want to set one or more of the following browser prefs: ::

  ; Make sure to unblock popups being spawned from http://localhost/.
  browser.popups.showPopupBlocker;false

  ; Don't ask the user to change the default browser when spawning the browser.
  browser.shell.checkDefaultBrowser;false

  ; Don't autorestore previous tabs, just open the one from the command line.
  browser.sessionstore.resume_from_crash;false
  services.sync.prefs.sync.browser.sessionstore.restore_on_demand;false
  browser.sessionstore.restore_on_demand;false

  ; Don't bring up the modal "Start in Safe Mode" dialog after browser is killed, since
  ; that is an expected path for --kill_start and --kill_exit options.
  browser.sessionstore.max_resumed_crashes;-1
  toolkip.startup.max_resumed_crashes;-1

  ; Don't fail on long-running scripts, but have emrun instead control execution termination.
  dom.max_script_run_time;0
  dom.max_chrome_script_run_time;0

  ; Accelerate browser update background timer tick so that autoupdates take place as quickly as possible.
  ; This is useful for continous integration servers wanting to always test the latest browser version.
  app.update.download.backgroundInterval;1

  ; Always run in private browsing mode to avoid caching any pages (but also disables IndexedDB persistency!).
  browser.privatebrowsing.autostart;true

  ; When switching between multiple Firefox browser versions/channels, suppress showing the first time welcome page.
  startup.homepage_override_url;about:blank
  startup.homepage_welcome_url;about:blank

To set a Firefox browser pref, navigate to the page ``about:config`` in the browser navigation bar.

Running web pages on an Android device
======================================

*emrun* can automate browser-based testing on Android. 

For this to work, you need to:

- Connect an Android phone to the local system via USB, with its developer mode enabled. There is no need to root the phone. 
- Install the *adb* tool on the host system and make sure it is present in the ``PATH`` environment variable. 
- Check that *adb* is working by calling ``adb devices`` to see that your device is listed. 
- Install any browser apk to the device that you want to be able to run.

To run on Android, add the ``--android`` command line flag and use the ``--browser <alias>`` command line flag to explicitly choose the correct browser to run. 

.. note:: Omitting ``--browser`` (to launch a default Android browser) is not supported. 

The following browser aliases have been tested and shown to work: ``firefox, firefox_beta, firefox_aurora, firefox_nightly, chrome, chrome_beta, opera``.

The following browser aliases are also supported, but have known issues:

- ``opera_mini``: The browser launches, but for some reason it times out when trying to load any page. 
- ``dolphin``: Works, but does not support WebGL.

Otherwise, using *emrun* for browser-based testing on Android is the same as when testing on the host system.
