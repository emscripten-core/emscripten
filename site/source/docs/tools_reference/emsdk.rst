.. _emsdk:

=====================================
emsdk - Emscripten SDK Manager
=====================================

**The Emscripten SDK management script (** ``emsdk`` **) is used to perform all SDK maintenance. You only need to "install" the SDK once; after that emsdk can do all further updates!**

Purpose
============================================

With ``emsdk`` you can download, install or remove *any* SDK or Tool, ranging from the very first, through to the bleeding edge updates still on Github. Most operations are of the form ``emsdk command``. To access the *emsdk*, launch the *Emscripten Command Prompt*.

This document provides the command syntax, and a :ref:`set of guides <emsdk_howto>` explaining how to perform both common and advanced maintenance operations.


Command line syntax
============================================

**emsdk** [**help** [**--old**] | **list** | **update** | **install** *<tool/sdk>* | **uninstall** *<tool/sdk>* | **activate** *<tool/sdk>*]

or

**emcmdprompt.bat**

.. note:: For Mac OSX the commands are called with  **./emsdk**  and **./emcmdprompt.bat** respectively.

Arguments
---------
 

.. list-table:: 
   :header-rows: 1
   :widths: 20 80
   :class: wrap-table-content 

   * - Command
     - Description
   * - ``list [--old]``
     - Lists all current SDKs and tools and their installation status. With the ``--old`` parameter, historical versions are also shown.
   * - ``update``
     - Fetches the latest list of all available tools and SDKs (but does not install them)
   * - ``install <tool/sdk>``
     - Downloads and installs the :ref:`specified tool or SDK <emsdk-specified-tool-sdk>`.
   * - ``uninstall <tool/sdk>``
     - Removes the :ref:`specified tool or SDK <emsdk-specified-tool-sdk>` from the disk.
   * - ``activate <tool/sdk>``
     - Sets the :ref:`specified tool or SDK <emsdk-specified-tool-sdk>` as the default tool in the system environment.
   * - ``help``
     - Lists all supported commands. The same list is output if no command is specified.	 

**emcmdprompt.bat** doesn't take any arguments. It spawns a new command prompt window with the Emscripten environment active.

.. _emsdk-specified-tool-sdk:

Tools and SDK targets
------------------------
	 
The ``<tool/sdk>`` given above is one of the targets listed using ``emsdk list`` (or ``emsdk list --old``). 

Note that some of the tools and SDK names include  *master* or *incoming*: these targets are used to clone and pull the very latest versions from the Emscripten incoming and master branches.

Finally, you can specify a target of ``latest`` to grab the most current SDK.

.. todo:: **HamishW**  emcmdprompt.bat does not appear to work. Need to check with Jukka



SDK manager concepts
==============================

The *emsdk* can fetch the different versions of all the tools and SDKs. These are put into different directories under the main SDK installation folder, grouped by tool and version. 

A user-specific file (**~/.emscripten**) stores the "compiler configuration"; this configuration defines the specific set of tools that are :term:`active <Active Tool/SDK>`, and will be used by default if Emscripten in called on the **Emscripten Command Prompt*.

The following terms are used when referring to the SDK and tool manager:

.. glossary::

	Tool
		The basic unit of software bundled in the SDK. A Tool has a name and a version. For example, 'clang-3.2-32bit' is a Tool that contains the 32-bit version of the *Clang* v3.2 compiler. Other tools include *Emscripten*, *Java*, *Git*, *Node*, etc.
		
	SDK
		A set of :term:`tools <Tool>`. For example, 'sdk-1.5.6-32bit' is an SDK consisting of the tools: clang-3.2-32bit, node-0.10.17-32bit, python-2.7.5.1-32bit and emscripten-1.5.6.
		
	Active Tool/SDK
		The SDK can store multiple versions of tools and SDKs. The active tools/SDK is the set of tools that are used by default on the *Emscripten Command Prompt*. This compiler configuration is stored in a user-specific persistent file (**~/.emscripten**) and can be changed using *emsdk*.
		
	emsdk
		This is the name of the manager script that Emscripten SDK is accessed through. Most operations are of the form ``emsdk command``. To access the *emsdk* script, launch the *Emscripten Command Prompt*.



.. _emsdk_howto:

"How to" guides
=========================

The following topics explain how to perform both common and advanced maintenance operations, ranging from installing the latest SDK through to installing your own fork from Github.


