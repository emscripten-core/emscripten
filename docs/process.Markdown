Release Processes
=================

Minor version updates (1.X.Y to 1.X.Y+1)
----------------------------------------

When:

 * Such an update ensures we clear the cache, so it should be done when a change is committed that requires that (for example, a change to libc or libc++).
 * The emsdk compiled versions are based on the version number, so periodically we can do this when we want a new precompiled emsdk version to be available.

Requirements:

 * We should check that there is no obvious breakage on the github CI at least before.

How:

 * The emscripten, emscripten-fastcomp, and emscripten-fastcomp-clang repos should each be updated: the emscripten-version.txt file in each, and a git tag (with the simple version number).
 * A tag should also be done in the binaryen repo.

Major version update (1.X.Y to 1.X+1.0)
---------------------------------------

When:

 * A time of very strong assurance of stability.

Requirements:

 * All 3 major automatic test runners are green: github CI, the [emscripten buildbot](http://clb.demon.fi:8112/#/) and the [wasm waterfall (emscripten part)](https://wasm-stat.us/console).
 * All tests pass locally for the person doing the update, including the main test suite (no params passed to `runner.py`), `other`, `browser`, `sockets`, `sanity`, `binaryen*`. Not all of those are run on all the bots.

How:

 * Ensuring all bots are green requires coordination and not landing anything in the meanwhile. An issue should be filed on github for this, with a specific date mentioned. At least one person that is very familiar with each of the 3 major automatic test runners should be involved.
 * Updating the repos and tags is the same as with a minor version update.
 * Also merge the `incoming` branch to `master`.

