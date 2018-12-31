.. _about-this-site:

===============
About this site
===============

The site is built using `Sphinx <http://sphinx-doc.org/latest/index.html>`_ (1.2.2), the open source tool used to create the official Python documentation and many other sites. This is a very mature and stable tool, and was selected for, among other reasons, its support for defining API items and linking to them from code. 

The site uses a custom theme, which is based on the :ref:`read-the-docs-theme`.

.. _about-this-site-search:

Searching the site
==================

Searching returns topics that contain **all** the specified keywords. 

.. tip:: Always start by searching for *single* words like "interacting" or "compiling". Generally this will be enough to find the relevant document. If not, you can refine the search by adding additional terms. 

.. note:: Searches that include characters like "-" and "+" will not work. There is no support for logical operators.

Reporting bugs
==============

Please :ref:`report documentation bugs <site-and-documentation-bug-reports>` as you would any other Emscripten bug. Help :ref:`fix them <writing-and-updating-articles>` by updating existing documents or by creating new ones.

.. _about-this-site-contributing:

Contributing to the site
========================

:ref:`Contributions <contributing>` to this site (and indeed any part of Emscripten) are welcome! 

Check out the rest of this article for instructions on how to :ref:`build the site <building-the-site>` and :ref:`write and update articles <writing-and-updating-articles>`.


.. _building-the-site:

Building the site
=================

The site sources are stored on Github `here <https://github.com/kripken/emscripten/tree/incoming/site>`_. Edits and additions should be submitted to this branch in the same way as any other change to the tool.

The site is published to the **kripken/emscripten-site** *gh-pages* branch (Github pages).

.. note:: Remember to update the :ref:`about-build-versions` for *public* builds.

Installing Sphinx
-----------------

Notes for installing Sphinx are provided `here <http://sphinx-doc.org/install.html>`_. 


Ubuntu
++++++
The version of Sphinx on Ubuntu package repository (apt-get) fails when building the site. This is an early version (1.1.3), which appears to be dependent on an old version of the Jinja templating library. 

The workaround is to use the *Python package installer* (pip) to get version 1.2.2, and then run an upgrade (note, you may have to uninstall Sphinx first): ::

	pip install sphinx
	pip install sphinx --upgrade
	

.. _about-site-builds:

Site builds
-----------

The site can be built from source on Ubuntu and Windows by navigating to the */emscripten/site* directory and using the command: ::

	make clean
	make html
	

.. _about-sdk-builds:

SDK Builds
----------

SDK builds are virtually identical to :ref:`about-site-builds`. The main difference is that on SDK builds the :ref:`home page <home-page>` has a clear notification that it is an SDK build.

SDK builds are enabled by enabling the ``sdkbuild`` tag. This is done through the ``SPHINXOPTS`` environment variable: ::

	# Set the sdkbuild tag. 
	set SPHINXOPTS=-t sdkbuild
	make html
	
	# Unset SPHINXOPTS
	set SPHINXOPTS=
	
.. _about-build-versions:

Build version
-------------

The documentation version should match the Emscripten version for the current build. For a general site build this will be the latest tagged release as defined in `Emscripten version <https://github.com/kripken/emscripten/blob/incoming/emscripten-version.txt>`_. For an SDK build it will be the Emscripten version for the SDK.

The version and release information is used in a few places in the documentation, for example :ref:`emscripten-authors`.

The version information is defined in **conf.py** — see variables ``version`` and ``release``. These variables can be overridden by setting new values in the ``SPHINXOPTS`` environment variable. For example, to update the ``release`` variable through the command line on Windows: ::

	# Set SPHINXOPTS
	set SPHINXOPTS=-D release=6.40
	make html
	
	# Unset SPHINXOPTS
	set SPHINXOPTS=
	

.. _writing-and-updating-articles:

Writing and updating articles
=============================

.. note:: Sphinx is `well documented <http://sphinx-doc.org/latest/index.html>`_. This section only attempts to highlight specific styles and features used on this site.

	The :ref:`building-the-site` section explains how to find the sources for articles and build the site. 

	
Site content is written using :term:`reStructured text`. We recommend you read the following articles to understand the syntax:

* `reStructured text primer <http://sphinx-doc.org/rest.html>`_.
* `Sphinx Domains <http://sphinx-doc.org/domains.html>`_ (define and link to code items).
* `Inline markup <http://sphinx-doc.org/markup/inline.html>`_.



