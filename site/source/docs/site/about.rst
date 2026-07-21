.. _about-this-site:

===============
About this site
===============

The site is built using `Sphinx <https://www.sphinx-doc.org/en/master/>`_ (7.4.7), the open source tool used to create the official Python documentation and many other sites. This is a very mature and stable tool, and was selected for, among other reasons, its support for defining API items and linking to them from code.

The site uses the `Shibuya theme <https://shibuya.lepture.com/>`_.

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

The site sources are stored on `GitHub <https://github.com/emscripten-core/emscripten/tree/main/site>`_. Edits and additions should be submitted to this branch in the same way as any other change to the tool.

The site is published automatically to the `emscripten-site <https://github.com/kripken/emscripten-site>`_ *gh-pages* branch (GitHub pages).


Installing Sphinx
-----------------

This site requires a specific version of Sphinx to be installed. Run the
following command to ensure you have the correct version installed: ::

  pip install -r requirements-dev.txt


.. _about-site-builds:

Site builds
-----------

The site can be built from source by running ``make html`` from source the ``emscripten/site`` directory.


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

New articles may be authored and discussed on the `wiki <https://github.com/emscripten-core/emscripten/wiki>`_ using Markdown syntax before being included in the documentation set. The easiest way to convert these to restructured text is to use a tool like `Pandoc <http://johnmacfarlane.net/pandoc/try/?text=&from=markdown_github&to=rst>`_.

.. note:: The *get_wiki.py* tool (**/site/source/get_wiki.py**) can be used to automate getting a snapshot of the wiki. It clones the wiki and calls *pandoc* on each file. The output is copied to a folder **wiki_static**. The tool also adds a heading, a note stating that the file is a "wiki snapshot", and fixes up links marked as "inline code" to matching links in the API Reference.


Site license
============

The site is licensed under the same :ref:`emscripten-license` as the rest of Emscripten. Contributors to the site should add themselves to :ref:`emscripten-authors`.
