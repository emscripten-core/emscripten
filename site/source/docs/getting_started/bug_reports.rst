.. _bug-reports:

=============
Bug Reporting
=============

All bugs should be filed in the GitHub `main
<https://github.com/emscripten-core/emscripten>`_ Emscripten repository `Issue
Tracker <https://github.com/emscripten-core/emscripten/issues?state=open>`_.

Please supply as much relevant information as possible, including:

- Original source code.
- Generated bitcode (**.bc**, **.o** or **.ll**)
- Environment information — including *emcc* and *clang* versions (as reported by ``emcc -v``).
- Error symptoms.
- Proposed solutions, ideally with a pull request.

.. Tip:: Compile with ``EMCC_DEBUG=1`` and grab the
   **/tmp/emscripten_temp/emcc-\*** files (these include the bitcode and
   JavaScript in several stages). Note that the **emscripten_temp** directory
   should be emptied manually first, so it only contains new content!


LLVM, wasm-ld, clang, Binaryen bugs
===================================

If uncertain, bugs can always be posted to the `main repository
<https://github.com/emscripten-core/emscripten>`_. But if you are sure a bug is
in an upstream project, you can file it there:

- `LLVM bug tracker <https://bugs.llvm.org/>`_ . For most issues use the
  "libraries" product and the "Backend: WebAssembly" component. For wasm-ld
  issues, use "lld" and "wasm".
- `Binaryen bug tracker <https://github.com/WebAssembly/binaryen/issues>`_

Pull requests must (of course) go to the proper repository.


.. _site-and-documentation-bug-reports:

Site and documentation bugs
===========================

Documentation (site) bugs should be filed in the same `Issue Tracker
<https://github.com/emscripten-core/emscripten/issues?state=open>`_.

Include relevant information including:

- The URL and title of the affected page(s).
- A description of the problem.
- Suggestions for a possible solution.

.. tip:: The `Page bug <https://github.com/emscripten-core/emscripten/issues/new?title=Bug%20in%20page:How%20to%20Report%20Bugs%20%28under-construction%29%20&body=REPLACE%20THIS%20TEXT%20WITH%20BUG%20DESCRIPTION%20%0A%0AURL:%20../../docs/site/Bug-Reports&labels=bug>`_ link on the bottom-right of every page opens the Issue Tracker pre-seeded with the current page URL and title.