Style guide
-----------

This section has a few very brief recommendations to help authors use common style. 

.. tip:: In terms of contributions, we value your coding and content writing far more than perfect prose! Just do your best, and then :ref:`ask for editorial review <contact>`.

**Spelling:** Where possible use US-English spelling.

**Avoid idiomatic expressions**: These can be particularly confusing to non-native speakers (for example "putting your foot in your mouth" actually means "saying something embarrassing").

**Emphasis:**

	- **Bold** : use for file names, and UI/menu instructions (for example: "Press **OK** to do something").
	- *Italic* : use for tool names - e.g. *Clang*, *emcc*, *Closure Compiler*.
	- ``monotype`` : use for inline code (where you can't link to the API reference) and for demonstrating tool command line options.
	
	.. note:: Other than the above rules, emphasis should be used sparingly.


**Lists**: Use a colon on the lead-in to the list where appropriate. Capitalize the first letter and use a full-stop for each item.
	

How to link to a document or heading
------------------------------------

To link to a page, first define a globally unique reference before the page title (e.g. ``_my-page-reference``) then link to it using the `ref <http://sphinx-doc.org/markup/inline.html#ref-role>`_ role as shown: ::

	.. _my-page-reference:

	My Page Title
	=============

	This is the text of the section.
	
	To link to page use either of the options below:
		ref:`my-reference-label` - the link text is the heading name after the reference
		ref:`some text <my-reference-label>` - the link text is "some text" 

This is a better approach than linking to documents using the *:doc:* role, because the links do not get broken if the articles are moved. 

This approach is also recommended for linking to arbitrary headings in the site.

.. note:: There are a number of other roles that are useful for linking — including `Sphinx Domains <http://sphinx-doc.org/domains.html>`_ for linking to code items, and **term** for linking to glossary terms.



Recommended section/heading markup
----------------------------------

reStructured text `defines <http://sphinx-doc.org/rest.html#sections>`_ section headings using a separate line of punctuation characters after (and optionally before) the heading text. The line of characters must be at least as long as the heading. For example: ::

	A heading
	=========

Different punctuation characters are used to specify nested sections. Although the system does not mandate which punctuation character is used for each nested level, it is important to be consistent. The recommended heading levels are: ::

	=======================================
	Page title (top and bottom bars of "=")
	=======================================
	
	Level 1 heading (single bar of "=" below)
	=========================================
	
	Level 2 heading (single bar of "-" below)
	-----------------------------------------
	
	Level 3 heading (single bar of "+" below)
	+++++++++++++++++++++++++++++++++++++++++
	
	Level 4 heading (single bar of "~" below)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	
Working in markdown
-------------------
	
New articles may be authored and discussed on the `wiki <https://github.com/kripken/emscripten/wiki>`_ using Markdown syntax before being included in the documentation set. The easiest way to convert these to restructured text is to use a tool like `Pandoc <http://johnmacfarlane.net/pandoc/try/?text=&from=markdown_github&to=rst>`_. 

.. note:: The *get_wiki.py* tool (**/site/source/get_wiki.py**) can be used to automate getting a snapshot of the wiki. It clones the wiki and calls *pandoc* on each file. The output is copied to a folder **wiki_static**. The tool also adds a heading, a note stating that the file is a "wiki snapshot", and fixes up links marked as "inline code" to matching links in the API Reference. 	
	
	
.. _read-the-docs-theme:
	
Read the docs theme 
===================

The site uses a modification of the `Read the docs theme <http://read-the-docs.readthedocs.org/en/latest/theme.html>`_ (this can be found in the source at */emscripten/site/source/_themes/emscripten_sphinx_rtd_theme*).

The main changes to the original theme are listed below. 

- **Footer.html** 

	- Copyright changed to link to Emscripten authors (some code was broken by translation markup)
	- Added footer menu bar
	
- **Layout.html**

	- Added header menu bar with items
	
- **Breadcrumb.html**
	
	- Changed the text of the first link from "docs" to "Home"
	- Moved the "View Page Source" code into the bottom footer

- **theme.css**
	
	- Changed to support 4 levels of depth in sidebar toc.
	- Centred theme. Made sidebar reach bottom of page using absolute positioning.


Site license 
============

The site is licensed under the same :ref:`emscripten-license` as the rest of Emscripten. Contributors to the site should add themselves to :ref:`emscripten-authors`.