How do I just get the latest SDK?
------------------------------------------------------------------------------------------------
Use the ``update`` argument to fetch the current registry of available tools, and then the ``latest`` target to get the most recent SDK: ::

	emsdk update 		// Fetch the latest registry of available tools.
	emsdk install latest 	// Download and install the latest SDK tools.
	emsdk activate latest	// Set up the compiler configuration to point to the "latest" SDK.

Note that the commands are the same on Mac OSX, but you need to replace **emsdk** with **./emsdk**.


How do I use emsdk?
--------------------------------

Use ``emsdk help`` or just ``emsdk`` to get information about all available commands.

	
How do I check which versions of the SDK and tools are installed?
------------------------------------------------------------------------------------------------

To get a list of all currently installed tools and SDK versions (and all available tools) run: ::
	emsdk list

A line will be printed for each tool/SDK that is available for installation. The text ``INSTALLED`` will be shown for each tool that has already been installed. If a tool/SDK is currently active, a star (\*) will be shown next to it. 

	
How do I install a tool/SDK version?
------------------------------------

Use the ``install`` argument to download and install a new tool or an SDK version: ::

	emsdk install <tool/sdk name>

	
.. _emsdk-remove-tool-sdk:
	
How do I remove a tool or an SDK?
----------------------------------------------------------------

Use the ``uninstall`` argument to delete the given tool or SDK from the local computer: ::

	emsdk uninstall <tool/sdk name>

	
How do I check for updates to the Emscripten SDK?
----------------------------------------------------------------

First use the ``update`` command to fetch package information for all new tools and SDK versions. Then use ``install <tool/sdk name>`` to install a new version: ::

	emsdk update			// Fetch the latest registry of available tools.
	emsdk install <tool/sdk name> 	// Download and install the specified new version.


How do I change the currently active SDK version?
----------------------------------------------------------------

Toggle between different tools and SDK versions using the :term:`activate <Active Tool/SDK>` command. This will set up ``~/.emscripten`` to point to that particular tool: ::

	emsdk activate <tool/sdk name>
	
	
How do I install an old Emscripten compiler version?
----------------------------------------------------------------

*Emsdk* contains a history of old compiler versions that you can use to maintain your migration path. Use the ``list --old`` argument to get a list of archived tool and SDK versions, and ``emsdk install <name_of_tool>`` to install it: ::

	emsdk list --old
	emsdk install <name_of_tool>
	
On Windows, you can directly install an old SDK version by using one of the :ref:`archived offline NSIS installers <archived-nsis-windows-sdk-releases>`. 
	
.. _emsdk-master-or-incoming-sdk:

How do I track the latest Emscripten development with the SDK?
------------------------------------------------------------------------------------------------

It is also possible to use the latest and greatest versions of the tools on the Github repositories! This allows you to obtain new features and latest fixes immediately as they are pushed to Github, without having to wait for release to be tagged. **No Github account or a fork of Emscripten is required.** 

To switch to using the latest upstream git development branch ``incoming``, run the following:

::

	# Install git. Skip if the system already has it.
	emsdk install git-1.8.3 
	
	# Clone+pull the latest kripken/emscripten/incoming.
	emsdk install sdk-incoming-64bit
	
	# Set the incoming SDK as the active.
	emsdk activate sdk-incoming-64bit 	

If you want to use the upstream stable branch ``master``, then replace ``-incoming-`` with ``-master-`` above.


	
How do I use my own Emscripten Github fork with the SDK?
----------------------------------------------------------------

It is also possible to use your own fork of the Emscripten repository via the SDK. This is useful in the case when you want to make your own modifications to the Emscripten toolchain, but still keep using the SDK environment and tools.

The way this works is that you first install the ``sdk-incoming`` SDK as in the :ref:`previous section <emsdk-master-or-incoming-sdk>`. Then you use familiar git commands to replace this branch with the information from your own fork:

::

	cd emscripten/incoming
	
	# Add a git remote link to your own repository.
	git remote add myremote https://github.com/mygituseraccount/emscripten.git
	
	# Obtain the changes in your link.
	git fetch myremote
	
	# Switch the emscripten-incoming tool to use your fork.
	git checkout -b myincoming --track myremote/incoming

You can switch back and forth between remotes via the ``git checkout`` command as usual.




