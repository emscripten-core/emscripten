.. _sdk-download-and-install:

====================
Download and install
====================

**Emscripten SDK provides the whole Emscripten toolchain in a single package, with integrated support for** :ref:`updating to newer SDKs <updating-the-emscripten-sdk>` **as they are released.**

.. tip:: If you are :ref:`contributing <contributing>` to Emscripten you should :ref:`build Emscripten from source <installing-from-source>`.


SDK Downloads
=============

Download one of the SDK installers below to get started with Emscripten development.

.. emscripten-sdk-windows-installers:

Windows
-------

- `64-bit Emscripten SDK for Windows <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable-64bit.zip>`_ (emsdk-portable-64bit.zip)
		A zipped package of the SDK that does not require system installation privileges. Follow the instructions below to :ref:`install Emscripten SDK on Windows <sdk-installation-instructions>`.

Linux and Mac OS X
------------------

.. _emscripten-sdk-linux-osx:
	
- `Emscripten SDK for Linux and OS X <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz>`_ (emsdk-portable.tar.gz) 
		A tar.gz archive package of the SDK that does not require system installation privileges. To install, follow the :ref:`platform-specific notes <platform-notes-installation_instructions-SDK>` and the :ref:`general instructions <sdk-installation-instructions>`.

.. _sdk-installation-instructions:

Installation instructions
=========================

First check the :ref:`Platform-specific notes <platform-notes-installation_instructions-SDK>` below and install any prerequisites.

Install or update the SDK using the following steps:

1. Download and unzip the SDK package to a directory of your choice. This directory will contain the Emscripten SDK.
#. Open a command prompt inside the SDK directory and run the following :ref:`emsdk <emsdk>` commands to get the latest tools from Github and set them as :term:`active <Active Tool/SDK>`:
	
	::

		# Fetch the latest registry of available tools.
		./emsdk update
		
		# Download and install the latest SDK tools.
		./emsdk install latest

		# Make the "latest" SDK "active" for the current user. (writes ~/.emscripten file)
		./emsdk activate latest
		
		# Activate PATH and other environment variables in the current terminal
		source ./emsdk_env.sh

	.. note:: On Windows, run ``emsdk`` instead of ``./emsdk``, and ``emsdk_env.bat`` instead of ``source ./emsdk_env.sh``. 

If you change the location of the SDK (e.g. take it to another computer on an USB), re-run the ``./emsdk activate latest`` and ``source ./emsdk_env.sh`` commands. 
		
.. _platform-notes-installation_instructions-SDK:

Platform-specific notes
----------------------------

Mac OS X
++++++++

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.

#. Install the *XCode Command Line Tools*. These are a precondition for *git*.

	-  Install XCode from the `Mac OS X App Store <http://superuser.com/questions/455214/where-is-svn-on-os-x-mountain-lion>`_.
	-  In **XCode | Preferences | Downloads**, install *Command Line Tools*.

#. Install *git*:

	- `Allow installation of unsigned packages <https://www.my-private-network.co.uk/knowledge-base/apple-related-questions/osx-unsigned-apps.html>`_, or installing the git package won't succeed.
	- Install XCode and the XCode Command Line Tools (should already have been done). This will provide *git* to the system PATH (see `this stackoverflow post <http://stackoverflow.com/questions/9329243/xcode-4-4-command-line-tools>`_).
	- Download and install git directly from http://git-scm.com/.	

#. Install *cmake* if you do not have it yet:

	-  Download and install latest CMake from `Kitware CMake downloads <http://www.cmake.org/download/>`_.
	
#. Install *node.js* from http://nodejs.org/ 

	.. _getting-started-on-osx-install-python2:

Linux
++++++++

