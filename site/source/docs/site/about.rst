===============================
About site (under-construction) 
===============================

The site is built using `Sphinx <http://sphinx-doc.org/latest/index.html>`_ (1.2.2), the (Python) open source tool used to create the official Python documentation, along with many other documentation sites. This is a very mature and stable tool, and was selected for, among other reasons, its support for defining API items and linking to them from code. 

The site uses a custom theme, which is based on the `Read the docs theme <http://read-the-docs.readthedocs.org/en/latest/theme.html>`_



Page management markup
============================

.. warning:: This site is still under construction and includes both new content and content from the wiki. 

	To make it easy to manage documents during early review, "status" markup has been added to documents. You can search on this text in order to find articles which have questions or issues. At the end of the project a search on any of these should return "nothing"

- "wiki-import" in the heading: The page has been imported automatically from the wiki - and not been checked. This is moved to "under construction" when the article is being properly reviewed.
- "under-construction" in the heading: The page is being worked on. Incomplete and not ready to review
- "placeholder" in the heading: The page is not being worked on, and may not even exist in the final build. Even if it exists it may not exist in the current location.
- "ready-for-review" in the heading - page is ready for review.
- "REMOVE FROM FINAL BUILDS" in the heading - page is a scratchpad or temporary file during development. To be removed/moved before publishing.
- If there is no header, the page is assumed to be published.
- "HamishW" in the body of the document - this is a specific question or issue with the document at that point

You can find documents in each type using Search: e.g. on "under review".


Writing articles
==================

TBD :doc:`placholder-to-give-me-warning-to-update`
	
Building the site
==================

The site sources are in the Emscripten repo, *incoming* branch, `/site <https://github.com/kripken/emscripten/tree/incoming/site>`_ directory. Changes should be committed to the incoming branch. 

The site is published to the *gh-pages* branch as part of a site build.

make html
---------

The site can be build from source on Ubuntu and Windows by navigating to the */emscripten/site* directory and using the command: ::

	make html

Installing Sphinx
-----------------

Notes for installing Sphinx are provided `here <http://sphinx-doc.org/install.html>`_. 


Ubuntu
++++++
The version of Sphinx on Ubuntu package repository (apt-get) is only 1.1.3. It appears to be dependent on an old version of the Jinja templating library. 

The workaround is to use the Python package installer "pip" to get version 1.2.2, and then run an upgrade (note, you may have to uninstall Sphnx first): ::

	pip install sphinx
	pip install sphinx --upgrade



	

Changes to the sphinx_rtd_theme theme 
=======================================

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

