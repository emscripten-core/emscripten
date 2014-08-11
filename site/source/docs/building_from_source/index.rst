.. _installing-from-source:

==================================================
Building Emscripten from Source (ready-for-review) 
==================================================

.. tip:: The *SDK* provides the **easiest** and **most reliable** method for **building from source**. Just :ref:`download the SDK <sdk-download-and-install>` and follow the brief instructions :ref:`here <emsdk-master-or-incoming-sdk>`.

	This section is provided for developers who prefer a manual approach and/or are contributing to Emscripten development.
	
.. note:: A user can't "manually " build from sources and use the SDK at the same time, because the SDK overwrites manual changes made to the user's :ref:`compiler configuration file <configuring-emscripten-settings>`.

This section shows how to get the source and dependencies and manually build the Emscripten toolchain.

The instructions for building Emscripten on the different platforms are given below, followed by guidance on how to update the configuration file and validate your environment once it is complete:

.. toctree::
   :maxdepth: 1
   
   toolchain_what_is_needed
   building_emscripten_from_source_on_linux
   building_emscripten_from_source_on_windows
   manually_integrating_emscripten_with_vs2010
   building_emscripten_from_source_on_mac_os_x
   LLVM-Backend
   configuring_emscripten_settings
   verify_emscripten_environment