.. note:: *Emsdk* does not install any tools to the system, or otherwise interact with Linux package managers. All file changes are done inside the **emsdk/** directory.

- The system must have a working :ref:`compiler-toolchain` (because *emsdk* builds software from the source): 

	::	
	
		#Update the package lists
		sudo apt-get update
		
		# Install *gcc* (and related dependencies)
		sudo apt-get install build-essential
		
		# Install cmake
		sudo apt-get install cmake
		
- *Python*, *node.js* or *Java* are not provided by *emsdk*. The user is expected to install these beforehand with the *system package manager*:

	::
	
		# Install Python 
		sudo apt-get install python2.7
		
		# Install node.js
		sudo apt-get install nodejs
		
		# Install Java (optional, only needed for Closure Compiler minification)
		sudo apt-get install default-jre

.. note:: Your system may provide Node.js as ``node`` instead of ``nodejs``. In that case, you may need to also update the ``NODE_JS`` attribute of your ``~/.emscripten`` file.
		
- *Git* is not installed automatically. Git is only needed if you want to use tools from one of the development branches **emscripten-incoming** or **emscripten-master**: 

	::
	
		# Install git
		sudo apt-get install git-core

More detailed instructions on the toolchain are provided in: :ref:`building-emscripten-on-linux`.


Verifying the installation
==========================

The easiest way to verify the installation is to compile some code using Emscripten. 

You can jump ahead to the :ref:`Tutorial`, but if you have any problems building you should run through the basic tests and troubleshooting instructions in :ref:`verifying-the-emscripten-environment`.


.. _updating-the-emscripten-sdk:

Updating the SDK
================

.. tip:: You only need to install the SDK once! After that you can update to the latest SDK at any time using :ref:`Emscripten SDK (emsdk) <emsdk>`. 

Type the following in a command prompt ::

	# Fetch the latest registry of available tools.
	./emsdk update
	
	# Download and install the latest SDK tools.
	./emsdk install latest
	
	# Set up the compiler configuration to point to the "latest" SDK.
	./emsdk activate latest
	
	# Activate PATH and other environment variables in the current terminal
	source ./emsdk_env.sh

The package manager can do many other maintenance tasks ranging from fetching specific old versions of the SDK through to using the :ref:`versions of the tools on Github <emsdk-master-or-incoming-sdk>` (or even your own fork). Check out all the possibilities in the :ref:`emsdk_howto`.

.. _downloads-uninstall-the-sdk:

Uninstalling the Emscripten SDK
========================================================

If you want to remove the whole SDK, just delete the directory containing the SDK.

It is also possible to :ref:`remove specific tools in the SDK using emsdk <emsdk-remove-tool-sdk>`.


.. _archived-nsis-windows-sdk-releases:

Archived releases
=================
 
You can always install old SDK and compiler toolchains using a *current SDK*. See :ref:`emsdk-install-old-tools` for more information.

On Windows, you can also install one of the **old versions** via an offline NSIS installer. These NSIS installers are now deprecated, but still available for archived reference:

- `emsdk-1.35.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.35.0-full-64bit.exe>`_
- `emsdk-1.34.1-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.34.1-full-64bit.exe>`_ (first release based on Clang 3.7)
- `emsdk-1.30.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.30.0-full-64bit.exe>`_ (first and last release based on Clang 3.5)
- `emsdk-1.29.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.29.0-full-64bit.exe>`_ (first and last release based on Clang 3.4)
- `emsdk-1.27.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.27.0-full-64bit.exe>`_
- `emsdk-1.25.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.25.0-full-64bit.exe>`_
- `emsdk-1.22.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.22.0-full-64bit.exe>`_
- `emsdk-1.21.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.21.0-full-64bit.exe>`_
- `emsdk-1.16.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.16.0-full-64bit.exe>`_ (first stable fastcomp release) 
- `emsdk-1.13.0-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.13.0-full-64bit.exe>`_ (a unstable first fastcomp release with Clang 3.3)
- `emsdk-1.12.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.12.0-full-64bit.exe>`_ (the last non-fastcomp version with Clang 3.2)
- `emsdk-1.12.0-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.12.0-full-32bit.exe>`_
- `emsdk-1.8.2-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.8.2-full-64bit.exe>`_
- `emsdk-1.8.2-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.8.2-full-32bit.exe>`_
- `emsdk-1.7.8-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.7.8-full-64bit.exe>`_
- `emsdk-1.7.8-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.7.8-full-32bit.exe>`_
- `emsdk-1.5.6.2-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.5.6.2-full-64bit.exe>`_
- `emsdk-1.5.6.2-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.5.6.2-full-32bit.exe>`_
- `emsdk-1.5.6.1-full.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.5.6.1-full.exe)>`_ (32-bit, first emsdk release)


A snapshot of all tagged Emscripten compiler releases (not full SDKs) can be found at `emscripten/releases <https://github.com/kripken/emscripten/releases>`_.

