.. _installing-from-source:

===============================
Building Emscripten from Source
===============================

Building from source allows you to access the very latest features and bug fixes, and is essential for developers who are :ref:`Contributing` to Emscripten.

There are two approaches to building from source:

- Manually setting up the sources and dependencies.
- Using the SDK package manager to set up the environment and get the sources.

The manual approach gives you a better understanding of the toolchain and all the build steps. The SDK method is easier because it automates and tests much of the setup process. It also enables a workflow where you can easily switch between source builds and SDK environments.
	
.. note:: You can't use both approaches at the same time because the SDK overwrites manual changes made to the user's :ref:`compiler configuration file <configuring-emscripten-settings>`.

The instructions for building Emscripten using both methods are given below, followed by guidance on how to update the configuration file and validate your environment once it is complete:

.. toctree::
   :maxdepth: 1
   
   toolchain_what_is_needed
   building_emscripten_from_source_using_the_sdk
   building_emscripten_from_source_on_linux
   building_emscripten_from_source_on_windows
   manually_integrating_emscripten_with_vs2010
   building_emscripten_from_source_on_mac_os_x
   LLVM-Backend
   building_fastcomp_manually_from_source
   configuring_emscripten_settings
   verify_emscripten_environment

